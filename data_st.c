#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <pthread.h>
#include "lib/list.h"
#include "lib/debug.h"
#include "data_st.h"

//#define DATA_ST_DEBUG
#ifdef DATA_ST_DEBUG
#define data_st_printf(...)  do{printf(__VA_ARGS__);}while(0)
#else
#define data_st_printf(...)
#endif

#define DECLARE_TXRX_LIST()  void *LIST_CONCAT(send_list,_list) = NULL; \
            list_t send_list = (list_t)&LIST_CONCAT(send_list,_list); \
            void *LIST_CONCAT(recv_list,_list) = NULL; \
            list_t recv_list = (list_t)&LIST_CONCAT(recv_list,_list); \

DECLARE_TXRX_LIST();

static pthread_mutex_t send_list_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t recv_list_lock = PTHREAD_MUTEX_INITIALIZER;

void spi_init_data(void)
{
    list_init(send_list);
    list_init(recv_list);
    data_st_printf("send_list:%p recv_list:%p\n",send_list,recv_list);
}

static int spi_tx_data_in(slave_id_t sid, uint8_t *tx_buf, unsigned int size, tx_priority_t priority)
{
  spi_tx_node_list  *sn_tx_list = NULL;

  if(sid == 0)
    return 1;

  if(tx_buf == NULL || size > BUF_SIZE){
    ERROR("tx_buf is NULL or tx_buf size exceed %d\n",BUF_SIZE);
    return 1;
  }

  sn_tx_list = (spi_tx_node_list*)malloc(sizeof(spi_tx_node_list));
  if(sn_tx_list ==  NULL){
    ERROR("memory allocated failed\n");
    return 1;
  }

  sn_tx_list->spi_tx_frm_node.slave_id = sid;
  data_st_printf("********%s***********\n",__FUNCTION__);
  data_st_printf("%p send_list length before:%d\n",send_list,list_length(send_list));
  data_st_printf("push:%p\n",sn_tx_list);
  //memory allocated successfully
  memcpy(sn_tx_list->spi_tx_frm_node.tx,tx_buf,size);

  pthread_mutex_lock(&send_list_lock);
  if(priority == PRIORIT_HIGH)
    list_push(send_list, sn_tx_list);
  else
    list_add(send_list, sn_tx_list);
  pthread_mutex_unlock(&send_list_lock);
  data_st_printf("%p send_list length after:%d\n",send_list,list_length(send_list));

  return 0;
}

int usr_tx_data_in(slave_id_t sid, uint8_t *tx_buf, unsigned int size, tx_priority_t priority)
{
    return spi_tx_data_in(sid, tx_buf, size, priority);
}


slave_id_t spi_tx_data_out(uint8_t *tx_buf, unsigned int size )
{
  slave_id_t ret_sid = 0;
  spi_tx_node_list  *sn_tx_list = NULL;

   if(tx_buf == NULL || size > BUF_SIZE){
    ERROR("tx_buf is NULL or tx_buf size exceed %d\n",BUF_SIZE);
    return 0;
  }

  if(list_length(send_list) == 0){
    WARNING("send list is empty\n");
    return 0;
  }

  data_st_printf("********%s***********\n",__FUNCTION__);
  data_st_printf("%p send_list length before:%d\n",send_list,list_length(send_list));

  pthread_mutex_lock(&send_list_lock);
  sn_tx_list = list_pop(send_list);
  pthread_mutex_unlock(&send_list_lock);

  data_st_printf("pop:%p\n",sn_tx_list);
  data_st_printf("%p send_list length after:%d\n",send_list,list_length(send_list));
  if(sn_tx_list ==  NULL){
    WARNING("No tx data\n");
    return 0;
  }

  memcpy(tx_buf,sn_tx_list->spi_tx_frm_node.tx,size);

  ret_sid = sn_tx_list->spi_tx_frm_node.slave_id;

  free(sn_tx_list);
  sn_tx_list = NULL;

  return ret_sid;
}


int spi_rx_data_in(slave_id_t sid, uint8_t *rx_buf, unsigned int size)
{
  spi_rx_node_list  *sn_rx_list = NULL;

  if(sid == 0)
    return 1;

  if(rx_buf == NULL || size > BUF_SIZE){
    ERROR("rx_buf is NULL or size exceed %d\n",BUF_SIZE);
    return 1;
  }

  sn_rx_list = (spi_rx_node_list*)malloc(sizeof(spi_rx_node_list));
  if(sn_rx_list ==  NULL){
    ERROR("memory allocated failed\n");
    return 1;
  }
  data_st_printf("********%s***********\n",__FUNCTION__);
  data_st_printf("%p recv_list length before:%d\n",recv_list,list_length(recv_list));
  data_st_printf("push:%p\n",sn_rx_list);
  sn_rx_list->spi_rx_frm_node.slave_id = sid;
  //memory allocated successfully
  memcpy(sn_rx_list->spi_rx_frm_node.rx,rx_buf,size);

  pthread_mutex_lock(&recv_list_lock);
  list_add(recv_list, sn_rx_list);
  pthread_mutex_unlock(&recv_list_lock);

  data_st_printf("%p recv_list length after:%d\n",recv_list,list_length(recv_list));
  return 0;
}

static slave_id_t spi_rx_data_out(uint8_t *rx_buf, unsigned int size )
{
  slave_id_t ret_sid = 0;
  spi_rx_node_list  *sn_rx_list = NULL;

   if(rx_buf == NULL || size > BUF_SIZE){
    ERROR("rx_buf is NULL or size exceed %d\n",BUF_SIZE);
    return 0;
  }

  if(list_length(recv_list) == 0){
    WARNING("recv list is empty\n");
    return 0;
  }

  data_st_printf("********%s***********\n",__FUNCTION__);
  data_st_printf("%p recv_list length before:%d\n",recv_list,list_length(recv_list));

  pthread_mutex_lock(&recv_list_lock);
  sn_rx_list = list_pop(recv_list);
  pthread_mutex_unlock(&recv_list_lock);
  data_st_printf("pop:%p\n",sn_rx_list);
  if(sn_rx_list ==  NULL){
    WARNING("No rx data\n");
    return 0;
  }
  data_st_printf("%p recv_list length after:%d\n",recv_list,list_length(recv_list));
  memcpy(rx_buf,sn_rx_list->spi_rx_frm_node.rx,size);

  ret_sid = sn_rx_list->spi_rx_frm_node.slave_id;

  free(sn_rx_list);
  sn_rx_list = NULL;

  return ret_sid;
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

