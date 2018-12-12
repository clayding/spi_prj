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

#define   SLAVE_ID_UNUSED 0
#define   SPI_SLAVE_INT_HANDLE(x)  (spi_slave##x##_isr_handle) 

static unsigned char slave_isr_cnt[SPI_SLAVE_NUM/5?4:8] = {0};

static unsigned char spi_init_done = 0;
static void spi_end_handle(int param);
static int exit_signal_register(void)
{
  if (signal(SIGINT, spi_end_handle) == SIG_ERR) {
    perror("signal");
    return 1;
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
  bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_2048); 
  bcm2835_spi_chipSelect(BCM2835_SPI_CS0);          
  bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);   // the default
	
  spi_init_done = 1;

  return 0;
}

static void gpio_ext_int_init(void)
{
  wiringPiSetup () ;

  /*GPIO 0-7 used for external interrupt handler*/
  wiringPiISR (0, INT_EDGE_FALLING, SPI_SLAVE_INT_HANDLE(1)) ;
  wiringPiISR (1, INT_EDGE_FALLING, SPI_SLAVE_INT_HANDLE(2)) ;
  wiringPiISR (2, INT_EDGE_FALLING, SPI_SLAVE_INT_HANDLE(3)) ;
  wiringPiISR (3, INT_EDGE_FALLING, SPI_SLAVE_INT_HANDLE(4)) ;
  //TODO
}

int spi_run(int argc, char **argv)
{
  static int8_t recv_from = -1;
  static uint8_t next_recv_sid = 0;

  if(exit_signal_register()) return 1;

  if(spi_init() != 0)
    return 1;
  
  gpio_ext_int_init();

  while(1){
    slave_id_t sid = 0;

    char  tx_buf[BUF_SIZE] = { 0x01, 0x02, 0x11, 0x33, 0x22,0x44,0x55,0x66,0x77,0x88,0x99 }; // Data to send
    char  rx_buf[BUF_SIZE] = { 0x07, 0x08, 0x77, 0x88 }; // Data to receive

    sid = spi_tx_data_out((uint8_t*)tx_buf,sizeof(tx_buf));//sid: 1-4(8)
    //if(sid)
     //dump_debug_log("spi_tx",tx_buf,sizeof(tx_buf));
#if 0
    if(sid && slave_isr_cnt[sid]){  //need to send and receive
      //TODO pull GPIO(sid) low 
      bcm2835_spi_transfernb(tx_buf, rx_buf, sizeof(tx_buf));
      //TODO pull GPIO(sid) High
      slave_isr_cnt[sid]--;
      if(sid == (next_recv_sid - 1))
        next_recv_sid = (next_recv_sid + 1)%(SPI_SLAVE_NUM+1);
      recv_from = sid;             //recv from:1-4(8)  sid: 1-4(8)
    }else if(sid){                 //just to send
      //TODO pull GPIO(sid) low 
      bcm2835_spi_writenb(tx_buf, sizeof(tx_buf));
      //TODO pull GPIO(sid) High
    }else{                        //just to receive according to the isr index and count
      if(slave_isr_cnt[next_recv_sid]){
        memset(tx_buf,0xff,sizeof(tx_buf));
        //TODO pull GPIO(next_recv_sid) low 
        bcm2835_spi_transfernb(tx_buf, rx_buf, sizeof(tx_buf));
        //TODO pull GPIO(next_recv_sid) High
        slave_isr_cnt[next_recv_sid]--;
        recv_from = next_recv_sid + 1;  //recv from:1-4(8)  next_recv_sid:0-3(7)
      }
      next_recv_sid = (next_recv_sid + 1)%SPI_SLAVE_NUM+1;
    }
    if(recv_from){
      spi_rx_data_in(recv_from,(uint8_t*)rx_buf,sizeof(rx_buf));
      {
        int i = 0;
        printf("Master write %02x Read from Slave",tx_buf[0]);
        for(;i < sizeof(rx_buf);i++)
          printf("%02X ",rx_buf[i]);
        printf("\n");
        memset(rx_buf,0,sizeof(rx_buf));
      }
    }
#endif
    memcpy(rx_buf, tx_buf,sizeof(tx_buf));
    recv_from = sid;
    spi_rx_data_in(recv_from,(uint8_t*)rx_buf,sizeof(rx_buf));
    //usleep(1000000);
  }
 
  bcm2835_spi_end();
  bcm2835_close();
  return 0;
}


void spi_slave1_isr_handle(void)
{
  printf("spi slave 1 data ready!\n");
  slave_isr_cnt[1]++;
  fflush (stdout) ;
}
void spi_slave2_isr_handle(void)
{
   printf("spi slave 2 data ready!\n");
   slave_isr_cnt[2]++;
   fflush (stdout) ;
}
void spi_slave3_isr_handle(void)
{
   printf("spi slave 3 data ready!\n");
   slave_isr_cnt[3]++;
   fflush (stdout) ;
}
void spi_slave4_isr_handle(void)
{
   printf("spi slave 4 data ready!\n");
   slave_isr_cnt[4]++;
   fflush (stdout) ;
}

void spi_end_handle(int param)
{
  printf("Spi end and free all list....\n");
  free_all_list();
  if(spi_init_done){
    bcm2835_spi_end();
    bcm2835_close();
  }
}
