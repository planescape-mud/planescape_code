#ifndef SPELLS_DECL_H
#define SPELLS_DECL_H
/* ************************************************************************
*   File: spells.h                                      Part of CircleMUD *
*  Usage: header file: constants and fn prototypes for spell system       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include <string>

//типы чудов
#define MIRC_MOVE 0
#define MIRC_DIE 1
#define MIRC_FIGHT 2
#define MIRC_HEAL 3
#define NUM_MIRC        4

//типы спеллов
#define SPL_DEF  1


#define TYPE_MESS_KILL 0
#define TYPE_MESS_HIT 1
#define TYPE_MESS_FAIL 2
#define TYPE_MESS_GOD 3

//типы спецударов
#define TYPE_SPEC_SPELL 0 //ЗАКЛИНАНИЕ
#define TYPE_SPEC_HIT 1 //РАЗРУШЕНИЕ
#define TYPE_SPEC_ROOM 2 //КОМНАТА

//типы повреждений
#define TYPE_HITS 1 //обычный удар
#define TYPE_FIRE 2 //повреждение огнем
#define TYPE_COLD 3 //холодом
#define TYPE_ELECTRO 4 //электричеством
#define TYPE_ACID 5 //кислотой
#define TYPE_POISON 6 //яд
#define TYPE_LIGHT 7 //светом
#define TYPE_DARK 8 //темнотой
#define TYPE_SKILL 9 //умение

//стоимость тренировки в exp

//определения принадлежности умения
#define SKILL_TWEAPON   0 // оружейные умения
#define SKILL_TMAGIC 1 // магические
#define SKILL_TWARRIOR 2 // воина
#define SKILL_TTHIEF 3 // вора
#define SKILL_TRANGER 4 // охотника
#define SKILL_TPRIEST 5 // магические
#define SKILL_TLANG 253 //языки
#define SKILL_TRACE 254 //расовые
#define SKILL_TOTHER 255 //остальные

//Определение сфер магов
#define SP_ENCHANTMENT 0// Зачарования
#define SP_DIVINITION 1// Прорицания
#define SP_NECROMANCE 2// Некромантия
#define SP_ALTERATION 3// Превращения
#define SP_INVOCATION 4// Проявления
#define SP_CONJURATION 5// Вызывания
#define SP_ILLUSION 6// Иллюзия
#define SP_ABJURATION 7// Отречения

//Определения сфер жрецов
#define SP_KINDNESS 8//Добро
#define SP_HEALING 9//Исцеление
#define SP_LIGHT 10//Свет
#define SP_SUN  11//Солнце
#define SP_WAR  12//Война
#define SP_PROTECTION 13//Защита
#define SP_NATURE 14//Природа
#define SP_SUCCESS 15//Удача
#define SP_TRAVEL 16//Путешествия
#define SP_FORCE 17//Сила
#define SP_MAGIC 18//Магия
#define SP_EVIL  19//Зло
#define SP_DEATH 20//Смерть
#define SP_DESTRUCTION 21//Разрушения
#define NUM_SPHERE 22


//определение уровня специализации
#define S_BASIC         0 // основной
#define S_NOVICE        1 // новичок
#define S_SPEC          2 // специалист
#define S_MASTER        3 // мастер
#define S_PROFES        4 // профессионал
#define S_GRANDM        5 // гранд мастер


#define DEFAULT_STAFF_LVL       12
#define DEFAULT_WAND_LVL        12
#define CAST_UNDEFINED  -1
#define CAST_SPELL      0
#define CAST_POTION     1
#define CAST_WAND       2
#define CAST_STAFF      3
#define CAST_SCROLL     4
#define CAST_WEAPON 5
#define CAST_RUNES  6

#define MTYPE_NEUTRAL (1 << 0)
#define MTYPE_AIR     (1 << 1)
#define MTYPE_FIRE    (1 << 2)
#define MTYPE_WATER   (1 << 3)
#define MTYPE_EARTH   (1 << 4)

#define MAG_DAMAGE          (1 << 0)
#define MAG_AFFECTS         (1 << 1)
#define MAG_UNAFFECTS   (1 << 2)
#define MAG_POINTS          (1 << 3)
#define MAG_ALTER_OBJS  (1 << 4)
#define MAG_GROUPS          (1 << 5)
#define MAG_MASSES          (1 << 6)
#define MAG_AREAS           (1 << 7)
#define MAG_SUMMONS         (1 << 8)
#define MAG_CREATIONS   (1 << 9)
#define MAG_MANUAL          (1 << 10)
#define MAG_ALL_AREAS          (1 << 11)

#define NPC_DAMAGE_PC           (1 << 16)
#define NPC_DAMAGE_PC_MINHP     (1 << 17)
#define NPC_AFFECT_PC           (1 << 18)
#define NPC_AFFECT_PC_CASTER    (1 << 19)
#define NPC_AFFECT_NPC          (1 << 20)
#define NPC_UNAFFECT_NPC        (1 << 21)
#define NPC_UNAFFECT_NPC_CASTER (1 << 22)
#define NPC_DUMMY               (1 << 23)
#define NPC_CALCULATE           (0xff << 16)
/***** Extra attack bit flags */
#define EAF_PARRY       (1 << 0)
#define EAF_BLOCK       (1 << 1)
#define EAF_TOUCH       (1 << 2)
#define EAF_CRITIC      (1 << 3)
#define EAF_DEVIATE     (1 << 4)
#define EAF_NOCRITIC    (1 << 5)
#define EAF_ADDSLASH_LEFT (1 << 6)
#define EAF_SLOW        (1 << 7)
#define EAF_ADDSHOT (1 << 8)
#define EAF_ADDSLASH (1 << 9)
#define EAF_FIRST       (1 << 10)
#define EAF_SECOND      (1 << 11)
#define EAF_STAND       (1 << 13)
#define EAF_USEDRIGHT   (1 << 14)
#define EAF_USEDLEFT    (1 << 15)
#define EAF_MULTYPARRY  (1 << 16)
#define EAF_SLEEP       (1 << 17)
#define EAF_CACT_RIGHT (1 << 18)
#define EAF_CACT_LEFT (1 << 19)

#define TYPE_NOPARRY                -2
#define TYPE_UNDEFINED              -1
#define SPELL_RESERVED_DBC          0  /* SKILL NUMBER ZERO -- RESERVED */

/* PLAYER SPELLS TYPES */
#define   SPELL_KNOW   (1 << 0)
#define   SPELL_TEMP   (1 << 1)
#define   SPELL_POTION (1 << 2)
#define   SPELL_WAND   (1 << 3)
#define   SPELL_SCROLL (1 << 4)
#define   SPELL_ITEMS  (1 << 5)
#define   SPELL_RUNES  (1 << 6)

#define   ITEM_RUNES       (1 << 0)
#define   ITEM_CHECK_USES  (1 << 1)
#define   ITEM_CHECK_LAG   (1 << 2)
#define   ITEM_CHECK_LEVEL (1 << 3)
#define   ITEM_DECAY_EMPTY (1 << 4)

#define   MI_LAG1s       (1 << 0)
#define   MI_LAG2s       (1 << 1)
#define   MI_LAG4s       (1 << 2)
#define   MI_LAG8s       (1 << 3)
#define   MI_LAG16s      (1 << 4)
#define   MI_LAG32s      (1 << 5)
#define   MI_LAG64s      (1 << 6)
#define   MI_LAG128s     (1 << 7)
#define   MI_LEVEL1      (1 << 8)
#define   MI_LEVEL2      (1 << 9)
#define   MI_LEVEL4      (1 << 10)
#define   MI_LEVEL8      (1 << 11)
#define   MI_LEVEL16     (1 << 12)

/* PLAYER SPELLS -- Numbered from 1 to MAX_SPELLS */
#define NUM_SPELLZ  142
#define NUM_SKILLZ  160

//новые заклинания
#define SPELL_IDENTIFY   1 //идентификация
#define SPELL_RECALL   2 //возврат
#define SPELL_CAST_HOLD   3 //вызов паралича

//сфера добра
#define SPELL_PROTECT_FROM_EVIL  10 //защита от зла
#define SPELL_CONSECRATE  11 //освящение
#define SPELL_CIRCLE_AGANIST_EVIL 12 //круг защиты от зла
#define SPELL_HOLY_SMITE  13 //святая кара
#define SPELL_DISPEL_EVIL  14 //изгнать зло
#define SPELL_ASPECT_GOD  15 //аспект бога
#define SPELL_HOLY_WORD   16 //святое слово
#define SPELL_HOLY_AURA   17 //святая аура
#define SPELL_SUMMON_ANGEL  18 //призвать монстра сфера добра

//сфера света
#define SPELL_ENDURE_ELEMENTS  20 //защита от стихии
#define SPELL_HEAT_METALL  21 //разогреть метал
#define SPELL_SEARING_LIGHT  22 //опаляющий луч
#define SPELL_FIRE_SHIELD  23 //огненый щит
#define SPELL_FLAME_STRIKE  24 //огненный удар
#define SPELL_FIRE_SEEDS  25 //огненные семена
#define SPELL_SUNBEAM   26 //луч света
#define SPELL_SUNBURST   27 //выспышка света
#define SPELL_FIRE_CROWN  28 //огненная корона
#define SPELL_PRISMA_WALL  29 //призматическая стена

//сфера исцеления
#define SPELL_CURE   30 //лечение ран
#define SPELL_REMOVE_ILLNESS  31 //лечеие болезни
#define SPELL_CURE_CRITICAL  32 //лечение смертельных ран
#define SPELL_CURE_MASS   33 //круг исцеления 
#define SPELL_HEAL   34 //исцеление
#define SPELL_HEAL_MASS   35 //массовое исцеление
#define SPELL_REMOVE_POISON  36 //лечение яда
#define SPELL_REMOVE_BLIND  37 //лечение слепоты
#define SPELL_REFRESH_MASS  38 //массовое восстановление
#define SPELL_RESSURECT   39 //воскрешение
#define SPELL_REMOVE_CURSE  123 //снять проклятье

//сфера войны
#define SPELL_MAGIC_WEAPON  40 //волшебное оружие
#define SPELL_SPIRIT_WEAPON  41 //духовное оружие
#define SPELL_MAGIC_VESTMENT  42 //волшебное облачение
#define SPELL_DIVINE_POWER  43 //божественная мощь
#define SPELL_FLAME_STRIKE_WAR  44 //удар пламени
#define SPELL_BLADE_BARRIER  45 //стена лезвий
#define SPELL_POWER_WORD_STUNE  46 //слово силы контузия
#define SPELL_POWER_WORD_BLIND  47 //слово силы слепота
#define SPELL_POWER_WORD_KILL  48 //слово силы убить

//сфера зла
#define SPELL_PROTECT_FROM_GOOD  50 //защита от добра
#define SPELL_DESECRATE   51 //осквернение
#define SPELL_CREATE_UNDEAD  52 //создать нежить
#define SPELL_UNHOLY_BLIGHT  53 //порча
#define SPELL_DISPEL_GOOD  54 //изгнать добро
#define SPELL_CREATE_SHADOW  55 //создать тень
#define SPELL_BLASPHEMY   56 //богохульство
#define SPELL_UNHOLY_AURA  57 //аура скверны
#define SPELL_SUMMON_MONSTER_2  58 //призвать монстра сфера зла

//сфера силы
#define SPELL_ENDURE_ELEMENTS_P  60 //защита от стихии_сфера войны
#define SPELL_BULL_STRENGH  61 //бычья сила
#define SPELL_MAGIC_VESTMENT_P  62 //волшебное облачение
#define SPELL_BROTH_WEAPON              63 //братство по оружию
#define SPELL_RIGHTEOUS_MIGHT  64 //праведная мощь
#define SPELL_STONE_SKIN  65 //каменая кожа
#define SPELL_GRASPING_HAND  66 //хватающая рука
#define SPELL_CLENCHED_FIST  67 //сжатый кулак
#define SPELL_CRUSHING_FIST  68 //сокрушающая рука

//сфера путешествия
#define SPELL_FASTER_MOVE  70 //быстрое передвижение
#define SPELL_LOCATE_OBJECT  71 //найти предмет
#define SPELL_LEVITATION  72 //левитация
#define SPELL_DIMENSION_DOOR  73 //призрачная дверь
#define SPELL_LOCATE_PERSON  74 //найти персону
#define SPELL_TELEPORT   75 //вспомнить место
#define SPELL_REFRESH   76 //восстановление бодрости
#define SPELL_FREE_MOVES  77 //свобода движения
#define SPELL_ASTRAL_PROJECT  78 //астральная проекция

//сфера защиты
#define SPELL_SANCTUARY               80 //святилище
#define SPELL_SHIELD_OTHER              81 //щит вампира
#define SPELL_IMUN_ELEMENTS  82 //иммунитет от стихий
#define SPELL_SAVE_WILL   83 //развитие воли
#define SPELL_NOMAGIC_FIELD  84 //поле антимагии
#define SPELL_REPULSION   85 //Отторжение
#define SPELL_MAGIC_IMMUNITY  86 //имунитет к магии
#define SPELL_MIND_BLANK  87 //Пустота разума
#define SPELL_PRISMA_SPHERE  88 //Призматическая сфера

//сфера удачи
#define SPELL_ENTROPIC_SHIELD  90 //щит энтропии
#define SPELL_AID   91 //помощь
#define SPELL_FPANTACLE   92 //пантакль фарлангуна
#define SPELL_MAGIC_PARRY  93 //отклонение магии
#define SPELL_EXPAND_SKILL  94 //увеличить знание
#define SPELL_EVIL_FATE   95 //злой рок
#define SPELL_FAST_LEARN  96 //быстрое обучение
#define SPELL_MISLEAD   97 //заблуждение
#define SPELL_MIRACLE   98 //чудо

//школа некромантии
#define SPELL_ANIMATE_ANIMAL  100 //оживить животное
#define SPELL_DETECT_UNDEAD  101 //видеть нежить
#define SPELL_PROTECT_UNDEAD  102 //защита от нежити
#define SPELL_CAUSE_FEAR  103 //нагнать страх
#define SPELL_SKULL_SNARE  104 //череп-ловушка
#define SPELL_ANIMATE_SKELET  105 //поднять скелет
#define SPELL_MAKE_SKELET  106 //разложить труп
#define SPELL_FREEZE_BLOOD  107 //охладить кровь
#define SPELL_SHADOW_DEATH  108 //тень смерти
#define SPELL_WAKEUP_DEAD  109 //поднять зомби
#define SPELL_OR   110 //образовать рану
#define SPELL_STEEL_BONES  111 //стальные кости
#define SPELL_ENERGY_UNDEAD  112 //неживая аура
#define SPELL_GHOST_FEAR  113 //призрачный ужас
#define SPELL_SLAVED_SHADOW  114 //порабощение тени
#define SPELL_MAKE_GHOLA  115 //превращение в гхолу
#define SPELL_IMPLANT_WEAPON  116 //вживить оружие
#define SPELL_IS_UNDEAD   117 //облик мертвеца
#define SPELL_BONES_WALL  118 //костяная стена
#define SPELL_BONES_PICK  119 //костяные шипы
#define SPELL_THR_DEATH   120 //предверье смерти
#define SPELL_FOUL_FLESH  121 //осквернение плоти
#define SPELL_PRISMATIC_SKIN  122 //призматическая кожа
#define SPELL_DEATH_ARROWS  124 //стрелы смерти
#define SPELL_SHADOW_PROTECT  125 //защита тени
#define SPELL_FREEDOM_UNDEAD  126 //освободить нежить
#define SPELL_MASS_FEAR   127 //массовый страх
#define SPELL_DEATH_WEAPON  128 //оружие смерти
#define SPELL_BURY   129 //упокоить
#define SPELL_POISON_FOG  130 //зловонное облако
#define SPELL_RESSURECT_NECRO  131 //вернуть дух

//Тестовые заклинания 300-350
#define SPELL_INFRAVISION  350 //видение ночью
#define SPELL_DETECT_MAGIC  351 //определение магии
#define SPELL_FLY   352 //полет
#define SPELL_INVISIBLE   353 //невидимость
//СпецЗаклинания
#define SPELL_BLIND   4 //Заклинание эффекта слепота
#define SPELL_HOLD   5 //Заклинание паралича
#define SPELL_ELEM_FIRE   6 //Защита от огня
#define SPELL_ELEM_COLD   7 //Защита от холода
#define SPELL_ELEM_ELEC   8 //Защита от электричества
#define SPELL_ELEM_ACID   9 //Защита от кислоты
//Моб заклинания
#define SPELL_DE_MIND   500 //сжечь разум

#define SPELL_DANCE   369 //эффект от танца
#define SPELL_MEDITATION                370 //медитация
#define SPELL_PRISMA_FIRE  371 //призма от огня
#define SPELL_PRISMA_COLD  372 //призма от холода
#define SPELL_PRISMA_ELEC  373 //призма от электричества
#define SPELL_PRISMA_ACID               374 //призма от кислоты
#define SPELL_PRISMA_HIT                375 //призма от удара

#define SPELL_IMM_FIRE   376 //иммунитет от огня
#define SPELL_IMM_COLD   377 //иммунитет от холода
#define SPELL_IMM_ELEC   378 //иммунитет от электричества
#define SPELL_IMM_ACID   379 //иммунитет от кислоты

#define SPELL_PREPLAGUE   380 //инкубация-чума
#define SPELL_PLAGUE   381 //болезнь-чума
#define SPELL_IMM_PLAGUE  382 //иммунитет-чума
#define SPELL_CAMOUFLAGE          384 //Маскировка
#define SPELL_HOLYLIGHT   385 //Видение
#define SPELL_COURAGE             386 //Ярость 
#define SPELL_BATTLE              387 //Эффект от битвы
#define SPELL_DRUNKED             388 //Опьянение
#define SPELL_FIRE_BLADES  389 //Огненные лезвия
#define SPELL_UNHOLY   390 //Заклинание эффекта слабости от ауры скверны
#define SPELL_GRASP   391 //несбежать
#define SPELL_ILLNESS   392 //болезнь
#define SPELL_POISON   393 //отравление
#define SPELL_STUNE   394 //контузия
#define SPELL_DB   395 //индефикация для эффектов обьекта 
//при загрузке из файла
#define SPELL_BANDAGE   396 //эффект наблюдения
#define SPELL_HIDE                397 //эффект пряток
#define SPELL_SNEAK               398 //эффект крадучести
#define SPELL_CHARM                   401 /* Reserved Skill[] DO NOT CHANGE */
#define SPELL_WATERD   402
#define SPELL_HAEMORRAGIA  403 //Заклинание кровотечение
#define SPELL_FIRE_BLADES_WEAP  404
#define SPELL_FIRE_SHIELD_WEAP  406
#define SPELL_BLADE_BARRIER_WEAP 405
#define SPELL_ENCHANT   406 /*Зачарование*/
//
#define SPELL_MAKE_GHOLA_S1  441
#define SPELL_MAKE_GHOLA_S2  442
#define SPELL_MAKE_GHOLA_S3  443
#define SPELL_MAKE_GHOLA_S4  444
#define SPELL_MAKE_GHOLA_S5  445
#define SPELL_SUMMON_HORSE  499

#define LAST_USED_SPELL                  399

/* PLAYER SKILLS - Numbered from 1 to MAX_SKILLS */
#define SKILL_THAC0                 0  /* Internal */
//#define SKILL_PROTECT               1 /**** Protect grouppers    */
#define SKILL_TOUCH                 2 /**** Touch attacker       */
#define SKILL_SHIT                  3
#define SKILL_STUPOR                5
#define SKILL_POISONED              6
//#define SKILL_SENSE               7
#define SKILL_HORSE                 8
#define SKILL_HIDETRACK             9
#define SKILL_COUNTERACT            10
#define SKILL_MAKEFOOD              11
#define SKILL_MULTYPARRY            12
#define SKILL_SAPPER            13
#define SKILL_MAKETRAP      14
#define SKILL_LEADERSHIP            20
//#define SKILL_ADDSHOT               21
//#define SKILL_AWAKE                 22
#define SKILL_IDENTIFY              23
#define SKILL_TRACKON      24
#define SKILL_ORENT      25
#define SKILL_CREATE_SCROLL         26
#define SKILL_CREATE_WAND           27
#define SKILL_LOOK_HIDE             28
#define SKILL_CRITIC                29
#define SKILL_RUNUPB                30
#define SKILL_AID                   31
#define SKILL_FIRE                  32
#define SKILL_GUARD                 33
#define SKILL_CHARM                 34
#define SKILL_SHOOT                 35
#define SKILL_HOLYLIGHT      36 /* расовое умение эйзимаров */
#define SKILL_HOLYDARK      37 /* расовое умение тифлингов */
#define SKILL_SWITCH      38 /* переключить внимание */
#define SKILL_CONCENTRATE     39 /* концентрация */
#define SKILL_ENCHANT      40 /* зачарование */
#define SKILL_LEARN                 41 /* обучение */

#define SKILL_SNAPSHOT              123 //меткий выстрел
#define SKILL_RDANCE                124 //охотничий танец
#define SKILL_CIRCLESTAB     125 //ложный выпад
#define SKILL_SHIELDHIT      126 //удар щитом
#define SKILL_BLOODLET      127 //кровопускание
#define SKILL_RUNUP                 128
#define SKILL_MIGHTHIT              129
#define SKILL_THROW                 130
#define SKILL_BACKSTAB              131 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_BASH                  132 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_ABASH                 1000 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_NBASH                 1001 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_HIDE                  133 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_KICK                  134 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_KICKB                 599 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_PICK_LOCK             135 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_PUNCH                 136 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_RESCUE                137 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_SNEAK                 138 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_STEAL                 139 /* Reserved Skill[] DO NOT CHANGE */
#define SKILL_TRACK                 140 /* Reserved Skill[] DO NOT CHANGE */
//Оружейные спеллы
#define SKILL_STAFFS                141 // Посохи
#define SKILL_AXES                  142 // Топоры
#define SKILL_SWORDS                143 // Мечи
#define SKILL_DAGGERS               144 // Кинжалы
#define SKILL_MACES                 145 // Палицы
#define SKILL_FLAILS                146 // Кистени
#define SKILL_WHIPS                 147 // Кнуты
#define SKILL_SPAEKS                148 // Копья
#define SKILL_BOWS                  149 // Луки
#define SKILL_CROSSBOWS             150 // Арбалеты
#define SKILL_BOTHHANDS             151 // Двуручное

#define SKILL_SATTACK               160
#define SKILL_DISARM                161
#define SKILL_PARRY                 162
#define SKILL_HEAL                  163
#define SKILL_TURN                  164
#define SKILL_ADDSHOT               165
#define SKILL_CAMOUFLAGE            166
#define SKILL_DEVIATE               167
#define SKILL_BLOCK                 168
#define SKILL_LOOKING               169
#define SKILL_CHOPOFF               170
//#define SKILL_REPAIR                171
#define SKILL_PRAY                  172
#define SKILL_COURAGE               173
#define SKILL_DIRFLEE               174
#define SKILL_CRASHDOOR      175
#define SKILL_FIND      176
#define SKILL_BALSAM      177

// языки
#define SKILL_LANG_COMMON         180
#define SKILL_LANG_ORC             181
#define SKILL_LANG_DWARN           182
#define SKILL_LANG_ELF             183
#define SKILL_LANG_HUMAN           184
#define SKILL_LANG_BARIAUR         185
#define SKILL_LANG_AASIMAR         186
#define SKILL_LANG_TIEFLING        187


// магические умения

//Определение сфер
#define TYPE_SPHERE_SKILL 190 //сколько нужно прибавить к номеру сферы
                              //чтобы получить номер скилла
#define SKILL_SP_ENCHANTMENT 190// Зачарования
#define SKILL_SP_DIVINITION     191// Прорицания
#define SKILL_SP_NECROMANCE 192// Некромантия
#define SKILL_SP_ALTERATION 193// Превращения
#define SKILL_SP_INVOCATION 194// Проявления
#define SKILL_SP_CONJURATION 195// Вызывания
#define SKILL_SP_ILLUSION 196// Иллюзия
#define SKILL_SP_ABJURATION 197// Отречения
#define SKILL_SP_KINDNESS 198//Добро
#define SKILL_SP_HEALING 199//Исцеление
#define SKILL_SP_LIGHT  200//Свет
#define SKILL_SP_WAR  202//Война
#define SKILL_SP_PROTECTION 203//Защита
#define SKILL_SP_NATURE  204//Природа
#define SKILL_SP_SUCCESS 205//Удача
#define SKILL_SP_TRAVEL  206//Путешествия
#define SKILL_SP_FORCE  207//Сила
#define SKILL_SP_MAGIC  208//Магия
#define SKILL_SP_EVIL  209//Зло
#define SKILL_SP_DEATH  210//Смерть
#define SKILL_SP_DESTRUCTION 211//Разрушения


/* New skills may be added here up to MAX_SKILLS (200) */


/*
 *  NON-PLAYER AND OBJECT SPELLS AND SKILLS
 *  The practice levels for the spells and skills below are _not_ recorded
 *  in the playerfile; therefore, the intended use is for spells and skills
 *  associated with objects (such as SPELL_IDENTIFY used with scrolls of
 *  identify) or non-players (such as NPC-only spells).
 */

#define SPELL_BLADE_BARRIER_OFF      501

#define TOP_SPELL_DEFINE                 399
#define TOP_SKILL_DEFINE             211

//сэйвы
#define SAV_FORT 0
#define SAV_REFL 1
#define SAV_WILL 2
#define SAV_FIRE 3
#define SAV_COLD 4
#define SAV_ELECTRO 5
#define SAV_ACID 6
#define SAV_XAOS 7
#define SAV_ORDER 8
#define SAV_NONE 9
#define SAV_POSITIVE 10
#define SAV_NEGATIVE 11
#define SAV_NONE2 12
#define SAV_POISON 13

#define TAR_IGNORE      (1 << 0) //     a
#define TAR_CHAR_ROOM   (1 << 1) //1    b
#define TAR_CHAR_WORLD  (1 << 2) //2    c
#define TAR_FIGHT_SELF  (1 << 3) //4    d
#define TAR_FIGHT_VICT  (1 << 4) //8    e
#define TAR_SELF_ONLY   (1 << 5) //16   f
#define TAR_NOT_SELF    (1 << 6) //32   g
#define TAR_OBJ_INV     (1 << 7) //64   h
#define TAR_OBJ_ROOM    (1 << 8) //128  i 
#define TAR_OBJ_WORLD   (1 << 9) //256  j  
#define TAR_OBJ_EQUIP   (1 << 10) //512 k
#define MAX_SLOT        10

struct skill_info_type {
    byte  min_position;   /* Position for caster */
    int k_improove[NUM_CLASSES][NUM_GODS];
    int ability[NUM_CLASSES][NUM_GODS];
    int learn_level[NUM_CLASSES][NUM_GODS];
    int max_percent;
    std::string name;
    int skill_type;
};

extern struct skill_info_type skill_info[];

/* Possible Targets:

   bit 0: IGNORE TARGET
   bit 1: PC/NPC in room
   bit 2: PC/NPC in world
   bit 3: Object held
   bit 4: Object in inventory
   bit 5: Object in room
   bit 6: Object in world
   bit 7: If fighting, and no argument, select tar_char as self
   bit 8: If fighting, and no argument, select tar_char as victim (fighting)
   bit 9: If no argument, select self, if argument check that it IS self.

*/

#define SPELL_TYPE_SPELL   0
#define SPELL_TYPE_POTION  1
#define SPELL_TYPE_WAND    2
#define SPELL_TYPE_STAFF   3
#define SPELL_TYPE_SCROLL  4


extern const char *unused_spellname;

extern int sphere_class[NUM_SPHERE];
extern int what_sky;
extern char cast_argument[MAX_STRING_LENGTH];

// внутренние номера некоторых популярных заклинаний
extern int spellnum_db;


#endif
