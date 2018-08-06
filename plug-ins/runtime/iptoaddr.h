#ifndef _IPTOADDR__H_
#define _IPTOADDR__H_

#define IPTABLE_WORLD "ipc.bin"
#define IPTABLE_RUSSIA "ipr.bin"

void load_iptoaddr();
char *ip_to_addr(unsigned long ip);

void iptoaddr_init();
void iptoaddr_destroy();

#endif                          //_IPTOADDR__H_
