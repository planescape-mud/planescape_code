/**************************************************************************
    νπν "ηÒΑΞΙ νΙÒΑ" (Σ) 2002-2004 αΞΔÒΕΚ εÒΝΙΫΙΞ
    ϊΑΗÒΥΪΛΑ ΤΕΝΠΜΕΚΤΟΧ ΠÒΕΔΝΕΤΟΧ
 **************************************************************************/

#include "xtempl.h"
#include "parser_id.h"


// οΠÒΕΔΕΜΕΞΙΕ ΟΣΞΟΧΞΩΘ ΠΑÒΑΝΕΤÒΟΝ
int A_NUMBER;  //ξονες
int A_NAME;    //ξαϊχαξιε
int A_DESC;    //
int A_ALIAS;
int A_TYPE;    //τιπ
int A_PTYPE;   //ποδτιπ
int A_WEIGHT;    //χεσ
int A_MATERIAL;  //νατεςιαμ
int A_PRICE;     //γεξα
int A_AC;        //βςοξρ
int A_ARM0;      //ϊςεφυύεε
int A_ARM1;      //ϊλομΰύεε
int A_ARM2;      //ϊυδαςξοε
int A_CLASS;     //λμασσ
int A_WEAR;  //ισπομψϊοχαξιε
int A_SPEED; //σλοςοστψ

//νατεςιαμ
int A_MAT_TYPE;  //τιπ
int A_MAT_VAL; //λομιώεστχο
int A_MAT_MAIN;  //οσξοχξοε

int A_P0; //πςεφυύεε
int A_P1; //πλομΰύεε
int A_P2; //πυδαςξοε

///////////////////////////////////////////////////////////////////////////////
CArmTmp::CArmTmp() {
}

CArmTmp::~CArmTmp() {
}
///////////////////////////////////////////////////////////////////////////////

CArmTmp ArmTmp;

bool CArmTmp::Initialization(void) {
    A_NUMBER = Proto->AddItem(TYPE_INT, "ϋαβμοξ");
    A_NAME  = Proto->AddItem(TYPE_STRING, "ξαϊχαξιε");
    A_ALIAS = Proto->AddItem(TYPE_STRING, "ιδεξτιζιλατος", false);
    A_DESC  = Proto->AddItem(TYPE_STRING, "οπισαξιε", false);
    A_TYPE  = Proto->AddItem(TYPE_LIST, "τιπ", true, ParserConst.GetList(LIST_TYPE_ARMOR));
    A_PTYPE = Proto->AddItem(TYPE_LIST, "ποδτιπ", false, ParserConst.GetList(LIST_PTYPE_ARMOR));
    A_WEIGHT = Proto->AddItem(TYPE_INT, "χεσ", false);
    A_PRICE = Proto->AddItem(TYPE_INT, "γεξα", false);
    A_AC  = Proto->AddItem(TYPE_INT, "βςοξρ", false);
    A_ARM0  = Proto->AddItem(TYPE_INT, "ϊςεφυύεε", false);
    A_ARM1  = Proto->AddItem(TYPE_INT, "ϊλομΰύεε", false);
    A_ARM2  = Proto->AddItem(TYPE_INT, "ϊυδαςξοε", false);
    A_CLASS = Proto->AddItem(TYPE_LIST, "λμασσ", false, ParserConst.GetList(LIST_TYPE_ACLASS));
    A_SPEED = Proto->AddItem(TYPE_INT, "σλοςοστψ", false);
    A_WEAR  = Proto->AddItem(TYPE_VECTOR, "ισπομψϊοχαξιε", false,
                             ParserConst.GetVector(VEC_OBJ_WEAR));

    A_P0  = Proto->AddItem(TYPE_INT, "πςεφυύεε", false);
    A_P1  = Proto->AddItem(TYPE_INT, "πλομΰύεε", false);
    A_P2  = Proto->AddItem(TYPE_INT, "πυδαςξοε", false);

    A_MATERIAL = Proto->AddItem(TYPE_STRUCT, "νατεςιαμ", true);
    CItemProto *mat = Proto->GetItem(A_MATERIAL);
    A_MAT_TYPE = mat->AddItem(TYPE_LIST, "τιπ", true, ParserConst.GetList(LIST_MATERIAL_TYPE));
    A_MAT_VAL = mat->AddItem(TYPE_INT, "λομιώεστχο", true);
    A_MAT_MAIN    = mat->AddItem(TYPE_INT, "οσξοχξοε", false);

    return CheckInit();
}

/******************************************************************************/
int W_NUMBER; //ϋαβμοξ
int W_NAME; //ξαϊχαξιε
int W_DESC; //οπισαξιε
int W_ALIAS; //ιδεξτιζιλατος
int W_TYPE; //τιπ
int W_MESS; //σοοβύεξιε
int W_HIT; //υδας
int W_HIT_TYPE; //υδας.τιπ
int W_HIT_DAM; //υδας,χςεδ
int W_WEIGHT; //χεσ
int W_PRICE; //γεξα
int W_SPEED; //σλοςοστψ
int W_CRITIC; //λςιτιώεσλικ
int W_MATERIAL; //νατεςιαμ
int W_MAT_TYPE;  //νατεςιαμ.τιπ
int W_MAT_VAL; //νατεςιαμ.λομιώεστχο
int W_MAT_MAIN;  //νατεςιαμ.οσξοχξοε
int W_WEAR; //ισπομψϊοχαξιε
///////////////////////////////////////////////////////////////////////////////
CWeapTmp::CWeapTmp() {
}

CWeapTmp::~CWeapTmp() {
}
///////////////////////////////////////////////////////////////////////////////

CWeapTmp WeapTmp;

bool CWeapTmp::Initialization(void) {
    W_NUMBER = Proto->AddItem(TYPE_INT, "ϋαβμοξ");
    W_NAME  = Proto->AddItem(TYPE_STRING, "ξαϊχαξιε");
    W_ALIAS  = Proto->AddItem(TYPE_STRING, "ιξδεξτιζιλατος", false);
    W_DESC  = Proto->AddItem(TYPE_STRING, "οπισαξιε", false);
    W_ALIAS = Proto->AddItem(TYPE_STRING, "ιδεξτιζιλατος");
    W_TYPE  = Proto->AddItem(TYPE_LIST, "τιπ", true, ParserConst.GetList(LIST_WEAPON_SKILLS));
    W_MESS  = Proto->AddItem(TYPE_LIST, "σοοβύεξιε", true, ParserConst.GetList(LIST_WEAPON_TYPE));
    W_HIT  = Proto->AddItem(TYPE_STRUCT, "υδας", true);
    CItemProto *hit = Proto->GetItem(W_HIT);
    W_HIT_TYPE = hit->AddItem(TYPE_LIST, "τιπ", true, ParserConst.GetList(LIST_DAMAGE));
    W_HIT_DAM = hit->AddItem(TYPE_STRING, "σχςεδ", true);
    W_WEIGHT = Proto->AddItem(TYPE_INT, "χεσ");
    W_PRICE = Proto->AddItem(TYPE_INT, "στοινοστψ");
    W_SPEED = Proto->AddItem(TYPE_INT, "σλοςοστψ");
    W_CRITIC = Proto->AddItem(TYPE_INT, "λςιτιώεσλοε");
    W_MATERIAL = Proto->AddItem(TYPE_STRUCT, "νατεςιαμ", true);
    CItemProto *mat = Proto->GetItem(W_MATERIAL);
    W_MAT_TYPE = mat->AddItem(TYPE_LIST, "τιπ", true, ParserConst.GetList(LIST_MATERIAL_TYPE));
    W_MAT_VAL = mat->AddItem(TYPE_INT, "λομιώεστχο", true);
    W_MAT_MAIN    = mat->AddItem(TYPE_INT, "οσξοχξοε", false);
    W_WEAR  = Proto->AddItem(TYPE_VECTOR, "ισπομψϊοχαξιε", true, ParserConst.GetVector(VEC_OBJ_WEAR));

    return CheckInit();
}

/**************************************************************************/
int M_NUMBER;    //ϋαβμοξ
int M_NAME;      //ξαϊχαξιε
int M_ALIAS; //ιδεξτιζιλατος
int M_DESC; //οπισαξιε
int M_TYPE; //τιπ
int M_HITS; //υδας
int M_HIT_TYPE; //υδας.τιπ
int M_HIT_DAM; //υδας,χςεδ
int M_MATERIAL; //νατεςιαμ
int M_MAT_TYPE;  //νατεςιαμ.τιπ
int M_MAT_VAL; //νατεςιαμ.λομιώεστχο
int M_MAT_MAIN;  //νατεςιαμ.οσξοχξοε

CMissTmp::CMissTmp() {
}

CMissTmp::~CMissTmp() {
}
///////////////////////////////////////////////////////////////////////////////

CMissTmp MissTmp;

bool CMissTmp::Initialization(void) {
    M_NUMBER = Proto->AddItem(TYPE_INT, "ϋαβμοξ", true);
    M_NAME  = Proto->AddItem(TYPE_STRING, "ξαϊχαξιε", true);
    M_ALIAS = Proto->AddItem(TYPE_STRING, "ιδεξτιζιλατος", false);
    M_DESC  = Proto->AddItem(TYPE_STRING, "οπισαξιε", false);
    M_TYPE  = Proto->AddItem(TYPE_LIST, "τιπ", true, ParserConst.GetList(LIST_MISSILE));
    M_HITS  = Proto->AddItem(TYPE_STRUCT, "υδας", true);
    CItemProto *hit = Proto->GetItem(M_HITS);
    M_HIT_TYPE = hit->AddItem(TYPE_LIST, "τιπ", true, ParserConst.GetList(LIST_DAMAGE));
    M_HIT_DAM = hit->AddItem(TYPE_STRING, "σχςεδ", true);

    M_MATERIAL = Proto->AddItem(TYPE_STRUCT, "νατεςιαμ", true);
    CItemProto *mat = Proto->GetItem(M_MATERIAL);
    M_MAT_TYPE = mat->AddItem(TYPE_LIST, "τιπ", true, ParserConst.GetList(LIST_MATERIAL_TYPE));
    M_MAT_VAL = mat->AddItem(TYPE_INT, "λομιώεστχο", true);
    M_MAT_MAIN    = mat->AddItem(TYPE_INT, "οσξοχξοε", false);

    return CheckInit();
}

/***************************************************************************/
int S_NUMBER;  //ξονες
int S_IDENT;           //ιδεξτιζιλατος
int S_OBJECTS;           //πςεδνετω
int S_VARIANTE;  //χαςιαξτ
int S_VAR_COUNT; //χαςιαξτ.λομιώεστχο
int S_VAR_DESC;  //χαςιαξτ.οπισαξιε
int S_VAR_AFFECT; //χαςιαξτ.όζζελτ
int S_VAR_APPLY; //χαςιαξτ.δοπομξεξιρ
int S_VAR_SKILL; //χαςιαξτ.υνεξιε
int S_VAR_SCORE; //χαςιαξτ.σώετ
int S_VAR_SCHAR; //χαςιαξτ.σιηςολ
int S_VAR_SROOM; //χαςιαξτ.σμολαγιρ
int S_VAR_ECHAR; //χαςιαξτ.λιηςολ
int S_VAR_EROOM; //χαςιαξτ.λμολαγιρ

CSetTmp::CSetTmp() {
}

CSetTmp::~CSetTmp() {
}
///////////////////////////////////////////////////////////////////////////////

CSetTmp SetTmp;

bool CSetTmp::Initialization(void) {
    S_NUMBER = Proto->AddItem(TYPE_INT, "ξαβος", true);
    S_IDENT = Proto->AddItem(TYPE_STRING, "ιδεξτιζιλατος", false);
    S_OBJECTS = Proto->AddItem(TYPE_SCRIPT, "πςεδνετω", true);
    S_VARIANTE = Proto->AddItem(TYPE_STRUCT, "χαςιαξτ", true);
    CItemProto *varnt = Proto->GetItem(S_VARIANTE);
    S_VAR_COUNT = varnt->AddItem(TYPE_INT, "λομιώεστχο", true);
    S_VAR_DESC  = varnt->AddItem(TYPE_STRING, "οπισαξιε", false);
    S_VAR_AFFECT = varnt->AddItem(TYPE_VECTOR, "όζζελτ", false, ParserConst.GetVector(VEC_MOB_AFF));
    S_VAR_APPLY = varnt->AddItem(TYPE_STRLIST, "δοπομξεξιρ", false, ParserConst.GetList(LIST_APPLY));
    S_VAR_SKILL = varnt->AddItem(TYPE_STRLIST, "υνεξιρ", false, ParserConst.GetList(LIST_SKILLS));
    S_VAR_SCORE = varnt->AddItem(TYPE_STRING, "σώετ", false);
    S_VAR_SCHAR = varnt->AddItem(TYPE_STRING, "σιηςολ", false);
    S_VAR_SROOM = varnt->AddItem(TYPE_STRING, "σμολαγιρ", false);
    S_VAR_ECHAR = varnt->AddItem(TYPE_STRING, "λιηςολ", false);
    S_VAR_EROOM = varnt->AddItem(TYPE_STRING, "λμολαγιρ", false);

    return CheckInit();
}

/////////////////////////////////////////////////////////////////////////////

int MS_TYPE;  //υδας
int MS_SUB_CHAR;  //ποδστςυλτυςα.ιηςολ
int MS_SUB_VICT;  //ποδστςυλτυςα.φεςτχα
int MS_SUB_ROOM;  //ποδστςυλτυςα.μολαγιρ
int MS_HIT_NONE;   //βεϊοςυφιρ
int MS_HIT_WEAP;  //σοςυφιεν
int MS_HWP_CHAR;  //ποδστςυλτυςα.ιηςολ
int MS_HWP_VICT;  //ποδστςυλτυςα.φεςτχα
int MS_HWP_ROOM;  //ποδστςυλτυςα.μολαγιρ
int MS_MISS_NONE; //πςοναθβεϊ
int MS_MMN_CHAR;  //ποδστςυλτυςα.ιηςολ
int MS_MMN_VICT;  //ποδστςυλτυςα.φεςτχα
int MS_MMN_ROOM;  //ποδστςυλτυςα.μολαγιρ

int MS_MISS_WEAP; //πςναθοςυφιε
int MS_KILL_NONE; //σνεςτψβεϊ
int MS_KILL_WEAP; //σνεςτψοςυφιε
int MS_ARM_NONE;  //δοσπεθβεϊ
int MS_ARM_WEAP;  //δοσπεθοςυφιε
int MS_SKIN_NONE; //λοφαβεϊ
int MS_SKIN_WEAP; //λοφαοςυφιε

CHitMess::CHitMess() {
}

CHitMess::~CHitMess() {
}
///////////////////////////////////////////////////////////////////////////////
CHitMess HitMess;

bool CHitMess::Initialization(void) {
    MS_TYPE = Proto->AddItem(TYPE_LIST, "σοοβύεξιε", true, ParserConst.GetList(LIST_MOB_HIT));

    MS_HIT_NONE = Proto->AddItem(TYPE_STRUCT, "βεϊοςυφιρ", true);
    CItemProto *mhn = Proto->GetItem(MS_HIT_NONE);
    MS_SUB_CHAR = mhn->AddItem(TYPE_STRING, "ιηςολ", true);
    MS_SUB_VICT = mhn->AddItem(TYPE_STRING, "φεςτχα", true);
    MS_SUB_ROOM = mhn->AddItem(TYPE_STRING, "μολαγιρ", true);

    MS_HIT_WEAP = Proto->AddItem(TYPE_STRUCT, "σοςυφιεν", true);
    CItemProto *mhw = Proto->GetItem(MS_HIT_WEAP);
    MS_HWP_CHAR = mhw->AddItem(TYPE_STRING, "ιηςολ", true);
    MS_HWP_VICT = mhw->AddItem(TYPE_STRING, "φεςτχα", true);
    MS_HWP_ROOM = mhw->AddItem(TYPE_STRING, "μολαγιρ", true);

    MS_MISS_NONE = Proto->AddItem(TYPE_STRUCT, "πςοναθβεϊ", true);
    CItemProto *mmn = Proto->GetItem(MS_MISS_NONE);
    MS_MMN_CHAR = mmn->AddItem(TYPE_STRING, "ιηςολ", true);
    MS_MMN_VICT = mmn->AddItem(TYPE_STRING, "φεςτχα", true);
    MS_MMN_ROOM = mmn->AddItem(TYPE_STRING, "μολαγιρ", true);

    MS_MISS_WEAP = Proto->AddItem(TYPE_STRUCT, "πςοναθοςυφιε", true);
    CItemProto *mmw = Proto->GetItem(MS_MISS_WEAP);
    MS_SUB_CHAR = mmw->AddItem(TYPE_STRING, "ιηςολ", true);
    MS_SUB_VICT = mmw->AddItem(TYPE_STRING, "φεςτχα", true);
    MS_SUB_ROOM = mmw->AddItem(TYPE_STRING, "μολαγιρ", true);

    MS_KILL_NONE = Proto->AddItem(TYPE_STRUCT, "σνεςτψβεϊ", true);
    CItemProto *mkn = Proto->GetItem(MS_KILL_NONE);
    MS_SUB_CHAR = mkn->AddItem(TYPE_STRING, "ιηςολ", true);
    MS_SUB_VICT = mkn->AddItem(TYPE_STRING, "φεςτχα", true);
    MS_SUB_ROOM = mkn->AddItem(TYPE_STRING, "μολαγιρ", true);

    MS_KILL_WEAP = Proto->AddItem(TYPE_STRUCT, "σνεςτψοςυφιε", true);
    CItemProto *mkw = Proto->GetItem(MS_KILL_WEAP);
    MS_SUB_CHAR = mkw->AddItem(TYPE_STRING, "ιηςολ", true);
    MS_SUB_VICT = mkw->AddItem(TYPE_STRING, "φεςτχα", true);
    MS_SUB_ROOM = mkw->AddItem(TYPE_STRING, "μολαγιρ", true);

    MS_ARM_NONE = Proto->AddItem(TYPE_STRUCT, "δοσπεθβεϊ", true);
    CItemProto *man = Proto->GetItem(MS_ARM_NONE);
    MS_SUB_CHAR = man->AddItem(TYPE_STRING, "ιηςολ", true);
    MS_SUB_VICT = man->AddItem(TYPE_STRING, "φεςτχα", true);
    MS_SUB_ROOM = man->AddItem(TYPE_STRING, "μολαγιρ", true);

    MS_ARM_WEAP = Proto->AddItem(TYPE_STRUCT, "δοσπεθοςυφιε", true);
    CItemProto *maw = Proto->GetItem(MS_ARM_WEAP);
    MS_SUB_CHAR = maw->AddItem(TYPE_STRING, "ιηςολ", true);
    MS_SUB_VICT = maw->AddItem(TYPE_STRING, "φεςτχα", true);
    MS_SUB_ROOM = maw->AddItem(TYPE_STRING, "μολαγιρ", true);

    MS_SKIN_NONE = Proto->AddItem(TYPE_STRUCT, "λοφαβεϊ", true);
    CItemProto *msn = Proto->GetItem(MS_SKIN_NONE);
    MS_SUB_CHAR = msn->AddItem(TYPE_STRING, "ιηςολ", true);
    MS_SUB_VICT = msn->AddItem(TYPE_STRING, "φεςτχα", true);
    MS_SUB_ROOM = msn->AddItem(TYPE_STRING, "μολαγιρ", true);

    MS_SKIN_WEAP = Proto->AddItem(TYPE_STRUCT, "λοφαοςυφιε", true);
    CItemProto *msw = Proto->GetItem(MS_SKIN_WEAP);
    MS_SUB_CHAR = msw->AddItem(TYPE_STRING, "ιηςολ", true);
    MS_SUB_VICT = msw->AddItem(TYPE_STRING, "φεςτχα", true);
    MS_SUB_ROOM = msw->AddItem(TYPE_STRING, "μολαγιρ", true);

    return CheckInit();
}

