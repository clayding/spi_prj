#ifndef __SPI_RUN_H_
#define __SPI_RUN_H_

#define SPI_SLAVE_NUM   4

#if SPI_SLAVE_NUM <= 4
void spi_slave1_isr_handle(void);
void spi_slave2_isr_handle(void);
void spi_slave3_isr_handle(void);
void spi_slave4_isr_handle(void);
#else 
void spi_slave5_isr_handle(void);
void spi_slave6_isr_handle(void);
void spi_slave7_isr_handle(void);
void spi_slave8_isr_handle(void);
#endif


#endif
