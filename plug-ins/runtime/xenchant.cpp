/**************************************************************************
    МПМ "Грани Мира" (с) 2002-2005 Андрей Ермишин
    Загрузка рецептов
 **************************************************************************/

#include "sysdep.h"
#include "structs.h"
#include "db.h"
#include "utils.h"
#include "interpreter.h"
#include "handler.h"
#include "xenchant.h"
#include "spells.h"
#include "case.h"
#include "parser_id.h"
#include "constants.h"
#include "comm.h"
#include "planescape.h"
#include "mudfile.h"


void boot_enchant(void)
{
    int number, i;

    if (!xEnchant.Initialization())
        exit(1);

    if (!xEnchant.ReadConfig(ShareFile(mud->enchantFile).getCPath())) {
        log("Отсутствует файл %s", mud->enchantFile.c_str());
        exit(1);
    }

    number = xEnchant.GetNumberItem();
    for (i = 0; i < number; i++) {
        CItem *ex = xEnchant.GetItem(i);

        ParserConst.SetList(LIST_ENCHANT, ex->GetItem(ECH_NUMBER)->GetInt(),
                            (char *) ex->GetItem(ECH_IDENTIFER)->GetString());
    }
}


int find_enchant_num(int enchant_no)
{
    int index, number;

    number = xEnchant.GetNumberItem();
    for (index = 0; index < number; index++) {
        CItem *enct = xEnchant.GetItem(index);

        if (enct->GetItem(ECH_NUMBER)->GetInt() == enchant_no)
            return (index);
    }

    return (-1);
}

int find_enchant_num(char *name)
{
    int index, ok, number;
    char *temp, *temp2;
    char first[256], first2[256], *realname;

    number = xEnchant.GetNumberItem();
    for (index = 0; index < number; index++) {
        CItem *enc = xEnchant.GetItem(index);

        realname = enc->GetItem(ECH_NAME)->GetString();
        if (!realname || !*realname)
            continue;
        if (is_abbrev(name, realname))
            return (index);
        ok = TRUE;
        temp = any_one_arg((char *) realname, first);
        temp2 = any_one_arg(name, first2);
        while (*first && *first2 && ok) {
            if (!is_abbrev(first2, first))
                ok = FALSE;
            temp = any_one_arg(temp, first);
            temp2 = any_one_arg(temp2, first2);
        }
        if (ok && !*first2)
            return (index);
    }

    return (-1);
}
