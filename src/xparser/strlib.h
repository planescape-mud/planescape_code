#ifndef STRLIB_H
#define STRLIB_H

extern bool char_isrus(char s);   // Русский символ
extern bool char_islat(char s);   // Латинский символ
extern bool char_ischar(char s);  // Русский или латинский символ
extern bool char_isdigit(char s);  // Цифра
extern bool char_isspace(char s);
extern bool char_isupper(char s);
extern bool char_islower(char s);
extern char TOUPPER(char s);
extern char TOLOWER(char s);

// Базовый класс для работы со строками
class STRING {
        char *String;
        int Length;
        void Allocated(int sz);
    public:
        // Конструкторы
        STRING();       // Создать пустой объект
        STRING(int len);     // Выделить классу len байт памяти
        STRING(const char *str);   // Создать объект содержащий строку
        STRING(const char *str, int len); // Создать объект содержащий строку длиной len
        STRING(const STRING &str);  // Копия объекта
        virtual ~STRING();    // Деструктор

        bool IsEmpty(void);  // Если объект пуст или строка нулевая, то true
        void Empty(void);  // Очистить объект

        int GetLength(void) const;   // Получить длину строки
        char* GetString(void) const;  // Получить указатель
        char operator [](int index) const; // Получить символ
        char GetAt(int index) const;
        void SetAt(int index, char ch);

        void DeleteChar(int index);

        void Format(const char*, ...);  // Форматированная запись

        // Операции работы со строками
        void operator =(const STRING &str);
        void operator +=(const STRING &str);
        void operator =(const char* str);
        void operator +=(const char* str);
        void operator +=(char s);
        STRING operator +(const STRING &str);
        // Функции сравнения
        int Compare(const STRING& str) const;
        int Compare(const char* str) const;

        void MakeUpper(void); // Вся строка в верхний регистр
        void MakeLower(void); // Вся строка в нижний регистр
        void StrFormat(void); // Вся строка в нижний, а первый символ в верхний
        void CAP(void);   // Первый символ в верхний регистр
        void DAP(void);   // Первый символ в нижний регистр
        bool StrFromFile(const char* fname); // Прочитать из файла
        void PruneCRLF(void); // Удалить символы 0A 0D в конце текста

        STRING operator >>(STRING &str);

        int Atoi(void) const;
        int AHtoi(void) const;
        bool Atob(void) const;
        void KillSpaces(void); // Удалить пробелы в начале и в конце строки
};

bool operator ==(const STRING &str1, const STRING &str2);
bool operator ==(const STRING &str1, const char* str2);
bool operator ==(const char* str1, const STRING &str2);
bool operator !=(const STRING &str1, const STRING &str2);
bool operator !=(const STRING &str1, const char* str2);
bool operator !=(const char* str1, const STRING &str2);
bool operator <(const STRING &str1, const STRING &str2);
bool operator <(const STRING &str1, const char* str2);
bool operator <(const char* str1, const STRING &str2);
bool operator >(const STRING &str1, const STRING &str2);
bool operator >(const STRING &str1, const char* str2);
bool operator >(const char* str1, const STRING &str2);
bool operator <=(const STRING &str1, const STRING &str2);
bool operator <=(const STRING &str1, const char* str2);
bool operator <=(const char* str1, const STRING &str2);
bool operator >=(const STRING &str1, const STRING &str2);
bool operator >=(const STRING &str1, const char* str2);
bool operator >=(const char* str1, const STRING &str2);

extern const char* FormatStringForSave(const char* str);

#endif
