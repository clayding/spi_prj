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



int main(void)
{
    uint8_t tx_head = 0;
    pthread_t spi_run_thread;
    static slave_id_t sid = 1; //slave id :1-8

    spi_init_data();

    if (pthread_create(&spi_run_thread, NULL,(void *)spi_run, NULL)) {
        perror("pthread_create");
        return 1;
    }
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