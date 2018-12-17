#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <bcm2835.h>
#include <wiringPi.h>
#include "lib/debug.h"
#include "data_st.h"
#include "spi_run.h"

#define   SPI_TXRX_LOOP_TEST       //for loop test

#ifndef   SPI_MULTI_SLAVE_INT
#define   SPI_SINGLE_SLAVE_INT_LINE 1
#endif

#define   SPI_SLAVE_INT_HANDLE(x)  (spi_slave##x##_isr_handle) 

static spi_int_cnt ss_isr_cnt;  //the union of spi slave isr(index: 1 or above) count

#define PI_GPIO_ISR_REGISTER(pin,edge_type,function_name) do{ \
              wiringPiISR (pin,edge_type,function_name); \
              /*put the total number of which isr registered into ss_isr_cnt.s_int_cnt.slave_isr_num */ \
              ss_isr_cnt.s_int_cnt.slave_isr_num++; \
            }while(0);

static unsigned char ss_isr_sig_registered = 0; //spi slave signal handler 0: not registered 1:registered

#define ISR_SIGNAL_SEND(ptr) do{ \
              sigval_t isr_sigval;  \
              isr_sigval.sival_ptr = ptr; \
              if(ss_isr_sig_registered && sigqueue(getpid(),SIGRTMIN,isr_sigval)==-1) \
                printf("sigqueue() failed ! error message:%s\n",strerror(errno)); \
            }while(0);
static uint8_t spi_auto_read = 0;//flag of auto reading from slave,put the packets to recv list

static unsigned char spi_init_done = 0;        //0: bcm2835 notinitialized 1: bcm2835 initialized

static void spi_end_handle(int param);

static int spi_signal_register(spi_sighandler user_handler)
{
  if (signal(SIGINT, spi_end_handle) == SIG_ERR) {  //handle the CTRL+C signal ,when exit those keys pressed
    perror("signal SIGINT");
    return 1;
  }

  if(user_handler){
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags=SA_SIGINFO;
    act.sa_sigaction=user_handler;

    if (sigaction(SIGRTMIN,&act,NULL) != 0) {
      perror("sigaction SIGRTMIN");
      return 1;
    }
    ss_isr_sig_registered = 1;
  }else{
    WARNING("signal user_handler register failed\n");
  }

  return 0;
}

static int spi_init(void)
{
  if (!bcm2835_init())
  {
   printf("bcm2835_init failed. Are you running as root??\n");
   return 1;
  }

  if (!bcm2835_spi_begin())  {
   printf("bcm2835_spi_begin failed. Are you running as root??\n");
   return 1;
  }

  bcm2835_spi_begin();
  bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);   // The default

  bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);          // The default
  bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_64);
  bcm2835_spi_chipSelect(BCM2835_SPI_CS0);          
  bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);   // the default
	
  spi_init_done = 1;

  return 0;
}

static void gpio_ext_int_init(void)
{
  wiringPiSetup () ;

  /*GPIO 0-7 used for external interrupt handler*/
  PI_GPIO_ISR_REGISTER (0, INT_EDGE_FALLING, SPI_SLAVE_INT_HANDLE(1)) ;

#if defined SPI_MULTI_SLAVE_INT
  PI_GPIO_ISR_REGISTER (1, INT_EDGE_FALLING, SPI_SLAVE_INT_HANDLE(2)) ;
  PI_GPIO_ISR_REGISTER (2, INT_EDGE_FALLING, SPI_SLAVE_INT_HANDLE(3)) ;
  PI_GPIO_ISR_REGISTER (3, INT_EDGE_FALLING, SPI_SLAVE_INT_HANDLE(4)) ;
  //TODO
#endif

}

int spi_run(void *arg)
{
  spi_ext_event *spi_et = NULL;

  if(arg){
    spi_et = (spi_ext_event*)arg;
    spi_auto_read = spi_et->spi_auto_read;
  }

  //uint8_t *spi_initialized = (uint8_t *)arg;

  if(spi_signal_register(spi_et->ext_event_handler))
    return 1;

  if(spi_init() != 0) return 1;

  spi_init_data();//initial the send list and recv list

  gpio_ext_int_init();//external gpio interrupt init

  if(spi_et->spi_ready)
    *(spi_et->spi_ready) = 1; //spi initialized

  while(1){
    slave_id_t sid = 0;       //slave id 0 is invalid
    slave_id_t recv_from = 0; //slave id 0 is invalid
#ifdef SPI_MULTI_SLAVE_INT
    static slave_id_t next_recv_sid = 1; //next_recv_sid:1-4(8),initialized to 1
#endif
    char  tx_buf[BUF_SIZE] = { 0x01, 0x02, 0x11, 0x33, 0x22,0x44,0x55,0x66,0x77,0x88,0x99 }; // Data to send
    char  rx_buf[BUF_SIZE] = { 0x07, 0x08, 0x77, 0x88 }; // Data to receive

    sid = spi_tx_data_out((uint8_t*)tx_buf,sizeof(tx_buf));//sid: 1-4(8)
    //if(sid)
     //dump_debug_log("spi_tx",tx_buf,sizeof(tx_buf));
#ifndef SPI_TXRX_LOOP_TEST
  #ifdef SPI_MULTI_SLAVE_INT
    /**********************************************************************************************
     ***************************Multi Interrupt Lines(1-8) Processing *****************************
     **********************************************************************************************/
    if(sid && ss_isr_cnt.slave_isr_cnt_grp[sid]){  //need to send and receive
      //TODO pull GPIO(sid) low 
      bcm2835_spi_transfernb(tx_buf, rx_buf, sizeof(tx_buf));
      //TODO pull GPIO(sid) High
      ss_isr_cnt.slave_isr_cnt_grp[sid]--;
      if(sid == next_recv_sid)//sid: 1-4(8) and next_recv_sid:1-4(8)
        next_recv_sid = next_recv_sid % SPI_SLAVE_NUM + 1;
      recv_from = sid;             //recv from:1-4(8)  sid: 1-4(8)
    }else if(sid){                 //just to send
      //TODO pull GPIO(sid) low 
      bcm2835_spi_writenb(tx_buf, sizeof(tx_buf));
      //TODO pull GPIO(sid) High
    }else{                        //just to receive according to the isr event and count
      if(spi_auto_read && ss_isr_cnt.slave_isr_cnt_grp[next_recv_sid]){
        memset(tx_buf,0xff,sizeof(tx_buf));
        //TODO pull GPIO(next_recv_sid) low
        bcm2835_spi_transfernb(tx_buf, rx_buf, sizeof(tx_buf));
        //TODO pull GPIO(next_recv_sid) High
        ss_isr_cnt.slave_isr_cnt_grp[next_recv_sid]--;
        recv_from = next_recv_sid;  //recv from:1-4(8)  next_recv_sid:1-4(8)
      }
      next_recv_sid = next_recv_sid % SPI_SLAVE_NUM + 1;
    }
  #else
    /**********************************************************************************************
     *************************Single Interrupt Line(Only 1) Processing ****************************
     **********************************************************************************************/
    if(sid && ss_isr_cnt.slave_isr_cnt_grp[SPI_SINGLE_SLAVE_INT_LINE]){  //need to send and receive
      bcm2835_spi_transfernb(tx_buf, rx_buf, sizeof(tx_buf));
      ss_isr_cnt.slave_isr_cnt_grp[SPI_SINGLE_SLAVE_INT_LINE]--;
      recv_from = sid;
    }else if(sid){                                       //just to send
        bcm2835_spi_writenb(tx_buf, sizeof(tx_buf));
    }else{                                               //just to receive according to the isr event and count
      if(spi_auto_read && ss_isr_cnt.slave_isr_cnt_grp[SPI_SINGLE_SLAVE_INT_LINE]){
        memset(tx_buf,0xff,sizeof(tx_buf));
        bcm2835_spi_transfernb(tx_buf, rx_buf, sizeof(tx_buf));
        ss_isr_cnt.slave_isr_cnt_grp[SPI_SINGLE_SLAVE_INT_LINE]--;
        recv_from = SPI_SINGLE_SLAVE_INT_LINE;
      }
    }
  #endif //SPI_MULTI_SLAVE_INT
    if(recv_from){
      spi_rx_data_in(recv_from,(uint8_t*)rx_buf,sizeof(rx_buf));
    }
     if(0)
     {
        int i = 0;
        printf("Master write %02x Read from Slave",tx_buf[0]);
        for(;i < sizeof(rx_buf);i++)
          printf("%02X ",rx_buf[i]);
        printf("\n");
        memset(rx_buf,0,sizeof(rx_buf));
      }
#else
    memcpy(rx_buf, tx_buf,sizeof(tx_buf));
    recv_from = sid;
    spi_rx_data_in(recv_from,(uint8_t*)rx_buf,sizeof(rx_buf));
#endif //SPI_TXRX_LOOP_TEST

    //usleep(1000000);
  }
 
  bcm2835_spi_end();
  bcm2835_close();
  return 0;
}


void spi_slave1_isr_handle(void)
{
  ss_isr_cnt.s_int_cnt.slave_isr1_cnt++;
  ISR_SIGNAL_SEND(&ss_isr_cnt);
  fflush (stdout) ;
}

#if defined SPI_MULTI_SLAVE_INT
void spi_slave2_isr_handle(void)
{
   printf("spi slave 2 data ready!\n");
   ss_isr_cnt.s_int_cnt.slave_isr2_cnt++;
   fflush (stdout) ;
}
void spi_slave3_isr_handle(void)
{
   printf("spi slave 3 data ready!\n");
   ss_isr_cnt.s_int_cnt.slave_isr3_cnt++;
   fflush (stdout) ;
}
void spi_slave4_isr_handle(void)
{
   printf("spi slave 4 data ready!\n");
   ss_isr_cnt.s_int_cnt.slave_isr4_cnt++;
   fflush (stdout) ;
}

#endif //SPI_MULTI_SLAVE_INT

void spi_end_handle(int param)
{
  printf("Spi end and free all list....\n");
  free_all_list();
  if(spi_init_done){
    bcm2835_spi_end();
    bcm2835_close();
  }
}
