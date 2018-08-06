/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/

#include "xsave.h"
#include "xspells.h"
#include "xobj.h"
#include "parser_id.h"

int RENT_NUMBER;
int RENT_NAME;
int RENT_TYPE;
int RENT_TIME;
int RENT_ROOM;
int RENT_QUALITY;

int PET_NUMBER; //מןםוע
int PET_GUID; //חלןגיה
int PET_NAME;   //יםס
int PET_RACE;   //עבףב
int PET_TYPE;
int PET_CLASS;
int PET_CLASS_TYPE; //נעןזוףףיס.כלבףף
int PET_CLASS_LEVEL; //נעןזוףףיס.ץעןקומר
int PET_HIT;  //פציתמר
int PET_MOVE;  //פגןהעןףפר
int PET_MANA;  //פםבמב
int PET_SPELL; //תבכלימבמיו
int PET_TIME; //קעוםס
int PET_EXP; //ןנשפ
int PET_LIKES; //עבגןפב
int PET_SKILLS; //ץםומיס

int VARS_NUMBER;  //נועוםוממבס
int VARS_NAME;  //מבתקבמיו
int VARS_VALUE;  //תמב‏ומיו
int VARS_TIME;  //קעוםס
///////////////////////////////////////////////////////////////////////////////
CRent::CRent() {
}

CRent::~CRent() {
}

///////////////////////////////////////////////////////////////////////////////

bool CRent::Initialization(void) {
    RENT_NUMBER = Proto->AddItem(TYPE_INT, "מןםוע");
    RENT_NAME = Proto->AddItem(TYPE_STRING, "יםס");
    RENT_TYPE = Proto->AddItem(TYPE_LIST, "פינ", true, ParserConst.GetList(LIST_RENT_TYPES));
    RENT_ROOM      = Proto->AddItem(TYPE_INT, "חןףפימידב");
    RENT_QUALITY   = Proto->AddItem(TYPE_INT, "כב‏וףפקן");
    RENT_TIME      = Proto->AddItem(TYPE_INT, "קעוםס");

    return CheckInit();
}


///////////////////////////////////////////////////////////////////////////////
CPet::CPet() {
}

CPet::~CPet() {
}

///////////////////////////////////////////////////////////////////////////////
bool CPet::Initialization(void) {

    PET_NUMBER = Proto->AddItem(TYPE_INT, "מןםוע", true);
    PET_GUID = Proto->AddItem(TYPE_LONGLONG, "חלןגיה", false);
    PET_NAME = Proto->AddItem(TYPE_STRING, "יםס", true);
    PET_RACE = Proto->AddItem(TYPE_LIST, "עבףב", true, ParserConst.GetList(LIST_RACE));
    PET_CLASS = Proto->AddItem(TYPE_STRUCT, "כלבףף", false);
    PET_TYPE = Proto->AddItem(TYPE_INT, "פינ", true);
    PET_SPELL = Proto->AddItem(TYPE_LIST, "תבכלימבמיו", true, ParserConst.GetList(LIST_SPELLS));
    PET_TIME = Proto->AddItem(TYPE_INT, "קעוםס", true);
    PET_EXP = Proto->AddItem(TYPE_INT, "ןנשפ", true);
    CItemProto *classes = Proto->GetItem(PET_CLASS);
    PET_CLASS_TYPE = classes->AddItem(TYPE_LIST, "פינ", true, ParserConst.GetList(LIST_CLASS));
    PET_CLASS_LEVEL = classes->AddItem(TYPE_INT, "ץעןקומר", true);

    PET_HIT = Proto->AddItem(TYPE_INT, "פציתמר", false);
    PET_MOVE = Proto->AddItem(TYPE_INT, "פגןהעןףפר", false);
    PET_MANA = Proto->AddItem(TYPE_INT, "פםבמב", false);
    PET_LIKES = Proto->AddItem(TYPE_INT, "עבגןפב", false);
    PET_SKILLS = Proto->AddItem(TYPE_STRLIST, "ץםומיס", false, ParserConst.GetList(LIST_SKILLS));

    return CheckInit();
}

///////////////////////////////////////////////////////////////////////////////
CVarSave::CVarSave() {
}

CVarSave::~CVarSave() {
}

///////////////////////////////////////////////////////////////////////////////

bool CVarSave::Initialization(void) {
    VARS_NUMBER = Proto->AddItem(TYPE_INT, "נועוםוממבס");
    VARS_NAME = Proto->AddItem(TYPE_STRING, "מבתקבמיו");
    VARS_VALUE = Proto->AddItem(TYPE_STRING, "תמב‏ומיו");
    VARS_TIME = Proto->AddItem(TYPE_INT, "קעוםס");

    return CheckInit();
}

