#include "parser_utils.h"
#include "strlib.h"
#include "expr.h"

#undef DEBUG_EXPR

#define RECREATE(result,type,number) do {\
        if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
        { syserr("SYSERR: realloc failure"); abort(); } } while(0)

EXP_FN_STR emptyExpFns[] = { { -1, } };
EXP_FN_STR *expFns = emptyExpFns;

EXP_FUNC_REGISTRY *expFuncRegistry = new EXP_FUNC_REGISTRY();

void EmptyExecExpFunction(EXP_FN_STR *FnStr, EXP_ITEM& item, EXPFN_PARAMS, EXP_ITEM **Par) {
    ALARM;
}

void (*ExecExpFunction)(EXP_FN_STR *FnStr, EXP_ITEM& item, EXPFN_PARAMS, EXP_ITEM **Par) = EmptyExecExpFunction;

EXP_TYPE::EXP_TYPE() {
    Type = LibType = 0;
}

EXP_TYPE::EXP_TYPE(int tp) {
    Type = tp;
    LibType = 0;
}

EXP_TYPE::EXP_TYPE(int tp, int lib) {
    Type = tp;
    LibType = lib;
}

EXP_TYPE::EXP_TYPE(const EXP_TYPE& exp) {
    Type = exp.GetType();
    LibType = exp.GetLibType();
}

int EXP_TYPE::GetType(void) const {
    return Type;
}

int EXP_TYPE::GetLibType(void) const {
    return LibType;
}

bool EXP_TYPE::operator ==(const EXP_TYPE& exp) const {
    if (exp.GetType() == Type && exp.GetLibType() == LibType)
        return true;
    return false;
}

bool EXP_TYPE::operator !=(const EXP_TYPE& exp) const {
    return !operator ==(exp);
}

bool EXP_TYPE::IsZero(void) const {
    if (Type == 0)
        return true;
    return false;
}

bool EXP_TYPE::CheckOperation(BYTE op) const {
    BYTE mask = op & 0xF0;
    if (Type == EXPTYPE_BOOL) {
        if (mask != EXP_COP_OR)
            return false;
        return true;
    }
    if (Type == EXPTYPE_STRING || Type == EXPTYPE_VECTOR || Type == EXPTYPE_LIST) {
        return true;
    }
    if (Type == EXPTYPE_DIGIT) {
        if (mask != EXP_COP_MUL && mask != EXP_COP_PLUS)
            return false;
        return true;
    }
    ALARM;
}

////////////////////////////////////////////////////////////////////////////////
EXP_ITEM::EXP_ITEM() {
    DigitParam = 0;
    BoolParam = false;
}

EXP_ITEM::EXP_ITEM(int par) {
    DigitParam = (double)par;
}

EXP_ITEM::EXP_ITEM(double par) {
    DigitParam = par;
}

EXP_ITEM::EXP_ITEM(bool par) {
    BoolParam = par;
}

EXP_ITEM::EXP_ITEM(const char* str) {
    DigitParam = 0;
    StringParam = str;
    StringParam.MakeLower();
}

EXP_ITEM::EXP_ITEM(const STRING& str) {
    DigitParam = 0;
    StringParam = str;
    StringParam.MakeLower();
}

EXP_ITEM::EXP_ITEM(const EXP_ITEM& item) {
    operator =(item);
}

EXP_ITEM::~EXP_ITEM() {
}

double EXP_ITEM::GetDigitDouble(void) const {
    return DigitParam;
}

int EXP_ITEM::GetDigitInt(void) const {
    return (int)DigitParam;
}

const char* EXP_ITEM::GetString(void) const {
    return StringParam.GetString();
}

void EXP_ITEM::operator =(const EXP_ITEM & item) {
    DigitParam = item.GetDigitDouble();
    StringParam = item.GetString();
    StringParam.MakeLower();
    BoolParam = item.GetBool();
}

bool EXP_ITEM::GetBool(void) const {
    return BoolParam;
}

void EXP_ITEM::SetParam(int par) {
    DigitParam = (double)par;
}

void EXP_ITEM::SetParam(double par) {
    DigitParam = par;
}

void EXP_ITEM::SetParam(bool par) {
    BoolParam = par;
}

void EXP_ITEM::SetParam(const char* par) {
    StringParam = par;
    StringParam.MakeLower();
}

void EXP_ITEM::SetParam(const STRING& par) {
    StringParam = par;
    StringParam.MakeLower();
}

void EXP_ITEM::Operation(BYTE op, const EXP_ITEM &item) {
    if (op == 0)
        return;
    switch (op) {
        case EXP_OP_10:
            DigitParam *= item.GetDigitDouble();
            break;
        case EXP_OP_11:
            DigitParam /= item.GetDigitDouble();
            break;
        case EXP_OP_20:
            DigitParam += item.GetDigitDouble();
            break;
        case EXP_OP_21:
            DigitParam -= item.GetDigitDouble();
            break;
        case EXP_OP_40:
            if (item.GetBool())
                BoolParam = true;
            break;
        case EXP_OP_41:
            if (!item.GetBool())
                BoolParam = false;
            break;
        default:
            ALARM;
    }
}

bool EXP_ITEM::Compare(BYTE op, const EXP_ITEM& item, const EXP_TYPE& type) {
    int tp = type.GetType();
    bool ret = false;
    bool use = false;
#ifdef DEBUG_EXPR
    STRING debugstr;
#endif

    if (tp == EXPTYPE_DIGIT) {
        use = true;
        double par = item.GetDigitDouble();
#ifdef DEBUG_EXPR
        debugstr.Format("[tp: digit] [op: %X] p0[%f] p1[%f]", op, DigitParam, par);
#endif
        switch (op) {
            case EXP_OP_30:
                if (DigitParam < par)
                    ret = true;
                break;
            case EXP_OP_31:
                if (DigitParam <= par)
                    ret = true;
                break;
            case EXP_OP_32:
                if (DigitParam == par)
                    ret = true;
                break;
            case EXP_OP_33:
                if (DigitParam >= par)
                    ret = true;
                break;
            case EXP_OP_34:
                if (DigitParam > par)
                    ret = true;
                break;
            case EXP_OP_35:
                if (DigitParam != par)
                    ret = true;
                break;
            default:
                ALARM;
        }
    }
    if (tp == EXPTYPE_STRING) {
        use = true;
        const char* str = item.GetString();
#ifdef DEBUG_EXPR
        debugstr.Format("[tp: string] [op: %X] p0[%s] p1[%s]", op, StringParam.GetString(), str);
#endif
        switch (op) {
            case EXP_OP_32:
                if (StringParam == str)
                    ret = true;
                break;
            case EXP_OP_35:
                if (StringParam != str)
                    ret = true;
                break;
            default:
                ALARM;
        }
    }
    if (!use)
        ALARM;
#ifdef DEBUG_EXPR
    log("EXPR_DEBUG: Compare %s, RESULT: %s", debugstr.GetString(), ret ? "TRUE" : "FALSE");
#endif
    return ret;
}

////////////////////////////////////////////////////////////////////////////////
EXP_VAR::EXP_VAR(int tp) {
    VarType = tp;
    //LevelDown = NULL;
    Next = NULL;
}

EXP_VAR::EXP_VAR(int tp, const EXP_TYPE& par) {
    VarType = tp;
    //LevelDown = NULL;
    Next = NULL;
    ParamType = par;
}

EXP_VAR::~EXP_VAR() {
    if (Next != NULL)
        delete Next;
}

bool EXP_VAR::CheckType(const EXP_TYPE& tp) {
    if (ParamType.IsZero())
        return true;
    if (ParamType != tp)
        return false;
    return true;
}

bool EXP_VAR::SetTypeNext(const EXP_TYPE& type) {
    if (Next == NULL)
        return true;
    if (!type.CheckOperation(NextOperation))
        return false;
    return Next->SetType(type);
}

bool EXP_VAR::FindType(EXP_TYPE& ret) {
    if (!ParamType.IsZero()) {
        ret = ParamType;
        return true;
    }
    if (Next == NULL)
        return false;
    return Next->FindType(ret);
}

void EXP_VAR::Exec(BYTE op, const EXP_ITEM& item, EXP_ITEM& ret, EXPFN_PARAMS) {
    ret.Operation(op, item);
    if (Next == NULL)
        return;
    // Есть операции дальше
    EXP_ITEM ritem;
    Next->Exec(NextOperation, ret, ritem, EXPFN_USE_PARAMS);
    ret = ritem;
}

////////////////////////////////////////////////////////////////////////////////
EXP_BRACKET::EXP_BRACKET() : EXP_VAR(EXPITEM_BRACKET) {
    Param = NULL;
}

EXP_BRACKET::EXP_BRACKET(const EXP_TYPE& tp) : EXP_VAR(EXPITEM_BRACKET, tp) {
    Param = NULL;
}

EXP_BRACKET::~EXP_BRACKET() {
    if (Param)
        delete Param;
}

bool EXP_BRACKET::SetType(const EXP_TYPE& type) {
    if (!CheckType(type))
        return false;
    SetParamType(type);
    if (Param) {
        if (!Param->SetType(type))
            return false;
    }
    return SetTypeNext(type);
}

bool EXP_BRACKET::FindType(EXP_TYPE& ret) {
    if (Param && Param->FindType(ret))
        return true;
    return EXP_VAR::FindType(ret);
}

void EXP_BRACKET::Exec(BYTE op, const EXP_ITEM& item, EXP_ITEM& ret, EXPFN_PARAMS) {
    if (Param == NULL)
        ALARM;
    EXP_ITEM ii;
    Param->Exec(0, ii, ret, EXPFN_USE_PARAMS);
    EXP_VAR::Exec(op, item, ret, EXPFN_USE_PARAMS);
}

////////////////////////////////////////////////////////////////////////////////
EXP_CONST::EXP_CONST(bool st_string, const char* str) : EXP_VAR(EXPITEM_CONST) {
    Param = str;
    StString = st_string;
}

EXP_CONST::~EXP_CONST() {
}

double EXP_CONST::GetDigit(void) const {
    return DigitParam;
}

bool EXP_CONST::SetType(const EXP_TYPE& type) {
    if (!CheckType(type))
        return false;
    if (type.GetType() == EXPTYPE_BOOL)
        return false;
    if (type.GetType() == EXPTYPE_DIGIT && StString)
        return false;
    SetParamType(type);
    if (type.GetType() == EXPTYPE_DIGIT) {
        // Проверяем чтобы было число
        const char *str = Param.GetString();
        bool pt = false;
        bool dg = false;
        for (int i = 0; str[i] != 0; i ++) {
            if (str[i] == '.') {
                if (pt)
                    return false;
                pt = true;
                continue;
            }
            if (str[i] < '0' || str[i] > '9')
                return false;
            dg = true;
        }
        if (!dg)
            // Цифр нет. Есть только одна точка
            return false;
        // Проверили
        DigitParam = atof(str);
        return SetTypeNext(type);
    }
    if (type.GetType() == EXPTYPE_STRING)
        return SetTypeNext(type);
    // С листами и векторами пока не работаем
    if (type.GetType() == EXPTYPE_VECTOR)
        return false;
    if (type.GetType() == EXPTYPE_LIST)
        return false;
    ALARM;  // Если тут, то ошибка
}

void EXP_CONST::Exec(BYTE op, const EXP_ITEM& item, EXP_ITEM& ret, EXPFN_PARAMS) {
    switch (GetParamType().GetType()) {
        case EXPTYPE_DIGIT:
            ret.SetParam(DigitParam);
            break;
        case EXPTYPE_STRING:
            ret.SetParam(Param);
            break;
        default:
            ALARM;
    }
    EXP_VAR::Exec(op, item, ret, EXPFN_USE_PARAMS);
}

////////////////////////////////////////////////////////////////////////////////
EXP_FUNC::EXP_FUNC(const char* name, EXP_FN_STR* fstr) : EXP_VAR(EXPITEM_FUNC) {
    FnStr = fstr;
    FnName = name;
    Param = NULL;
    ExecItem = NULL;
    SetParamType(FnStr->TypeResult);
    NumParam = 0;

    expFuncRegistry->insert(this);
}

EXP_FUNC::~EXP_FUNC() {
    expFuncRegistry->erase(this);

    for (int i = 0; i < NumParam; i ++) {
        delete Param[i];
        delete ExecItem[i];
    }
    if (Param != NULL)
        free(Param);
    if (ExecItem != NULL)
        free(ExecItem);
}

void EXP_FUNC::AddParameter(EXP_VAR* var) {
    RECREATE(Param, EXP_VAR*, NumParam + 1);
    Param[NumParam] = var;
    RECREATE(ExecItem, EXP_ITEM*, NumParam + 1);
    ExecItem[NumParam] = new EXP_ITEM;
    NumParam ++;
}

bool EXP_FUNC::CheckNumParam(void) {
    if (FnStr->NumParam != NumParam)
        return false;
    return true;
}

bool EXP_FUNC::SetType(const EXP_TYPE& type) {
    if (!CheckType(type))
        return false;
    if (NumParam != FnStr->NumParam)
        ALARM;
    for (int i = 0; i < NumParam; i ++) {
        if (!Param[i]->SetType(FnStr->TypeParam[i]))
            return false;
    }
    return SetTypeNext(type);
}

void EXP_FUNC::Exec(BYTE op, const EXP_ITEM& item, EXP_ITEM& ret, EXPFN_PARAMS) {
    for (int i = 0; i < NumParam; i ++) {
        EXP_ITEM ii;
        Param[i]->Exec(0, ii, (*ExecItem[i]), EXPFN_USE_PARAMS);
    }
    ExecExpFunction(FnStr, ret, EXPFN_USE_PARAMS, ExecItem);
    EXP_VAR::Exec(op, item, ret, EXPFN_USE_PARAMS);
}

////////////////////////////////////////////////////////////////////////////////
EXP_BOOL::EXP_BOOL() : EXP_VAR(EXPITEM_BOOL, EXP_TYPE(EXPTYPE_BOOL)) {
    Param = NULL;
    Operation = NULL;
    NumParam = 0;
    ExecItem = NULL;
}

EXP_BOOL::~EXP_BOOL() {
    for (int i = 0; i < NumParam; i ++) {
        delete Param[i];
        delete ExecItem[i];
    }
    if (Param != NULL)
        free(Param);
    if (Operation)
        free(Operation);
    if (ExecItem)
        free(ExecItem);
}

void EXP_BOOL::AddParameter(EXP_VAR* var, BYTE op) {
    RECREATE(Param, EXP_VAR*, NumParam + 1);
    Param[NumParam] = var;
    RECREATE(Operation, BYTE, NumParam + 1);
    Operation[NumParam] = op;
    RECREATE(ExecItem, EXP_ITEM*, NumParam + 1);
    ExecItem[NumParam] = new EXP_ITEM;
    NumParam ++;
}

bool EXP_BOOL::SetType(const EXP_TYPE& type) {
    if (!CheckType(type))
        return false;
    // Ищем тип переменных внутри сравнений
    EXP_TYPE btp;
    bool fnd = false;
    int i;
    for (i = 0; i < NumParam; i ++) {
        if (Param[i]->FindType(btp)) {
            fnd = true;
            break;
        }
    }
    if (!fnd)
        return false;
    for (i = 0; i < NumParam; i ++) {
        if (!Param[i]->SetType(btp))
            return false;
    }
    return SetTypeNext(type);
}

void EXP_BOOL::Exec(BYTE op, const EXP_ITEM& item, EXP_ITEM& ret, EXPFN_PARAMS) {
#ifdef DEBUG_EXPR
    log("*** Выполняю выражения");
#endif

    int i;
    for (i = 0; i < NumParam; i ++) {
        EXP_ITEM ii;
        Param[i]->Exec(0, ii, (*ExecItem[i]), EXPFN_USE_PARAMS);
    }
    bool cmp = true;
    for (i = 0; i < NumParam - 1; i ++) {
        if (!ExecItem[i]->Compare(Operation[i], (*ExecItem[i+1]), Param[i]->GetParamType())) {
            cmp = false;
            break;
        }
    }
    ret.SetParam(cmp);
    EXP_VAR::Exec(op, item, ret, EXPFN_USE_PARAMS);
}

////////////////////////////////////////////////////////////////////////////////
CExpression::CExpression() {
    Expression = NULL;
    RetErr = false;
}

CExpression::~CExpression() {
    ClearExp(Expression);
}

void CExpression::ClearExp(EXP_VAR *var) {
    delete var;
    /*
    EXP_VAR *list = var;
    while(list)
    {
     ClearExp(list->GetDown());
     EXP_VAR *tmp = list;
     list = list->GetNext();
     delete tmp;
    }
    */
}

bool CExpression::CheckSpWord(const char *buf, int &ptr, STRING *out) {
    const char *spw[] = { "и",  "или", "" };
    const char *rpw[] = { "&&", "||",  "" };
    for (int i = 0; spw[i][0] != 0; i ++) {
        int len = strlen(spw[i]);
        if ((int)strlen(&buf[ptr]) < len)
            continue;
        if (strncmp(&buf[ptr], spw[i], len) != 0)
            continue;
        if ((buf[ptr+len] >= 0 && buf[ptr+len] <= 0x20) || buf[ptr+len] == '(' ||
                buf[ptr+len] == '"') {
            (*out) += rpw[i];
            ptr += len - 1;
            return true;
        }
    }
    return false;
}

bool CExpression::PrepareBuffer(const char* inbuf, STRING& out) {
    STRING cbuf;
    for (int i = 0; inbuf[i] != 0; i ++) {
        if (inbuf[i] == 0x0D)
            continue;
        cbuf += inbuf[i];
    }
    cbuf.MakeLower();

    out = "";
    bool st_str = false;
    int br = 0;
    char prv = 0;
    const char *buf = cbuf.GetString();
    int ptr = -1;
    while (1) {
        ptr ++;
        if (buf[ptr] == 0)
            break;
        char sym = buf[ptr];
        if (sym == 0x1B) {
            StErr = EXPERR_BAD_CHAR;
            return false;
        }
        if (sym == '"') {
            out += sym;
            st_str = !st_str;
            prv = 0;
            continue;
        }
        if (st_str) {
            prv = 0;
            if (sym == 0x0A) {
                StErr = EXPERR_0AINSTRING;
                return false;
            }
            out += sym;
            continue;
        }
        if (sym > 0 && sym <= 0x20) {
            prv = 0;
            continue;
        }
        if (sym == '(') {
            out += sym;
            br ++;
            prv = '#';
            continue;
        }
        if (sym == ')') {
            br --;
            if (br < 0) {
                StErr = EXPERR_CLBRACKET;
                return false;
            }
            prv = 0;
            out += sym;
            continue;
        }
        if (prv == 0) {
            // Был пробел или перевод строки
            // Заменяем И и ИЛИ
            prv = '#';
            if (CheckSpWord(buf, ptr, &out))
                continue;
        }
        prv = '#';
        out += sym;
    }
    if (st_str) {
        StErr = EXPERR_STRNOTEND;
        return false;
    }
    if (br != 0) {
        StErr = EXPERR_OPBRACKET;
        return false;
    }
    return true;
}

void CExpression::ConvertBuffer(const char* buf, STRING& out) {
    out = "";
    int ptr = 0;
    bool str = false;
    while (1) {
        if (buf[ptr] == 0)
            break;
        if (buf[ptr] == '"') {
            out += buf[ptr];
            str = !str;
            ptr ++;
            continue;
        }
        if (str) {
            out += buf[ptr];
            ptr ++;
            continue;
        }
        if (buf[ptr] == '<') {
            out += (char)0x1B;
            ptr ++;
            if (buf[ptr] == '=') {
                out += (char)EXP_OP_31;
                ptr ++;
            } else
                out += (char)EXP_OP_30;
            continue;
        }
        if (buf[ptr] == '>') {
            out += (char)0x1B;
            ptr ++;
            if (buf[ptr] == '=') {
                out += (char)EXP_OP_33;
                ptr ++;
            } else
                out += (char)EXP_OP_34;
            continue;
        }
        if (buf[ptr] == '=') {
            out += (char)0x1B;
            ptr ++;
            if (buf[ptr] == '=')
                ptr ++;
            out += (char)EXP_OP_32;
            continue;
        }
        if (buf[ptr] == '!' && buf[ptr+1] == '=') {
            out += (char)0x1B;
            out += (char)EXP_OP_35;
            ptr += 2;
            continue;
        }
        if (buf[ptr] == '|' && buf[ptr+1] == '|') {
            out += (char)0x1B;
            out += (char)EXP_OP_40;
            ptr += 2;
            continue;
        }
        if (buf[ptr] == '&' && buf[ptr+1] == '&') {
            out += (char)0x1B;
            out += (char)EXP_OP_41;
            ptr += 2;
            continue;
        }
        switch (buf[ptr]) {
            case '*':
                out += (char)0x1B;
                out += (char)EXP_OP_10;
                break;
            case '/':
                out += (char)0x1B;
                out += (char)EXP_OP_11;
                break;
            case '+':
                out += (char)0x1B;
                out += (char)EXP_OP_20;
                break;
            case '-':
                out += (char)0x1B;
                out += (char)EXP_OP_21;
                break;
            case ',':
                out += (char)0x1B;
                out += (char)EXP_OP_50;
                break;
            default:
                out += buf[ptr];
        }
        ptr ++;
    }
}

bool CExpression::CheckBuffer(const char* buf) {
    bool st = false;
    for (int i = 0; buf[i] != 0; i ++) {
        char s = buf[i];
        if (s < 0)
            continue;
        if (s == 0x1B) {
            i ++;
            continue;
        }
        if (s > 0 && s <= 0x20) {
            StErr = EXPERR_BAD_CHAR;
            return false;
        }
        if (s == '"') {
            st = !st;
            continue;
        }
        if (st)
            continue;
        if (s == '<' || s == '>' || s == '=' || s == '&' || s == '|') {
            StErr = EXPERR_BAD_OPERATION;
            return false;
        }
    }
    return true;
}

int CExpression::FindTypeOp(const char* buf) {
    int ptr = -1;
    int nbr = 0;
    bool cln = false;
    int max_tp = 0;
    while (1) {
        ptr ++;
        if (buf[ptr] == 0)
            break;
        if (buf[ptr] == '"') {
            cln = !cln;
            continue;
        }
        if (cln)
            continue;
        if (buf[ptr] == '(') {
            nbr ++;
            continue;
        }
        if (buf[ptr] == ')') {
            nbr --;
            continue;
        }
        if (nbr > 0)
            continue;
        if (buf[ptr] != 0x1B)
            continue;
        ptr ++;
        int tp = (buf[ptr] & 0xF0);
        if (tp > max_tp)
            max_tp = tp;
    }
    return max_tp;
}

EXP_VAR* CExpression::ParseLine(const char* line) {
    //debug("ParseLine: %s", line);
    int tp = FindTypeOp(line);
    //debug("FindTypeOp: %2.2X", (BYTE)tp);
    if (tp == 0) {
        // Операции не найдены
        // Это либо константа, либо ф-ция, либо выражение в скобках
        int len = strlen(line);
        if (len == 0) {
            StErr = EXPERR_PARAMETER;
            return NULL;
        }
        // Строка
        if (line[0] == '"') {
            //debug("STRING");
            if (line[len-1] != '"') {
                StErr = EXPERR_PARAMETER;
                //debug("ret 1");
                return NULL;
            }
            for (int i = 1; i < len - 1; i ++)
                if (line[i] == '"') {
                    StErr = EXPERR_PARAMETER;
                    //debug("ret 2");
                    return NULL;
                }
            // Строка
            STRING str(&line[1], len - 2);
            EXP_VAR* var = new EXP_CONST(true, str.GetString());
            return var;
        }
        // Скобки
        if (line[0] == '(') {
            if (line[len-1] != ')') {
                StErr = EXPERR_PARAMETER;
                return NULL;
            }
            int lv = 1;
            bool ststr = false;
            for (int i = 1; i < len - 1; i ++) {
                if (line[i] == 0x1B) {
                    i ++;
                    continue;
                }
                if (line[i] == '"') {
                    ststr = !ststr;
                    continue;
                }
                if (ststr)
                    continue;
                if (line[i] == '(') {
                    lv ++;
                    continue;
                }
                if (line[i] == ')') {
                    lv --;
                    if (lv == 0) {
                        StErr = EXPERR_PARAMETER;
                        return NULL;
                    }
                }
            }
            if (ststr) {
                StErr = EXPERR_PARAMETER;
                return NULL;
            }
            STRING str(&line[1], len - 2);
            // Получили текст в скобках
            EXP_VAR* var = ParseLine(str.GetString());
            if (var == NULL)
                return NULL;
            EXP_BRACKET* br = new EXP_BRACKET;
            br->SetParam(var);
            return (EXP_VAR*)br;
        }
        // Функция
        if (line[len-1] == ')') {
            //debug("FN");
            int fnb = 0;
            while (fnb < len && line[fnb] != '(')
                fnb ++;
            //debug("fnb %d", fnb);
            if (fnb >= len || line[fnb] != '(') {
                StErr = EXPERR_PARAMETER;
                return NULL;
            }
            STRING namefn(line, fnb);
            fnb ++;
            int plen = len - 1 - fnb;
            EXP_FN_STR *fstr = NULL;
            for (int i = 0; expFns[i].Name[0] != 0; i ++) {
                if (namefn == expFns[i].Name) {
                    fstr = &expFns[i];
                    break;
                }
            }
            if (fstr == NULL) {
                StErr = EXPERR_UNKNOWN_FN;
                return NULL;
            }
            EXP_FUNC* fnvar = new EXP_FUNC(namefn.GetString(), fstr);
            if (plen == 0) {
                // Функция без параметров
                if (!fnvar->CheckNumParam()) {
                    StErr = EXPERR_FN_NUMPAR;
                    delete fnvar;
                    return NULL;
                }
                return (EXP_VAR*)fnvar;
            }
            STRING param(&line[fnb], plen);
            // Ищем разделение на параметры
            const char* spar = param.GetString();
            while (1) {
                int ptr = 0;
                if (spar[ptr] == 0)
                    break;
                bool st_str = false;
                int nbr = 0;
                while (1) {
                    if (spar[ptr] == 0)
                        break;
                    if (spar[ptr] == '"') {
                        st_str = !st_str;
                        ptr ++;
                        continue;
                    }
                    if (st_str) {
                        ptr ++;
                        continue;
                    }
                    if (spar[ptr] == '(') {
                        nbr ++;
                        ptr ++;
                        continue;
                    }
                    if (spar[ptr] == '(') {
                        nbr --;
                        if (nbr < 0)
                            ALARM;
                        ptr ++;
                        continue;
                    }
                    if (nbr != 0) {
                        ptr ++;
                        continue;
                    }
                    if (spar[ptr] == 0x1B && spar[ptr+1] == EXP_OP_50)
                        break;
                    ptr ++;
                }
                if (nbr != 0)
                    ALARM;
                if (ptr == 0) {
                    StErr = EXPERR_FNEMPTY_PAR;
                    delete fnvar;
                    return NULL;
                }
                STRING chpar(spar, ptr);
                EXP_VAR *fn_par = ParseLine(chpar.GetString());
                if (fn_par == NULL) {
                    delete fnvar;
                    return NULL;
                }
                fnvar->AddParameter(fn_par);
                if (spar[ptr] != 0)
                    ptr += 2;
                spar = &spar[ptr];
            }
            if (!fnvar->CheckNumParam()) {
                //debug("Err 2");
                StErr = EXPERR_FN_NUMPAR;
                delete fnvar;
                return NULL;
            }
            return (EXP_VAR*)fnvar;
        }
        // Число или строка без кавычек
        for (int i = 0; i < len; i ++) {
            char sym = line[i];
            if (sym == '"' || sym == '(' || sym == ')') {
                StErr = EXPERR_PARAMETER;
                return NULL;
            }
        }
        EXP_VAR* var = new EXP_CONST(false, line);
        return var;
    }
    if (tp == EXP_COP_FNP) {
        StErr = EXPERR_FNRAZ_EXT;
        return NULL;
    }
    if (tp == EXP_COP_EQ) {
        // Операции сравнения
        EXP_BOOL* boolvar = new EXP_BOOL;
        // Ищем разделение на параметры
        const char* spar = line;
        while (1) {
            if (spar[0] == 0)
                break;
            int ptr = 0;
            BYTE op = FindOperator(spar, EXP_COP_EQ, ptr);
            if (ptr == 0) {
                StErr = EXPERR_COMP_EMPTY;
                delete boolvar;
                return NULL;
            }
            STRING chpar(spar, ptr);
            EXP_VAR *bool_par = ParseLine(chpar.GetString());
            if (bool_par == NULL) {
                delete boolvar;
                return NULL;
            }
            boolvar->AddParameter(bool_par, op);
            if (op == 0)
                break;
            spar = &spar[ptr+2];
        }
        return (EXP_VAR*)boolvar;
    }
    // Остались обычные операции
    int ptr;
    BYTE op = FindOperator(line, tp, ptr);
    //debug("get op %2.2X", op);
    if (op == 0) {
        StErr = EXPERR_INT_ERROR;
        return NULL;
    }
    STRING lpar(line, ptr);
    STRING rpar(&line[ptr+2]);
    //debug("lpar: [%s] rpar: [%s]", lpar.GetString(), rpar.GetString());
    if (lpar.GetLength() == 0 || rpar.GetLength() == 0) {
        StErr = EXPERR_EMPTY_PARAM;
        return NULL;
    }
    //debug("Start LPAR");
    EXP_VAR *l_var = ParseLine(lpar.GetString());
    if (l_var == NULL)
        return NULL;
    //debug("Start RPAR");
    EXP_VAR *r_var = ParseLine(rpar.GetString());
    if (r_var == NULL) {
        delete l_var;
        return NULL;
    }
    EXP_VAR *left = l_var;
    if (l_var->GetNext() != NULL) {
        left = new EXP_BRACKET;
        ((EXP_BRACKET*)left)->SetParam(l_var);
    }
    EXP_VAR *right = r_var;
    if (r_var->GetNext() != NULL) {
        right = new EXP_BRACKET;
        ((EXP_BRACKET*)right)->SetParam(r_var);
    }
    left->SetNext(right, op);
    return left;
}

BYTE CExpression::FindOperator(const char* buf, BYTE mask, int& retptr) {
    int nbr = 0;
    bool st_str = false;
    int ptr = -1;
    while (1) {
        ptr ++;
        if (buf[ptr] == 0) {
            retptr = ptr;
            return 0;
        }
        if (buf[ptr] == '"') {
            st_str = !st_str;
            continue;
        }
        if (st_str)
            continue;
        if (buf[ptr] == '(') {
            nbr ++;
            continue;
        }
        if (buf[ptr] == ')') {
            nbr --;
            if (nbr < 0)
                ALARM;
            continue;
        }
        if (buf[ptr] != 0x1B)
            continue;
        ptr ++;
        if (buf[ptr] == 0)
            ALARM;
        if (nbr > 0)
            continue;
        if ((buf[ptr] & 0xF0) != mask)
            continue;
        retptr = ptr - 1;
        return (BYTE)buf[ptr];
    }
}

bool CExpression::fnSetExpression(const char* inbuf) {
    StErr = 0;
    if (Expression != NULL) {
        StErr = EXPERR_EXPRAGAIN;
        return false;
    }
    STRING buf;
    if (!PrepareBuffer(inbuf, buf))
        return false;
    //debug("Prepare: %s", buf.GetString());
    STRING wbuf;
    ConvertBuffer(buf.GetString(), wbuf);
    if (!CheckBuffer(wbuf.GetString()))
        return false;
    Expression = new EXP_BRACKET(EXP_TYPE(EXPTYPE_BOOL));
    EXP_VAR* ret = ParseLine(wbuf.GetString());
    if (ret == NULL)
        return false;
    ((EXP_BRACKET*)Expression)->SetParam(ret);

    if (!Expression->SetType(EXP_TYPE(EXPTYPE_BOOL))) {
        StErr = EXPERR_TYPE_ERR;
        return false;
    }
    return true;
}

bool CExpression::SetExpression(const char* str) {
    if (RetErr)
        return false;
    if (!fnSetExpression(str)) {
        RetErr = true;
        if (Expression)
            delete Expression;
        Expression = NULL;
        return false;
    }
    return true;
}

bool CExpression::Expr(EXPFN_PARAMS) {
    if (RetErr)
        return false;

    if (Expression == NULL)
        return true;

    EXP_ITEM ii, ret;
    Expression->Exec(0, ii, ret, EXPFN_USE_PARAMS);
    return ret.GetBool();
}

void CExpression::ReleaseExpr(CExpression* expr) {
    expr->SetNewExpression(Expression);
    Expression = NULL;
}

void CExpression::SetNewExpression(EXP_VAR* expr) {
    RetErr = false;
    if (Expression)
        delete Expression;
    Expression = expr;
}

