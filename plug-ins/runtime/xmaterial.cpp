
#include "sysdep.h"
#include "structs.h"
#include "db.h"
#include "utils.h"
#include "xmaterial.h"
#include "xboot.h"
#include "parser_id.h"
#include "spells.h"
#include "planescape.h"
#include "mudfile.h"



///////////////////////////////////////////////////////////////////////////////


void boot_materials(void)
{
    CMaterial Mat;
    int number, i;

    if (!Mat.Initialization())
        exit(1);

    if (!Mat.ReadConfig(ShareFile(mud->materialsFile).getCPath()))
        exit(1);

    number = Mat.GetNumberItem();
    for (i = 0; i < number; i++) {
        CItem *m = Mat.GetItem(i);

        new_material(m->GetItem(MAT_NUMBER)->GetInt(),
                     (char *) m->GetItem(MAT_NAME)->GetString(),
                     m->GetItem(MAT_WEIGHT)->GetInt(),
                     m->GetItem(MAT_COST)->GetInt(),
                     m->GetItem(MAT_DURAB)->GetInt(),
                     m->GetItem(MAT_AC)->GetInt(),
                     m->GetItem(MAT_ARM0)->GetInt(),
                     m->GetItem(MAT_ARM1)->GetInt(),
                     m->GetItem(MAT_ARM2)->GetInt(),
                     m->GetItem(MAT_TYPE)->GetInt(),
                     m->GetItem(MAT_HITS)->GetInt(),
                     m->GetItem(MAT_FIRE)->GetInt(),
                     m->GetItem(MAT_COLD)->GetInt(),
                     m->GetItem(MAT_ELECTRO)->GetInt(),
                     m->GetItem(MAT_ACID)->GetInt(), m->GetItem(MAT_INCLUDE)->GetInt()
            );

        ParserConst.SetList(LIST_MAT_TYPES, m->GetItem(MAT_NUMBER)->GetInt(),
                            (char *) m->GetItem(MAT_ALIAS)->GetString());
    }

};

///////////////////////////////////////////////////////////////////////////////

void new_material(int number, char *name, int weight, int price, int durab, int ac, int arm0,
                  int arm1, int arm2, int type, int hits, int fire, int cold, int electro, int acid,
                  int include)
{
    struct material_data *k;

    CREATE(k, material_data, 1);
    k->number = number;
    CREATE(k->name, char, strlen(name) + 1);

    strcpy(k->name, name);
    k->weight = weight;
    k->price = price;
    k->durab = durab;
    k->ac = ac;
    k->armor0 = arm0;
    k->armor1 = arm1;
    k->armor2 = arm2;
    k->type = type;
    k->include = include;
    k->save[TYPE_HITS] = hits;
    k->save[TYPE_FIRE] = fire;
    k->save[TYPE_COLD] = cold;
    k->save[TYPE_ELECTRO] = electro;
    k->save[TYPE_ACID] = acid;

    k->next = material_table;
    material_table = k;

}
