/**************************************************************************
    МПМ "Грани Мира" (с) 2002-2003 Андрей Ермишин
    Загрузка файлов игрового мира
 **************************************************************************/

#include "sysdep.h"
#include "structs.h"
#include "db.h"
#include "utils.h"
#include "xzon.h"
#include "xboot.h"
#include "parser_id.h"
#include "planescape.h"
#include "mudfilereader.h"



///////////////////////////////////////////////////////////////////////////////


int get_xrec_zon(void)
{
    if (!Zon.Initialization())
        exit(1);

    MudFileReader reader(mud->zonDir);

    while (reader.hasNext())
        if (!Zon.ReadConfig(reader.next().getCPath()))
            exit(1);

    return Zon.GetNumberItem();
}

///////////////////////////////////////////////////////////////////////////////

void boot_zon(void)
{
    int i, number;
    char buf[MAX_STRING_LENGTH];

    for (i = 0; i < NUM_ZONES; i++)
        count_zones[i] = 0;

    number = Zon.GetNumberItem();
    for (i = 0; i < number; i++) {
        CItem *z = Zon.GetItem(i);

        zone_table[zone_nr].number = z->GetItem(ZON_NUMBER)->GetInt();

        if (z->GetItem(ZON_NAME_MAJ)->GetString()) {
            CREATE(zone_table[zone_nr].name_maj, char,
                   strlen(z->GetItem(ZON_NAME_MAJ)->GetString()) + 1);
            strcpy(zone_table[zone_nr].name_maj, z->GetItem(ZON_NAME_MAJ)->GetString());
        }

        CREATE(zone_table[zone_nr].name, char, strlen(z->GetItem(ZON_NAME_MIN)->GetString()) + 1);

        strcpy(zone_table[zone_nr].name, z->GetItem(ZON_NAME_MIN)->GetString());

        if (z->GetItem(ZON_AUTHOR)->GetString()) {
            CREATE(zone_table[zone_nr].author, char,
                   strlen(z->GetItem(ZON_AUTHOR)->GetString()) + 1);
            strcpy(zone_table[zone_nr].author, z->GetItem(ZON_AUTHOR)->GetString());
        }
        if (z->GetItem(ZON_DESCRIPTION)->GetString()) {
            CREATE(zone_table[zone_nr].description, char,
                   strlen(z->GetItem(ZON_DESCRIPTION)->GetString()) + 1);
            strcpy(zone_table[zone_nr].description, z->GetItem(ZON_DESCRIPTION)->GetString());
        }

        sprintf(buf, "%d99", z->GetItem(ZON_NUMBER)->GetInt());
        //zone_table[zone_nr].top  = z->GetItem(ZON_TOP)->GetInt();
        zone_table[zone_nr].top = atoi(buf);

        if (z->GetItem(ZON_TIME)->GetInt())
            zone_table[zone_nr].time_offset = z->GetItem(ZON_TIME)->GetInt();
        else
            zone_table[zone_nr].time_offset = 0;

        if (z->GetItem(ZON_MESTO)->GetNumberItem()) {
            CItem *mesto = z->GetItem(ZON_MESTO)->GetItem(0);

            if (mesto->GetItem(ZON_MESTO_PLANE)->GetInt())
                zone_table[zone_nr].plane = mesto->GetItem(ZON_MESTO_PLANE)->GetInt();
            else
                zone_table[zone_nr].plane = 0;
        }

        if (z->GetItem(ZON_RESET_TIME)->GetInt())
            zone_table[zone_nr].lifespan = z->GetItem(ZON_RESET_TIME)->GetInt();
        else
            zone_table[zone_nr].lifespan = 0;

        if (z->GetItem(ZON_RESET_TYPE)->GetInt())
            zone_table[zone_nr].reset_mode = z->GetItem(ZON_RESET_TYPE)->GetInt();
        else
            zone_table[zone_nr].reset_mode = 0;

        zone_table[zone_nr].type = z->GetItem(ZON_TYPE)->GetInt();
        count_zones[z->GetItem(ZON_TYPE)->GetInt()]++;

        //log("ZONE %d zone_nr %d top %d",zone_table[zone_nr].number,
        //zone_nr,top_of_zone_table);
        zone_table[zone_nr].recall = z->GetItem(ZON_RECALL)->GetInt();
        top_of_zone_table = zone_nr++;
    }
}
