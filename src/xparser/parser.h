#ifndef PARSER_H
#define PARSER_H

#include "parser_items.h"
#include "parser_types.h"

class CParser : public CItemStructList {
    protected:
        // Проверить правильность инициализации
        bool CheckInit(void);
        bool ReadData(void *Config, const char*);
    public:
        CParser();
        virtual ~CParser();
        virtual bool Initialization(void) = 0;
        // Чтение конфига
        bool ReadConfig(const char*);

        // Сохранить структуру в файл
        // Параметры:
        // 1 - имя файла
        // 2 - true   - обнулить файл и писать заново
        //     false  - продолжить писать в существующий файл
        //              (если файла такого нет, то он создается заново)
        bool SaveAll(const char*, bool);

};

#endif
