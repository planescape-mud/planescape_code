#ifndef XSHOP_H
#define XSHOP_H
#include "parser.h"
#include "parser_const.h"


extern int SHOP_NUMBER; //םבחבתימ
extern int SHOP_MOB;  //םןמףפע
extern int SHOP_LIST;  //םומא

class CShop : public CParser {
    public:
        CShop();
        ~CShop();
        bool Initialization(void);
};

#endif
