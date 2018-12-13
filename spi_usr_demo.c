//测试spi_run程序
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include "lib/debug.h"
#include "data_st.h"
#include "spi_run.h"

static uint8_t tx_buf[BUF_SIZE] = {0};
static uint8_t rx_buf[BUF_SIZE] = {0};

static volatile uint8_t spi_ready = 0;

void demo_test_func(void *arg)
{
    uint8_t tx_head = 0; //递增
    static slave_id_t sid = 1; //slave id :1-8
    
    while(!spi_ready){}; //等待spi初始化完毕

    for(;;){
        //使用随机数作为测试发送buf的数据
        int i = 0;
        srand( (unsigned)time( NULL ));
        for( ; i < sizeof(tx_buf);i++ )
            tx_buf[i] =  rand();
        printf("\n");
        tx_buf[0] = tx_head++;
        dump_debug_log("Tx",tx_buf,sizeof(tx_buf));
        usr_tx_data_in(sid,tx_buf,sizeof(tx_buf),PRIORIT_LOW);

        sid = usr_rx_data_out(rx_buf,sizeof(rx_buf));
        if(sid) //可能没有数据，使用sid判断
            dump_debug_log("Rx",rx_buf,sizeof(rx_buf));
        printf("\n");
        //usleep(10000);
        sid++;
        sid = (sid%4)+1;
    }

}


int main(void)
{
    pthread_t spi_run_thread,demo_test_thread;

    spi_init_data();//初始化发送和接收链表

    //创建spi 工作线程
    if (pthread_create(&spi_run_thread, NULL,(void *)spi_run,(uint8_t*)&spi_ready)) {
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