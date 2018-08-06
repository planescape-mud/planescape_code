
#include "parser_types.h"
#include "parser_utils.h"
#include "parser_items.h"
#include "parser_const.h"
#include "parser_readcfg.h"
#include "strlib.h"
#include "expr.h"

///////////////////////////////////////////////////////////////////////////////
bool CheckTypes(int TYPE_CH, int PARAM_CH) {
    if (PARAM_CH == PARAM_INT) {
        switch (TYPE_CH) {
            case TYPE_INT:
                return true;
            case TYPE_LIST:
                return true;
        }
        return false;
    }
    if (PARAM_CH == PARAM_LONGLONG) {
        if (TYPE_CH == TYPE_LONGLONG)
            return true;
        return false;
    }
    if (PARAM_CH == PARAM_FLOAT) {
        if (TYPE_CH == TYPE_FLOAT)
            return true;
        return false;
    }
    if (PARAM_CH == PARAM_STRING) {
        switch (TYPE_CH) {
            case TYPE_STRING:
                return true;
            case TYPE_VECTOR:
                return true;
            case TYPE_RANDOM:
                return true;
            case TYPE_EXPR:
                return true;
        }
        return false;
    }
    return false;
}
///////////////////////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
CItemProto::CItemProto(int tp, const char *name, bool imp, void *list) {
    Data.Type = tp;
    Data.Name = new char[strlen(name) + 1];
    strcpy(Data.Name, name);
    Data.Important = imp;
    Data.List = list;
}
///////////////////////////////////////////////////////////////////////////////
CItemProto::~CItemProto() {
    delete[] Data.Name;
}
///////////////////////////////////////////////////////////////////////////////
struct CProto* CItemProto::GetData(void) {
    return &Data;
}
///////////////////////////////////////////////////////////////////////////////
bool CItemProto::CheckInit(void) {
    if ((Data.Type == TYPE_LIST || Data.Type == TYPE_VECTOR)
            && Data.List == NULL) {
        log("ОШИБКА ИНИЦИАЛИЗАЦИИ! Не задан диапазон допустимых значений для %s",
            Data.Name);
        return false;
    }
    return true;
}
///////////////////////////////////////////////////////////////////////////////
int CItemProto::AddItem(int tp, const char* name) {
    return AddItem(tp, name, true);
}
///////////////////////////////////////////////////////////////////////////////
int CItemProto::AddItem(int tp, const char* name, bool imp) {
    return AddItem(tp, name, imp, NULL);
}
///////////////////////////////////////////////////////////////////////////////
int CItemProto::AddItem(int tp, const char* name, bool imp, void* list) {
    log("ОШИБКА! Попытка обращения к прототипу простого элемента как к структуре");
    return -1;
}
///////////////////////////////////////////////////////////////////////////////
CItemProto* CItemProto::GetItem(int) {
    log("ОШИБКА! Попытка обращения к прототипу простого элемента как к структуре");
    return NULL;
}
///////////////////////////////////////////////////////////////////////////////
// PROTO_STRUCT
CItemProtoStruct::CItemProtoStruct(const char *name, bool imp) :
        CItemProto(TYPE_STRUCT, name, imp, NULL) {
    Number = 0;
    ItemProto = NULL;
}
///////////////////////////////////////////////////////////////////////////////
CItemProtoStruct::~CItemProtoStruct() {
    if (ItemProto != NULL) {
        for (int i = 0; i < Number; i ++)
            if (ItemProto[i] != NULL)
                delete ItemProto[i];
        free(ItemProto);
    }
}
///////////////////////////////////////////////////////////////////////////////
bool CItemProtoStruct::CheckInit(void) {
    if (Number == 0 || ItemProto == NULL) {
        log("ОШИБКА ИНИЦИАЛИЗАЦИИ! Не задано ниодного параметра для %s",
            GetData()->Name);
        return false;
    }
    for (int i = 0; i < Number; i ++)
        if (!ItemProto[i]->CheckInit())
            return false;
    return true;
}
///////////////////////////////////////////////////////////////////////////////
int CItemProtoStruct::AddItem(int tp, const char *name, bool imp, void *list) {
    if (ItemProto == NULL)
        ItemProto = (CItemProto**)malloc(sizeof(CItemProto*));
    else
        ItemProto = (CItemProto**)realloc(ItemProto, sizeof(CItemProto*) *
                                          (Number + 1));
    if (tp != TYPE_STRUCT)
        ItemProto[Number] = new CItemProto(tp, name, imp, list);
    else
        ItemProto[Number] = new CItemProtoStruct(name, imp);
    Number ++;
    //log("Добавлен элемент в структуру: %s", name);
    return Number - 1;
}
///////////////////////////////////////////////////////////////////////////////
CItemProto** CItemProtoStruct::GetAllItems(int &num) {
    num = Number;
    return ItemProto;
}
///////////////////////////////////////////////////////////////////////////////
CItemProto* CItemProtoStruct::GetItem(int num) {
    return ItemProto[num];
}
///////////////////////////////////////////////////////////////////////////////






///////////////////////////////////////////////////////////////////////////////
CItem::CItem(int tp, const char* name, bool imp, void* list) {
    Type = tp;
    Name = name;
    Important = imp;
    List = list;
}
///////////////////////////////////////////////////////////////////////////////
CItem::~CItem() {
}
///////////////////////////////////////////////////////////////////////////////
bool CItem::GetImportant(void) {
    return Important;
}
///////////////////////////////////////////////////////////////////////////////
void* CItem::GetList(void) {
    return List;
}
///////////////////////////////////////////////////////////////////////////////
const char* CItem::GetName(void) {
    return Name;
}
///////////////////////////////////////////////////////////////////////////////
int CItem::GetType(void) {
    return Type;
}
///////////////////////////////////////////////////////////////////////////////
int CItem::GetNumberItem(void) {
    log("ОШИБКА! (CItem CNI) Несоответствие типов при обращении к переменной %s", GetName());
    return 0;
}
///////////////////////////////////////////////////////////////////////////////
CItem* CItem::GetItem(int) {
    log("ОШИБКА! (CItem GetItem) Несоответствие типов при обращении к переменной %s", GetName());
    return NULL;
}
///////////////////////////////////////////////////////////////////////////////
int CItem::GetInt(void) {
    log("ОШИБКА! (int) Несоответствие типов при обращении к переменной %s", GetName());
    return 0;
}
///////////////////////////////////////////////////////////////////////////////
long long CItem::GetLongLong(void) {
    log("ОШИБКА! (long long) Несоответствие типов при обращении к переменной %s", GetName());
    return 0;
}
///////////////////////////////////////////////////////////////////////////////
float CItem::GetFloat(void) {
    log("ОШИБКА! (float) Несоответствие типов при обращении к переменной %s", GetName());
    return 0;
}
///////////////////////////////////////////////////////////////////////////////
void* CItem::GetExpression() {
    log("ОШИБКА! (expression) Несоответсвие типов при обращении к переменной %s", GetName());
    return NULL;
}

char* CItem::GetString(void) {
    log("ОШИБКА! (char *) Несоответствие типов при обращении к переменной %s", GetName());
    return NULL;
}
///////////////////////////////////////////////////////////////////////////////
int* CItem::GetScript(int&) {
    log("ОШИБКА! (int *) Несоответствие типов при обращении к переменной %s", GetName());
    return NULL;
}
///////////////////////////////////////////////////////////////////////////////
int CItem::GetStrListNumber(void) {
    log("ОШИБКА! (CItem GSLN) Несоответствие типов при обращении к переменной %s", GetName());
    return 0;
}
///////////////////////////////////////////////////////////////////////////////
void CItem::GetStrList(int n, int& p, int& c) {
    log("ОШИБКА! (void GSL) Несоответствие типов при обращении к переменной %s", GetName());
    p = c = 0;
}
///////////////////////////////////////////////////////////////////////////////
void CItem::SetParam(int) {
    log("ОШИБКА! Несоответствие типов при обращении к переменной %s", GetName());
}
///////////////////////////////////////////////////////////////////////////////
void CItem::SetParam(float) {
    log("ОШИБКА! Несоответствие типов при обращении к переменной %s", GetName());
}
///////////////////////////////////////////////////////////////////////////////
void CItem::SetParam(long long) {
    log("ОШИБКА! Несоответствие типов при обращении к переменной %s", GetName());
}
///////////////////////////////////////////////////////////////////////////////
void CItem::SetParam(const char*) {
    log("ОШИБКА! Несоответствие типов при обращении к переменной %s", GetName());
}
///////////////////////////////////////////////////////////////////////////////
void CItem::SetParam(int, int*) {
    log("ОШИБКА! Несоответствие типов при обращении к переменной %s", GetName());
}
///////////////////////////////////////////////////////////////////////////////
void CItem::SetParam(int, int, int) {
    log("ОШИБКА! Несоответствие типов при обращении к переменной %s", GetName());
}
///////////////////////////////////////////////////////////////////////////////
void CItem::AddParam(int, int) {
    log("ОШИБКА! Несоответствие типов при обращении к переменной %s", GetName());
}
///////////////////////////////////////////////////////////////////////////////
void CItem::AddParam(int) {
    log("ОШИБКА! Несоответствие типов при обращении к переменной %s", GetName());
}
///////////////////////////////////////////////////////////////////////////////
int CItem::NewSubItem(void) {
    log("ОШИБКА! Несоответствие типов при обращении к переменной %s", GetName());
    return 0;
}
///////////////////////////////////////////////////////////////////////////////
bool CItem::DeleteSubItem(int) {
    log("ОШИБКА! Несоответствие типов при обращении к переменной %s", GetName());
    return false;
}
///////////////////////////////////////////////////////////////////////////////
CItem* CItem::FindItem(int par) {
    log("ОШИБКА! Несоответствие типов при вызове FindItem");
    return NULL;
}
///////////////////////////////////////////////////////////////////////////////
CItem* CItem::FindItem(float par) {
    log("ОШИБКА! Несоответствие типов при вызове FindItem");
    return NULL;
}
///////////////////////////////////////////////////////////////////////////////
CItem* CItem::FindItem(char* par) {
    log("ОШИБКА! Несоответствие типов при вызове FindItem");
    return NULL;
}
///////////////////////////////////////////////////////////////////////////////
void CItem::ReleaseExpression(void*) {
    log("ОШИБКА! Несоответствие типов при вызове ReleaseExpression");
}
///////////////////////////////////////////////////////////////////////////////








///////////////////////////////////////////////////////////////////////////////
// INT
///////////////////////////////////////////////////////////////////////////////
CItemInt::CItemInt(int tp, char* name, bool imp, void *list) :
        CItem(tp, name, imp, list) {
    Parameter = 0;
    Status = false;
}
///////////////////////////////////////////////////////////////////////////////
CItemInt::~CItemInt() {
}
///////////////////////////////////////////////////////////////////////////////
bool CItemInt::SetParameter(char *param) {
    if (Status) {
        log("ОШИБКА! Параметр %s задан повторно.", GetName());
        return false;
    }
    for (int i = 0; param[i] != 0; i ++)
        if ((param[i] < '0' || param[i] > '9') && param[i] != '-') {
            log("ОШИБКА! Параметр %s может содержать только цифры", GetName());
            return false;
        }
    Parameter = atoi(param);
    Status = true;
    return true;
}
///////////////////////////////////////////////////////////////////////////////
void CItemInt::SetParam(int par) {
    Status = true;
    Parameter = par;
}
///////////////////////////////////////////////////////////////////////////////
bool CItemInt::GetStatus(void) {
    if (!GetImportant())
        return true;
    if (Status)
        return true;
    return false;
}
///////////////////////////////////////////////////////////////////////////////
int CItemInt::GetInt(void) {
    if (!Status)
        return 0;
    return Parameter;
}
///////////////////////////////////////////////////////////////////////////////
void CItemInt::SaveItem(CParserFile *file) {
    if (!Status)
        return;

    file->AddCommand("%s %d", GetName(), Parameter);
}
///////////////////////////////////////////////////////////////////////////////
// LONGLONG 
///////////////////////////////////////////////////////////////////////////////
CItemLongLong::CItemLongLong(char* name, bool imp) :
        CItem(TYPE_LONGLONG, name, imp, NULL) {
    Parameter = 0;
    Status = false;
}
///////////////////////////////////////////////////////////////////////////////
CItemLongLong::~CItemLongLong() {
}
///////////////////////////////////////////////////////////////////////////////
bool CItemLongLong::SetParameter(char *param) {
    if (Status) {
        log("ОШИБКА! Параметр %s задан повторно.", GetName());
        return false;
    }
    for (int i = 0; param[i] != 0; i ++)
        if ((param[i] < '0' || param[i] > '9') && param[i] != '-') {
            log("ОШИБКА! Параметр %s может содержать только цифры", GetName());
            return false;
        }
    Parameter = atoll(param);
    Status = true;
    return true;
}
///////////////////////////////////////////////////////////////////////////////
void CItemLongLong::SetParam(long long par) {
    Status = true;
    Parameter = par;
}
///////////////////////////////////////////////////////////////////////////////
bool CItemLongLong::GetStatus(void) {
    if (!GetImportant())
        return true;
    if (Status)
        return true;
    return false;
}
///////////////////////////////////////////////////////////////////////////////
long long CItemLongLong::GetLongLong(void) {
    if (!Status)
        return 0;
    return Parameter;
}
///////////////////////////////////////////////////////////////////////////////
void CItemLongLong::SaveItem(CParserFile *file) {
    if (!Status)
        return;

    file->AddCommand("%s %lld", GetName(), Parameter);
}
///////////////////////////////////////////////////////////////////////////////
// FLOAT
///////////////////////////////////////////////////////////////////////////////
CItemFloat::CItemFloat(char *name, bool imp) : CItem(TYPE_FLOAT, name, imp, NULL) {
    Parameter = 0;
    Status = false;
}
///////////////////////////////////////////////////////////////////////////////
CItemFloat::~CItemFloat() {
}
///////////////////////////////////////////////////////////////////////////////
bool CItemFloat::SetParameter(char *param) {
    if (Status) {
        log("ОШИБКА! Параметр %s задан повторно.", GetName());
        return false;
    }
    for (int i = 0; param[i] != 0; i ++)
        if ((param[i] < '0' || param[i] > '9') && param[i] != '.' &&
                param[i] != '-') {
            log("ОШИБКА! Параметр %s не является числовым.", GetName());
            return false;
        }
    Parameter = (float)atof(param);
    Status = true;
    return true;
}
///////////////////////////////////////////////////////////////////////////////
bool CItemFloat::GetStatus(void) {
    if (!GetImportant())
        return true;
    if (Status)
        return true;
    return false;
}
///////////////////////////////////////////////////////////////////////////////
void CItemFloat::SetParam(float par) {
    Status = true;
    Parameter = par;
}
///////////////////////////////////////////////////////////////////////////////
float CItemFloat::GetFloat(void) {
    if (!Status)
        return 0;
    return Parameter;
}
///////////////////////////////////////////////////////////////////////////////
void CItemFloat::SaveItem(CParserFile *file) {
    if (!Status)
        return;
    file->AddCommand("%s %f", GetName(), Parameter);
}
///////////////////////////////////////////////////////////////////////////////
// STRING
///////////////////////////////////////////////////////////////////////////////
CItemString::CItemString(int tp, char *name, bool imp, void *list) :
        CItem(tp, name, imp, list) {
    Parameter = NULL;
}
///////////////////////////////////////////////////////////////////////////////
CItemString::~CItemString() {
    if (Parameter != NULL)
        delete[] Parameter;
}
///////////////////////////////////////////////////////////////////////////////
bool CItemString::SetParameter(char *param) {
    if (Parameter != NULL) {
        log("ОШИБКА! Параметр %s задан повторно.", GetName());
        return false;
    }
    Parameter = new char[strlen(param) + 1];
    if (Parameter == NULL) {
        log("ОШИБКА выделения памяти!");
        log("Не могу записать параметр %s", GetName());
        return false;
    }
    strcpy(Parameter, param);
    return true;
}
///////////////////////////////////////////////////////////////////////////////
void CItemString::SetParam(const char *par) {
    if (Parameter != NULL)
        delete Parameter;
    Parameter = new char[strlen(par) + 1];
    strcpy(Parameter, par);
}
///////////////////////////////////////////////////////////////////////////////
bool CItemString::GetStatus(void) {
    if (!GetImportant())
        return true;
    if (Parameter == NULL)
        return false;
    return true;
}
///////////////////////////////////////////////////////////////////////////////
char* CItemString::GetString(void) {
    return Parameter;
}
///////////////////////////////////////////////////////////////////////////////
char* CItemString::MakeForSave(void) {
    int sz = strlen(Parameter) + 1;
    int i;
    for (i = 0; Parameter[i] != 0; i ++)
        if (Parameter[i] == '"' || Parameter[i] == CR)
            sz ++;
    char *ret = new char[sz];
    memset(ret, 0, sz);
    int out = 0;
    for (i = 0; Parameter[i] != 0; i ++) {
        char s = Parameter[i];
        if (s == '"') {
            ret[out] = '\\';
            out ++;
            ret[out] = '"';
            out ++;
            continue;
        }
        if (s == CR) {
            ret[out] = '\\';
            out ++;
            ret[out] = 'n';
            out ++;
            continue;
        }
        if (s == 0x0D)
            continue;
        ret[out] = s;
        out ++;
    }
    return ret;
}
///////////////////////////////////////////////////////////////////////////////
void CItemString::SaveItem(CParserFile *file) {
    if (Parameter == NULL)
        return;
    if (GetType() == TYPE_RANDOM) {
        file->AddCommand("%s %s", GetName(), Parameter);
        return;
    }
    char *svpar = MakeForSave();
    file->AddString(GetName(), svpar);
    delete[] svpar;
}
///////////////////////////////////////////////////////////////////////////////
// EXPRESSION
///////////////////////////////////////////////////////////////////////////////
CItemExpr::CItemExpr(char* name) : CItemString(TYPE_EXPR, name, false, NULL) {
    Expression = new CExpression;
}

CItemExpr::~CItemExpr() {
    if (Expression != NULL)
        delete(CExpression*)Expression;
}

bool CItemExpr::SetParameter(char* par) {
    if (!CItemString::SetParameter(par))
        return false;
    if (Expression == NULL) {
        log("ОШИБКА! Параметр %s задан повторно.", GetName());
        return false;
    }
    if (!((CExpression*)Expression)->SetExpression(Parameter)) {
        log("ОШИБКА! (%s) Ошибка обработки выражения %d.", GetName(),
            ((CExpression*)Expression)->GetErrCode());
        return false;
    }
    return true;
}

void CItemExpr::ReleaseExpression(void* expr) {
    ((CExpression*)Expression)->ReleaseExpr((CExpression*)expr);
}

void* CItemExpr::GetExpression() {
    return Expression;
}

///////////////////////////////////////////////////////////////////////////////
// LIST
///////////////////////////////////////////////////////////////////////////////
CItemList::CItemList(char *name, bool imp, void *lst)
        : CItemInt(TYPE_LIST, name, imp, lst) {
}
///////////////////////////////////////////////////////////////////////////////
CItemList::~CItemList() {
}
///////////////////////////////////////////////////////////////////////////////
bool CItemList::SetParameter(char *param) {
    if (Status) {
        log("ОШИБКА! Параметр %s задан повторно.", GetName());
        return false;
    }
    int i = 0;
    int ret;
    CListConst_item *list = (CListConst_item*)GetList();
    CListConst_item *tmp;
    while (1) { // Проверка на то, что запись была сделана числом
        if (param[i] == 0) {
            ret = atoi(param);
            tmp = list;
            while (tmp != NULL) {
                if (tmp->key == ret) {
                    Status = true;
                    Parameter = ret;
                    return true;
                }
                tmp = tmp->Next;
            }
            log("ОШИБКА! Неизвестный параметр поля %s", GetName());
            return false;
        }
        if (param[i] < '0' || param[i] > '9')
            break;
        i ++;
    }
    // Запись константой
    tmp = list;
    while (tmp != NULL) {
        if (strcmp(tmp->text, param) == 0) {
            Parameter = tmp->key;
            Status = true;
            return true;
        }
        tmp = tmp->Next;
    }

    //ВСТАВКА ДЛЯ ОБСЛУЖИВАНИЯ СТАРОГО СПРАВОЧНИКА
    if (strcmp(param, "УДАР") == 0) {
        tmp = list->Next;
        if (tmp != NULL && strcmp(tmp->text, "РЕЖУЩЕЕ") == 0) {
            Status = true;
            Parameter = tmp->key;
            return true;
        }
    }

    log("ОШИБКА! Неизвестный параметр поля %s", GetName());
    return false;
}
///////////////////////////////////////////////////////////////////////////////
void CItemList::SaveItem(CParserFile *file) {
    if (!Status)
        return;
    CListConst_item *list = (CListConst_item*)GetList();
    while (list != NULL) {
        if (list->key == Parameter) {
            file->AddCommand("%s %s", GetName(), list->text);
            return;
        }
        list = list->Next;
    }
    if (!Status)
        return;
    if (Parameter == 0)
        return;

    // Если попали сюда, то хуево. Значит где-то в программе ошибка
    ALARM;
}
///////////////////////////////////////////////////////////////////////////////
// VECTOR
///////////////////////////////////////////////////////////////////////////////
CItemVector::CItemVector(char *name, bool imp, void *list)
        : CItemString(TYPE_VECTOR, name, imp, list) {
}
///////////////////////////////////////////////////////////////////////////////
CItemVector::~CItemVector() {
}
///////////////////////////////////////////////////////////////////////////////
bool CItemVector::SetParameter(char *param) {
    if (!CItemString::SetParameter(param))
        return false;
    // Разбор вектора
    Vptr = 0;
    char *tmp = new char[strlen(Parameter) * 2 + 1];
    tmp[0] = 0;
    int ret;
    while ((ret = GetWord(&tmp[strlen(tmp)])) > 0)
        ;
    if (ret < 0) {
        log("ОШИБКА! Неизвестный параметр поля %s", GetName());
        delete[] tmp;
        return false;
    }
    delete[] Parameter;
    Parameter = new char[strlen(tmp) + 1];
    strcpy(Parameter, tmp);
    delete[] tmp;
    return true;
}
///////////////////////////////////////////////////////////////////////////////
int CItemVector::GetWord(char* ret) {
    CVectorConst_item *vector = (CVectorConst_item*)GetList();
    int pos = Vptr;
    while (1) {
        if (Parameter[pos] == 0)
            break;
        if (Parameter[pos] == '+')
            break;
        pos ++;
    }
    if (pos == Vptr) {
        if (Parameter[pos] == 0)
            return 0;
        return -1;
    }
    memcpy(ret, &Parameter[Vptr], pos - Vptr);
    ret[pos - Vptr] = 0;
    if (Parameter[pos] != 0)
        pos ++;
    Vptr = pos;
    while (vector != NULL) {
        if (strcmp(vector->key, ret) == 0 || strcmp(vector->text, ret) == 0) {
            strcpy(ret, vector->key);
            return 1;
        }
        vector = vector->Next;
    }
    return -1;
}
///////////////////////////////////////////////////////////////////////////////
void CItemVector::SaveItem(CParserFile *file) {
    CVectorConst_item *vector = (CVectorConst_item*)GetList();
    if (Parameter == NULL)
        return;
    char buf[1000];
    bool first = true;
    strcpy(buf, GetName());
    strcat(buf, " ");
    for (int i = 0; i < (int)strlen(Parameter); i += 2) {
        bool debug = true;
        CVectorConst_item* tmp = vector;
        while (tmp != NULL) {
            //log("ключ %s поле %s ищем '%s'",tmp->key,tmp->text,&Parameter[i]);
            if (strncmp(tmp->key, &Parameter[i], 2) == 0) {
                //log("поле найдено");
                if (!first)
                    strcat(buf, "+");
                first = false;
                strcat(buf, tmp->text);
                debug = false;
                break;
            }
            tmp = tmp->Next;
        }
        if (debug) {
            // Если попали сюда, то хуево. Значит где-то в программе ошибка
            log("ВНИМАНИЕ! Ошибка в программе - PARSER_VECTOR01.");
            log("[%s]Parameter %s buf %s", GetName(), Parameter, buf);
            //log("Exit блин!");
            exit(11);
        }
    }
    file->AddCommand(buf);
}
///////////////////////////////////////////////////////////////////////////////
// SCRIPT
///////////////////////////////////////////////////////////////////////////////
CItemScript::CItemScript(char *name, bool imp) : CItem(TYPE_SCRIPT, name, imp, NULL) {
    Script = NULL;
    Number = 0;
}
///////////////////////////////////////////////////////////////////////////////
CItemScript::~CItemScript() {
    if (Script != NULL)
        free(Script);
}
///////////////////////////////////////////////////////////////////////////////
bool CItemScript::GetStatus(void) {
    if (!GetImportant())
        return true;
    if (Script == NULL)
        return false;
    return true;
}
///////////////////////////////////////////////////////////////////////////////
int* CItemScript::GetScript(int& num) {
    num = Number;
    return Script;
}
///////////////////////////////////////////////////////////////////////////////
bool CItemScript::SetParameter(char *param) {
    int ptr = 0;
    int st = 0;
    bool end = false;
    for (int i = 0; param[i] != 0; i ++)
        if ((param[i] < '0' || param[i] > '9') && param[i] != '+') {
            log("ОШИБКА при задании параметра %s", GetName());
            return false;
        }
    while (1) {
        while (param[ptr] != '+' && param[ptr] != 0)
            ptr ++;
        if (ptr == st) {
            log("ОШИБКА при задании параметра %s", GetName());
            return false;
        }
        if (param[ptr] == 0)
            end = true;
        param[ptr] = 0;
        ptr ++;
        int sc = atoi(&param[st]);
        if (Script == NULL) {
            Script = (int*)malloc(sizeof(int));
            *Script = sc;
            Number = 1;
        } else {
            Script = (int*)realloc(Script, (Number + 1) * sizeof(int));
            Script[Number] = sc;
            Number ++;
        }
        st = ptr;
        if (end)
            break;
    }
    return true;
}
///////////////////////////////////////////////////////////////////////////////
void CItemScript::SetParam(int numsc, int *sc) {
    if (Script != NULL)
        delete Script;
    Script = (int*)malloc(sizeof(int) * numsc);
    memcpy(Script, sc, sizeof(int) * numsc);
    Number = numsc;
}
///////////////////////////////////////////////////////////////////////////////
void CItemScript::AddParam(int sc) {
    if (Script == NULL)
        Script = (int*)malloc(sizeof(int));
    else
        Script = (int*)realloc(Script, sizeof(int) * (Number + 1));
    Script[Number] = sc;
    Number ++;
}
///////////////////////////////////////////////////////////////////////////////
void CItemScript::SaveItem(CParserFile *file) {
    if (Number == 0)
        return;
    file->AddScripts(GetName(), Script, Number);
}
///////////////////////////////////////////////////////////////////////////////
// STRLIST
///////////////////////////////////////////////////////////////////////////////
CItemStrList::CItemStrList(char *name, void *list) :
        CItem(TYPE_STRLIST, name, false, list) {
    Number = 0;
    Parameters = NULL;
}
///////////////////////////////////////////////////////////////////////////////
CItemStrList::~CItemStrList() {
    if (Parameters != NULL)
        free(Parameters);
}
///////////////////////////////////////////////////////////////////////////////
bool CItemStrList::GetStatus(void) {
    return true;
}
///////////////////////////////////////////////////////////////////////////////
int CItemStrList::GetStrListNumber(void) {
    return Number;
}
///////////////////////////////////////////////////////////////////////////////
void CItemStrList::GetStrList(int num, int& com, int& par) {
    com = par = 0;
    if (num < 0 || num >= Number)
        return;
    com = Parameters[num * 2];
    par = Parameters[num * 2 + 1];
}
///////////////////////////////////////////////////////////////////////////////
void CItemStrList::SaveItem(CParserFile *file) {
    CListConst_item *list = (CListConst_item*)GetList();
    if (Number == 0)
        return;
    file->NewStrList();
    for (int i = 0; i < Number; i ++) {
        char *par = NULL;
        CListConst_item *tmp = list;
        while (tmp != NULL) {
            if (tmp->key == Parameters[i * 2]) {
                par = tmp->text;
                break;
            }
            tmp = tmp->Next;
        }
        char buf[200];
        if (par != NULL)
            sprintf(buf, "%s=%d", par, Parameters[i * 2 + 1]);
        else
            sprintf(buf, "%d=%d", Parameters[i * 2], Parameters[i * 2 + 1]);
        file->AddStrList(GetName(), buf);
    }
}
///////////////////////////////////////////////////////////////////////////////
bool CItemStrList::SetParameter(char *par) {
    int start = 0;
    int pos = 0;
    bool end = false;
    while (1) {
        while (par[pos] != '+' && par[pos] != 0)
            pos ++;
        // Получили полностью одну команду с параметром
        if (par[pos] == 0)
            end = true;
        par[pos] = 0;
        if (!SetComm(&par[start]))
            return false;
        if (end)
            break;
        pos ++;
        start = pos;
    }
    return true;
}
///////////////////////////////////////////////////////////////////////////////
void CItemStrList::SetParam(int num, int com, int des) {
    if (num < 0 || num >= Number) {
        log("Ошибка задания параметра %s", GetName());
        return;
    }
    Parameters[num * 2] = com;
    Parameters[num * 2 + 1] = des;
}
///////////////////////////////////////////////////////////////////////////////
void CItemStrList::AddParam(int com, int des) {
    if (Parameters == NULL)
        Parameters = (int*)malloc(sizeof(int) * 2);
    else
        Parameters = (int*)realloc(Parameters, sizeof(int) * 2 * (Number + 1));
    Parameters[Number*2] = com;
    Parameters[Number*2+1] = des;
    Number ++;
}
///////////////////////////////////////////////////////////////////////////////
int CItemStrList::NewSubItem(void) {
    if (Parameters == NULL)
        Parameters = (int*)malloc(sizeof(int) * 2);
    else
        Parameters = (int*)realloc(Parameters, sizeof(int) * 2 * (Number + 1));
    Number ++;
    return Number - 1;
}
///////////////////////////////////////////////////////////////////////////////
bool CItemStrList::DeleteSubItem(int num) {
    if (num < 0 || num >= Number)
        return false;
    for (int i = num; i < Number - 1; i ++) {
        Parameters[i * 2] = Parameters[(i + 1) * 2];
        Parameters[i * 2 + 1] = Parameters[(i + 1) * 2 + 1];
    }
    Number --;
    if (Number == 0) {
        delete Parameters;
        Parameters = NULL;
        return true;
    }
    Parameters = (int*)realloc(Parameters, sizeof(int) * 2 * Number);
    return true;
}
///////////////////////////////////////////////////////////////////////////////
bool CItemStrList::SetComm(char *par) {
    if (Parameters == NULL)
        Parameters = (int*)malloc(sizeof(int) * 2);
    else
        Parameters = (int*)realloc(Parameters, sizeof(int) * 2 * (Number + 1));
    CListConst_item *list = (CListConst_item*)GetList();
    int pos = 0;
    int i;
    while (par[pos] != '=' && par[pos] != 0)
        pos ++;
    bool sv = false;
    while (list != NULL) {
        if (strncmp(list->text, par, pos) == 0) {
            Parameters[Number * 2] = list->key;
            sv = true;
            break;
        }
        list = list->Next;
    }
    if (!sv) {
        bool pint = true;
        for (int j = 0; j < pos; j ++)
            if (par[j] < '0' || par[j] > '9') {
                pint = false;
                break;
            }
        if (pint) {
            char *tmp = new char[pos + 1];
            strncpy(tmp, par, pos);
            tmp[pos] = 0;
            Parameters[Number * 2] = atoi(tmp);
            delete[] tmp;
            sv = true;
        }
    }
    if (!sv) {
        par[pos] = 0;
        log("Ошибка задания параметра %s. Ключ %s не найден", GetName(), par);
        return false;
    }
    Parameters[Number * 2 + 1] = 1;
    if (par[pos] == 0) {
        Number ++;
        return true;
    }
    pos ++;
    for (i = pos; par[i] != 0; i ++)
        if ((par[i] < '0' || par[i] > '9') && par[i] != '-') {
            log("Ошибка задания параметра %s.", GetName());
            return false;
        }
    Parameters[Number * 2 + 1] = atoi(&par[pos]);
    Number ++;
    return true;
}
///////////////////////////////////////////////////////////////////////////////
// STRUCT (element)
///////////////////////////////////////////////////////////////////////////////
CItemStruct::CItemStruct(CItemProtoStruct* proto) :
        CItem(proto->GetData()->Type, proto->GetData()->Name, false, NULL) {
    //int num;
    CItemProto** items = proto->GetAllItems(Number);
    Item = (CItem**)malloc(sizeof(CItem*) * Number);
    for (int i = 0; i < Number; i ++) {
        CItem *tmp = NULL;
        int tp = items[i]->GetData()->Type;
        char* name = items[i]->GetData()->Name;
        bool imp = items[i]->GetData()->Important;
        void* list = items[i]->GetData()->List;
        switch (tp) {
            case TYPE_INT:
                tmp = new CItemInt(TYPE_INT, name, imp, NULL);
                break;
            case TYPE_FLOAT:
                tmp = new CItemFloat(name, imp);
                break;
            case TYPE_LONGLONG:
                tmp = new CItemLongLong(name, imp);
                break;
            case TYPE_STRING:
                tmp = new CItemString(TYPE_STRING, name, imp, NULL);
                break;
            case TYPE_LIST:
                tmp = new CItemList(name, imp, list);
                break;
            case TYPE_VECTOR:
                tmp = new CItemVector(name, imp, list);
                break;
            case TYPE_SCRIPT:
                tmp = new CItemScript(name, imp);
                break;
            case TYPE_STRLIST:
                tmp = new CItemStrList(name, list);
                break;
            case TYPE_RANDOM:
                tmp = new CItemString(TYPE_RANDOM, name, imp, NULL);
                break;
            case TYPE_STRUCT:
                tmp = new CItemStructList((CItemProtoStruct*)items[i]);
                break;
            case TYPE_EXPR:
                tmp = new CItemExpr(name);
                break;
        }
        if (tmp == NULL) {
            log("PARSER ERROR!  Ошибка рождения копии структуры!");
            log("Либо нет памяти, либо какая-то хрень...");
            log("Exiting...");
            exit(11);
        }
        Item[i] = tmp;
    }
}
///////////////////////////////////////////////////////////////////////////////
CItemStruct::~CItemStruct() {
    if (Item != NULL) {
        for (int i = 0; i < Number; i ++)
            delete Item[i];
        free(Item);
    }
}
///////////////////////////////////////////////////////////////////////////////
bool CItemStruct::GetStatus(void) {
    for (int i = 0; i < Number; i ++)
        if (!Item[i]->GetStatus()) {
            log("Не определен обязательный параметр %s", Item[i]->GetName());
            return false;
        }
    return true;
}
///////////////////////////////////////////////////////////////////////////////
int CItemStruct::GetNumberItem(void) {
    return Number;
}
///////////////////////////////////////////////////////////////////////////////
CItem* CItemStruct::GetItem(int num) {
    return Item[num];
}
///////////////////////////////////////////////////////////////////////////////
bool CItemStruct::SetParameter(char *param) {
    char com[10000];
    char *par = new char[strlen(param) + 1];
    CReadCfg Config;
    Config.SetActiveBuffer(param);
    while (1) {
        int ret = Config.GetCommand(com, par);
        if (ret == FIELD_NONE) {
            delete[] par;
            return true;
        }
        if (ret == FIELD_ERROR) {
            log("Ошибка обработки структурной записи (%s)", GetName());
            delete par;
            return false;
        }
        if (!AddParameter(ret, com, par)) {
            delete par;
            return false;
        }
    }
}
///////////////////////////////////////////////////////////////////////////////
bool CItemStruct::AddParameter(int ret, char* com, char* par) {
    int num = -1;
    for (int i = 0; i < Number; i ++)
        if (strcmp(Item[i]->GetName(), com) == 0) {
            num = i;
            break;
        }
    if (num < 0) {
        log("ОШИБКА! Параметер %s не найден", com);
        return false;
    }
    int tp = Item[num]->GetType();
    switch (ret) {
        case FIELD_CONST:
            if (tp != TYPE_INT && tp != TYPE_VECTOR && tp != TYPE_LIST &&
                    tp != TYPE_RANDOM && tp != TYPE_SCRIPT && tp != TYPE_STRLIST &&
                    tp != TYPE_FLOAT && tp != TYPE_LONGLONG) {
                log("ОШИБКА! Не совпадение типов для параметра %s", com);
                return false;
            }
            break;
        case FIELD_STRING:
            if (tp != TYPE_STRING && tp != TYPE_EXPR) {
                log("ОШИБКА! Не совпадение типов для параметра %s", com);
                return false;
            }
            break;
        case FIELD_STRUCT:
            if (tp != TYPE_STRUCT) {
                log("ОШИБКА! Не совпадение типов для параметра %s", com);
                return false;
            }
            break;
        default:
            log("ОШИБКА! Не совпадение типов для параметра %s", com);
            return false;
    }
    //log("Set parameter: %s %s", Item[num]->GetName(), par);
    return Item[num]->SetParameter(par);
}
///////////////////////////////////////////////////////////////////////////////
void CItemStruct::SaveItem(CParserFile *file) {
    int i;
    file->EndLine();
    if (file->GetLevel() != 0) {
        file->AddCommand("%s (", GetName());
        file->EndLine();
    }
    if (file->GetLevel() != 0)
        file->NextLevel();
    for (i = 0; i < Number; i ++)
        Item[i]->SaveItem(file);
    file->CloseStruct();
}
///////////////////////////////////////////////////////////////////////////////
// STRUCT LIST
///////////////////////////////////////////////////////////////////////////////
CItemStructList::CItemStructList(CItemProtoStruct *proto) :
        CItem(proto->GetData()->Type, proto->GetData()->Name, proto->GetData()->Important, NULL) {
    Proto = proto;
    Number = 0;
    Item = NULL;
}
///////////////////////////////////////////////////////////////////////////////
CItemStructList::CItemStructList() : CItem(TYPE_STRUCT, "", true, NULL) {
    Proto = NULL;
    Number = 0;
    Item = NULL;
}
///////////////////////////////////////////////////////////////////////////////
CItemStructList::~CItemStructList() {
    if (Item != NULL) {
        for (int i = 0; i < Number; i ++)
            if (Item[i] != NULL)
                delete Item[i];
        free(Item);
    }
}
///////////////////////////////////////////////////////////////////////////////
bool CItemStructList::SetParameter(char* param) {
    CItemStruct *tmp = new CItemStruct((CItemProtoStruct*)Proto);
    if (Item == NULL)
        Item = (CItemStruct**)malloc(sizeof(CItemStruct*));
    else
        Item = (CItemStruct**)realloc(Item, sizeof(CItemStruct*) * (Number + 1));
    Item[Number] = tmp;
    Number ++;
    return tmp->SetParameter(param);
}
///////////////////////////////////////////////////////////////////////////////
bool CItemStructList::GetStatus(void) {
    if (Number == 0)
        return true;
    for (int i = 0; i < Number; i ++) {
        if (!Item[i]->GetStatus())
            return false;
    }
    return true;
}
///////////////////////////////////////////////////////////////////////////////
void CItemStructList::SaveItem(CParserFile *file) {
    for (int i = 0; i < Number; i ++)
        Item[i]->SaveItem(file);
}
///////////////////////////////////////////////////////////////////////////////
int CItemStructList::GetNumberItem(void) {
    return Number;
}
///////////////////////////////////////////////////////////////////////////////
CItem* CItemStructList::GetItem(int num) {
    return Item[num];
}
///////////////////////////////////////////////////////////////////////////////
int CItemStructList::NewSubItem(void) {
    CItemStruct *tmp = new CItemStruct((CItemProtoStruct*)Proto);
    if (Item == NULL)
        Item = (CItemStruct**)malloc(sizeof(CItemStruct*));
    else
        Item = (CItemStruct**)realloc(Item, sizeof(CItemStruct*) * (Number + 1));
    Item[Number] = tmp;
    Number ++;
    return Number - 1;
}
///////////////////////////////////////////////////////////////////////////////
bool CItemStructList::DeleteSubItem(int num) {
    if (num < 0 || num >= Number)
        return false;
    delete Item[num];
    for (int i = num + 1; i < Number; i ++)
        Item[i-1] = Item[i];
    Number --;
    Item = (CItemStruct**)realloc(Item, sizeof(CItemStruct*) * Number);
    return true;
}
///////////////////////////////////////////////////////////////////////////////
CItem* CItemStructList::FindItem(int par) {
    CProto *proto = ((CItemProtoStruct*)Proto)->GetItem(0)->GetData();
    if (!CheckTypes(proto->Type, PARAM_INT)) {
        log("Ошибка вызова FindItem. Несоответствие типа поиска");
        return NULL;
    }
    for (int i = 0; i < Number; i ++) {
        if (Item[i]->GetItem(0)->GetInt() == par)
            return Item[i];
    }
    return NULL;
}
///////////////////////////////////////////////////////////////////////////////
CItem* CItemStructList::FindItem(float par) {
    CProto *proto = ((CItemProtoStruct*)Proto)->GetItem(0)->GetData();
    if (!CheckTypes(proto->Type, PARAM_FLOAT)) {
        log("Ошибка вызова FindItem. Несоответствие типа поиска");
        return NULL;
    }
    for (int i = 0; i < Number; i ++) {
        if (Item[i]->GetItem(0)->GetFloat() == par)
            return Item[i];
    }
    return NULL;
}
///////////////////////////////////////////////////////////////////////////////
CItem* CItemStructList::FindItem(char* par) {
    CProto *proto = ((CItemProtoStruct*)Proto)->GetItem(0)->GetData();
    if (!CheckTypes(proto->Type, PARAM_STRING)) {
        log("Ошибка вызова FindItem. Несоответствие типа поиска");
        return NULL;
    }
    for (int i = 0; i < Number; i ++) {
        if (strcmp(Item[i]->GetItem(0)->GetString(), par) == 0)
            return Item[i];
    }
    return NULL;
}
///////////////////////////////////////////////////////////////////////////////
