/**************************************************************************
    МПМ "Грани Мира" (с) 2002-2003 Андрей Ермишин
    Загрузка файлов игрового мира
 **************************************************************************/

#include "sysdep.h"
#include "structs.h"
#include "db.h"
#include "utils.h"
#include "xspells.h"
#include "parser_id.h"
#include "spells.h"
#include "planescape.h"
#include "mudfile.h"

///////////////////////////////////////////////////////////////////////////////

void boot_spells(void)
{
    int number, i;

    if (!Spl.Initialization())
        exit(1);

    if (!Spl.ReadConfig(ShareFile(mud->spellsFile).getCPath()))
        exit(1);

    number = Spl.GetNumberItem();
    for (i = 0; i < number; i++) {
        CItem *s = Spl.GetItem(i);

        ParserConst.SetList(LIST_SPELLS, s->GetItem(SPL_NUMBER)->GetInt(),
                            (char *) s->GetItem(SPL_ALIAS)->GetString());
    }

    spellnum_db = find_spell_num(SPELL_DB);
}

void GetSpellMessage(int spellnum, struct P_message &pMess)
{

    //log("Вызываю GetSpellMessage()");

    pMess.valid = false;
    if (!Spl.GetItem(spellnum)->GetItem(SPL_MESSAGES_DAMAGE)->GetNumberItem())
        return;

    CItem *mess = Spl.GetItem(spellnum)->GetItem(SPL_MESSAGES_DAMAGE)->GetItem(0);

    if (!mess)
        return;

    pMess.valid = true;

    pMess.mChar = mess->GetItem(SPL_MESSAGE_MIS_CHAR)->GetString();
    pMess.mVict = mess->GetItem(SPL_MESSAGE_MIS_VICT)->GetString();
    pMess.mRoom = mess->GetItem(SPL_MESSAGE_MIS_ROOM)->GetString();

    pMess.hChar = mess->GetItem(SPL_MESSAGE_HIT_CHAR)->GetString();
    pMess.hVict = mess->GetItem(SPL_MESSAGE_HIT_VICT)->GetString();
    pMess.hRoom = mess->GetItem(SPL_MESSAGE_HIT_ROOM)->GetString();

    pMess.kChar = mess->GetItem(SPL_MESSAGE_KIL_CHAR)->GetString();
    pMess.kVict = mess->GetItem(SPL_MESSAGE_KIL_VICT)->GetString();
    pMess.kRoom = mess->GetItem(SPL_MESSAGE_KIL_ROOM)->GetString();

    pMess.pChar = mess->GetItem(SPL_MESSAGE_GOD_CHAR)->GetString();
    pMess.pVict = mess->GetItem(SPL_MESSAGE_GOD_VICT)->GetString();
    pMess.pRoom = mess->GetItem(SPL_MESSAGE_GOD_ROOM)->GetString();

    pMess.aChar = mess->GetItem(SPL_MESSAGE_MIS_CHAR)->GetString();
    pMess.aVict = mess->GetItem(SPL_MESSAGE_MIS_VICT)->GetString();
    pMess.aRoom = mess->GetItem(SPL_MESSAGE_MIS_ROOM)->GetString();

    pMess.bChar = mess->GetItem(SPL_MESSAGE_MIS_CHAR)->GetString();
    pMess.bVict = mess->GetItem(SPL_MESSAGE_MIS_VICT)->GetString();
    pMess.bRoom = mess->GetItem(SPL_MESSAGE_MIS_ROOM)->GetString();

    //log("Конец GetSpellMessage()");
}
