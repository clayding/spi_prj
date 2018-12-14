#ifndef __SPI_RUN_H_
#define __SPI_RUN_H_

#define     SPI_SLAVE_NUM   4
//#define     SPI_MULTI_SLAVE_INT        //enable single interrupt line or multi interrupt lines

typedef void (*spi_sighandler)(int signo, siginfo_t *info,void *p);

typedef struct spi_ext_event_st{
    uint8_t *spi_ready;
    spi_sighandler ext_event_handler;
    uint8_t spi_auto_read;
}spi_ext_event;


typedef union spi_int_cnt_un{
    int slave_isr_cnt_grp[SPI_SLAVE_NUM+1];
    struct slave_int_cnt_st{
        int slave_isr_num;
        int slave_isr1_cnt;
#if defined SPI_MULTI_SLAVE_INT
        int slave_isr2_cnt;
        int slave_isr3_cnt;
        int slave_isr4_cnt;
#endif //SPI_MULTI_SLAVE_INT
    }s_int_cnt;
}spi_int_cnt;

void spi_slave1_isr_handle(void);
#if defined SPI_MULTI_SLAVE_INT
void spi_slave2_isr_handle(void);
void spi_slave3_isr_handle(void);
void spi_slave4_isr_handle(void);
#endif //SPI_MULTI_SLAVE_INT

int spi_run(void *arg);

#endif //__SPI_RUN_H_
