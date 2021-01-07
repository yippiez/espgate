#define STASSID "ssidname"
#define STAPSK  "ssidpass"

#define APSSID  "accespointname"

#include <ESP8266WiFi.h>
#include <IPAddress.h>
#include <dhcpserver.h>
#include <lwip/napt.h>
#include <lwip/ip4_addr.h>
#include <lwip/prot/dns.h>
#include "iplookup.h"

extern "C"{
  #include "netfilter.h"
}

IPAddress SOFTAP_LOCAL_IP(192,168,2,1);
#define SOFTAP_GATEWAY_IP SOFTAP_LOCAL_IP
IPAddress SOFTAP_SUBNET_IP(255,255,255,0);

struct netfilter n;

err_t test_link_output(netif *netif, pbuf *p){

  uint16_t off = 0;
  
  struct eth_hdr* hdr = get_ethernet_header(p, off);
  off += sizeof(eth_hdr);

  if(hdr) {
   
    if(hdr->type == PP_HTONS(ETHTYPE_IP)){
      
      struct ip_hdr* ip_header = get_ip_header(p, off);
      off += IPH_HL(ip_header) << 2;

      if(IPH_PROTO(ip_header) == IP_PROTO_UDP){

        struct udp_hdr* udp_header = get_udp_header(p, off);
        off += UDP_HLEN;

        if(ntohs(udp_header->dest) == 53){ // if filtering on NETIF_STA check dest instead of src

          const char *qname;
          char buffer[50];

          off += SIZEOF_DNS_HDR;

          qname = DNS_GET_QNAME(p, off);
          off += strlen(qname) + 1;

          strcpy(buffer, ++qname);
          
          // convert from 3www7example3com0 to www.example.com

          for(unsigned int i=1;i < strlen(buffer) - 1; i++)
            if(buffer[i] < 60)
              buffer[i] = '.';

          for(int i=0; i<IP_LOOKUP_LIST_LN; i++)
            if(strcmp(buffer, ip_lookup_list[i]) == 0){
              Serial.println("Got a request:");
              Serial.println(buffer);
			  pbuf_free(p);
              return ERR_OK;
            }

        }
      }
    }
  }

  return netfilter_out(n, netif, p);
}

void setup() {

  Serial.begin(115200);

  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(STASSID, STAPSK);
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }

  WiFi.softAPConfig(SOFTAP_LOCAL_IP, SOFTAP_GATEWAY_IP, SOFTAP_SUBNET_IP);

  #ifdef APPSK
    WiFi.softAP(APSSID, APPSK);
  #else
    WiFi.softAP(APSSID);
  #endif

  Serial.println("\nEnable nat");

  ip_napt_init(IP_NAPT_MAX, IP_PORTMAP_MAX);
  ip_napt_enable_no(1, 1); // enable napt for ap interface

  Serial.println("Setting DNS servers for DHCPS");
  dhcps_set_dns(0, WiFi.dnsIP(0));
  dhcps_set_dns(1, WiFi.dnsIP(1));


  Serial.println("Creating netfilter object");
  n = netfilter_new(test_link_output);

  Serial.println("Starting netfilter");
  netfilter_start(&n, NETIF_STA);

}

void loop() {
  delay(500);
}
