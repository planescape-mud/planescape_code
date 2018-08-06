#ifndef XQUESTS_H
#define XQUESTS_H
#include "parser.h"
#include "parser_const.h"
#include "parser_items.h"

/*****************************************************************************/
/* Определения для загрузки файлов квестов */
/*****************************************************************************/

class CQstSave : public CParser {
    public:
        CQstSave();
        ~CQstSave();
        bool Initialization(void);
};


extern int QUEST_QUEST;  //ЗАДАНИЕ
extern int QUEST_MOB_VNUM; //МОНСТР
extern int QUEST_NUMBER;  //НОМЕР
extern int QUEST_DONE_MOB; //МОНСТРЫ (STRLIST, аргумент внут, значение кол-во)
extern int QUEST_DONE_OBJ; //ПРЕДМЕТЫ(STRLIST, аргумент внут, значение кол-во)
extern int QUEST_DONE_FLW; //ПОСЛЕДОВАТЕЛИ (STRLIST, аргумент внут, значение кол-во)
extern int QUEST_VAR;  //ПЕРЕМЕННАЯ
extern int QUEST_VAR_NAME; //ПЕРЕМЕННАЯ.НАЗВАНИЕ
extern int QUEST_VAR_CURRENT; //ПЕРЕМЕННАЯ.ЗНАЧЕНИЕ

#endif
