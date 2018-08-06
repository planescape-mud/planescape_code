#ifndef XHELP_H
#define XHELP_H

#include "parser.h"
#include "parser_const.h"

extern int HELP_NUMBER; //Номер справки
extern int HELP_TOPIC;
extern int HELP_TITLE;  //Заголовок (стринг)
extern int HELP_ALIAS;  //Синонимы (стринг)
extern int HELP_TYPE;  //Тип (лист)
extern int HELP_CONTENT; //Содержание (стринг)
extern int HELP_FORMAT; //Формат команды (стринг)
extern int HELP_LINKS;  //Линк (стринг)

class CHelp : public CParser {
    public:
        CHelp();
        ~CHelp();
        bool Initialization(void);
};

#endif
