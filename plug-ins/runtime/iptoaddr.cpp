/************************************************************************/
/*                                                                      */
/*    Aladon MUD                                                        */
/*               IP to Address code                                     */
/*                                                                      */
/*       August 2004, Kerd.                                             */
/*                                                                      */
/************************************************************************/

#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "db.h"
#include "iptoaddr.h"
#include "planescape.h"
#include "dlfileop.h"
#include "mudfile.h"

/***********************************************************/
/* TYPES */

#pragma pack(1)

typedef char t_ccode[3];
typedef char t_cname[32];

typedef struct _t_ip {
    unsigned long ipf;
    unsigned long ipt;
    unsigned char ipc;
} t_ip, *pt_ip;

t_ccode country[512];
int ccnt;

#pragma pack()

/***********************************************************/
/* VARIABLES */

pt_ip ipw_list = NULL;          // list of world ip networks
size_t ipw_size;                // number of world ip networks
pt_ip ipr_list = NULL;          // list of russian ip networks
size_t ipr_size;                // number of russian ip networks

t_ccode *namew_list = NULL;     // country names for the world ip networks
size_t namew_size;              // number of country names
t_cname *namer_list = NULL;     // city names for the russian ip networks
size_t namer_size;              // number of city names

void load_iptoaddr()
{
    FILE *fp;
    int flg;

    log("Загружаю таблицу мировых IP-сетей");

    DLFileRead file1(MudFile(mud->adminDir, IPTABLE_WORLD));

    file1.open();
    fp = file1.getFP();

    if (fp) {
        flg = 0;

        if (fread(&ipw_size, 4, 1, fp) != 1)
            goto loadw_end;

        ipw_list = (pt_ip) malloc(sizeof(t_ip) * ipw_size);

        if (!ipw_list)
            goto loadw_end;

        if (fread(ipw_list, sizeof(t_ip), ipw_size, fp) != ipw_size)
            goto loadw_end;

        if (fread(&namew_size, 4, 1, fp) != 1)
            goto loadw_end;

        namew_list = (t_ccode *) malloc(sizeof(t_ccode) * namew_size);

        if (!namew_list) {
            namew_size = 0;
            goto loadw_end;
        }

        if (fread(namew_list, sizeof(t_ccode), namew_size, fp) != namew_size)
            goto loadw_end;

        flg = 1;

      loadw_end:
        if (!flg) {
            log("Ошибка загрузки мировых подсетей из файла '%s'!", IPTABLE_WORLD);
            if (ipw_list)
                free(ipw_list);
            if (namew_list)
                free(namew_list);
            ipw_list = NULL;
            namew_list = NULL;
        } else
            log("Загруженно %zu мировых сетей и %zu стран.", ipw_size, namew_size);
    }

    log("Загрузка российских подсетей");

    DLFileRead file2(MudFile(mud->adminDir, IPTABLE_RUSSIA));

    file2.open();
    fp = file2.getFP();

    if (fp) {
        flg = 0;

        if (fread(&ipr_size, 4, 1, fp) != 1)
            goto loadr_end;

        ipr_list = (pt_ip) malloc(sizeof(t_ip) * ipr_size);

        if (!ipr_list)
            goto loadr_end;

        if (fread(ipr_list, sizeof(t_ip), ipr_size, fp) != ipr_size)
            goto loadr_end;

        if (fread(&namer_size, 4, 1, fp) != 1)
            goto loadr_end;

        namer_list = (t_cname *) malloc(sizeof(t_cname) * namer_size);

        if (!namer_list) {
            namer_size = 0;
            goto loadr_end;
        }

        if (fread(namer_list, sizeof(t_cname), namer_size, fp) != namer_size)
            goto loadr_end;

        flg = 1;

      loadr_end:
        if (!flg) {
            log("Failed to load russian IP networks from %s!", IPTABLE_RUSSIA);
            if (ipr_list)
                free(ipr_list);
            if (namer_list)
                free(namer_list);
            ipr_list = NULL;
            namer_list = NULL;
        } else
            log("%zu russian networks and %zu cities loaded.", ipr_size, namer_size);
    }
}

int iptable_lookup(pt_ip table, int size, unsigned long ip)
{
    int pos;
    int min;
    int max;
    int step;

    min = 0;
    max = size - 1;
    step = 0;



    while (min <= max) {

        step++;

        pos = min + (max + 1 - min) / 2;

        if (pos > max)
            break;

        //log("%d: %ld-%ld: %ld-%ld %ld", step, min, max, table[pos].ipf, table[pos].ipt, ip);

        if ((table[pos].ipf <= ip) && (table[pos].ipt >= ip)) {
            //  log("ip found on step %d. Range: %08X-%08X", step, table[pos].ipf, table[pos].ipt);
            return table[pos].ipc;
        }

        if (ip < table[pos].ipf)
            max = pos - 1;
        else
            min = pos + 1;
    }
//    log("ip to found after %d steps!", step);
    return -1;
}

char *ip_to_addr(unsigned long ip)
{
    static char buf[64];
    int code;

    if (!ipw_list && !ipr_list)
        return NULL;

    buf[0] = '\0';

    if (ipw_list) {
        code = iptable_lookup(ipw_list, ipw_size, ip);
        if (code != -1) {
            memcpy(buf, namew_list + code, sizeof(t_ccode));
            buf[sizeof(t_ccode)] = '\0';
        }
    }

    if (ipr_list) {
        code = iptable_lookup(ipr_list, ipr_size, ip);
        if (code != -1) {
            if (buf[0])
                strcat(buf, ".");
            strcat(buf, namer_list[code]);
        }
    }

    return buf[0] ? buf : NULL;
}


char *get_addr_ip(char *arg)
{
    static char buf[64];
    char *addr;


// if(arg[0] == '!')
    {
        unsigned long ip;

        ip = inet_addr(arg);
        if (ip == 0) {
            sprintf(buf, "Failed IP.");
            return (buf);
        }
        addr = ip_to_addr(ntohl(ip));
        if (addr)
            sprintf(buf, addr);
        else
            sprintf(buf, "Адрес не найден");
        return (buf);
    }

    return (NULL);
}

void iptoaddr_init()
{
    /* in the middle of boot_db, before reading bans/proxies */
    log("Загружаю IP адреса.");
    load_iptoaddr();
}

void iptoaddr_destroy()
{
    FREEPTR(ipw_list);
    FREEPTR(ipr_list);
    FREEPTR(namew_list);
    FREEPTR(namer_list);
}
