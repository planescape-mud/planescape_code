#ifndef CASE_H
#define CASE_H

// ВНИМАНИЕ !
// Не переделывать значения параметров PAD.
// Это может сказаться на дельнейшей работе ПО

enum {
    PAD_IMN = 0,
    PAD_ROD,
    PAD_DAT,
    PAD_VIN,
    PAD_TVR,
    PAD_PRD
};

#define PAD_OBJECT  0
#define PAD_MONSTER  1

// Получения падежа слова или выражения
// Вызов: исходный текст, падеж, тип (одущевленный/неодушевленный)
char *get_name_pad(const char *, int word_case, int type);
char *get_name_pad(const char *, int word_case);

void ActivateEndingTable(void);

void cases_init();
void cases_destroy();

#endif
