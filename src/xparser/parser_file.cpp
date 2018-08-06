
#include "parser_file.h"
#include "parser_utils.h"

#define MAX_LINE 80

///////////////////////////////////////////////////////////////////////////////
CParserFile::CParserFile() {
    fd = -1;
}
///////////////////////////////////////////////////////////////////////////////
CParserFile::~CParserFile() {
    Close();
}
///////////////////////////////////////////////////////////////////////////////
void CParserFile::Close(void) {
    SaveOld();
    if (fd >= 0)
        close(fd);
    fd = -1;
}
///////////////////////////////////////////////////////////////////////////////
bool CParserFile::Open(const char* name, bool Clear) {
    Close();
    if (Clear)
        unlink(name);
    fd = open(name, O_WRONLY | O_CREAT | O_APPEND, 00664);
    if (fd < 0) {
        syserr("open(%s)", name);
        return false;
    }
    Level = 0;
    Buffer[0] = 0;
    return true;
}
///////////////////////////////////////////////////////////////////////////////
void CParserFile::SaveOld(void) {
    if (fd < 0)
        return;
    if (strlen(Buffer) == 0)
        return;
    strcat(Buffer, "\n");
    write(fd, Buffer, strlen(Buffer));
    Buffer[0] = 0;
}
///////////////////////////////////////////////////////////////////////////////
void CParserFile::StartLevel(void) {
    SaveOld();
    char b[] = "\n;-----------------------------------------------------------------------\n";
    write(fd, b, strlen(b));
    Level = 0;
}
///////////////////////////////////////////////////////////////////////////////
void CParserFile::AddCommand(const char *format, ...) {
    char com[1000];
    va_list args;
    va_start(args, format);
    vsprintf(com, format, args);
    va_end(args);

    char nl[] = "\n";
    if (Level == 0) {
        write(fd, com, strlen(com));
        write(fd, nl, strlen(nl));
        Level ++;
        return;
    }
    if (strlen(Buffer) == 0) {
        strcat(Buffer, GetNumSp());
        strcat(Buffer, com);
        if (strlen(Buffer) >= MAX_LINE)
            SaveOld();
        return;
    }
    // Продолжаем писать в строку
    if (strlen(Buffer) + 3 + strlen(com) > MAX_LINE) {
        SaveOld();
        strcat(Buffer, GetNumSp());
        strcat(Buffer, com);
        if (strlen(Buffer) >= MAX_LINE)
            SaveOld();
        return;
    }
    strcat(Buffer, " : ");
    strcat(Buffer, com);
    if (strlen(Buffer) >= MAX_LINE)
        SaveOld();
}
///////////////////////////////////////////////////////////////////////////////
char* CParserFile::GetNumSp(void) {
    static char buf[100];
    memset(buf, ' ', sizeof(buf));
    int len = Level * 4 - 2;
    if (len < 0)
        len = 0;
    buf[len] = 0;
    return buf;
}
///////////////////////////////////////////////////////////////////////////////
void CParserFile::EndLine(void) {
    SaveOld();
}
///////////////////////////////////////////////////////////////////////////////
int CParserFile::GetLevel(void) {
    return Level;
}
///////////////////////////////////////////////////////////////////////////////
void CParserFile::NextLevel(void) {
    Level ++;
}
///////////////////////////////////////////////////////////////////////////////
void CParserFile::CloseStruct(void) {
    if (Level - 1 == 0) {
        SaveOld();
        return;
    }
    if (strlen(Buffer) == 0 || strlen(Buffer) + 2 >= MAX_LINE) {
        SaveOld();
        AddCommand(")");
        SaveOld();
        Level --;
        return;
    }
    strcat(Buffer, " )");
    SaveOld();
    Level --;
}
///////////////////////////////////////////////////////////////////////////////
void CParserFile::AddString(const char* name, char *svpar) {
    SaveOld();
    WordPos = 0;
    bool firstln = true;
    int SvLen;
    char *SvWord = GetWord(svpar, SvLen);
    while (1) {
        if (firstln) {
            if (strlen(Buffer) != 0 &&
                    strlen(Buffer) + strlen(name) + SvLen + 3 + 1 + 2 >= MAX_LINE)
                SaveOld();
            if (strlen(Buffer) != 0)
                strcat(Buffer, " : ");
            else
                strcat(Buffer, GetNumSp());
            strcat(Buffer, name);
            strcat(Buffer, " ");
            firstln = false;
        } else {
            strcat(Buffer, GetNumSp());
            strcat(Buffer, "   ");
        }
        strcat(Buffer, "\"");
        // Готовы к формированию текста
        strncat(Buffer, SvWord, SvLen);
        while (1) {
            SvWord = GetWord(svpar, SvLen);
            if (SvLen == 0)
                break;
            if (strlen(Buffer) + SvLen >= MAX_LINE)
                break;
            strncat(Buffer, SvWord, SvLen);
        }
        strcat(Buffer, "\"");
        if (SvLen == 0)
            break;
        SaveOld();
    }
    SaveOld();
}
///////////////////////////////////////////////////////////////////////////////
char* CParserFile::GetWord(char *par, int &len) {
    int pos = WordPos;
    while (par[pos] == ' ')
        pos ++;
    while (par[pos] != ' ' && par[pos] != 0)
        pos ++;
// int i = pos;
// while(par[i] == ' ')
//  i ++;
// if(par[i] == 0)
//  pos = i;
    while (par[pos] == ' ')
        pos ++;
    len = pos - WordPos;
    char *ret = &par[WordPos];
    WordPos = pos;
    return ret;
}
///////////////////////////////////////////////////////////////////////////////
void CParserFile::AddScripts(const char* name, int* script, int number) {
    char buf[100];
    sprintf(buf, "%d", script[0]);
    if (strlen(Buffer) != 0 &&
            strlen(Buffer) + 3 + strlen(name) + 1 + strlen(buf) >= MAX_LINE)
        SaveOld();

    if (strlen(Buffer) == 0)
        strcat(Buffer, GetNumSp());
    else
        strcat(Buffer, " : ");

    strcat(Buffer, name);
    strcat(Buffer, " ");
    strcat(Buffer, buf);

    for (int i = 1; i < number; i ++) {
        sprintf(buf, "%d", script[i]);
        if (strlen(Buffer) != 0 && strlen(Buffer) + 1 + strlen(buf) >= MAX_LINE)
            SaveOld();
        if (strlen(Buffer) == 0) {
            strcat(Buffer, GetNumSp());
            strcat(Buffer, name);
            strcat(Buffer, " ");
        } else
            strcat(Buffer, "+");
        strcat(Buffer, buf);
    }
}
///////////////////////////////////////////////////////////////////////////////
void CParserFile::NewStrList(void) {
    StrListOut = false;
}
///////////////////////////////////////////////////////////////////////////////
void CParserFile::AddStrList(const char *name, char* par) {
    if (!StrListOut) {
        if (strlen(Buffer) != 0 &&
                strlen(Buffer) + 3 + strlen(name) + 1 + strlen(par) >= MAX_LINE)
            SaveOld();
        if (strlen(Buffer) == 0)
            strcat(Buffer, GetNumSp());
        else
            strcat(Buffer, " : ");
        strcat(Buffer, name);
        strcat(Buffer, " ");
        strcat(Buffer, par);
        StrListOut = true;
        return;
    }
    if (strlen(Buffer) != 0 && strlen(Buffer) + 1 + strlen(par) >= MAX_LINE)
        SaveOld();
    if (strlen(Buffer) == 0) {
        strcat(Buffer, GetNumSp());
        strcat(Buffer, name);
        strcat(Buffer, " ");
    } else
        strcat(Buffer, "+");
    strcat(Buffer, par);
}
///////////////////////////////////////////////////////////////////////////////
