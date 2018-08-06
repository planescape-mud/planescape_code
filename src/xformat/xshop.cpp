#include "xshop.h"
#include "parser_id.h"

/* הÌÑ ÒÁÂÏÔÙ Ó ÆÁÊÌÁÍÉ */
int SHOP_NUMBER; //םבחבתימ
int SHOP_MOB;  //םןמףפע
int SHOP_LIST;  //םומא



CShop::CShop() {
}

CShop::~CShop() {
}

///////////////////////////////////////////////////////////////////////////////

bool CShop::Initialization(void) {

    SHOP_NUMBER =  Proto->AddItem(TYPE_INT, "םבחבתימ");
    SHOP_MOB =   Proto->AddItem(TYPE_INT, "םןמףפע");
    SHOP_LIST =  Proto->AddItem(TYPE_STRLIST, "םומא");

    return CheckInit();
}

