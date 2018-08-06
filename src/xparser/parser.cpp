///////////////////////////////////////////////////////////////////////////////
//
//          УНИВЕРСАЛЬНАЯ БИБЛИОТЕКА ЧТЕНИЯ ФАЙЛОВ КОНФИГУРАЦИИ
//
// (С) 2003 Александр Ризов
///////////////////////////////////////////////////////////////////////////////

#include "parser.h"
#include "parser_utils.h"
#include "parser_file.h"
#include "parser_readcfg.h"

///////////////////////////////////////////////////////////////////////////////
CParser::CParser() : CItemStructList() {
    Proto = new CItemProtoStruct("", true);
}
///////////////////////////////////////////////////////////////////////////////
CParser::~CParser() {
    delete Proto;
}
///////////////////////////////////////////////////////////////////////////////

bool CParser::ReadConfig(const char *name) {
    CReadCfg Config;
    log("Чтение файла конфигурации (%s)...", name);
    if (!Config.StartRead(name))
        return false;
    return ReadData(&Config, name);
}

bool CParser::ReadData(void *szConfig, const char* name) {
    CReadCfg *Config = (CReadCfg*)szConfig;
    char com[10000];
    char par[100000];
    // Все остальное почти как было, за некоторыми исключениями
    // Все Config. меняем на Config->
    int ret = Config->GetCommand(com, par);  // ТУТ
    if (ret == FIELD_NONE) {
        log("ВНИМАНИЕ! Файл пуст!");
        return true;
    }
    if (ret == FIELD_ERROR) {
        log("ОШИБКА чтения файла конфигурации");
        return false;
    }
    if (ret != FIELD_CONST) {
        // ТУТ
        log("Ошибка в файле конфигурации [FIELD_CONST]. Строка %d", Config->GetLine());
        return false;
    }
    if (strcmp(com, Proto->GetItem(0)->GetData()->Name) != 0) {
        log("Первой командой должна быть: %s", Proto->GetItem(0)->GetData()->Name);
        // ТУТ
        log("Ошибка в файле конфигурации. Строка %d", Config->GetLine());
        return false;
    }
    bool end = false;
    while (1) {
        //log("Создается основной элемент: %s %s", com, par);
        NewSubItem(); // Новая копия структуры
        if (!(Item[Number-1]->AddParameter(ret, com, par))) {
            // ТУТ
            log("Ошибка в файле конфигурации []. Строка %d", Config->GetLine());
            return false;
        }

        while (1) {
            ret = Config->GetCommand(com, par);  // ТУТ
            if (ret == FIELD_NONE) {
                end = true;
                break;
            }
            if (ret == FIELD_ERROR) {
                // ТУТ
                log("Ошибка в файле конфигурации [FIELD_ERROR]. Строка %d", Config->GetLine());
                return false;
            }
            if (strcmp(com, Proto->GetItem(0)->GetData()->Name) == 0)
                break;
            //log("Обработка: %d %s %s", ret, com, par);
            if (!(Item[Number-1]->AddParameter(ret, com, par))) {
                // ТУТ
                log("Ошибка в файле конфигурации [1]. Строка %d", Config->GetLine());
                return false;
            }
        }
        if (!(Item[Number-1]->GetStatus())) {
            log("Ошибка в файле конфигурации. Запись не окончена. Строка %d", Config->GetLine());
            return false;
        }
        if (end)
            break;
    }
    log("Чтение завершено. Записей: %d", Number);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
bool CParser::CheckInit(void) {
    if (Proto == NULL) {
        log("ОШИБКА ИНИЦИАЛИЗАЦИИ! Не задано ниодного параметра.");
        return false;
    }
    return Proto->CheckInit();
}
///////////////////////////////////////////////////////////////////////////////
bool CParser::SaveAll(const char *name, bool clr) {
    CParserFile file;
    if (!file.Open(name, clr))
        return false;
    for (int i = 0; i < Number; i ++) {
        file.StartLevel();
        Item[i]->SaveItem(&file);
    }
    file.Close();
    return true;
}
///////////////////////////////////////////////////////////////////////////////
