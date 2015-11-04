//#include "espmissingincludes.h"
#include "c_types.h"
#include "user_interface.h"
//#include "espconn.h"
#include "mem.h"
#include "osapi.h"
//#include "driver/uart.h"

#include "client.h"
#include "user_config.h"
#include "LED_Watchdog.h"

extern os_timer_t led_timer;

struct espconn *pconn = NULL;

void ICACHE_FLASH_ATTR networkServerFoundCb(const char *name, ip_addr_t *ip, void *arg) 
{

}

void ICACHE_FLASH_ATTR networkSentCb(void *arg) 
{
  //uart0_tx_buffer("sent",4);
}

void ICACHE_FLASH_ATTR networkRecvCb(void *arg, char *data, unsigned short len) 
{
  //uart0_tx_buffer("recv",4);
  
  struct espconn *conn=(struct espconn *)arg;
  uart0_tx_buffer(data,len);
}

void ICACHE_FLASH_ATTR networkConnectedCb(void *arg) 
{
  //uart0_tx_buffer("conn",4);
  struct espconn *conn=(struct espconn *)arg;
}

void ICACHE_FLASH_ATTR networkReconCb(void *arg, sint8 err) 
{
  //uart0_tx_buffer("rcon",4);
  network_init();
}

void ICACHE_FLASH_ATTR networkDisconCb(void *arg) 
{
  //uart0_tx_buffer("dcon",4);
  // Thank to Familienpapi@FHEM-Foum
  os_timer_disarm(&network_timer);
  os_delay_us(20000000); //20000ms = 20sec!
  network_init();
}


void ICACHE_FLASH_ATTR network_start() 
{
  static struct espconn conn;
  static esp_tcp tcp;
  uint32_t target = ipaddr_addr(SERVERIP);
    
  pconn = &conn;
  
  //uart0_tx_buffer("look",4);
  
  conn.type=ESPCONN_TCP;
  conn.state=ESPCONN_NONE;
  conn.proto.tcp=&tcp;
  conn.proto.tcp->local_port=espconn_port();
  conn.proto.tcp->remote_port=SERVERPORT;

  //char page_buffer[20];
  //os_sprintf(page_buffer,"IP: %d.%d.%d.%d",IP2STR(&target));
  //uart0_tx_buffer(page_buffer,strlen(page_buffer));
  
  os_memcpy(conn.proto.tcp->remote_ip, &target, 4);
  espconn_regist_connectcb(&conn, networkConnectedCb);
  espconn_regist_disconcb(&conn, networkDisconCb);
  espconn_regist_reconcb(&conn, networkReconCb);
  espconn_regist_recvcb(&conn, networkRecvCb);
  espconn_regist_sentcb(&conn, networkSentCb);
  int  iRet = espconn_connect(&conn);  

  //os_sprintf(page_buffer,"\nConected =0: %d\n\n",iRet);
  //uart0_tx_buffer(page_buffer,strlen(page_buffer)); 
}

void ICACHE_FLASH_ATTR network_check_ip(void) 
{
  
  struct ip_info ipconfig;
  os_timer_disarm(&network_timer);
  
  wifi_get_ip_info(STATION_IF, &ipconfig);
  
  if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0) 
  {
   
    //char page_buffer[20];
    //os_sprintf(page_buffer,"IP: %d.%d.%d.%d",IP2STR(&ipconfig.ip));
    //uart0_tx_buffer(page_buffer,strlen(page_buffer));
    
 	  os_timer_disarm(&led_timer);
	  os_timer_setfn(&led_timer, (os_timer_func_t *)LedTimer, NULL);
	  os_timer_arm(&led_timer, 500, 1);
    
    network_start();
  } 
  else 
  {
 	  //uart0_tx_buffer("!!NOIP!!",8); 
 	  
 	  os_timer_disarm(&led_timer);
	  os_timer_setfn(&led_timer, (os_timer_func_t *)LedTimer, NULL);
	  os_timer_arm(&led_timer, 2000, 1);

    os_timer_setfn(&network_timer, (os_timer_func_t *)network_check_ip, NULL);
    os_timer_arm(&network_timer, 1000, 0);
  }
}

void ICACHE_FLASH_ATTR network_init() 
{
  //uart0_tx_buffer("net init",8);
  
  os_timer_disarm(&network_timer);
  os_timer_setfn(&network_timer, (os_timer_func_t *)network_check_ip, NULL);
  os_timer_arm(&network_timer, 1000, 0);
}

