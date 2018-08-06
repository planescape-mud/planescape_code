/**************************************************************************
    νπν "ηÒΑΞΙ νΙÒΑ" (Σ) 2002-2005 αΞΔÒΕΚ εÒΝΙΫΙΞ
    ϊΑΗÒΥΪΛΑ ÒΕΓΕΠΤΟΧ
 **************************************************************************/

#include "xenchant.h"
#include "parser_id.h"

int ECH_NUMBER;  //ϊαώαςοχαξιε
int ECH_IDENTIFER; //ιξδεξτιζιλατος
int ECH_NAME;  //ξαϊχαξιε
int ECH_ALIAS;  //σιξοξιν
int ECH_DESCRIPTION; //οπισαξιε
int ECH_INCLUDE; //σοσταχ
int ECH_INC_TYPE; //τιπ
int ECH_INC_COUNT; //λομιώεστχο
int ECH_OBJECT_TYPE; //πςεδνετ
int ECH_WEAR_TYPE; //ισπομψϊοχαξιε
int ECH_APPLY;  //χμιρξιε
int ECH_AFFECT;  //όζζελτω
int ECH_SKILL;  //υνεξιε
int ECH_MINIMAL;  //νιξινυν
int ECH_MAXIMUM;  //ναλσινυν

///////////////////////////////////////////////////////////////////////////////
CEnchant::CEnchant() {
}

CEnchant::~CEnchant() {
}
///////////////////////////////////////////////////////////////////////////////

bool CEnchant::Initialization(void) {

    ECH_NUMBER = Proto->AddItem(TYPE_INT, "ζοςνυμα");
    ECH_IDENTIFER = Proto->AddItem(TYPE_STRING, "ιδεξτιζιλατος");
    ECH_NAME = Proto->AddItem(TYPE_STRING, "ξαϊχαξιε");
    ECH_ALIAS = Proto->AddItem(TYPE_STRING, "σιξοξιν");
    ECH_DESCRIPTION = Proto->AddItem(TYPE_STRING, "οπισαξιε");
    ECH_INCLUDE = Proto->AddItem(TYPE_STRUCT, "σοσταχ");
    CItemProto *include = Proto->GetItem(ECH_INCLUDE);
    ECH_INC_TYPE = include->AddItem(TYPE_INT, "τιπ");
    ECH_INC_COUNT = include->AddItem(TYPE_INT, "λομιώεστχο");
    ECH_OBJECT_TYPE = Proto->AddItem(TYPE_LIST, "πςεδνετ", true, ParserConst.GetList(LIST_OBJ_TYPES));
    ECH_WEAR_TYPE = Proto->AddItem(TYPE_VECTOR, "ισπομψϊοχαξιε", false, ParserConst.GetVector(VEC_OBJ_WEAR));
    ECH_APPLY = Proto->AddItem(TYPE_STRLIST, "χμιρξιε", false, ParserConst.GetList(LIST_APPLY));
    ECH_AFFECT = Proto->AddItem(TYPE_VECTOR, "όζζελτω", false, ParserConst.GetVector(VEC_MOB_AFF));
    ECH_SKILL = Proto->AddItem(TYPE_STRLIST, "υνεξιε", false, ParserConst.GetList(LIST_SKILLS));
    ECH_MINIMAL = Proto->AddItem(TYPE_INT, "νιξινυν", true);
    ECH_MAXIMUM = Proto->AddItem(TYPE_INT, "ναλσινυν", true);

    return CheckInit();
}

CEnchant xEnchant;

