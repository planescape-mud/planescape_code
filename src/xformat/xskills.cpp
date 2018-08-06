/**************************************************************************
    νπν "ηÒΑΞΙ νΙÒΑ" (Σ) 2002-2003 αΞΔÒΕΚ εÒΝΙΫΙΞ
    ϊΑΗÒΥΪΛΑ ΖΑΚΜΟΧ ΙΗÒΟΧΟΗΟ ΝΙÒΑ
 **************************************************************************/

#include "xskills.h"
#include "parser_id.h"


// οΠÒΕΔΕΜΕΞΙΕ ΟΣΞΟΧΞΩΘ ΠΑÒΑΝΕΤÒΟΝ
int SKL_NUMBER;  //υνεξιε
int SKL_NAME;   //ξαϊχαξιε
int SKL_ALIAS;  //ιδεξτιζιλατος
int SKL_USELESS; //ισπομψϊοχαξιε (ΣΤÒΥΛΤΥÒΑ)
int SKL_USE_CLASS; //πςοζεσσιρ
int SKL_USE_TEMPL; //τενπμεκτ
int SKL_USE_PRACT; //τςεξιςοχλα
int SKL_USE_LEVEL; //υςοχεξψ
int SKL_USE_EFFECT; //όζζελτιχξοστψ
int SKL_MISS_ATTACK; //πςοναθ (ΣΤÒΥΛΤΥÒΑ)
int SKL_MA_CHAR;
int SKL_MA_VICT;
int SKL_MA_ROOM;
int SKL_HIT_ATTACK; //ποςαφεξιε
int SKL_HA_CHAR;
int SKL_HA_VICT;
int SKL_HA_ROOM;
int SKL_DTH_ATTACK; //σνεςτψ
int SKL_DA_CHAR;
int SKL_DA_VICT;
int SKL_DA_ROOM;
int SKL_NHS_ATTACK; //ξευρϊχινοστψ
int SKL_NA_CHAR;
int SKL_NA_VICT;
int SKL_NA_ROOM;
int SKL_ARM_ATTACK; //ποημούεξιε
int SKL_AM_CHAR;
int SKL_AM_VICT;
int SKL_AM_ROOM;
int SKL_ARM2_ATTACK; //αμψτεςξατιχα
int SKL_A2_CHAR;
int SKL_A2_VICT;
int SKL_A2_ROOM;
int SKL_TMPL;
int SKL_TM_CHAR;
int SKL_TM_VICT;
int SKL_TM_ROOM;
int SKL_TMPS;
int SKL_TS_CHAR;
int SKL_TS_VICT;
int SKL_TS_ROOM;

///////////////////////////////////////////////////////////////////////////////
CSkl::CSkl() {
}

CSkl::~CSkl() {
}
///////////////////////////////////////////////////////////////////////////////

CSkl Skl;

bool CSkl::Initialization(void) {
    SKL_NUMBER = Proto->AddItem(TYPE_INT, "υνεξιε");
    SKL_NAME = Proto->AddItem(TYPE_STRING, "ξαϊχαξιε");
    SKL_ALIAS = Proto->AddItem(TYPE_STRING, "ιδεξτιζιλατος");
    SKL_USELESS = Proto->AddItem(TYPE_STRUCT, "ισπομψϊοχαξιε");
    CItemProto *skuse = Proto->GetItem(SKL_USELESS);
    SKL_USE_CLASS = skuse->AddItem(TYPE_LIST, "πςοζεσσιρ", true, ParserConst.GetList(LIST_CLASS));
    SKL_USE_TEMPL  = skuse->AddItem(TYPE_LIST, "τενπμεκτ", true, ParserConst.GetList(LIST_GODS));
    SKL_USE_PRACT = skuse->AddItem(TYPE_INT, "τςεξιςοχλα");
    SKL_USE_LEVEL  = skuse->AddItem(TYPE_INT, "υςοχεξψ");
    SKL_USE_EFFECT = skuse->AddItem(TYPE_INT, "όζζελτιχξοστψ");
    SKL_MISS_ATTACK = Proto->AddItem(TYPE_STRUCT, "πςοναθ");
    CItemProto *mMis = Proto->GetItem(SKL_MISS_ATTACK);
    SKL_MA_CHAR = mMis->AddItem(TYPE_STRING, "πεςσοξαφ");
    SKL_MA_VICT = mMis->AddItem(TYPE_STRING, "φεςτχα");
    SKL_MA_ROOM = mMis->AddItem(TYPE_STRING, "μολαγιρ");
    SKL_HIT_ATTACK = Proto->AddItem(TYPE_STRUCT, "ποςαφεξιε");
    CItemProto *mHit = Proto->GetItem(SKL_HIT_ATTACK);
    SKL_HA_CHAR = mHit->AddItem(TYPE_STRING, "πεςσοξαφ");
    SKL_HA_VICT = mHit->AddItem(TYPE_STRING, "φεςτχα");
    SKL_HA_ROOM = mHit->AddItem(TYPE_STRING, "μολαγιρ");
    SKL_DTH_ATTACK = Proto->AddItem(TYPE_STRUCT, "σνεςτψ");
    CItemProto *mDth = Proto->GetItem(SKL_DTH_ATTACK);
    SKL_DA_CHAR = mDth->AddItem(TYPE_STRING, "πεςσοξαφ");
    SKL_DA_VICT = mDth->AddItem(TYPE_STRING, "φεςτχα");
    SKL_DA_ROOM = mDth->AddItem(TYPE_STRING, "μολαγιρ");
    SKL_NHS_ATTACK = Proto->AddItem(TYPE_STRUCT, "ξευρϊχινοστψ");
    CItemProto *mNhs = Proto->GetItem(SKL_NHS_ATTACK);
    SKL_NA_CHAR = mNhs->AddItem(TYPE_STRING, "πεςσοξαφ");
    SKL_NA_VICT = mNhs->AddItem(TYPE_STRING, "φεςτχα");
    SKL_NA_ROOM = mNhs->AddItem(TYPE_STRING, "μολαγιρ");
    SKL_ARM_ATTACK = Proto->AddItem(TYPE_STRUCT, "ποημούεξιε");
    CItemProto *mArm = Proto->GetItem(SKL_ARM_ATTACK);
    SKL_AM_CHAR = mArm->AddItem(TYPE_STRING, "πεςσοξαφ");
    SKL_AM_VICT = mArm->AddItem(TYPE_STRING, "φεςτχα");
    SKL_AM_ROOM = mArm->AddItem(TYPE_STRING, "μολαγιρ");
    SKL_ARM2_ATTACK = Proto->AddItem(TYPE_STRUCT, "αμψτεςξατιχα");
    CItemProto *mA2 = Proto->GetItem(SKL_ARM2_ATTACK);
    SKL_A2_CHAR = mA2->AddItem(TYPE_STRING, "πεςσοξαφ");
    SKL_A2_VICT = mA2->AddItem(TYPE_STRING, "φεςτχα");
    SKL_A2_ROOM = mA2->AddItem(TYPE_STRING, "μολαγιρ");

    /* SKL_TMPL = Proto->AddItem(TYPE_STRUCT,"ϋαβμοξ");
     CItemProto *tm = Proto->GetItem(SKL_TMPL);
      SKL_TM_CHAR = tm->AddItem(TYPE_STRING,"πεςσοξαφ");
      SKL_TM_VICT = tm->AddItem(TYPE_STRING,"φεςτχα");
      SKL_TM_ROOM = tm->AddItem(TYPE_STRING,"μολαγιρ");

     SKL_TMPS = Proto->AddItem(TYPE_STRUCT,"ϋαβμοξ_πςοναθ");
     CItemProto *ts = Proto->GetItem(SKL_TMPS);
      SKL_TS_CHAR   = ts->AddItem(TYPE_STRING,"πεςσοξαφ");
      SKL_TS_VICT   = ts->AddItem(TYPE_STRING,"φεςτχα");
      SKL_TS_ROOM   = ts->AddItem(TYPE_STRING,"μολαγιρ"); */

    return CheckInit();
}

