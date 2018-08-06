/**************************************************************************
    МПМ "Грани Мира" (с) 2002-2003 Андрей Ермишин
    Загрузка файлов игрового мира
 **************************************************************************/

#include "sysdep.h"
#include "structs.h"
#include "db.h"
#include "utils.h"
#include "xskills.h"
#include "parser_id.h"
#include "spells.h"
#include "planescape.h"
#include "mudfile.h"


void boot_skills(void)
{
    if (!Skl.Initialization())
        exit(1);

    if (!Skl.ReadConfig(ShareFile(mud->xskillsFile).getCPath()))
        exit(1);

    /* Пока не будут готовы все скиллы временно закоментируем
       number = Skl.GetNumberItem();
       for (i=0;  i < number; i++)
       {
       CItem *s = Skl.GetItem(i);
       ParserConst.SetList(LIST_SKILLS, s->GetItem(SKL_NUMBER)->GetInt(), (char *) s->GetItem(SKL_ALIAS)->GetString());
       } */

}

//возвращает внутрений номер умения
int find_skill_num(int skill_no)
{
    int index, number;

    number = Skl.GetNumberItem();
    for (index = 0; index < number; index++) {
        CItem *skill = Skl.GetItem(index);

        if (skill->GetItem(SKL_NUMBER)->GetInt() == skill_no)
            return (index);
    }

    return (-1);
}

void GetSkillMessage(int skill, int hTyp, struct P_message &pMess)
{

    //log("Вызываю GetSkillMessage(%d)",skill);

    if (skill)
        skill = find_skill_num(skill);

    if (!Skl.GetItem(skill)->GetItem(SKL_MISS_ATTACK)->GetNumberItem()) {
        pMess.valid = false;
        return;
    }

    pMess.valid = true;

    CItem *mMis = Skl.GetItem(skill)->GetItem(SKL_MISS_ATTACK)->GetItem(0);

    pMess.mChar = mMis->GetItem(SKL_MA_CHAR)->GetString();
    pMess.mVict = mMis->GetItem(SKL_MA_VICT)->GetString();
    pMess.mRoom = mMis->GetItem(SKL_MA_ROOM)->GetString();

    CItem *mHit = Skl.GetItem(skill)->GetItem(SKL_HIT_ATTACK)->GetItem(0);

    pMess.hChar = mHit->GetItem(SKL_HA_CHAR)->GetString();
    pMess.hVict = mHit->GetItem(SKL_HA_VICT)->GetString();
    pMess.hRoom = mHit->GetItem(SKL_HA_ROOM)->GetString();

    CItem *kHit = Skl.GetItem(skill)->GetItem(SKL_DTH_ATTACK)->GetItem(0);

    pMess.kChar = kHit->GetItem(SKL_DA_CHAR)->GetString();
    pMess.kVict = kHit->GetItem(SKL_DA_VICT)->GetString();
    pMess.kRoom = kHit->GetItem(SKL_DA_ROOM)->GetString();

    CItem *pHit = Skl.GetItem(skill)->GetItem(SKL_NHS_ATTACK)->GetItem(0);

    pMess.pChar = pHit->GetItem(SKL_NA_ROOM)->GetString();
    pMess.pVict = pHit->GetItem(SKL_NA_ROOM)->GetString();
    pMess.pRoom = pHit->GetItem(SKL_NA_ROOM)->GetString();

    CItem *pArm = Skl.GetItem(skill)->GetItem(SKL_ARM_ATTACK)->GetItem(0);

    pMess.aChar = pArm->GetItem(SKL_AM_CHAR)->GetString();
    pMess.aVict = pArm->GetItem(SKL_AM_VICT)->GetString();
    pMess.aRoom = pArm->GetItem(SKL_AM_ROOM)->GetString();

    CItem *pAr2 = Skl.GetItem(skill)->GetItem(SKL_ARM2_ATTACK)->GetItem(0);

    pMess.bChar = pAr2->GetItem(SKL_A2_CHAR)->GetString();
    pMess.bVict = pAr2->GetItem(SKL_A2_VICT)->GetString();
    pMess.bRoom = pAr2->GetItem(SKL_A2_ROOM)->GetString();

    /* CItem *tm = Skl.GetItem(skill)->GetItem(SKL_TMPL)->GetItem(0);
       pMess.tChar = tm->GetItem(SKL_TM_CHAR)->GetString();
       pMess.tVict = tm->GetItem(SKL_TM_VICT)->GetString();
       pMess.tRoom = tm->GetItem(SKL_TM_ROOM)->GetString();

       CItem *ts = Skl.GetItem(skill)->GetItem(SKL_TMPS)->GetItem(0);
       pMess.sChar = ts->GetItem(SKL_TS_CHAR)->GetString();
       pMess.sVict = ts->GetItem(SKL_TS_VICT)->GetString();
       pMess.sRoom = ts->GetItem(SKL_TS_ROOM)->GetString(); */

    //log("Конец GetSkillMessage()");
}
