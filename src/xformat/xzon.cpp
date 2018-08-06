/**************************************************************************
    νπν "ηÒΑΞΙ νΙÒΑ" (Σ) 2002-2003 αΞΔÒΕΚ εÒΝΙΫΙΞ
    ϊΑΗÒΥΪΛΑ ΖΑΚΜΟΧ ΙΗÒΟΧΟΗΟ ΝΙÒΑ
 **************************************************************************/

#include "xzon.h"
#include "parser_id.h"

// οΠÒΕΔΕΜΕΞΙΕ ΟΣΞΟΧΞΩΘ ΠΑÒΑΝΕΤÒΟΝ
int ZON_NUMBER;  // ϊοξα
int ZON_NAME_MAJ;  // οβμαστψ
int ZON_NAME_MIN;  // ξαϊχαξιε
int ZON_AUTHOR;  // αχτος
int ZON_DESCRIPTION;  // οπισαξιε
int ZON_TYPE;   // τιπ
int ZON_TOP;   // πςεδεμ
int ZON_RESET_TYPE;  // σβςοσ
int ZON_RESET_TIME;  // φιϊξψ
int ZON_TIME;  // χςενρ
int ZON_RECALL;  // ςελομμ
int ZON_COMMET;  // λοννεξταςικ
int ZON_MESTO;  // νεστοπομοφεξιε
int ZON_MESTO_PLANE; // νεστοπομοφεξιε.ηςαξψ
int ZON_MESTO_WARD; // νεστοπομοφεξιε.ςακοξ
///////////////////////////////////////////////////////////////////////////////
CZon::CZon() {
}
CZon::~CZon() {
}
///////////////////////////////////////////////////////////////////////////////

/*****************************************************************************/
bool CZon::Initialization(void) {

    ZON_NUMBER   = Proto->AddItem(TYPE_INT, "ϊοξα");
    ZON_NAME_MAJ = Proto->AddItem(TYPE_STRING, "οβμαστψ", false);
    ZON_NAME_MIN = Proto->AddItem(TYPE_STRING, "ξαϊχαξιε");
    ZON_AUTHOR   = Proto->AddItem(TYPE_STRING, "αχτος");
    ZON_DESCRIPTION = Proto->AddItem(TYPE_STRING, "οπισαξιε");
    ZON_TYPE     = Proto->AddItem(TYPE_LIST, "τιπ", false,
                                  ParserConst.GetList(LIST_ZONE_TYPE));
    ZON_TOP      = Proto->AddItem(TYPE_INT, "πςεδεμ", false);
    ZON_RESET_TYPE = Proto->AddItem(TYPE_LIST, "σβςοσ", true, ParserConst.GetList(LIST_RESET_TYPES));
    ZON_RESET_TIME = Proto->AddItem(TYPE_INT, "φιϊξψ", false);
    ZON_TIME     = Proto->AddItem(TYPE_INT, "χςενρ", false);
    ZON_RECALL = Proto->AddItem(TYPE_INT, "χοϊχςατ", false);
    ZON_COMMET = Proto->AddItem(TYPE_STRING, "λοννεξταςικ", false);
    ZON_MESTO = Proto->AddItem(TYPE_STRUCT, "νεστοπομοφεξιε", false);
    CItemProto *mesto = Proto->GetItem(ZON_MESTO);
    ZON_MESTO_PLANE   = mesto->AddItem(TYPE_LIST, "ηςαξψ", false, ParserConst.GetList(LIST_PLANE_TYPES));
    ZON_MESTO_WARD    = mesto->AddItem(TYPE_LIST, "ςακοξ", false, ParserConst.GetList(LIST_WARD_TYPES));

    return CheckInit();
};

CZon Zon;

