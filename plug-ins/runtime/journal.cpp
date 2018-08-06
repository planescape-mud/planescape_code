
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "constants.h"
#include "case.h"
#include "screen.h"
#include "events.h"


#define VNUM_JOURNAL 99

struct obj_data *make_journal(void)
{
    struct obj_data *obj;
    struct extra_descr_data *page;
    char buf[256];
    int i;

    obj = read_object(VNUM_JOURNAL, VIRTUAL, TRUE);

    CREATE(page, struct extra_descr_data, 1);

    page->keyword = str_dup("1");

    strcpy(buf, "\r\n&WВАШ ЛИЧНЫЙ ДНЕВНИК&n ( &y1&n )               ");
    for (i = 0; i < 78; i++)
        strcat(buf, "_");
    strcat(buf, "\r\n\r\n");
    strcat(buf, "2го Сакрилегион, 589 года: ");
    strcat(buf, "Я был убит игроком по имени Иванко.\r\n");
    strcat(buf, "4го Сакрилегион, 589 года: ");
    strcat(buf, "Очистил склад от мышей.\r\n");


    page->description = str_dup(strformat(buf, 78, FALSE, ' ', '_', ' ', '|', '|', '|', '_', '|'));
    page->next = obj->ex_description;
    obj->ex_description = page;

    return obj;
}
