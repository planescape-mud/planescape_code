#ifndef PARSER_ITEMS_H
#define PARSER_ITEMS_H

#include "parser_file.h"

struct CProto {
    int Type;
    char* Name;
    bool Important;
    void* List;
};

///////////////////////////////////////////////////////////////////////////////
// Прототип простой переменной (не структуры)
class CItemProto {
    private:
        struct CProto Data;
    public:
        CItemProto(int, const char*, bool, void*);
        virtual ~CItemProto();

        struct CProto* GetData(void);
        virtual bool CheckInit(void);

        int AddItem(int, const char*);
        int AddItem(int, const char*, bool);
        virtual int AddItem(int, const char*, bool, void*);
        virtual CItemProto* GetItem(int);
};
///////////////////////////////////////////////////////////////////////////////
class CItemProtoStruct : public CItemProto {
    private:
        int Number;
        CItemProto **ItemProto;
    public:
        CItemProtoStruct(const char*, bool);
        ~CItemProtoStruct();

        bool CheckInit(void);

        // Добавить элемент в структуру
        // Возвращает номер созданного элемента
        int AddItem(int, const char*, bool, void*);
        // Получить элемент структуры
        CItemProto* GetItem(int);

        CItemProto** GetAllItems(int&);
};
///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////
// Базовый класс для всех типов данных
class CItem {
    private:
        int Type;  // Тип переменной
        const char *Name;  // Имя переменой
        bool Important; // Является ли она обязательной
        void *List;  // Указатель на вспомогательные константы
    protected:
        bool GetImportant(void); // Получить important
        void *GetList(void);  // Получить список констант
    public:
        // Инициализация: тип, имя, обязательность, список констант
        CItem(int, const char*, bool, void*);
        virtual ~CItem();

        const char* GetName(void);  // Получить имя
        int GetType(void);   // Получить тип
        virtual bool SetParameter(char*) = 0; // Обработать строку текста
        virtual bool GetStatus(void) = 0;  // Прочитать статус
        virtual void SaveItem(CParserFile*) = 0; // Сохранить данные

        virtual int GetNumberItem(void);  // Получить кол-во переменных
        virtual CItem* GetItem(int);   // Прочесть нужную переменную

        // Чтение простых переменных
        virtual int GetInt(void);    // INT, LIST
	virtual long long GetLongLong(void); // LONGLONG
        virtual float GetFloat(void);   // FLOAT
        virtual char* GetString(void);   // STRING, VECTOR, RANDOM
        // SCRIPT
        virtual int* GetScript(int&);
        // STRLIST
        virtual int GetStrListNumber(void);
        virtual void GetStrList(int, int&, int&);
        // EXPRESSION
        virtual void ReleaseExpression(void*);
        virtual void* GetExpression(void);

        // Установить новые параметры
        virtual void SetParam(int);    // INT, LIST
        virtual void SetParam(long long);   // LONGLONG 
        virtual void SetParam(float);   // FLOAT
        virtual void SetParam(const char*);   // STRING, VECTOR, RANDOM
        virtual void SetParam(int, int*);  // SCRIPT
        virtual void SetParam(int, int, int); // STRLIST

        // Изменение количества элементов в TYPE_STRUCT и TYPE_STRLIST
        virtual int NewSubItem(void);
        virtual bool DeleteSubItem(int);

        // Добавить новую запись в STRLIST и SCRIPT
        virtual void AddParam(int, int);  // STRLIST
        virtual void AddParam(int);    // SCRIPT

        // Поиск элементов внутри TYPE_STRUCT
        virtual CItem* FindItem(int);   // INT, LIST
        virtual CItem* FindItem(float);   // FLOAT
        virtual CItem* FindItem(char*);   // STRING, VECTOR, RANDOM
};
///////////////////////////////////////////////////////////////////////////////
// INT
class CItemInt : public CItem {
    protected:
        int Parameter;
        bool Status;
    public:
        CItemInt(int, char*, bool, void*);
        virtual ~CItemInt();

        virtual bool SetParameter(char*);
        bool GetStatus(void);

        int GetInt(void);
        virtual void SaveItem(CParserFile*);

        void SetParam(int);  // Установить параметр руками
};
///////////////////////////////////////////////////////////////////////////////
// LONGLONG 
class CItemLongLong : public CItem {
    protected:
        long long Parameter;
        bool Status;
    public:
        CItemLongLong(char*, bool);
        virtual ~CItemLongLong();

        virtual bool SetParameter(char*);
        bool GetStatus(void);

        long long GetLongLong(void);
        virtual void SaveItem(CParserFile*);

        void SetParam(long long);  // Установить параметр руками
};

///////////////////////////////////////////////////////////////////////////////
// FLOAT
class CItemFloat : public CItem {
    protected:
        float Parameter;
        bool Status;
    public:
        CItemFloat(char*, bool);
        virtual ~CItemFloat();

        virtual bool SetParameter(char*);
        bool GetStatus(void);

        float GetFloat(void);
        virtual void SaveItem(CParserFile*);

        void SetParam(float);
};
///////////////////////////////////////////////////////////////////////////////
// STRING
class CItemString : public CItem {
    private:
        char* MakeForSave(void);
    protected:
        char *Parameter;
    public:
        CItemString(int, char*, bool, void*);
        virtual ~CItemString();

        virtual bool SetParameter(char*);
        void SetParam(const char*);
        bool GetStatus(void);

        char* GetString(void);
        virtual void SaveItem(CParserFile*);
};
///////////////////////////////////////////////////////////////////////////////
// EXPRESSION
class CItemExpr : public CItemString {
        void* Expression;
    public:
        CItemExpr(char* name);
        ~CItemExpr();

        bool SetParameter(char*);
        void ReleaseExpression(void*);
        void* GetExpression(void);
};
///////////////////////////////////////////////////////////////////////////////
// LIST
class CItemList : public CItemInt {
    public:
        CItemList(char*, bool, void*);
        virtual ~CItemList();

        bool SetParameter(char*);
        virtual void SaveItem(CParserFile*);
};
///////////////////////////////////////////////////////////////////////////////
// VECTOR
class CItemVector : public CItemString {
    private:
        int Vptr;
        int GetWord(char*);
    public:
        CItemVector(char*, bool, void*);
        virtual ~CItemVector();
        bool SetParameter(char*);
        virtual void SaveItem(CParserFile*);
};
///////////////////////////////////////////////////////////////////////////////
// SCRIPT
class CItemScript : public CItem {
    private:
        int *Script;
        int Number;
    public:
        CItemScript(char*, bool);
        virtual ~CItemScript();
        bool SetParameter(char*);
        void SetParam(int, int*);
        void AddParam(int);
        bool GetStatus(void);

        int *GetScript(int&);
        virtual void SaveItem(CParserFile*);
};
///////////////////////////////////////////////////////////////////////////////
// STRLIST
class CItemStrList : public CItem {
    private:
        bool SetComm(char*);
    protected:
        int Number;
        int *Parameters;
    public:
        CItemStrList(char*, void*);
        virtual ~CItemStrList();

        virtual bool SetParameter(char*);
        bool GetStatus(void);

        void SetParam(int, int, int);
        void AddParam(int, int);
        int NewSubItem(void);
        bool DeleteSubItem(int);

        int GetStrListNumber(void);
        void GetStrList(int, int&, int&);
        virtual void SaveItem(CParserFile*);
};
///////////////////////////////////////////////////////////////////////////////
// STRUCT
class CItemStruct : public CItem {
        int Number;
        CItem **Item;
    public:
        CItemStruct(CItemProtoStruct*);
        virtual ~CItemStruct();

        bool SetParameter(char*);  // Обработать строку текста
        bool AddParameter(int, char*, char*); // Ввод обработаной строки
        bool GetStatus(void);   // Прочитать статус
        void SaveItem(CParserFile*);  // Сохранить данные

        int GetNumberItem(void);  // Получить кол-во переменных
        CItem* GetItem(int);   // Прочесть нужную переменную
};
///////////////////////////////////////////////////////////////////////////////
// STRUCT LIST
class CItemStructList : public CItem {
    protected:
        CItemProto *Proto;

        int Number;
        CItemStruct **Item;
    public:
        CItemStructList(CItemProtoStruct*);
        CItemStructList();    // Конструктор (для основного класса)
        virtual ~CItemStructList();

        virtual bool SetParameter(char*);
        bool GetStatus(void);
        void SaveItem(CParserFile*);

        int GetNumberItem(void);  // Получить кол-во переменных
        CItem* GetItem(int);   // Прочесть нужную переменную

        // Функции ручного управления
        int NewSubItem(void);   // Создать новую копию структуры
        // Возвращает номер созданной копии
        bool DeleteSubItem(int);

        // Поиск элементов внутри TYPE_STRUCT
        CItem* FindItem(int);
        CItem* FindItem(float);
        CItem* FindItem(char*);
};

#endif
