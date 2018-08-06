#include "xquests.h"
#include "parser_id.h"
#include "strlib.h"
#include "expr.h"


int QUEST_QUEST;  //תבהבמיו
int QUEST_MOB_VNUM; //םןמףפע
int QUEST_NUMBER;  //מןםוע
int QUEST_DONE_MOB; //םןמףפעש (STRLIST, ֱַׂױֵֽ־װ ׳־ױװ, Ú־ֱÞֵ־ֵֹ ֻּֿ-׳ֿ)
int QUEST_DONE_OBJ; //נעוהםופש(STRLIST, ֱַׂױֵֽ־װ ׳־ױװ, Ú־ֱÞֵ־ֵֹ ֻּֿ-׳ֿ)
int QUEST_DONE_FLW; //נןףלוהןקבפולי (STRLIST, ֱַׂױֵֽ־װ ׳־ױװ, Ú־ֱÞֵ־ֵֹ ֻּֿ-׳ֿ)
int QUEST_VAR;  //נועוםוממבס
int QUEST_VAR_NAME; //נועוםוממבס.מבתקבמיו
int QUEST_VAR_CURRENT; //נועוםוממבס.תמב‏ומיו



///////////////////////////////////////////////////////////////////////////////
CQstSave::CQstSave() {
}

CQstSave::~CQstSave() {
}

///////////////////////////////////////////////////////////////////////////////

bool CQstSave::Initialization(void) {
    QUEST_QUEST = Proto->AddItem(TYPE_INT, "תבהבמיו");
    QUEST_MOB_VNUM = Proto->AddItem(TYPE_INT, "םןמףפע");
    QUEST_NUMBER = Proto->AddItem(TYPE_INT, "מןםוע");
    QUEST_DONE_MOB = Proto->AddItem(TYPE_STRLIST, "םןמףפעש", false);
    QUEST_DONE_OBJ = Proto->AddItem(TYPE_STRLIST, "נעוהםופש", false);
    QUEST_DONE_FLW = Proto->AddItem(TYPE_STRLIST, "נןףלוהןקבפולי", false);
    QUEST_VAR = Proto->AddItem(TYPE_STRUCT, "נועוםוממבס", false);
    CItemProto *var = Proto->GetItem(QUEST_VAR);
    QUEST_VAR_NAME = var->AddItem(TYPE_STRING, "מבתקבמיו");
    QUEST_VAR_CURRENT = var->AddItem(TYPE_STRING, "תמב‏ומיו");

    return CheckInit();
}

