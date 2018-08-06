
#include "parser_const.h"
#include "parser_utils.h"


CParserConst ParserConst;

#define CR_KEY "\x02\x5D\x91\xF4\xCA\x37\xE9"
#define CONST_NONE  0
#define CONST_VECTOR 1
#define CONST_LIST  2

///////////////////////////////////////////////////////////////////////////////
CParserConst::CParserConst() {
    VectorConst = NULL;
    ListConst = NULL;
    CurrentConst = CONST_NONE;
    last_vector = NULL;
    last_list = NULL;
}
///////////////////////////////////////////////////////////////////////////////
CParserConst::~CParserConst() {
}
///////////////////////////////////////////////////////////////////////////////
bool CParserConst::ReadConst(const char* name) {
    //log("Чтение файла конфигурации: %s", name);
    line = 0;
    std::ifstream inFile(name);
    if (!inFile) {
        log("Не могу открыть файл %s на чтение", name);
        return false;
    }
    char readbuf[500];
    while (1) {
        line ++;
        memset(readbuf, 0, sizeof(readbuf));
        if (!inFile.getline(readbuf, sizeof(readbuf), '\n')) {
            inFile.close();
            return true;
        }
        // Прочитали строку текста
        int i = 0;
        // Коментарии в пизду
        while (1) {
            char s = readbuf[i];
            if (s == 0)
                break;
            if (s == 0x0A || s == 0x0D || s == ';' || s == '#') {
                readbuf[i] = 0;
                break;
            }
            i ++;
        }
        if (strlen(readbuf) == 0)
            continue;
        if (!ParseLine(readbuf)) {
            log("Ошибка обработки файла конфигурации %s. Строка %d", name, line);
            return false;
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
bool CParserConst::ParseLine(char* buf) {
    // Коментарии уже убиты и строка не пустая
    // (Хотя бы пробелы там есть)
    std::istrstream in(buf);
    char par[200];
    *par = '\0';
    in >> par;
    if (strlen(par) == 0) {
        // Только пробелы там и были...
        return true;
    }
    if (strcmp(par, "[VECTOR]") == 0) {
        CurrentConst = CONST_VECTOR;
        return true;
    }
    if (strcmp(par, "[LIST]") == 0) {
        CurrentConst = CONST_LIST;
        return true;
    }
    if (strcmp(par, "ID") == 0) {
        // Начало нового парметра
        int id;
        in >> id;
        if (!NewParameter(id))
            return false;
        return true;
    }
    if (par[0] == '[')
        return false;
    char com[500];
    in >> com;
    return ParseLineConf(par, com);
}
///////////////////////////////////////////////////////////////////////////////
bool CParserConst::NewParameter(int id) {
    if (CurrentConst == CONST_VECTOR) {
        CVectorConst *tmp_vector = new CVectorConst;
        tmp_vector->ID = id;
        tmp_vector->Item = NULL;
        tmp_vector->last = NULL;
        tmp_vector->Next = NULL;
        if (last_vector == NULL) {
            VectorConst = last_vector = tmp_vector;
            return true;
        }
        last_vector->Next = tmp_vector;
        last_vector = tmp_vector;
        return true;
    }
    if (CurrentConst == CONST_LIST) {
        CListConst *tmp_list = new CListConst;
        tmp_list->ID = id;
        tmp_list->Item = NULL;
        tmp_list->last = NULL;
        tmp_list->Next = NULL;
        if (last_list == NULL) {
            ListConst = last_list = tmp_list;
            return true;
        }
        last_list->Next = tmp_list;
        last_list = tmp_list;
        return true;
    }
    // Тип входных данных не определен
    log("Ошибка чтения. Тип входных данных не определен.");
    log("Отсутствуют параметры [VECTOR] или [LIST]");
    return false;
}
///////////////////////////////////////////////////////////////////////////////
bool CParserConst::ParseLineConf(char* par, char* com) {
    if (CurrentConst != CONST_VECTOR && CurrentConst != CONST_LIST) {
        log("Ошибка чтения. Тип входных данных не определен.");
        log("Отсутствуют параметры [VECTOR] или [LIST]");
        return false;
    }
    if ((CurrentConst == CONST_VECTOR && last_vector == NULL) ||
            (CurrentConst == CONST_LIST && last_list == NULL)) {
        log("Ошибка чтения. ID записи не определен");
        return false;
    }
    if (CurrentConst == CONST_VECTOR) {
        CVectorConst_item* tmp_vitem = new CVectorConst_item;
        tmp_vitem->key = new char[strlen(par) + 1];
        strcpy(tmp_vitem->key, par);
        tmp_vitem->text = new char[strlen(com) + 1];
        strcpy(tmp_vitem->text, com);
        tmp_vitem->Next = NULL;
        if (last_vector->last == NULL) {
            last_vector->Item = last_vector->last = tmp_vitem;
            return true;
        }
        last_vector->last->Next = tmp_vitem;
        last_vector->last = tmp_vitem;
        return true;
    }
    // CONST_LIST
    CListConst_item* tmp_litem = new CListConst_item;
    tmp_litem->key = atoi(par);
    tmp_litem->text = new char[strlen(com) + 1];
    strcpy(tmp_litem->text, com);
    tmp_litem->Next = NULL;
    if (last_list->last == NULL) {
        last_list->Item = last_list->last = tmp_litem;
        return true;
    }
    last_list->last->Next = tmp_litem;
    last_list->last = tmp_litem;
    return true;
}
///////////////////////////////////////////////////////////////////////////////
void* CParserConst::GetList(int id) {
    CListConst *list = ListConst;
    while (list != NULL) {
        if (list->ID == id)
            return list->Item;
        list = list->Next;
    }
    return NULL;
}
///////////////////////////////////////////////////////////////////////////////
void* CParserConst::GetVector(int id) {
    CVectorConst *list = VectorConst;
    while (list != NULL) {
        if (list->ID == id)
            return list->Item;
        list = list->Next;
    }
    return NULL;
}
///////////////////////////////////////////////////////////////////////////////
void CParserConst::SetVector(int id, char* key, char* text) {
    CVectorConst* list = VectorConst;
    while (1) {
        if (list->ID == id)
            break;
        list = list->Next;
        if (list == NULL)
            return;
    }
    CVectorConst_item* tmp = new CVectorConst_item;
    tmp->key = new char[strlen(key) + 1];
    strcpy(tmp->key, key);
    tmp->text = new char[strlen(text) + 1];
    strcpy(tmp->text, text);
    tmp->Next = NULL;
    if (list->last == NULL) {
        list->Item = list->last = tmp;
        return;
    }
    list->last->Next = tmp;
    list->last = tmp;
}
///////////////////////////////////////////////////////////////////////////////
void CParserConst::SetList(int id, int key, char* text) {
    CListConst* list = ListConst;
    while (1) {
        if (list->ID == id)
            break;
        list = list->Next;
        if (list == NULL)
            return;
    }
    CListConst_item* tmp = new CListConst_item;
    tmp->key = key;
    tmp->text = new char[strlen(text) + 1];
    strcpy(tmp->text, text);
    tmp->Next = NULL;
    if (list->last == NULL) {
        list->Item = list->last = tmp;
        return;
    }
    list->last->Next = tmp;
    list->last = tmp;
}
///////////////////////////////////////////////////////////////////////////////
