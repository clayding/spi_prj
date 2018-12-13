#ifndef __SPI_RUN_H_
#define __SPI_RUN_H_

#define     SPI_SLAVE_NUM   4
//#define     SPI_MULTI_SLAVE_INT        //enable single interrupt line or multi interrupt lines


void spi_slave1_isr_handle(void);
#if defined SPI_MULTI_SLAVE_INT
void spi_slave2_isr_handle(void);
void spi_slave3_isr_handle(void);
void spi_slave4_isr_handle(void);
#endif //SPI_MULTI_SLAVE_INT

int spi_run(void *arg);

#endif //__SPI_RUN_H_
