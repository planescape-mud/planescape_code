#include "structs.h"
#include "utils.h"
#include "interpreter.h"
#include "comm.h"
#include "handler.h"
#include "spells-decl.h"
#include "gameconfig.h"
#include "db.h"

ACMD(do_save);
void msg_return(struct char_data *ch, int load_room);

void remort_char(struct char_data *ch) 
{
    while (ch->affected) 
        affect_remove(ch, ch->affected);

    //снимаем весь стафф
    for (int i = 0; i < NUM_WEARS; i++)
        if (GET_EQ(ch, i))
            obj_to_char(unequip_char(ch, i), ch);

    //сбрасываем заклинания
    for (int i = 1; i <= MAX_SPELLS; i++)
        GET_SPELL_TYPE(ch, i) = 0;

    //сбрасываем умения
    for (int i = 1; i <= MAX_SKILLS; i++) {
        SET_SKILL(ch, i) = 0;
        SET_SKILL_LEVEL(ch, i, 0);
    }

    //сбрасываем историю получения уровней, оставил 60ур, дабы старые персы не баговали
    for (int i = 1; i <= 60; i++)
        GET_HLEVEL(ch, i) = CLASS_UNDEFINED;

    //удаляем классы
    for (int i = 0; i < NUM_CLASSES; i++)
        del_class(ch, i);

    //убираем запомненые места
    for (int count = 0;count <= GET_MAXMEM(ch); count++)
        del_memory(ch, count);

    //убивает группу и последователей
    if (ch->followers || ch->master)
        die_follower(ch);

    //устанавливаем уровень
    GET_LEVEL(ch) = 0;

    //устанавливаем опыт
    GET_EXP(ch) = 0;

    //пересчитываем стартовые параметры
    check_stats(ch);

    //перекраска глаз от нового алигмента
    GET_EYES(ch) = generate_eyes(ch);

    //устанавливаем хп, ману, бодрость, клас защиты.
    GET_HIT(ch) = GET_INIT_HIT(ch);
    GET_MOVE(ch) = GET_MAX_MOVE(ch);
    GET_MANA(ch) = GET_INIT_MANA(ch);
    GET_WIMP_LEV(ch) = 0;
    GET_AC(ch) = 0;
    GET_REMORT(ch) = GET_REMORT(ch) + 1;
    ch->Questing.count  = 0; //сброс квестов

    //Морченых героев по истечению времени не удаляем
    SET_BIT(PLR_FLAGS(ch, PLR_NODELETE), PLR_NODELETE);
    CLR_GOD_FLAG(ch, GF_REMORT);

    // TODO: сбросить возраст? (time.played) Виталий: думаю лучше продолжать
    // TODO: добавить морты в скрипты
    // TODO: добавить смену религий
}

ACMD(do_remort)
{
    if (IS_NPC(ch) || IS_IMMORTAL(ch))
    {
        send_to_char("Вам это, похоже, совсем ни к чему.\r\n", ch);
        return;
    }

    if (!IS_MAX_EXP(ch))
    {
        send_to_char("Вы еще не достигли своего максимума.\r\n", ch);
        return;
    }

    if (RENTABLE(ch)) 
    {
        send_to_char("Вы не можете перевоплотиться в связи с боевыми действиями.\r\n", ch);
        return;
    }

    if (!subcmd) 
    {
        send_to_char("Если вы так настойчивы в желании начать все заново -\r\n" "наберите <перевоплотиться> полностью.\r\n", ch);
        return;
    }

    log("Remort %s", GET_NAME(ch));
    act("$n вспыхнул$g ослепительным пламенем и пропал$g!\r\n", FALSE, ch, 0, 0, TO_ROOM);
    
    remort_char(ch);

    //телепортируем героя из текущей комнаты в стартовую(потом поменяем ее)
    char_from_room(ch);
    char_to_room(ch, real_room(mortal_start_room));
    look_at_room(ch, 0);

    // выдать начальную экипировку
    //do_newbie(ch);

    // сохраниться
    do_save(ch, NULL, 0, 0, 0);
    
    msg_return(ch, ch->in_room);
    act("\r\n&WВаше тело обновилось!&n", FALSE, ch, 0, 0, TO_CHAR);
}