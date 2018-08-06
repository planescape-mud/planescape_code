/**************************************************************************
    МПМ "Грани Мира" (с) 2002-2004 Андрей Ермишин
    Загрузка темплейтов предметов
 **************************************************************************/

#include "sysdep.h"
#include "structs.h"
#include "db.h"
#include "utils.h"
#include "handler.h"
#include "xtempl.h"
#include "xboot.h"
#include "spells.h"
#include "case.h"
#include "parser_id.h"
#include "constants.h"
#include "comm.h"
#include "planescape.h"
#include "mudfile.h"




void boot_armtempl(void)
{

    if (!ArmTmp.Initialization())
        exit(1);

    if (!ArmTmp.ReadConfig(ShareFile(mud->armorTemplFile).getCPath()))
        exit(1);

}

/******************************************************************************/

void boot_weaptempl(void)
{

    if (!WeapTmp.Initialization())
        exit(1);

    if (!WeapTmp.ReadConfig(ShareFile(mud->weaponTemplFile).getCPath()))
        exit(1);
}

/**************************************************************************/

void boot_misstempl(void)
{

    if (!MissTmp.Initialization())
        exit(1);

    if (!MissTmp.ReadConfig(ShareFile(mud->missTemplFile).getCPath()))
        exit(1);
}

/***************************************************************************/

void boot_sets(void)
{
    int count, i, numadd, j, l, nadd, n, t[2];

    if (!SetTmp.Initialization())
        exit(1);

    if (!SetTmp.ReadConfig(ShareFile(mud->setsFile).getCPath())) {
        log("ПРЕДУПРЕЖДЕНИЕ: не найдет файл описания наборов предметов %s", mud->setsFile.c_str());
        return;
    }
    count = SetTmp.GetNumberItem();

    CREATE(set_table, struct set_items, count);

    for (i = 0; i < count; i++) {
        CItem *set = SetTmp.GetItem(i);

        set_table[i].list_objects = new std::vector < int >;

        set_table[i].number = set->GetItem(S_NUMBER)->GetInt();
        const int *objects = set->GetItem(S_OBJECTS)->GetScript(numadd);

        for (l = 0; l < numadd; l++)
            set_table[i].list_objects->push_back(objects[l]);

        numadd = set->GetItem(S_VARIANTE)->GetNumberItem();
        if (numadd)
            set_table[i].variante = new std::vector < struct set_variante_data >;

        for (j = 0; j < numadd; j++) {
            CItem *varnt = set->GetItem(S_VARIANTE)->GetItem(j);
            struct set_variante_data varz;

            varz.count_objects = varnt->GetItem(S_VAR_COUNT)->GetInt();
            if (varnt->GetItem(S_VAR_SCORE)->GetString())
                varz.score = str_dup(varnt->GetItem(S_VAR_SCORE)->GetString());
            else
                varz.score = '\0';

            if (varnt->GetItem(S_VAR_SCHAR)->GetString())
                varz.start_to_char = str_dup(varnt->GetItem(S_VAR_SCHAR)->GetString());
            else
                varz.start_to_char = '\0';
            if (varnt->GetItem(S_VAR_SROOM)->GetString())
                varz.start_to_room = str_dup(varnt->GetItem(S_VAR_SROOM)->GetString());
            else
                varz.start_to_room = '\0';

            if (varnt->GetItem(S_VAR_ECHAR)->GetString())
                varz.stop_to_char = str_dup(varnt->GetItem(S_VAR_ECHAR)->GetString());
            else
                varz.stop_to_char = '\0';
            if (varnt->GetItem(S_VAR_EROOM)->GetString())
                varz.stop_to_room = str_dup(varnt->GetItem(S_VAR_EROOM)->GetString());
            else
                varz.stop_to_room = '\0';

            if (varnt->GetItem(S_VAR_AFFECT)->GetString())
                asciiflag_conv((char *) varnt->GetItem(S_VAR_AFFECT)->GetString(), &varz.affects);
            else
                varz.affects = clear_flags;

            nadd = varnt->GetItem(S_VAR_APPLY)->GetStrListNumber();
            for (n = 0; n < nadd; n++) {
                struct obj_affected_type apply;

                varnt->GetItem(S_VAR_APPLY)->GetStrList(n, t[0], t[1]);
                apply.location = t[0];
                apply.modifier = t[1];
                varz.addons.push_back(apply);
            }
            nadd = varnt->GetItem(S_VAR_SKILL)->GetStrListNumber();
            for (n = 0; n < nadd; n++) {
                struct obj_affected_type skill;

                varnt->GetItem(S_VAR_SKILL)->GetStrList(n, t[0], t[1]);
                skill.location = t[0];
                skill.modifier = t[1];
                varz.skills.push_back(skill);
            }

            set_table[i].variante->push_back(varz);
        }
        top_of_sets++;
    }
}

/////////////////////////////////////////////////////////////////////////////

void boot_hit_messages(void)
{

    if (!HitMess.Initialization())
        exit(1);

    if (!HitMess.ReadConfig(ShareFile(mud->hitMessageFile).getCPath())) {
        log("ОШИБКА: не найден файл сообщений ударов %s", mud->hitMessageFile.c_str());
        exit(1);
    }

}
