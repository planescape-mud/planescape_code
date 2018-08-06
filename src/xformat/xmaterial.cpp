
#include "xmaterial.h"
#include "parser_id.h"


///////////////////////////////////////////////////////////////////////////////
// οΠÒΕΔΕΜΕΞΙΕ ΟΣΞΟΧΞΩΘ ΠΑÒΑΝΕΤÒΟΝ
int MAT_NUMBER; //νατεςιαμ (ΛΜΐή)
int MAT_DESCRIPTION;
int MAT_NAME; //νατεςιαμ (ΞΑΪΧΑΞΙΕ)
int MAT_WEIGHT; //χεσ
int MAT_COST; //γεξα
int MAT_DURAB; //πςοώξοστψ
int MAT_AC; //βςοξρ
int MAT_ARM0; //ςεφυύεε
int MAT_ARM1; //λομΰύεε
int MAT_ARM2; //υδαςξοε
int MAT_HITX;
int MAT_HITY;
int MAT_HITZ;
int MAT_ALIAS;
int MAT_TYPE;
int MAT_HITS;
int MAT_FIRE;
int MAT_COLD;
int MAT_ELECTRO;
int MAT_ACID;
int MAT_INCLUDE; //λονποξεξτ

CMaterial::CMaterial() {
}

CMaterial::~CMaterial() {
}
///////////////////////////////////////////////////////////////////////////////

bool CMaterial::Initialization(void) {
    MAT_NUMBER = Proto->AddItem(TYPE_INT, "νατεςιαμ");
    MAT_NAME   = Proto->AddItem(TYPE_STRING, "ξαϊχαξιε");
    MAT_DESCRIPTION = Proto->AddItem(TYPE_STRING, "οπισαξιε", false);
    MAT_WEIGHT = Proto->AddItem(TYPE_INT, "χεσ");
    MAT_COST   = Proto->AddItem(TYPE_INT, "γεξα");
    MAT_DURAB  = Proto->AddItem(TYPE_INT, "πςοώξοστψ");
    MAT_AC     = Proto->AddItem(TYPE_INT, "βςοξρ", false);
    MAT_ARM0   = Proto->AddItem(TYPE_INT, "ςεφυύεε", false);
    MAT_ARM1   = Proto->AddItem(TYPE_INT, "λομΰύεε", false);
    MAT_ARM2   = Proto->AddItem(TYPE_INT, "υδαςξοε", false);
    MAT_ALIAS  = Proto->AddItem(TYPE_STRING, "ιδεξτιζιλατος", true);
    MAT_TYPE   = Proto->AddItem(TYPE_LIST, "τιπ", true, ParserConst.GetList(LIST_MATERIAL_TYPE));
    MAT_HITS   = Proto->AddItem(TYPE_INT, "υδας", false);
    MAT_FIRE   = Proto->AddItem(TYPE_INT, "οηοξψ", false);
    MAT_COLD   = Proto->AddItem(TYPE_INT, "θομοδ", false);
    MAT_ELECTRO = Proto->AddItem(TYPE_INT, "όμελτςιώεστχο", false);
    MAT_ACID   = Proto->AddItem(TYPE_INT, "λισμοτα", false);
    MAT_INCLUDE = Proto->AddItem(TYPE_INT, "λονποξεξτ", false);
    return CheckInit();
}

///////////////////////////////////////////////////////////////////////////////

