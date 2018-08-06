
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "case.h"
#include "case_table.h"

struct CTypeEnding {
    char *end[6];
};

struct CEnding {
    CTypeEnding Type[2];
    CEnding *Next;
};

struct CEnding *Ending = NULL;



///////////////////////////////////////////////////////////////////////////////
void AddLine(CEnding * End, const char *line);

void ActivateEndingTable(void)
{
    int i = 0;
    CEnding *list = NULL;

    while (tmpEnding[i][0] != 0) {      // Движемся до последней пустой строки
        CEnding *ending = new CEnding;

        AddLine(ending, tmpEnding[i]);
        if (Ending == NULL)
            list = Ending = ending;
        else {
            list->Next = ending;
            list = ending;
        }
        i++;
    }
}

///////////////////////////////////////////////////////////////////////////////
void AddLine(CEnding * End, const char *line)
{
    End->Next = NULL;
    // Количество параметров в строке всегда должно быть равно 6
    // (проверки нет)
    //char str[6][50];
    std::istrstream inLine(line);
    for (int i = 0; i < 6; i++) {
        char str[50], str0[50], str1[50];

        inLine >> str;
        //log("str: %s",str);
        std::istrstream param(str);
        param.get(str0, 50, '-');
        param.ignore(1, EOF);
        param.get(str1, 50, ' ');
        //param >> str1;
        //log("[%s] [%s]", str0, str1);
        if (str1[0] == 0)
            strcpy(str1, str0);
        //log("[%s] [%s]", str0, str1);
        if (strcmp(str0, "*") == 0)
            str0[0] = 0;
        if (strcmp(str1, "*") == 0)
            str1[0] = 0;
        //log("[%s] [%s]", str0, str1);
        End->Type[0].end[i] = new char[strlen(str0) + 1];

        strcpy(End->Type[0].end[i], str0);
        End->Type[1].end[i] = new char[strlen(str1) + 1];

        strcpy(End->Type[1].end[i], str1);
    }
}

///////////////////////////////////////////////////////////////////////////////
char *get_name_pad(const char *line, int word_case)
{
    return get_name_pad(line, word_case, PAD_MONSTER);
}

///////////////////////////////////////////////////////////////////////////////
char *get_name_pad(const char *line, int word_case, int type)
{
    extern char *ConvertEndPad(char *str, int word_case, int type);
    static char OutBuf[500];

    memset(OutBuf, 0, sizeof(OutBuf));
    std::ostrstream out(OutBuf, sizeof(OutBuf), std::ios::out);
    std::istrstream inLine(line);
    while (1) {
        char s;

        while (1) {
            if (!inLine.get(s))
                return OutBuf;
            if (s == '(' || s == '[')
                break;
            out << s;
        }
        // Начало особой зоны
        char spBuf[500];

        memset(spBuf, 0, sizeof(spBuf));
        std::ostrstream special(spBuf, sizeof(spBuf), std::ios::out);
        special << s;
        while (1) {
            if (!inLine.get(s)) {
                out << spBuf;
                return OutBuf;
            }
            if (s == ')' && spBuf[0] == '(')
                break;
            if (s == ']' && spBuf[0] == '[')
                break;
            special << s;
        }
        // Вырезали
        int tp = type;

        if (spBuf[0] == '[')
            tp = PAD_OBJECT;
        char *ret = ConvertEndPad(&spBuf[1], word_case, tp);

        if (ret == NULL)
            // Не нашли такого окончания. Оставляем то, что стояло раньше
            out << spBuf << s;
        else
            out << ret;
    }
    return OutBuf;
}

///////////////////////////////////////////////////////////////////////////////
char *ConvertEndPad(char *str, int word_case, int type)
{
    static char ret[50];
    CEnding *line = Ending;

    while (line != NULL) {
        if (strcmp(line->Type[0].end[0], str) == 0)
            return line->Type[type].end[word_case];

        line = line->Next;
    }
    // Не нашли. Может быть особый случай, заданный руками
    std::istrstream in(str);
    char s;

    for (int i = 0; i < word_case; i++) {
        // Пропускаем ненужные падежи
        while (1) {
            if (!in.get(s))
                // Нет такого падежа
                return NULL;
            if (s == ' ' || s == ',')
                break;
        }
        // Лишние пробелы нахуй
        while (in.peek() == ' ')
            in.ignore(1);
    }
    // Стоим на начале нащего падежа
    char strPad[50];

    memset(strPad, 0, sizeof(strPad));
    std::strstream pad(strPad, sizeof(strPad), std::ios::in | std::ios::out);
    while (1) {
        if (!in.get(s))
            break;
        if (s == ' ' || s == ',')
            break;
        pad << s;
    }
    if (strlen(strPad) == 0)
        return NULL;
    // Что-то прочитали
    char str_pad[2][50];

    pad.seekg(0, std::ios::beg);
    pad.get(str_pad[0], 50, '-');
    pad.ignore(1, EOF);
    pad.get(str_pad[1], 50, ' ');
    if (strlen(str_pad[1]) == 0)
        strcpy(str_pad[1], str_pad[0]);
    strcpy(ret, str_pad[type]);
    if (strcmp(ret, "*") == 0)
        ret[0] = 0;

    return ret;
}

///////////////////////////////////////////////////////////////////////////////


void cases_init()
{
    /* first in boot_db */
    log("Инициализация падежей");
    ActivateEndingTable();
}

void cases_destroy()
{
    CEnding *tmpEnding;

    while (Ending) {
        tmpEnding = Ending->Next;

        for (int i = 0; i < NUM_PADS; i++) {
            delete Ending->Type[0].end[i];
            delete Ending->Type[1].end[i];
        }

        delete Ending;

        Ending = tmpEnding;
    }
}
