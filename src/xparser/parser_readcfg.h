#ifndef PARSER_READCFG_H
#define PARSER_READCFG_H

#define FIELD_CONST  1
#define FIELD_STRING 2
#define FIELD_STRUCT 3
#define FIELD_NONE  0
#define FIELD_ERROR  -1
#define FIELD_NC  4

#define CR    0x0A

class CReadCfg {
        int fd;
        int NumLine;
        char *Buffer;

        char *ReadBuffer;
        int ReadSize;
        int ReadPos;

        char GetChar(void);
        void BackChar(void);
        ///////////////////////////////////////////////////////////////////////////
        // Получение очередного поля
        // Выход:   1 - Константа
        //     2 - Строковая константа
        //     3 - Структура
        //     4 - Новая команда (:)
        //     0 - Ничего не найдено
        //    -1 - Ошибка. Не законченная константа или структура
        int GetField(bool &NextLine, char *dst);

    public:
        CReadCfg();
        ~CReadCfg();

        bool StartRead(const char*);
        void StartRead(char*, int);
        int GetLine(void);
        void SetActiveBuffer(char*);
        ///////////////////////////////////////////////////////////////////////////
        // Получить команду с параметром
        // Возвращает тип параметра или ошибку
        // FIELD_CONST - Константа
        // FIELD_STRING - Строковая константа (текст в кавычках)
        // FIELD_STRUCT - Структура (текст в круглых скобках)
        // FIELD_NONE - Ничего (буфер чтения пуст или закончено чтение файла)
        // FIELD_ERROR - ОШИБКА
        int GetCommand(char *comm, char *param);
};

#endif
