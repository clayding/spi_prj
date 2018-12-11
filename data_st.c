#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include "lib/list.h"
#include "lib/debug.h"
#include "data_st.h"

#define DECLARE_TXRX_LIST()  void *LIST_CONCAT(send_list,_list) = NULL; \
            list_t send_list = (list_t)&LIST_CONCAT(send_list,_list); \
            void *LIST_CONCAT(recv_list,_list) = NULL; \
            list_t recv_list = (list_t)&LIST_CONCAT(recv_list,_list); \

DECLARE_TXRX_LIST();

void spi_init_data(void)
{
    list_init(send_list);
    list_init(recv_list);
}

static int spi_tx_data_in(slave_id_t sid, uint8_t *tx_buf, unsigned int size, tx_priority_t priority)
{
  spi_tx_node_list  *sn_tx_list = NULL;

  if(tx_buf == NULL || size > BUF_SIZE){
    ERROR("tx_buf is NULL or tx_buf size exceed %d\n",BUF_SIZE);
    return 1;
  }
  sn_tx_list = malloc(sizeof(spi_tx_data_frm));
  if(sn_tx_list ==  NULL){
    ERROR("memory allocated failed\n");
    return 1;
  }
  
  sn_tx_list->spi_tx_frm_node.slave_id = sid;
  //memory allocated successfully
  memcpy(sn_tx_list->spi_tx_frm_node.tx,tx_buf,size);

  if(priority == PRIORIT_HIGH)
    list_push(send_list, sn_tx_list);
  else
    list_add(send_list, sn_tx_list);

  return 0;
}

int usr_tx_data_in(slave_id_t sid, uint8_t *tx_buf, unsigned int size, tx_priority_t priority)
{
    return spi_tx_data_in(sid, tx_buf, size, priority);
}


slave_id_t spi_tx_data_out(uint8_t *tx_buf, unsigned int size )
{
  spi_tx_node_list  *sn_tx_list = NULL;

   if(tx_buf == NULL || size > BUF_SIZE){
    ERROR("tx_buf is NULL or tx_buf size exceed %d\n",BUF_SIZE);
    return 0;
  }

  if(list_length(send_list)){
    WARNING("send list is empty\n");
    return 0;
  }

  sn_tx_list = list_pop(send_list);

  if(sn_tx_list ==  NULL){
    WARNING("No tx data\n");
    return 0;
  }

  memcpy(tx_buf,sn_tx_list->spi_tx_frm_node.tx,size);

  free(sn_tx_list);

  return sn_tx_list->spi_tx_frm_node.slave_id;
}


int spi_rx_data_in(slave_id_t sid, uint8_t *rx_buf, unsigned int size)
{
  spi_rx_node_list  *sn_rx_list = NULL;

  if(rx_buf == NULL || size > BUF_SIZE){
    ERROR("rx_buf is NULL or size exceed %d\n",BUF_SIZE);
    return 1;
  }
  sn_rx_list = malloc(sizeof(spi_rx_data_frm));
  if(sn_rx_list ==  NULL){
    ERROR("memory allocated failed\n");
    return 1;
  }
  
  sn_rx_list->spi_rx_frm_node.slave_id = sid;
  //memory allocated successfully
  memcpy(sn_rx_list->spi_rx_frm_node.rx,rx_buf,size);

  list_push(recv_list, sn_rx_list);
    
  return 0;
}

static slave_id_t spi_rx_data_out(uint8_t *rx_buf, unsigned int size )
{
  spi_rx_node_list  *sn_rx_list = NULL;

   if(rx_buf == NULL || size > BUF_SIZE){
    ERROR("rx_buf is NULL or size exceed %d\n",BUF_SIZE);
    return 0;
  }

  if(list_length(recv_list)){
    WARNING("recv list is empty\n");
    return 0;
  }

  sn_rx_list = list_pop(recv_list);

  if(sn_rx_list ==  NULL){
    WARNING("No rx data\n");
    return 0;
  }

  memcpy(rx_buf,sn_rx_list->spi_rx_frm_node.rx,size);

  free(sn_rx_list);

  return sn_rx_list->spi_rx_frm_node.slave_id;
}


slave_id_t usr_rx_data_out(uint8_t *rx_buf, unsigned int size )
{
    return spi_rx_data_out(rx_buf, size );
}

static void free_send_list(void)
{
  spi_tx_node_list  *sn_tx_list = NULL;

  sn_tx_list = list_pop(send_list);

  while(sn_tx_list != NULL){
    free(sn_tx_list);
    sn_tx_list = list_pop(send_list);
  }
}


static void free_recv_list(void)
{
  spi_rx_node_list  *sn_rx_list = NULL;

  sn_rx_list = list_pop(recv_list);

  while(sn_rx_list != NULL){
    free(sn_rx_list);
    sn_rx_list = list_pop(recv_list);
  }
}


void free_all_list(void)
{
  free_send_list();
  free_recv_list();
}

