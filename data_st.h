#ifndef __DATA_ST_
#define __DATA_ST_

#define   BUF_SIZE  128  //the length of buffer
typedef unsigned char uint8_t;
typedef unsigned char  slave_id_t;

typedef enum{
    PRIORIT_LOW,
    PRIORIT_HIGH,
}tx_priority_t;

#pragma pack(push)
#pragma pack(1)
typedef struct spi_tx_data_frm_st{
    slave_id_t slave_id; //from 1 to 8, 0: invalid id
    uint8_t tx[BUF_SIZE]; //packet to send
}spi_tx_data_frm;

typedef struct spi_rx_data_frm_st{
    slave_id_t slave_id; //from 0 to 7
    uint8_t rx[BUF_SIZE]; //packet received
}spi_rx_data_frm;

typedef struct spi_tx_node_list_st{
    void* next;
    spi_tx_data_frm spi_tx_frm_node;
    uint8_t retry_time;
}spi_tx_node_list;

typedef struct spi_rx_node_list_st{
    void* next;
    spi_rx_data_frm spi_rx_frm_node;
}spi_rx_node_list;

#pragma pack(pop)


void spi_init_data(void);

int usr_tx_data_in(slave_id_t sid, uint8_t *tx_buf, unsigned int size, tx_priority_t priority);

slave_id_t spi_tx_data_out(uint8_t *tx_buf, unsigned int size);

int spi_rx_data_in(slave_id_t sid, uint8_t *rx_buf, unsigned int size);

slave_id_t usr_rx_data_out(uint8_t *rx_buf, unsigned int size );

void free_all_list(void);



#endif //__DATA_ST_
