
#include "xhelp.h"
#include "parser_id.h"

int HELP_NUMBER; //Номер справки
int HELP_TOPIC;
int HELP_TITLE;  //Заголовок (стринг)
int HELP_ALIAS;  //Синонимы (стринг)
int HELP_TYPE;  //Тип (лист)
int HELP_CONTENT; //Содержание (стринг)
int HELP_FORMAT; //Формат команды (стринг)
int HELP_LINKS;  //Линк (стринг)

///////////////////////////////////////////////////////////////////////////////
CHelp::CHelp() {
}
///////////////////////////////////////////////////////////////////////////////
CHelp::~CHelp() {
}
///////////////////////////////////////////////////////////////////////////////
bool CHelp::Initialization(void) {

    HELP_NUMBER = Proto->AddItem(TYPE_INT, "НОМЕР");
    HELP_TITLE  = Proto->AddItem(TYPE_STRING, "ЗАГОЛОВОК");
    HELP_TOPIC  = Proto->AddItem(TYPE_INT, "ТОПИК", false);
    HELP_ALIAS  = Proto->AddItem(TYPE_STRING, "СИНОНИМЫ");
    HELP_TYPE   = Proto->AddItem(TYPE_LIST, "ТИП", false, ParserConst.GetList(LIST_HELP_TYPES));
    HELP_CONTENT = Proto->AddItem(TYPE_STRING, "СОДЕРЖАНИЕ");
    HELP_FORMAT = Proto->AddItem(TYPE_STRING, "ФОРМАТ", false);
    HELP_LINKS  = Proto->AddItem(TYPE_STRING, "ССЫЛКИ", false);

    return CheckInit();
}


