#include "strlib.h"
#include "parser_utils.h"
#include "parser_types.h"

// Русский символ
bool char_isrus(char s) {
    BYTE cs = (BYTE)s;
    if ((cs & 0xC0) == 0xC0)
        return true;
    if (cs == 0xA3 || cs == 0xB3)
        return true;
    return false;
}

// Латинский символ
bool char_islat(char s) {
    if (s >= 'a' && s <= 'z')
        return true;
    if (s >= 'A' && s <= 'Z')
        return true;
    return false;
}

// Русский или латинский символ
bool char_ischar(char s) {
    if (char_isrus(s) || char_islat(s))
        return true;
    return false;
}

// Цифра
bool char_isdigit(char s) {
    if (s >= '0' && s <= '9')
        return true;
    return false;
}

bool char_isspace(char s) {
    if (s == ' ' || s == '\t')
        return true;
    return false;
}

bool char_isupper(char s) {
    if (char_islat(s)) {
        if (s >= 'A' && s <= 'Z')
            return true;
        return false;
    }
    if (char_isrus(s)) {
        if ((BYTE)s == 0xB3)
            return true;
        if ((BYTE)s == 0xA3)
            return false;
        if ((s & 0x20) == 0x20)
            return true;
        return false;
    }
    return false;
}

bool char_islower(char s) {
    if (char_islat(s)) {
        if (s >= 'a' && s <= 'z')
            return true;
        return false;
    }
    if (char_isrus(s)) {
        if ((BYTE)s == 0xB3)
            return false;
        if ((BYTE)s == 0xA3)
            return true;
        if ((s & 0x20) == 0x20)
            return false;
        return true;
    }
    return false;
}

char TOUPPER(char s) {
    if (char_isrus(s)) {
        if ((BYTE)s == 0xA3)
            return 0xB3;
        if ((BYTE)s == 0xB3)
            return 0xB3;
        s |= 0x20;
        return s;
    }
    return toupper(s);
}

char TOLOWER(char s) {
    if (char_isrus(s)) {
        if ((BYTE)s == 0xA3)
            return 0xA3;
        if ((BYTE)s == 0xB3)
            return 0xA3;
        s &= 0xDF;
        return s;
    }
    return tolower(s);
}

int charbuf_cmp[] = {
    31,  0,  1, 23,  4,  5, 21,  3, 22,  9, 10, 11, 12, 13, 14, 15,
    16, 32, 17, 18, 19, 20,  7,  2, 29, 28,  8, 25, 30, 26, 24, 27
};

int charcmp(char s1, char s2) {
    if (!char_isrus(s1) && !char_isrus(s2))
        return (int)s1 - (int)s2;
    if (char_isrus(s1) && !char_isrus(s2))
        return 1;
    if (!char_isrus(s1) && char_isrus(s2))
        return -1;
    // Оба символа кирилица
    int num1 = -1, num2 = -1;
    if ((BYTE)s1 == 0xA3)
        num1 = 6 + 100;
    if ((BYTE)s1 == 0xB3)
        num1 = 6;
    if ((BYTE)s2 == 0xA3)
        num2 = 6 + 100;
    if ((BYTE)s2 == 0xB3)
        num2 = 6;
    if (num1 < 0) {
        BYTE ns = (BYTE)s1;
        if (ns < 0xE0) {
            num1 = 100;
            ns += 0x20;
        }
        ns -= 0xE0;
        num1 += charbuf_cmp[ns];
    }
    if (num2 < 0) {
        BYTE ns = (BYTE)s2;
        if (ns < 0xE0) {
            num2 = 100;
            ns += 0x20;
        }
        ns -= 0xE0;
        num2 += charbuf_cmp[ns];
    }
    return num1 - num2;
}

void vstrcpy(char* dst, const char *src) {
    if (src == NULL)
        return;
    strcpy(dst, src);
}

void vstrcat(char* dst, const char *src) {
    if (src == NULL)
        return;
    strcat(dst, src);
}

int vstrcmp(const char* str1, const char* str2) {
    if (str1 == NULL && str2 == NULL)
        return 0;
    if (str1 == NULL)
        return -1;
    if (str2 == NULL)
        return 1;
    for (int i = 0; str1[i] != 0 && str2[i] != 0; i ++) {
        int st = charcmp(str1[i], str2[i]);
        if (st != 0)
            return st;
    }
    if (strlen(str1) == strlen(str2))
        return 0;
    if (strlen(str1) < strlen(str2))
        return -1;
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
// Класс работы со строками
STRING::STRING() {
    String = NULL;
    Length = 0;
}

STRING::STRING(int len) {
    String = NULL;
    Allocated(len + 1);
}

STRING::STRING(const char* str) {
    String = NULL;
    Length = 0;
    if (str) {
        Allocated(strlen(str) + 1);
        strcpy(String, str);
    }
}

STRING::STRING(const char* str, int len) {
    String = NULL;
    Allocated(len + 1);
    if (str)
        strncpy(String, str, std::min((int)strlen(str), len));
}

STRING::STRING(const STRING &str) {
    String = NULL;
    Length = 0;
    operator =(str);
}

STRING::~STRING() {
    if (String != NULL)
        delete String;
}

void STRING::Allocated(int sz) {
    if (String != NULL)
        delete String;
    String = NULL;
    Length = 0;
    if (sz == 0)
        return;
    String = new char[sz];
    if (String == NULL) {
        MEMALARM(sz);
    }
    memset(String, 0, sz);
    Length = sz;
}

void STRING::Empty() {
    Allocated(0);
}

bool STRING::IsEmpty() {
    if (String == NULL)
        return true;
    if (Length == 0 || strlen(String) == 0)
        return true;
    return false;
}

int STRING::GetLength() const {
    if (String != NULL)
        return strlen(String);
    return 0;
}

char* STRING::GetString() const {
    return String;
}

char STRING::operator [](int index) const {
    return GetAt(index);
}

char STRING::GetAt(int index) const {
    if (String == NULL)
        return 0;
    if (index < 0 || index >= Length)
        ALARM;
    return String[index];
}

void STRING::SetAt(int index, char ch) {
    if (String == NULL)
        return;
    if (index < 0 || index >= Length)
        ALARM;
    String[index] = ch;
}

void STRING::DeleteChar(int index) {
    int len = strlen(String);
    if (index < 0 || index >= len)
        ALARM;
    memmove(&String[index], &String[index+1], len - index);
}

void STRING::operator =(const STRING & str) {
    operator =(str.GetString());
}

void STRING::operator =(const char * str) {
    Empty();
    if (str == NULL)
        return;
    Allocated(strlen(str) + 1);
    strcpy(String, str);
}

void STRING::operator +=(const STRING & str) {
    operator +=(str.GetString());
}

void STRING::operator +=(const char * str) {
    if (str == NULL)
        return;
    int len = GetLength() + strlen(str) + 1;
    char *tmp = String;
    String = NULL;
    Allocated(len);
    memset(String, 0, len);
    vstrcpy(String, tmp);
    vstrcat(String, str);
    if (tmp != NULL)
        delete tmp;
}

void STRING::operator +=(char s) {
    char str[2];
    str[0] = s;
    str[1] = 0;
    operator +=(str);
}

STRING STRING::operator +(const STRING &str) {
    STRING ret = *this;
    ret += str;
    return ret;
}

void STRING::Format(const char* format, ...) {
    char buf[500];
    va_list ap;
    va_start(ap, format);
    vsnprintf(buf, 500, format, ap);
    va_end(ap);
    Allocated(strlen(buf) + 1);
    strcpy(String, buf);
}

int STRING::Compare(const STRING &str) const {
    return vstrcmp(GetString(), str.GetString());
}

int STRING::Compare(const char* str) const {
    return vstrcmp(GetString(), str);
}

void STRING::MakeUpper() {
    if (String == NULL)
        return;
    for (int i = 0; String[i] != 0; i ++)
        String[i] = TOUPPER(String[i]);
}

void STRING::MakeLower() {
    if (String == NULL)
        return;
    for (int i = 0; String[i] != 0; i ++)
        String[i] = TOLOWER(String[i]);
}

void STRING::StrFormat() {
    if (String == NULL)
        return;
    for (int i = 1; String[i] != 0; i ++)
        String[i] = TOLOWER(String[i]);
    String[0] = TOUPPER(String[0]);
}

void STRING::CAP(void) {
    String[0] = TOUPPER(String[0]);
}

void STRING::DAP(void) {
    String[0] = TOLOWER(String[0]);
}

bool STRING::StrFromFile(const char* fname) {
    Empty();
    int fd = open(fname, O_RDONLY);
    if (fd < 0)
        return false;
    struct stat bstat;
    if (fstat(fd, &bstat)) {
        int err = errno;
        close(fd);
        errno = err;
        return false;
    }
    int fsz = bstat.st_size;
    Allocated(fsz + 1);
    memset(String, 0, fsz + 1);
    if (read(fd, String, fsz) < 0) {
        int err = errno;
        close(fd);
        errno = err;
        return false;
    }
    close(fd);
    return true;
}

void STRING::PruneCRLF() {
    int sz = (int)strlen(String) - 1;
    while (sz >= 0 && (String[sz] == 0x0A || String[sz] == 0x0D)) {
        String[sz] = 0;
        sz --;
    }
}

STRING STRING::operator >>(STRING &str) {
    STRING ret;
    str.Empty();
    if (String == NULL)
        return ret;
    int ptr = 0;
    while (String[ptr] == ' ' || String[ptr] == '\t')
        ptr ++;
    while (String[ptr] != 0 && String[ptr] != ' ' && String[ptr] != '\t') {
        str += String[ptr];
        ptr ++;
    }
    while (String[ptr] == ' ' || String[ptr] == '\t')
        ptr ++;
    ret = &String[ptr];
    return ret;
}

int STRING::Atoi(void) const {
    if (String == NULL)
        return 0;
    return atoi(GetString());
}

int STRING::AHtoi(void) const {
    if (String == NULL)
        return 0;
    int ret;
    sscanf(String, "%x", &ret);
    return ret;
}

bool STRING::Atob(void) const {
    if (Atoi() == 0)
        return false;
    return true;
}

void STRING::KillSpaces(void) {
    if (IsEmpty())
        return;
    int pos = GetLength() - 1;
    while (pos >= 0 && (String[pos] == ' ' || String[pos] == '\t')) {
        String[pos] = 0;
        pos --;
    }
    if (IsEmpty())
        return;
    pos = 0;
    while (String[pos] == ' ' || String[pos] == '\t')
        pos ++;
    char *tmp = String;
    String = NULL;
    operator =(&tmp[pos]);
    delete tmp;
}

///////////////////////////////////////////////////////////////////////////////
// Функции сравнения строк
bool operator ==(const STRING &str1, const STRING &str2) {
    if (str1.Compare(str2) == 0)
        return true;
    return false;
}

bool operator ==(const STRING &str1, const char* str2) {
    if (str1.Compare(str2) == 0)
        return true;
    return false;
}

bool operator ==(const char* str1, const STRING &str2) {
    if (str2.Compare(str1) == 0)
        return true;
    return false;
}

bool operator !=(const STRING &str1, const STRING &str2) {
    if (str1.Compare(str2) != 0)
        return true;
    return false;
}

bool operator !=(const STRING &str1, const char* str2) {
    if (str1.Compare(str2) != 0)
        return true;
    return false;
}

bool operator !=(const char* str1, const STRING &str2) {
    if (str2.Compare(str1) != 0)
        return true;
    return false;
}

bool operator <(const STRING &str1, const STRING &str2) {
    if (str1.Compare(str2) < 0)
        return true;
    return false;
}

bool operator <(const STRING &str1, const char* str2) {
    if (str1.Compare(str2) < 0)
        return true;
    return false;
}

bool operator <(const char* str1, const STRING &str2) {
    if (str2.Compare(str1) >= 0)
        return true;
    return false;
}

bool operator >(const STRING &str1, const STRING &str2) {
    if (str1.Compare(str2) > 0)
        return true;
    return false;
}

bool operator >(const STRING &str1, const char* str2) {
    if (str1.Compare(str2) > 0)
        return true;
    return false;
}

bool operator >(const char* str1, const STRING &str2) {
    if (str2.Compare(str1) <= 0)
        return true;
    return false;
}

bool operator <=(const STRING &str1, const STRING &str2) {
    if (str1.Compare(str2) <= 0)
        return true;
    return false;
}

bool operator <=(const STRING &str1, const char* str2) {
    if (str1.Compare(str2) <= 0)
        return true;
    return false;
}

bool operator <=(const char* str1, const STRING &str2) {
    if (str2.Compare(str1) > 0)
        return true;
    return false;
}

bool operator >=(const STRING &str1, const STRING &str2) {
    if (str1.Compare(str2) >= 0)
        return true;
    return false;
}

bool operator >=(const STRING &str1, const char* str2) {
    if (str1.Compare(str2) >= 0)
        return true;
    return false;
}

bool operator >=(const char* str1, const STRING &str2) {
    if (str2.Compare(str1) < 0)
        return true;
    return false;
}

static char *retfsbuf = NULL;
const char* FormatStringForSave(const char* str) {
    int sz = 0;
    for (int i = 0; str[i] != 0; i ++) {
        if (str[i] == '\\' || str[i] == '\n' || str[i] == '\t' || str[i] == '"' ||
                str[i] == '#' || str[i] == '\r')
            sz ++;
        sz ++;
    }
    sz ++;
    if (retfsbuf != NULL)
        delete retfsbuf;
    retfsbuf = new char[sz];
    if (retfsbuf == NULL) {
        MEMALARM(sz);
    }
    int ptr = 0;
    for (int i = 0; str[i] != 0; i ++) {
        switch (str[i]) {
            case '\\':
                retfsbuf[ptr] = '\\';
                retfsbuf[ptr+1] = '\\';
                ptr += 2;
                break;
            case '\n':
                retfsbuf[ptr] = '\\';
                retfsbuf[ptr+1] = 'n';
                ptr += 2;
                break;
            case '\r':
                retfsbuf[ptr] = '\\';
                retfsbuf[ptr+1] = 'r';
                ptr += 2;
                break;
            case '\t':
                retfsbuf[ptr] = '\\';
                retfsbuf[ptr+1] = 't';
                ptr += 2;
                break;
            case '"':
                retfsbuf[ptr] = '\\';
                retfsbuf[ptr+1] = '"';
                ptr += 2;
                break;
            case '#':
                retfsbuf[ptr] = '\\';
                retfsbuf[ptr+1] = '#';
                ptr += 2;
                break;
            default:
                retfsbuf[ptr] = str[i];
                ptr ++;
        }
    }
    retfsbuf[ptr] = 0;
    return retfsbuf;
}
