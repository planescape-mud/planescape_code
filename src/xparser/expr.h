#ifndef EXPR_H
#define EXPR_H

#include "parser_types.h"
#include "strlib.h"
#include <set>
using std::set;

#define MAX_FN_NAME  50
#define MAX_FN_PARAM 10

#define EXP_OP_10   0x30 // *
#define EXP_OP_11   0x31 // /
#define EXP_OP_20   0x40 // +
#define EXP_OP_21   0x41 // -
#define EXP_OP_30   0x50 // <
#define EXP_OP_31   0x51 // <=
#define EXP_OP_32   0x52 // ==
#define EXP_OP_33   0x53 // >=
#define EXP_OP_34   0x54 // >
#define EXP_OP_35   0x55 // !=
#define EXP_OP_40   0x60 // ||
#define EXP_OP_41   0x61 // &&
#define EXP_OP_50   0x70 // ,

#define EXP_COP_MUL   0x30 // * /
#define EXP_COP_PLUS  0x40 // + -
#define EXP_COP_EQ   0x50 // < > = >= <= = == !=
#define EXP_COP_OR   0x60 // || &&
#define EXP_COP_FNP   0x70 // ,

#define EXPITEM_CONST  0
#define EXPITEM_BRACKET  1
#define EXPITEM_FUNC  2
#define EXPITEM_BOOL  3
#define EXPITEM_COMP  4
#define EXPITEM_FNPAR  5


#define EXPTYPE_DIGIT  0x0001
#define EXPTYPE_STRING  0x0002
#define EXPTYPE_VECTOR  0x0004
#define EXPTYPE_LIST  0x0008
#define EXPTYPE_BOOL  0x0100

#define EXPERR_EXPRAGAIN  1 // Повторная запись выражения
#define EXPERR_0AINSTRING  2 // Перевод строки внутри строки
#define EXPERR_CLBRACKET  3 // Закрывающих скобок больше открывающих
#define EXPERR_OPBRACKET  4 // Открывающих скобок больше закрывающих
#define EXPERR_STRNOTEND  5 // Строка не окончена (нет закрывающих кавычек)
#define EXPERR_PARAMETER  6 // Неверный формат переменной
#define EXPERR_FNEMPTY_PAR  7 // Разделить параметров стоит, но параметра нет
#define EXPERR_FNRAZ_EXT  8 // Внешний разделитель параметров ф-ции
#define EXPERR_COMP_EMPTY  9 // Не с чем сравнивать (нет параметра)
#define EXPERR_BAD_CHAR   10 // Запрещенный символ (1B)
#define EXPERR_BAD_OPERATION 11 // Неверный оператор
#define EXPERR_INT_ERROR  12 // Внутренняя ошибка
#define EXPERR_EMPTY_PARAM  13 // Нет параметра слева или справа от операции
#define EXPERR_UNKNOWN_FN  14 // Неизвестная функция
#define EXPERR_FN_NUMPAR  15 // Неверное количество параметров у ф-ции
#define EXPERR_TYPE_ERR   16 // Несоответствие типов

#define EXPFN_PARAMS   struct char_data *actor, struct char_data *victim, char *arg
#define EXPFN_USE_PARAMS  actor, victim, arg

#define EXPR(x) ((CExpression*)(x))

class EXP_TYPE {
        int Type;  // Тип данных
        int LibType; // Норме справочника (если LIST или VECTOR)
    public:
        EXP_TYPE();
        EXP_TYPE(int tp);
        EXP_TYPE(int tp, int lib);
        EXP_TYPE(const EXP_TYPE& exp);
        virtual ~EXP_TYPE() {};

        int GetType(void) const;
        int GetLibType(void) const;

        bool operator ==(const EXP_TYPE& exp) const;
        bool operator !=(const EXP_TYPE& exp) const;
        bool IsZero(void) const;

        bool CheckOperation(BYTE op) const;
};

struct EXP_FN_STR {
    int Number;
    char Name[MAX_FN_NAME];
    EXP_TYPE TypeResult;
    int NumParam;
    EXP_TYPE TypeParam[MAX_FN_PARAM];
};


class EXP_ITEM {
        double DigitParam;
        STRING StringParam;
        bool BoolParam;
    public:
        EXP_ITEM();
        EXP_ITEM(int par);
        EXP_ITEM(double par);
        EXP_ITEM(bool par);
        EXP_ITEM(const char* str);
        EXP_ITEM(const STRING& str);
        EXP_ITEM(const EXP_ITEM& item);
        virtual ~EXP_ITEM();

        double GetDigitDouble(void) const;
        int GetDigitInt(void) const;
        const char* GetString(void) const;
        bool GetBool(void) const;

        void operator =(const EXP_ITEM& item);

        void SetParam(int par);
        void SetParam(double par);
        void SetParam(bool par);
        void SetParam(const char* par);
        void SetParam(const STRING& par);

        void Operation(BYTE op, const EXP_ITEM &item);
        bool Compare(BYTE op, const EXP_ITEM& item, const EXP_TYPE& type);
};

// Прототип переменной
class EXP_VAR {
        int VarType;
        //EXP_VAR* LevelDown;
        EXP_VAR* Next;
        BYTE NextOperation;
        EXP_TYPE ParamType;
    protected:
        void SetParamType(const EXP_TYPE& tp) {
            ParamType = tp;
        };
        bool CheckType(const EXP_TYPE& tp);
        bool SetTypeNext(const EXP_TYPE& type);
    public:
        EXP_VAR(int tp);
        EXP_VAR(int tp, const EXP_TYPE& par);
        virtual ~EXP_VAR();

        int GetVarType(void) {
            return VarType;
        };
        void SetNext(EXP_VAR* var, BYTE op) {
            Next = var;
            NextOperation = op;
        };
        EXP_VAR* GetNext(void) {
            return Next;
        };
        BYTE GetNextOperation(void) {
            return NextOperation;
        };

        EXP_TYPE GetParamType(void) {
            return ParamType;
        };

        virtual bool SetType(const EXP_TYPE& type) = 0;

        virtual bool FindType(EXP_TYPE& ret);

        virtual void Exec(BYTE op, const EXP_ITEM& item, EXP_ITEM& ret, EXPFN_PARAMS);
};

// Выражение в скобках
class EXP_BRACKET : public EXP_VAR {
        EXP_VAR *Param;
    public:
        EXP_BRACKET();
        EXP_BRACKET(const EXP_TYPE& tp);
        virtual ~EXP_BRACKET();

        void SetParam(EXP_VAR* var) {
            Param = var;
        };
        EXP_VAR* GetParam(void) {
            return Param;
        };

        bool SetType(const EXP_TYPE& type);
        bool FindType(EXP_TYPE& ret);

        void Exec(BYTE op, const EXP_ITEM& item, EXP_ITEM& ret, EXPFN_PARAMS);
};

class EXP_FUNC : public EXP_VAR {
friend class EXP_FUNC_REGISTRY;    
        EXP_FN_STR *FnStr;
        STRING FnName;
        EXP_VAR **Param;
        int NumParam;
        EXP_ITEM **ExecItem;
    public:
        EXP_FUNC(const char* name, EXP_FN_STR* fstr);
        virtual ~EXP_FUNC();
        void AddParameter(EXP_VAR* var);

        //int GetNumParam(void) { return NumParam; };
        bool CheckNumParam(void);

        bool SetType(const EXP_TYPE& type);

        void Exec(BYTE op, const EXP_ITEM& item, EXP_ITEM& ret, EXPFN_PARAMS);
};

class EXP_BOOL : public EXP_VAR {
        EXP_VAR **Param;
        BYTE *Operation;
        int NumParam;
        EXP_ITEM **ExecItem;
    public:
        EXP_BOOL();
        virtual ~EXP_BOOL();
        void AddParameter(EXP_VAR* var, BYTE op);

        int GetNumParam(void) {
            return NumParam;
        };

        bool SetType(const EXP_TYPE& type);

        void Exec(BYTE op, const EXP_ITEM& item, EXP_ITEM& ret, EXPFN_PARAMS);
};

class EXP_CONST : public EXP_VAR {
        STRING Param;
        bool StString;
        double DigitParam;
    public:
        EXP_CONST(bool st_string, const char* str);
        virtual ~EXP_CONST();

        bool SetType(const EXP_TYPE& type);

        double GetDigit(void) const;

        void Exec(BYTE op, const EXP_ITEM& item, EXP_ITEM& ret, EXPFN_PARAMS);
};

class CExpression {
        EXP_VAR *Expression;
        bool RetErr;
        void ClearExp(EXP_VAR *var);
        int StErr;
        // Вся строка в нижний регистр
        // Вырезаем пробелы и переводы строки
        // Заменяем И и ИЛИ на && и || соответственно
        bool PrepareBuffer(const char* inbuf, STRING& out);
        bool CheckSpWord(const char *buf, int &ptr, STRING *out);
        // Заменяем знаки операции на коды
        void ConvertBuffer(const char* inbuf, STRING& out);
        // Проверяем, остались ли кривые символы
        bool CheckBuffer(const char* buf);

        //int CheckTypeOp(const char *buf, int &len);
        int FindTypeOp(const char* buf);
        EXP_VAR* ParseLine(const char* line);
        BYTE FindOperator(const char* buf, BYTE mask, int& retptr);

        bool fnSetExpression(const char* str);
    public:
        CExpression();
        virtual ~CExpression();

        bool SetExpression(const char* str);
        int GetErrCode(void) {
            return StErr;
        };

        bool Expr(EXPFN_PARAMS);

        void ReleaseExpr(CExpression* expr);
        void SetNewExpression(EXP_VAR* expr);
};

// Типы результата и параметров
// EXPTYPE_DIGIT  - число (любое. int, float, double)
// EXPTYPE_STRING - строка
// EXPTYPE_VECTOR - вектор
// EXPTYPE_LIST   - лист

extern EXP_FN_STR *expFns;
extern void (*ExecExpFunction)(EXP_FN_STR *FnStr, EXP_ITEM& item, EXPFN_PARAMS, EXP_ITEM **Par);

extern EXP_FN_STR emptyExpFns[];
void EmptyExecExpFunction(EXP_FN_STR *FnStr, EXP_ITEM& item, EXPFN_PARAMS, EXP_ITEM **Par);

// registry of all existing EXP_FUNC: it's used to reset/clear FnStr pointer
// when plugin with expFns implementation is initialized/destroyed 
struct EXP_FUNC_REGISTRY : public set<EXP_FUNC *> {
    void backup();
    void recover();
};
extern EXP_FUNC_REGISTRY *expFuncRegistry;

#endif
