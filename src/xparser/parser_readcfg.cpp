
#include "parser_readcfg.h"
#include "parser_utils.h"

///////////////////////////////////////////////////////////////////////////////
CReadCfg::CReadCfg() {
    fd = -1;
    Buffer = NULL;
    NumLine = 0;
    ReadSize = 0;
}
///////////////////////////////////////////////////////////////////////////////
CReadCfg::~CReadCfg() {
    if (fd >= 0)
        close(fd);
}
///////////////////////////////////////////////////////////////////////////////
bool CReadCfg::StartRead(const char *name) {
    fd = open(name, O_RDONLY);
    if (fd < 0) {
        syserr("Ошибка открытия файла (%s)", name);
        return false;
    }
    NumLine = 1;
    return true;
}
///////////////////////////////////////////////////////////////////////////////
int CReadCfg::GetLine(void) {
    return NumLine;
}
///////////////////////////////////////////////////////////////////////////////

void CReadCfg::StartRead(char* buf, int sz) {
    ReadBuffer = buf;
    ReadSize = sz;
    ReadPos = -1;
    NumLine = 1;
}

///////////////////////////////////////////////////////////////////////////////
char CReadCfg::GetChar(void) {
    char s = 0;
    // if(Buffer == NULL && fd < 0)   БЫЛО
    if (Buffer == NULL && fd < 0 && ReadSize == 0) // Теперь так
        return 0;
    if (Buffer == NULL) {
        // Начало изменений
        if (ReadSize == 0) {
            if (read(fd, &s, 1) <= 0)
                return 0;
        } else {
            ReadPos ++;
            if (ReadPos >= ReadSize) {
                ReadSize = 0;
                return 0;
            }
            s = ReadBuffer[ReadPos];
        }
        // Конец изменений
        if (s == 0x0A)
            NumLine ++;
    } else {
        s = *Buffer;
        if (s != 0)
            Buffer ++;
    }
    if (s == 0x0D)
        s = ' ';
    return s;
}

///////////////////////////////////////////////////////////////////////////////
void CReadCfg::BackChar(void) {
    if (Buffer != NULL) {
        Buffer --;
        return;
    }
    // Начало изменений
    if (ReadSize == 0)
        lseek(fd, -1, SEEK_CUR);
    else
        ReadPos --;
    // Конец изменений
}

///////////////////////////////////////////////////////////////////////////////
int CReadCfg::GetField(bool &NextLine, char *dst) {
    int Field = FIELD_CONST;
    char s;
    int ptr = 0;
    char PrevSym;
    NextLine = false;
    dst[0] = 0;
    while (1) {
        s = GetChar();
        if (s == ' ' || s == '\t')
            continue;
        if (s == 0x0A) {
            NextLine = true;
            continue;
        }
        if (s == ';') {
            while (1) {
                s = GetChar();
                if (s == 0x0A || s == 0)
                    break;
            }
            NextLine = true;
            continue;
        }
        break;
    }
    // Найден первый значащий символ
    if (s == 0)
        return FIELD_NONE;
    if (s == ':')
        return FIELD_NC;
    if (s == '(')
        Field = FIELD_STRUCT;
    if (s == '"')
        Field = FIELD_STRING;
    // Определились с типом поля, теперь начинаем его парсить
    /////////////////////////////////////////
    if (Field == FIELD_CONST) {
        while (1) {
            dst[ptr] = s;
            ptr ++;
            s = GetChar();
            if (s == ' ' || s == '\t')
                break;
            if (s == 0x0A) {
                BackChar();
                if (Buffer == NULL)
                    NumLine --;
                break;
            }
            if (s == 0)
                break;
        }
        dst[ptr] = 0;
        return Field;
    }
    /////////////////////////////////////////
    if (Field == FIELD_STRUCT) {
        int level = 1;
        while (1) {
            s = GetChar();
            if (s == '(') {
                level ++;
                //continue;
            }
            if (s == ')') {
                level --;
                if (level == 0)
                    break;
                //continue;
            }
            if (s == 0)
                return FIELD_ERROR;
            if (s == ';') { // Коментарий внутри структуры
                while (1) {
                    s = GetChar();
                    if (s == 0x0A || s == 0)
                        break;
                }
                dst[ptr] = 0x0A;
                ptr ++;
                continue;
            }
            if (s == '"') { // Строка врутри структуры
                PrevSym = 0;
                while (1) {
                    dst[ptr] = s;
                    ptr ++;
                    s = GetChar();
                    if (s == '"' && PrevSym != '\\') {
                        // Строковая константа окончена
                        break;
                    }
                    if (s == 0x0A) {
                        // Перевод строки внутри константы (ЗАПРЕЩЕНО)
                        return FIELD_ERROR;
                    }
                    if (s == 0) {
                        // Опс...
                        return FIELD_ERROR;
                    }
                    PrevSym = s;
                }
                dst[ptr] = s;
                ptr ++;
                continue;
            }
            dst[ptr] = s;
            ptr ++;
        }
        dst[ptr] = 0;
        return Field;
    }
    /////////////////////////////////////////
    if (Field == FIELD_STRING) {
        while (1) {
            PrevSym = 0;
            while (1) {
                s = GetChar();
                if (s == '"' && PrevSym != '\\') {
                    // Строковая константа окончена
                    break;
                }
                if (s == 0x0A) {
                    // Перевод строки внутри константы (ЗАПРЕЩЕНО)
                    return FIELD_ERROR;
                }
                if (s == 0) {
                    // Опс...
                    return FIELD_ERROR;
                }
                if (s == '"' && PrevSym == '\\') {
                    ptr --;
                    //dst[ptr] = s;
                }
                // Добавлено 14.09.2003
                // Обработка команды ПЕРЕВОД СТРОКИ
                if (s == 'n' && PrevSym == '\\') {
                    ptr --;
                    //dst[ptr] = CR;
                    s = CR;
                }
                dst[ptr] = s;
                ptr ++;
                PrevSym = s;
            }
            // Строковая константа окончена
            dst[ptr] = 0;
            // Просматриваем, может там есть продолжение
            while (1) {
                s = GetChar();
                if (s == ' '  || s == 0x0A || s == '\t')
                    continue;
                if (s == 0)
                    return Field;
                break;
            }
            if (s == '"')
                continue;
            BackChar();
            return Field;
        }
    }
    return FIELD_ERROR;
}
///////////////////////////////////////////////////////////////////////////////
void CReadCfg::SetActiveBuffer(char *buf) {
    Buffer = buf;
}
///////////////////////////////////////////////////////////////////////////////
int CReadCfg::GetCommand(char *comm, char *param) {
    bool NextLine;
    int ret;
    while (1) {
        ret = GetField(NextLine, comm);
        if (ret == FIELD_NONE || ret == FIELD_ERROR)
            return ret;
        if (ret == FIELD_NC)
            continue;
        if (ret != FIELD_CONST)
            return FIELD_ERROR;
        break;
    }
    //log("COMMAND: [%s]", comm);
    // Команду получили, теперь параметр
    ret = GetField(NextLine, param);
    if (ret == FIELD_NONE || ret == FIELD_ERROR || ret == FIELD_NC)
        return FIELD_ERROR;
    if (NextLine)
        return FIELD_ERROR;
    return ret;
}
///////////////////////////////////////////////////////////////////////////////
