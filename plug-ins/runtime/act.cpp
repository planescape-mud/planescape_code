
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "spells.h"
#include "handler.h"
#include "case.h"

#define ACT_ERROR_NUM "<?>"
#define ACT_MAX_ARGS 40
#define ACT_CH   'м'
#define ACT_OBJ  'п'
#define ACT_NUM  'ч'
#define ACT_TXT  'т'
#define ACT_INP  'р'
#define ACT_RAW  'ц'
#define ACT_LST  'с'
#define ACT_BUF  'б'

#define TKN_CH   '$'
#define TKN_OBJ  '@'
#define TKN_NUM  '#'
#define TKN_TXT  '%'

#define LOG_AND  '*'
#define LOG_OR   '+'

#define SEND_OK(ch)      ((ch->desc || (to_sleeping && (!IS_AFFECTED(ch,AFF_MEDITATION) || AWAKE(ch)))) && !PLR_FLAGGED(ch, PLR_WRITING))



const char *pron_nom[] = { "оно", "он", "она", "они" };
const char *pron_gen[] = { "его", "его", "ее", "их" };
const char *pron_dat[] = { "ему", "ему", "ей", "им" };
const char *pron_acc[] = { "его", "его", "ее", "их" };
const char *pron_ins[] = { "им", "им", "ею", "ими" };
const char *pron_pre[] = { "нем", "нем", "ней", "них" };

class act_data {
  public:
    act_data()
    :type(0)
    , show_self(false)
    , show_room(false)
    , buf_only(false)
    , list_type(0)
    , list_num(0)
    , buf(NULL)
    , target(NULL)
    , target_alt(NULL)
    , value(0) {
    } int type;
    bool show_self;
    bool show_room;
    bool buf_only;
    int list_type;
    int list_num;
    char *buf;

    void *target;
    void *target_alt;
    long int value;
};


/* Различные дефайны */

#define ACT_CAP_VARS \
    char *cap = NULL;  \
    bool single = true

#define ACT_CAP_PREPARE      \
    do if (**in == '`') {      \
            if (*(++(*in)) == '`') { \
                single = false;        \
                (*in)++;               \
            }                        \
            cap = *out;              \
        } while(0)

#define ACT_CAP_PROCESS      \
    do if (cap) {              \
            if (!single) {           \
                while (cap != *out) {  \
                    *cap = UPPER(*cap);            \
                    cap++;               \
                }                      \
            }                        \
            *cap = UPPER(*cap);                \
        } while(0)

/* Returns visible gender of a character: if invisible then masculine */
#define ACT_CH_SEX(ch, vis, to) ((vis) ? (IS_DARK(IN_ROOM(ch)) && IS_DARK(IN_ROOM(to)) && !CAN_SEE_IN_DARK_ALL(to) ? get_sex_infra(ch) : GET_SEX(ch)) : SEX_MALE)

/* Returns visible gender of an object: if invisible then neuter */
#define ACT_OBJ_SEX(obj, vis) ((vis) ? GET_OBJ_SEX(obj) : SEX_NEUTRAL)

#define ACT_ESEP(i) (*(i) == ',')
#define ACT_ABRT(i) (!*(i) || *(i) == ')')

/*****************************************************************************/
void act_parse_ending(char **out, const char **in, int sex)
{
    const char *t = *in + 1;

    switch (sex) {
        default:
        case SEX_POLY:
            while (!ACT_ABRT(t) && !ACT_ESEP(t))
                t++;
            if (*t == ',')
                t++;
        case SEX_NEUTRAL:
            while (!ACT_ABRT(t) && !ACT_ESEP(t))
                t++;
            if (*t == ',')
                t++;
        case SEX_FEMALE:
            while (!ACT_ABRT(t) && !ACT_ESEP(t))
                t++;
            if (*t == ',')
                t++;
        case SEX_MALE:
            while (!ACT_ABRT(t) && !ACT_ESEP(t)) {
                *((*out)++) = *t;
                t++;
            }
            break;
    }
    while (*t && *t != ')')
        t++;
    if (*t == ')')
        t++;
    *in = t - 1;                /* it will be incremented */
}

/*****************************************************************************/

int find_ending_num(int val)
{
    if (val < 0)
        val = -val;

    if ((val % 100 >= 11 && val % 100 <= 14) || val % 10 >= 5 || val % 10 == 0)
        return 2;

    if (val % 10 == 1)
        return 0;
    else
        return 1;
}

/*****************************************************************************/

int act_get_number(const char **s)
{
    int num;

    if (!IS_DIGIT(**s))
        return 1;
    num = atoi(*s);
    do
        (*s)++;
    while (IS_DIGIT(**s));
    return num;
}

/*****************************************************************************/

void act_append(char **to, const char *from)
{
    int len = (int) strlen(from);

    memcpy(*to, from, len);
    (*to) += len;
}


/*****************************************************************************/

void act_next_arg(const char **f, int *arg_type,
                  bool * show_self, bool * show_room, int *list_type, int *list_num)
{
    const char *t = *f;
    char let;

    let = LOWER(*t);
    *show_self = *t != let;
    *arg_type = let;
    t++;

    if (let == ACT_LST) {       /* Get list type and number */
        *list_type = *t;
        if (*t)
            t++;
        *list_num = (*t == '2') ? 2 : 1;
    } else
        *list_type = '\0';

    if (*t == 'К') {
        *show_room = true;
        t++;
    } else {
        *show_room = false;
        if (*t == 'к')
            t++;
    }

    *f = t;
}

/******************************************************************************/
void act_parse_number_ending(char **out, const char **in, int number)
{
    const char *t = *in + 1;

    switch (find_ending_num(number)) {
        case 2:
            while (!ACT_ABRT(t) && !ACT_ESEP(t))
                t++;
            if (*t == ',')
                t++;
        case 1:
            while (!ACT_ABRT(t) && !ACT_ESEP(t))
                t++;
            if (*t == ',')
                t++;
        default:
        case 0:
            while (!ACT_ABRT(t) && !ACT_ESEP(t)) {
                *(*out)++ = *t;
                t++;
            }
            break;
    }
    while (*t && *t != ')')
        t++;
    if (*t == ')')
        t++;
    *in = t;
}

/****************************************************************************/
void act_number(char **out, const char **in, int number)
{
    char buf[MAX_INPUT_LENGTH];

    ACT_CAP_VARS;

    ACT_CAP_PREPARE;

    sprintf(buf, "%d", number);

    switch (**in) {
        case '(':
            act_parse_number_ending(out, in, number);
            break;
        default:
            act_append(out, buf);
            break;
    }

    ACT_CAP_PROCESS;
}

/****************************************************************************/

void act_text(char **out, const char **in, char *txt)
{
    ACT_CAP_VARS;

    ACT_CAP_PREPARE;

    act_append(out, txt);

    ACT_CAP_PROCESS;
}

/****************************************************************************/
void act_obj(char **out, const char **in, obj_data * obj, bool can_see,
             act_data * arg, char_data * to)
{
    ACT_CAP_VARS;

    ACT_CAP_PREPARE;

    if (!obj) {
        log("ОШИБКА: act_obj: Передан нулевой указатель. Строка: \"%s\".", *in);
        return;
    }

    switch (**in) {
        case '(':
            act_parse_ending(out, in, ACT_OBJ_SEX(obj, can_see));
            break;
        case 'е':
            (*in)++;
            switch (**in) {
                case 'и':
                    act_append(out, pron_nom[ACT_OBJ_SEX(obj, can_see)]);
                    break;
                case 'р':
                    act_append(out, pron_gen[ACT_OBJ_SEX(obj, can_see)]);
                    break;
                case 'д':
                    act_append(out, pron_dat[ACT_OBJ_SEX(obj, can_see)]);
                    break;
                case 'в':
                    act_append(out, pron_acc[ACT_OBJ_SEX(obj, can_see)]);
                    break;
                case 'т':
                    act_append(out, pron_ins[ACT_OBJ_SEX(obj, can_see)]);
                    break;
                case 'п':
                    act_append(out, pron_pre[ACT_OBJ_SEX(obj, can_see)]);
                    break;
                default:
                    log("ОШИБКА: act_obj: Неизвестная команда в определении "
                        "местоимения \"%c\". Строка: \"%s\".", **in, *in);
                    break;
            }
            break;
        case 'и':
            act_append(out, OBJN(obj, to, 0));
            break;
        case 'р':
            act_append(out, OBJN(obj, to, 1));
            break;
        case 'д':
            act_append(out, OBJN(obj, to, 2));
            break;
        case 'в':
            act_append(out, OBJN(obj, to, 3));
            break;
        case 'т':
            act_append(out, OBJN(obj, to, 4));
            break;
        case 'п':
            act_append(out, OBJN(obj, to, 5));
            break;

        case 'И':
            act_append(out, GET_OBJ_PNAME(obj, 0));
            break;
        case 'Р':
            act_append(out, GET_OBJ_PNAME(obj, 1));
            break;
        case 'Д':
            act_append(out, GET_OBJ_PNAME(obj, 2));
            break;
        case 'В':
            act_append(out, GET_OBJ_PNAME(obj, 3));
            break;
        case 'Т':
            act_append(out, GET_OBJ_PNAME(obj, 4));
            break;
        case 'П':
            act_append(out, GET_OBJ_PNAME(obj, 5));
            break;

        default:
            log("ОШИБКА: act_obj: Неизвестная команда \"%c\". " "Строка: \"%s\".", **in, *in);
            break;
    }

    ACT_CAP_PROCESS;
    (*in)++;
}

/****************************************************************************/

void act_ch(char **out, const char **in, char_data * ch, bool can_see,
            act_data * arg, char_data * to)
{

    ACT_CAP_VARS;

    ACT_CAP_PREPARE;

    if (!ch) {
        log("ОШИБКА: act_ch: Передан нулевой указатель. Строка: \"%s\".", *in);
        return;
    }

    switch (**in) {
        case '(':
            act_parse_ending(out, in, ACT_CH_SEX(ch, can_see, to));
            break;
        case 'е':
            (*in)++;
            switch (**in) {
                case 'и':
                    act_append(out, pron_nom[ACT_CH_SEX(ch, can_see, to)]);
                    break;
                case 'р':
                    act_append(out, pron_gen[ACT_CH_SEX(ch, can_see, to)]);
                    break;
                case 'д':
                    act_append(out, pron_dat[ACT_CH_SEX(ch, can_see, to)]);
                    break;
                case 'в':
                    act_append(out, pron_acc[ACT_CH_SEX(ch, can_see, to)]);
                    break;
                case 'т':
                    act_append(out, pron_ins[ACT_CH_SEX(ch, can_see, to)]);
                    break;
                case 'п':
                    act_append(out, pron_pre[ACT_CH_SEX(ch, can_see, to)]);
                    break;
                default:
                    log("ОШИБКА: act_ch: Неизвестная команда в определении "
                        "местоимения \"%c\". Строка: \"%s\".", **in, *in);
                    break;
            }
            break;
        case 'и':
            act_append(out, PERS(ch, to, 0));
            break;
        case 'р':
            act_append(out, PERS(ch, to, 1));
            break;
        case 'д':
            act_append(out, PERS(ch, to, 2));
            break;
        case 'в':
            act_append(out, PERS(ch, to, 3));
            break;
        case 'т':
            act_append(out, PERS(ch, to, 4));
            break;
        case 'п':
            act_append(out, PERS(ch, to, 5));
            break;

        case 'И':
            act_append(out, GET_PAD(ch, 0));
            break;
        case 'Р':
            act_append(out, GET_PAD(ch, 1));
            break;
        case 'Д':
            act_append(out, GET_PAD(ch, 2));
            break;
        case 'В':
            act_append(out, GET_PAD(ch, 3));
            break;
        case 'Т':
            act_append(out, GET_PAD(ch, 4));
            break;
        case 'П':
            act_append(out, GET_PAD(ch, 5));
            break;

        default:
            log("ОШИБКА: act_ch: Неизвестная команда \"%c\". " "Строка: \"%s\".", **in, *in);
            break;
    }

    ACT_CAP_PROCESS;
    (*in)++;

}



/****************************************************************************/
void perform_nact(const char *s, act_data * args, int num_args, char_data * to, int to_sleeping)
{

    char lbuf[MAX_STRING_LENGTH], *out = lbuf;
    char_data *ch;
    obj_data *obj;
    int num = 0, i, visible = -1;
    bool can_see;

    if (!to->desc || !SENDOK(to))
        return;

    while (*s) {
        //Обрабатываем токены
        if (*s == TKN_OBJ) {
            s++;
            num = act_get_number(&s);
            for (i = 0; i < num_args && num; i++)
                if (args[i].type == ACT_OBJ || args[i].list_type == ACT_OBJ)
                    if (!--num) {
                        obj = (obj_data *) args[i].target;
                        if (!args[i].list_type)
                            can_see = CAN_SEE_OBJ(to, obj);
                        else
                            can_see = TRUE;
                        if (*s == LOG_AND) {
                            if (!can_see)
                                return;
                            s++;
                        } else if (*s == LOG_OR) {
                            if (can_see)
                                visible = 1;
                            else if (visible == -1)
                                visible = 0;
                            s++;
                        }
                        act_obj(&out, &s, obj, can_see, &args[i], to);
                    }
        }
        //Ч
        else if (*s == TKN_NUM) {
            s++;
            num = act_get_number(&s);
            for (i = 0; i < num_args && num; i++)
                if (args[i].type == ACT_NUM)
                    if (!--num)
                        act_number(&out, &s, args[i].value);
        }
        //Т
        else if (*s == TKN_TXT) {
            s++;
            num = act_get_number(&s);
            for (i = 0; i < num_args && num; i++)
                if (args[i].type == ACT_TXT)
                    if (!--num) {
                        if (!args[i].target_alt || !PRF_FLAGGED(to, PRF_CURSES))
                            act_text(&out, &s, (char *) args[i].target);
                        else
                            act_text(&out, &s, (char *) args[i].target_alt);
                    }
        }
        //М
        else if (*s == TKN_CH || IS_DIGIT(*s)) {
            if (*s == TKN_CH)
                s++;
            num = act_get_number(&s);
            for (i = 0; i < num_args && num; i++)
                if (args[i].type == ACT_CH || args[i].list_type == ACT_CH)
                    if (!--num) {
                        ch = (char_data *) args[i].target;
                        if (!args[i].list_type)
                            can_see = CAN_SEE(to, ch);
                        else
                            can_see = TRUE;
                        if (*s == LOG_AND) {
                            if (!can_see)
                                return;
                            s++;
                        } else if (*s == LOG_OR) {
                            if (can_see)
                                visible = 1;
                            else if (visible == -1)
                                visible = 0;
                            s++;
                        }
                        act_ch(&out, &s, ch, can_see, &args[i], to);
                    }
        } else {
            *out++ = *s;
            s++;
            continue;
        }

        if (num) {
            act_append(&out, ACT_ERROR_NUM);    /* Задан несуществующий номер */
            num = 0;
        }
    }

    if (!visible)
        return;

    *out++ = '\r';
    *out++ = '\n';
    *out = '\0';

    del_spaces(lbuf);
    send_to_char(CAP(lbuf), to);

}

/***************************************************************************/
void act(const char *str, const char *format, ...)
{
    va_list l_args;
    static char curse_buf[MAX_STRING_LENGTH];
    static act_data args[ACT_MAX_ARGS];
    int num_args, i;
    char_data *tch, *to;
    const char *f;
    int show_room = -1;
    void *addr = NULL;
    bool to_sleeping = false, catch_curses = false;


    if (!str || !*str)          //Ничего не передавали?
        return;

    va_start(l_args, format);
    f = format;
    num_args = 0;
    while (num_args < ACT_MAX_ARGS && *f) {
        if (*f == 'р') {
            f++;
            catch_curses = true;
            continue;
        } else if (*f == 'к') {
            f++;
            continue;
        } else if (*f == 'К') {
            f++;
            show_room = num_args;
            continue;
        } else if (*f == '!') {
            f++;
            to_sleeping = true;
            continue;
        }
        act_next_arg(&f, &args[num_args].type,
                     &args[num_args].show_self, &args[num_args].show_room,
                     &args[num_args].list_type, &args[num_args].list_num);

        addr = va_arg(l_args, void *);

        if (!addr) {
            /* commented out to prevent spam. also, if format tokens don't correspond to params,
             * we'll get crash, not just warning, and will be able to diagnose 
             */
//            log("Вызван _act с нулевым параметром '%c' строка '%s'", args[num_args].type, str);
//            continue;
        }

        switch (args[num_args].type) {
            case ACT_CH:
                //args[num_args].target = va_arg(l_args, char_data *);
                args[num_args].target = (char_data *) addr;
                /*
                 * Correct up show_room index, it must point to first character
                 * after 'К' command:
                 */
                if (show_room > -1 && args[show_room].type != ACT_CH)
                    show_room = num_args;
                break;
            case ACT_OBJ:
                args[num_args].target = (obj_data *) addr;
                break;
            case ACT_NUM:
                args[num_args].value = (long int) addr;
                break;
            case ACT_TXT:
                //args[num_args].target = va_arg(l_args, char *);
                args[num_args].target = (char *) addr;
                if (!catch_curses)
                    args[num_args].target_alt = NULL;
                else {
                    args[num_args].target_alt = strcpy(curse_buf, (char *) args[num_args].target);
                    curses_check((char *) args[num_args].target_alt);
                    catch_curses = false;
                }
                break;
            default:
                log("ОШИБКА: act: неизвестный тип %d ('%c'). Строка: \"%s\".", args[num_args].type,
                    args[num_args].type, str);
                break;
        }
        num_args++;
    }

// ^ Цели для сообщения определили

    if (show_room == num_args) {
        log("ОШИБКА: act: Тип аргумента К задан последним в списке. Строка: \"%s\".", str);
        return;
    }
    if (show_room > -1 && args[show_room].type != ACT_CH) {
        log("ОШИБКА: act: После аргумента К не задан аргумент типа персонаж. Строка: \"%s\".", str);
        return;
    }
//Отправляем строку на обратотку
    if (show_room > -1) {
        tch = (char_data *) args[show_room].target;
        if (tch && tch->in_room != NOWHERE) {
            for (to = world[tch->in_room].people; to; to = to->next_in_room) {
                //Проверка на видимость
                for (i = 0; i < num_args; i++) {
                    if (args[i].type == ACT_CH && (char_data *) args[i].target == to)
                        if (!args[i].show_self)
                            break;
                }
                if (i == num_args)      // Все видно
                    perform_nact(str, args, num_args, to, to_sleeping);
            }
        }
    } else {
        for (i = 0; i < num_args; i++) {
            if (args[i].type == ACT_CH) {
                tch = (char_data *) args[i].target;
                if (args[i].show_self)
                    perform_nact(str, args, num_args, tch, to_sleeping);
                if (args[i].show_room && tch->in_room != NOWHERE)
                    for (to = world[tch->in_room].people; to; to = to->next_in_room)
                        if (to != tch)
                            perform_nact(str, args, num_args, to, to_sleeping);
            }
        }
    }

    va_end(l_args);
}
