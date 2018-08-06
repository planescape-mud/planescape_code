#ifndef EVENTS_DECL_H
#define EVENTS_DECL_H

#define AEVENT(function) void (function)(struct event_param_data *params)

#define STOP_NONE 0
#define STOP_HIDDEN 1
#define STOP_ALL 255

/***********************************************************************************************************************/
/* Описание системы отложенных действий */
/***********************************************************************************************************************/

struct event_item_data {
    int time;   //Время отложения события (в секундах)
    int ctime;   //Текущеее время события, уменьшает раз в секунду
    int stopflag;   //Стоп флаг события
    bool show_meter;  //Показывать actorу строку прогресса события
    bool saved;   //Флаг записи события у actor (true - записывать)
    struct event_param_data *params;
    int script;   //Вномер действия которое выполняет при событии
    AEVENT(*func);   //Название процедуры которые выполняет при событии

    struct event_item_data *next;
};

struct event_param_data {

    struct char_data *actor;      //Участник события (персонаж или монстр) может быть NULL
    struct char_data *victim; //Подчиненный события (персонаж или монстр) может быть NULL
    struct obj_data *object; //Предмет события (объект в мире) может быть NULL
    int stopflag;   //Стоп флаг события
    bool show_meter;  //Показывать actorу строку прогресса события
    bool saved;   //Флаг записи события у actor (true - записывать)
    int narg[4];   //Массив int-переменных для хранения данных
    const char *sarg;   //Строковый аргумент
    const char *action;   //Строка которая показывается у actor в локации при выполнении события
    const char *status;   //Строка которая показывается у actor в счете при выполнении события
    const char *vaction;  //Строка которая показывается у victim в локации при выполнении события
    const char *vstatus;  //Строка которая показывается у victim в счете при выполнении события
    const char *raction;  //Строка которая показывается в локацию victim (или actor) при выполнении события
    const char *rstatus;  //Строка которая показывается в локацию
    const char *sto_actor;  //Строка выводится actor при старте события
    const char *sto_victim;  //Строка выводится victim при старте события
    const char *sto_room;  //Строка выводится в локацию actor при старте события
    const char *bto_actor;  //Строка выводится actor при прерывании события
    const char *bto_victim;  //Строка выводится victim при прерывании события
    const char *bto_room;  //Строка выводится в локацию actor при прерывании события
};

extern struct event_item_data *events_list;

#endif
