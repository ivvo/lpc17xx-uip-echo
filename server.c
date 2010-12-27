#include <stdio.h>
#include <string.h>
#include <lpc17xx_gpio.h>
#include <lpc17xx_pinsel.h>
#include <lpc17xx_emac.h>
#include "uip.h"
#include "uip_arp.h"
#include "timer.h"
uint8_t packet_received = 0;
uint8_t mac_addr[]={0x01, 0x02, 0x03, 0x04, 0x05, 0x06};

void ENET_IRQHandler (void)
{
    uint32_t int_stat;

    EMAC_PACKETBUF_Type RxDatbuf = {
        .pbDataBuf = uip_buf,
    };

    // Get EMAC interrupt status
    while ((int_stat = (LPC_EMAC->IntStatus & LPC_EMAC->IntEnable)) != 0) {
            LPC_EMAC->IntClear = int_stat; /*clear interrupt*/
           
            if ((int_stat & EMAC_INT_RX_DONE))
            {
                if (EMAC_CheckReceiveDataStatus(EMAC_RINFO_ERR_MASK)) {
                    /*error*/
                    goto rel;
                }
                /*copy frame to uip buffer*/
                uip_len = RxDatbuf.ulDataLen = EMAC_GetReceiveDataSize() - 3;
                EMAC_ReadPacketBuffer(&RxDatbuf);
                packet_received = 1;
            rel:
                /*release frame from EMAC buffer*/
                EMAC_UpdateRxConsumeIndex();
            }
    }
}

extern uint32_t SysTickCnt;

uint32_t clock_time()
{
    return SysTickCnt;
}

void uip_app_call(void)
{
    if (uip_newdata() || uip_rexmit()) {
        uip_send(uip_appdata, uip_datalen());
    }
}


void network_device_send()
{
    EMAC_PACKETBUF_Type pkt = {
        .pbDataBuf = uip_buf,
        .ulDataLen = uip_len,
    };
 
    EMAC_WritePacketBuffer(&pkt);
    EMAC_UpdateTxProduceIndex();
}

int main()
{
    PINSEL_CFG_Type pin_cfg = {
        .Funcnum    = 1,
        .OpenDrain  = 0,
        .Pinmode    = 0,
        .Portnum    = 1,
    };

    EMAC_CFG_Type emac_cfg  = {
        .Mode        = EMAC_MODE_100M_FULL,
        .pbEMAC_Addr = mac_addr,
    };

    pin_cfg.Pinnum = 0;
    PINSEL_ConfigPin(&pin_cfg);
    pin_cfg.Pinnum = 1;
    PINSEL_ConfigPin(&pin_cfg);
    pin_cfg.Pinnum = 4;
    PINSEL_ConfigPin(&pin_cfg);
    pin_cfg.Pinnum = 8;
    PINSEL_ConfigPin(&pin_cfg);
    pin_cfg.Pinnum = 9;
    PINSEL_ConfigPin(&pin_cfg);
    pin_cfg.Pinnum = 10;
    PINSEL_ConfigPin(&pin_cfg);
    pin_cfg.Pinnum = 14;
    PINSEL_ConfigPin(&pin_cfg);
    pin_cfg.Pinnum = 15;
    PINSEL_ConfigPin(&pin_cfg);
    pin_cfg.Pinnum = 16;
    PINSEL_ConfigPin(&pin_cfg);
    pin_cfg.Pinnum = 17;
    PINSEL_ConfigPin(&pin_cfg);

    while (EMAC_Init(&emac_cfg) == ERROR) {        
        printf("EMAC_Init error, retrying in one second...\n");
        sleep(1000);
    }

    //Enable interrupts on RX/TX done
    EMAC_IntCmd(EMAC_INT_RX_DONE | EMAC_INT_TX_DONE, ENABLE);
    NVIC_SetPriority(ENET_IRQn, 0);
    NVIC_EnableIRQ(ENET_IRQn);

    printf("mac addr: 0x%.2X:0x%.2X:0x%.2X:0x%.2X:0x%.2X:0x%.2X\n",
               mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);


    int i;
    uip_ipaddr_t ipaddr;
    struct uip_eth_addr eth_addr;
    struct timer periodic_timer;
    struct timer arp_timer;
  
    timer_set(&periodic_timer, CLOCK_SECOND / 2);
    timer_set(&arp_timer, CLOCK_SECOND * 10);

    uip_init();
    uip_arp_init();

    uip_ipaddr(ipaddr, 192, 168, 0, 2);    
    memcpy(eth_addr.addr, mac_addr, 6);

    uip_sethostaddr(ipaddr);
    uip_setethaddr(eth_addr);

    uip_listen(HTONS(80));

    struct uip_eth_hdr *eth_hdr = uip_buf;

    while(1) {
        if (packet_received) {
            switch (eth_hdr->type) {
            case HTONS(UIP_ETHTYPE_ARP):
                uip_arp_arpin(); //read ARP header
                break;
            case HTONS(UIP_ETHTYPE_IP): 
                uip_input();    //read IP header
                break;
            default:
                uip_len = 0;
            }
            
            //send outbound packet
            if (uip_len > 0) {
                uip_arp_out();
                network_device_send();
            }

            packet_received = 0;
        } 

        if (timer_expired(&periodic_timer)) {
          timer_reset(&periodic_timer);
          for (i = 0; i < UIP_CONNS; i++) {
            uip_periodic(i);
            if (uip_len > 0) {
                uip_arp_out();
                network_device_send();
            }
          }
        }
    }
    return 0;
}
