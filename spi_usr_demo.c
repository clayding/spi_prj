//测试spi_run程序
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include "lib/debug.h"
#include "data_st.h"
#include "spi_run.h"

static uint8_t tx_buf[BUF_SIZE] = {0};
static uint8_t rx_buf[BUF_SIZE] = {0};

static uint8_t spi_ready = 0;
static volatile unsigned txrx_times = 0;
static unsigned int total_times = 0;

//主要的测试程序,如果只是测试链表等的功能可以把spi_run.c的宏SPI_TXRX_LOOP_TEST取消注释
void demo_test_func(void *arg)
{
    //uint8_t k = 0;
    uint8_t tx_head = 0; //递增，用于测试的时候查看数据
    static slave_id_t sid = 1; //slave id :1-8

    while(!spi_ready){}; //等待另一个线程的spi初始化完毕

    for(;;){
    for(;txrx_times ;__sync_fetch_and_sub(&txrx_times,1)){
        //使用随机数作为测试发送buf的数据
        int i = 0;
        srand( (unsigned)time( NULL ));
        for( ; i < sizeof(tx_buf);i++ )
            tx_buf[i] =  rand();
        printf("\n");
        tx_buf[0] = tx_head++;
        tx_buf[1] = tx_head+1;
        tx_buf[126] = tx_buf[127]+tx_head -2 ;
        tx_buf[127] = tx_buf[127]+tx_head;
        total_times++;
        printf("total_times:%d\n",total_times);
        dump_debug_log("Tx",tx_buf,sizeof(tx_buf));
        usr_tx_data_in(sid,tx_buf,sizeof(tx_buf),PRIORIT_LOW);

        sid = usr_rx_data_out(rx_buf,sizeof(rx_buf));
        if(sid) //可能没有数据，使用sid判断
            dump_debug_log("Rx",rx_buf,sizeof(rx_buf));
        printf("\n");
        sid = (sid%4)+1;
    }
    }


/*  //顺序测试demo
    for(;k < 5;k++){
        //使用随机数作为测试发送buf的数据
        int i = 0;
        srand( (unsigned)time( NULL ));
        for( ; i < sizeof(tx_buf);i++ )
            tx_buf[i] =  rand();
        printf("\n");
        tx_buf[0] = tx_head++;
        dump_debug_log("Tx",tx_buf,sizeof(tx_buf));
        usr_tx_data_in(sid,tx_buf,sizeof(tx_buf),PRIORIT_LOW);
    }
    for(k = 0;k < 5;k++){
        sid = usr_rx_data_out(rx_buf,sizeof(rx_buf));
        if(sid) //可能没有数据，使用sid判断
            dump_debug_log("Rx",rx_buf,sizeof(rx_buf));
        printf("\n");
        //usleep(10000);
        sid++;
        sid = (sid%4)+1;
    }
*/

}

//这个中断处理函数，主要是接收GPIO中断处理函数发过来的信号，收到一次，证明GPIO收到了中断
static void demo_interrupt_sig_handler(int signo, siginfo_t *info,void *p)
{
    spi_int_cnt *ss_int_cnt_base = (spi_int_cnt*)info->si_int;//从信号info中获取中断信息
    /*ss_int_cnt_base->s_int_cnt.slave_isr_num 注册的用于中断的GPIO的数量
      ss_int_cnt_base->slave_isr1_cnt  GPIO0获取的中断的计数
      ss_int_cnt_base->slave_isr2_cnt  GPIO1获取的中断的计数 以此类推*/
    //printf("p:%x int_num:%d %p\n",info->si_int,ss_int_cnt_base->s_int_cnt.slave_isr_num,ss_int_cnt_base);
    printf("外部中断计数:%d\n",ss_int_cnt_base->slave_isr_cnt_grp[1]);
    //txrx_times++;
    __sync_fetch_and_add(&txrx_times,1);
}


int main(void)
{
    spi_ext_event event_reg;
    pthread_t spi_run_thread,demo_test_thread;

    memset(&event_reg,0,sizeof(spi_ext_event));
    //注册事件spi ready和中断处理函数
    event_reg.spi_ready = &spi_ready;
    event_reg.ext_event_handler = demo_interrupt_sig_handler;
    //event_reg.spi_auto_read = 1;//设置为1，再收到中断以后master 自动读取slave的数据到recv list

    //创建spi 收发工作线程
    if (pthread_create(&spi_run_thread, NULL,(void *)spi_run,
        (spi_ext_event*)&event_reg)) {
        perror("pthread_create");
        return 1;
    }

    //创建用户线程，用于向链表中添加发送数据和获取接收的数据
    if (pthread_create(&demo_test_thread, NULL,(void *)demo_test_func,NULL)) {
        perror("pthread_create");
        return 1;
    }

    pthread_join(spi_run_thread,NULL);
    pthread_join(demo_test_thread,NULL);
}