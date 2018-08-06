/**************************************************************************
    νπν "ηÒΑΞΙ νΙÒΑ" (Σ) 2002-2003 αΞΔÒΕΚ εÒΝΙΫΙΞ
    ϊΑΗÒΥΪΛΑ ΖΑΚΜΟΧ ΙΗÒΟΧΟΗΟ ΝΙÒΑ
 **************************************************************************/

#include "xbody.h"
#include "parser_id.h"

// οΠÒΕΔΕΜΕΞΙΕ ΟΣΞΟΧΞΩΘ ΠΑÒΑΝΕΤÒΟΝ
int TBD_RACE;  //ςασα
int TBD_HEALTH;
int TBD_BODY;  //τεμο
int TBD_BODY_POSITION; //τεμο.ποϊιγιρ
int TBD_BODY_SNAME; //τεμο.σπςξαϊχαξιε
int TBD_BODY_NAME; //τεμο.ξαϊχαξιε
int TBD_BODY_CHANCE; //τεμο.σμοφξοστψ
int TBD_BODY_KDAM; //τεμο.πςοώξοστψ
int TBD_AC; //τεμο.βςοξρ
int TBD_ARM0; //τεμο.ϊςεφυύεε
int TBD_ARM1; //τεμο.ϊλομΰύεε
int TBD_ARM2; //τεμο.ϊυδαςξοε
int TBD_FIRE;
int TBD_COLD;
int TBD_ELECTRO;
int TBD_ACID;
int TBD_POISON;
int TBD_XAOS;
int TBD_ORDER;
int TBD_POSITIVE;
int TBD_NEGATIVE;

int TCL_CLASS;  //πςοζεσσιρ
int TCL_ADDSTR;  //δοβαχμεξιε σιμω
int TCL_ADDCON;
int TCL_ADDDEX;
int TCL_ADDINT;
int TCL_ADDWIS;
int TCL_ADDCHA;  //δοβαχμεξιε οβαρξιρ
int TCL_HEALTH;
int TCL_MANA;
int TCL_KFORT;
int TCL_KREFL;
int TCL_KWILL;
int TCL_HROLL;
int TCL_AC;
int TCL_ARM0;
int TCL_ARM1;
int TCL_ARM2;


int TBD_BODY_WEAPON; //οςυφιε
int TBD_BWP_NAME; //οςυφιε.ξαϊχαξιε
int TBD_BWP_SEX;  //οςυφιε.ςοδ


///////////////////////////////////////////////////////////////////////////////
CBody::CBody() {
}

CBody::~CBody() {
}

CProf::CProf() {
}

CProf::~CProf() {
}

///////////////////////////////////////////////////////////////////////////////

CBody tBody;
CProf tProf;

bool CBody::Initialization(void) {

    TBD_RACE = Proto->AddItem(TYPE_LIST, "ςασα", true, ParserConst.GetList(LIST_RACE));
    TBD_AC = Proto->AddItem(TYPE_FLOAT, "βςοξρ", false);
    TBD_ARM0 = Proto->AddItem(TYPE_FLOAT, "ϊςεφυύεε", false);
    TBD_ARM1 = Proto->AddItem(TYPE_FLOAT, "ϊλομΰύεε", false);
    TBD_ARM2 = Proto->AddItem(TYPE_FLOAT, "ϊυδαςξοε", false);
    TBD_FIRE = Proto->AddItem(TYPE_INT, "ϊοηοξψ", false);
    TBD_COLD = Proto->AddItem(TYPE_INT, "ϊθομοδ", false);
    TBD_ELECTRO = Proto->AddItem(TYPE_INT, "ϊόμελτςο", false);
    TBD_ACID = Proto->AddItem(TYPE_INT, "ϊλισμοτα", false);
    TBD_POISON = Proto->AddItem(TYPE_INT, "ϊρδ", false);
    TBD_XAOS = Proto->AddItem(TYPE_INT, "ϊθαοσ", false);
    TBD_ORDER = Proto->AddItem(TYPE_INT, "ϊποςρδολ", false);
    TBD_POSITIVE = Proto->AddItem(TYPE_INT, "ϊποϊιτιχ", false);
    TBD_NEGATIVE = Proto->AddItem(TYPE_INT, "ϊξεηατιχ", false);
    TBD_HEALTH = Proto->AddItem(TYPE_FLOAT, "ϊδοςοχψε", false);
    TBD_BODY = Proto->AddItem(TYPE_STRUCT, "τεμο", false);
    CItemProto *body = Proto->GetItem(TBD_BODY);
    TBD_BODY_POSITION = body->AddItem(TYPE_LIST, "ποϊιγιρ", true,
                                      ParserConst.GetList(LIST_BODY_POSITION));
    TBD_BODY_SNAME = body->AddItem(TYPE_LIST, "σπςξαϊχαξιε", false, ParserConst.GetList(LIST_BODY_NAME));
    TBD_BODY_NAME = body->AddItem(TYPE_STRING, "ξαϊχαξιε", false);
    TBD_BODY_CHANCE = body->AddItem(TYPE_INT, "σμοφξοστψ", true);
    TBD_BODY_KDAM = body->AddItem(TYPE_INT, "πςοώξοστψ", false);

    TBD_BODY_WEAPON = Proto->AddItem(TYPE_STRUCT, "οςυφιε", false);
    CItemProto *bwp = Proto->GetItem(TBD_BODY_WEAPON);
    TBD_BWP_NAME = bwp->AddItem(TYPE_STRING, "ξαϊχαξιε");
    TBD_BWP_SEX = bwp->AddItem(TYPE_LIST, "ςοδ", true, ParserConst.GetList(LIST_SEX));

    return CheckInit();
}


bool CProf::Initialization(void) {

    TCL_CLASS = Proto->AddItem(TYPE_LIST, "πςοζεσσιρ", true, ParserConst.GetList(LIST_CLASS));
    TCL_ADDSTR = Proto->AddItem(TYPE_FLOAT, "δσιμα");
    TCL_ADDCON = Proto->AddItem(TYPE_FLOAT, "δτεμο");
    TCL_ADDDEX = Proto->AddItem(TYPE_FLOAT, "δμοχλοστψ");
    TCL_ADDINT = Proto->AddItem(TYPE_FLOAT, "δςαϊυν");
    TCL_ADDWIS = Proto->AddItem(TYPE_FLOAT, "δνυδςοστψ");
    TCL_ADDCHA = Proto->AddItem(TYPE_FLOAT, "δοβαρξιε");
    TCL_HEALTH = Proto->AddItem(TYPE_FLOAT, "ϊδοςοχψε");
    TCL_MANA  = Proto->AddItem(TYPE_FLOAT, "ναξα", false);
    TCL_KFORT = Proto->AddItem(TYPE_INT, "λχωξοσ");
    TCL_KREFL = Proto->AddItem(TYPE_INT, "λςεζμ");
    TCL_KWILL = Proto->AddItem(TYPE_INT, "λχομρ");
    TCL_HROLL = Proto->AddItem(TYPE_FLOAT, "αταλα");
    TCL_AC  = Proto->AddItem(TYPE_FLOAT, "ϊαύιτα");
    TCL_ARM0 = Proto->AddItem(TYPE_FLOAT, "πςεφυύεε");
    TCL_ARM1 = Proto->AddItem(TYPE_FLOAT, "πλομΰύεε");
    TCL_ARM2 = Proto->AddItem(TYPE_FLOAT, "πυδαςξοε");

    return CheckInit();
}

