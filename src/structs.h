#ifndef STRUCTS_H
#define STRUCTS_H
/*
  ����� ��� - structs.h
 */
#include <set>
#include <map>
#include <vector>
#include <string>
#include "sysdep.h"
#include "wrappertarget.h"

using namespace std;

#define _CIRCLEMUD  0x030020 /* Major/Minor/Patchlevel - MMmmPP */
//#define M_LOG2        0.955565432132456
#define M_LOG2          0.788152565432132

//#define M_LOG2          0.608152565432132
#define log2fl(x) (logf (x) / (float) M_LOG2)


#define USE_AUTOEQ  1 /* TRUE/FALSE aren't defined yet. */

#define PK_OK            0
#define PK_ONE_OVERFLOW -1
#define PK_ALL_OVERFLOW -2

#define ROOM_ID_BASE    150000
#define MOBOBJ_ID_BASE 400000
#define CASTER_PROXY 1
#define MOB_KILL_VICTIM  0
#define PC_KILL_MOB      1
#define PC_KILL_PC       2
#define PC_REVENGE_PC    3

#define MAX_PKILLER_MEM  100
#define MAX_PK_ONE_ALLOW 10
#define MAX_PK_ALL_ALLOW 100
#define MAX_PKILL_LAG    2
#define MAX_REVENGE_LAG  2*60
#define MAX_PTHIEF_LAG   10*60
#define MAX_REVENGE      2
#define EXPERT_WEAPON    80
#define MAX_DEST         50
#define MAX_REMEMBER_TELLS          15
#define MAX_QUESTS 20

#define MAX_INVALID_NAMES 16000

#define NUM_PADS 6 /* ����� ������� */

//����������� ��������������
#define FLW_NONE 0 //�����
#define FLW_GROUP 1 //���������
#define FLW_CHARM 2 //�����������
#define FLW_SUMMON 3 //���������
#define FLW_UNDEAD 4 //������
#define FLW_HORSE 5 //�������

/************************************************************************/
#define OBJ_FOOT 001
#define OBJ_SHIELD 002
#define OBJ_HITR 003
#define OBJ_FOOT_BAR 004
#define OBJ_KNIF 100

/* preamble *************************************************************/

#define NOWHERE    -1    /* nil reference for room-database */
#define NOTHING    -1    /* nil reference for objects   */
#define NOBODY     -1    /* nil reference for mobiles   */

#define SPECIAL(name) \
    int (name)(struct char_data *ch, void *me, int cmd, char *argument)

/* misc editor defines **************************************************/

/* format modes for format_text */
#define FORMAT_INDENT   (1 << 0)

#define KT_ALT        1
#define KT_WIN        2
#define KT_WINZ       3
#define KT_WINZ6      4
#define KT_LAST       5
#define KT_SELECTMENU 255


#define NUM_SAV         14

#define RENT_NONE 0
#define RENT_NORMAL 1
#define RENT_LD  2
#define RENT_CRASH 3
#define RENT_REBOOT 4

#define DIST_NOTFOUND -1
#define DIST_0  0
#define DIST_1  1


#define MIN_LANGUAGES       180
#define MAX_LANGUAGES       187

#define EVNT_NONE 0
#define EVNT_ENTER      1
#define EVNT_HITPOINT 2

/* room-related defines *************************************************/

/* The cardinal directions: used as index to room_data.dir_option[] */
#define NORTH          0
#define EAST           1
#define SOUTH          2
#define WEST           3
#define UP             4
#define DOWN           5
#define NUM_OF_DIRS    6 /* number of directions in a room (nsewud) */

/* ������������ ����������� */
#define MOVE_STEP 0 //������
#define MOVE_RUN 1 //��������
#define MOVE_SWIM 2 //�������
#define MOVE_JUMP 3 //���������
#define MOVE_FLY 4 //��������
#define MOVE_CROUCH 5 //�������
#define MOVE_GALOP 6 //���������
#define MOVE_PKAT       7 //����������
#define MOVE_PKOV       8 //����������

/* ������������ ����� ����������� */
#define HIT_NONE 0
#define HIT_SLASH 1
#define HIT_PICK 2
#define HIT_BLOW 3
#define HIT_FIRE 4
#define HIT_COLD 5
#define HIT_ELECTRO 6
#define HIT_ACID 7
#define HIT_POISON 8
#define HIT_POSITIVE 9
#define HIT_NEGATIVE 10

/**/
#define M_NONE  0
#define M_HIT  -1
#define M_KILL  -2
#define M_NOHS  -3
#define M_ARM  -4
#define M_SKIN  -5

/* �������������� �������� ����� */
#define WEAP_NONE 0
#define WEAP_RIGHT 1
#define WEAP_LEFT 2
#define WEAP_DUAL 3

/* ���� �������� */
#define MISSILE_NONE 0
#define MISSILE_ARROW 1
#define MISSILE_BOLT 2
#define MISSILE_CORE 3


/* �������������� �������� �������� */
#define N_POWER  0 //��� ��������, ������ ��������
#define C_POWER  1 //�������� ��������������
#define A_POWER  2 //��� ��������, ������ ������

/* ������������ �������� ������ ���-��� */
#define RD_NONE  0  //������
#define RD_NORMAL 1  //������� ����
#define RD_KILL  -10 //����������� ����
#define RD_FAR  -1 //���� �� ���������� �� ����������
#define RD_DEVIATE -2 //���� �� ������ ������ (�����, ����, ���� � �.�.)
#define RD_ARMOR -3 //���� ��������� �������� ���������
#define RD_MAGIC -4 //���� ��������� �������� ������
#define RD_POWER -5 //���� �� ���������� �� ��������
#define RD_FAIL  -6 //����������� ������
#define RD_ERROR -7 //����� �� ���������� ������
#define RD_NOARROW -8 //����������� ������

#define WPOS_NONE 0
#define WPOS_BODY 1
#define WPOS_HEAD 2
#define WPOS_FACE 3
#define WPOS_NECK 4
#define WPOS_TOP_ARMS 5
#define WPOS_BOT_ARMS 6
#define WPOS_FRONT_LEGS 7
#define WPOS_BACK_LEGS 8
#define WPOS_ABOUT 9
#define WPOS_TAIL 10
#define WPOS_FLY 11
#define WPOS_WAIST 12

/* ������������ ������ */
#define PLAN_SIGIL 0
#define PLAN_ELYSIUM 1
#define PLAN_BEASTLANDS 2
#define PLAN_ARBOREA 3
#define PLAN_YSGARD 4
#define PLAN_LIMBO 5
#define PLAN_PANDEMONIUM 6
#define PLAN_ABYSS 7
#define PLAN_CARCERI 8
#define PLAN_WASTE 9
#define PLAN_GEHANNA 10
#define PLAN_BAATOR 11
#define PLAN_ACHERON 12
#define PLAN_MECHANUS 13
#define PLAN_ARCADIA 14
#define PLAN_CELESTIA 15
#define PLAN_BYROPIA 16
#define PLAN_ASTRAL 17
#define PLAN_DUNGEON 31
#define PLAN_MAZE 32

#define MSPEC_ENEMY (1 << 0)
#define MSPEC_ALL (1 << 1)
#define MSPEC_BASH (1 << 2)
#define MSPEC_MOVED (1 << 3)
#define MSPEC_HEAL (1 << 4)
#define MSPEC_DRAIN (1 << 5)

#define AGRO_NONE (1 << 0)
#define AGRO_ALL (1 << 1)
#define AGRO_EVIL (1 << 2)
#define AGRO_NEUTRAL (1 << 3)
#define AGRO_GOOD (1 << 4)
#define AGRO_ORDER (1 << 5)
#define AGRO_LAWS (1 << 6)
#define AGRO_XAOS (1 << 7)

/* Room flags: used in room_data.room_flags */
/* WARNING: In the world files, NEVER set the bits marked "R" ("Reserved") */
#define ROOM_DARK           (1 << 0)   /* ������      */
#define ROOM_DEATH          (1 << 1)   /* �������    */
#define ROOM_NOMOB          (1 << 2)   /* !�������    */
#define ROOM_INDOORS        (1 << 3)   /* ���������     */
#define ROOM_PEACEFUL       (1 << 4)   /* ������  */
#define ROOM_SOUNDPROOF     (1 << 5)   /* ������  */
#define ROOM_NOTRACK        (1 << 6)   /* !���������  */
#define ROOM_NOMAGIC        (1 << 7)   /* !�����   */
#define ROOM_TUNNEL         (1 << 8)   /* �����  */
#define ROOM_PRIVATE        (1 << 9)   /* ������ ��� 2�  */
#define ROOM_GODROOM        (1 << 10)  /* !�������� */
#define ROOM_HOUSE          (1 << 11)  /* ����_�������� */
#define ROOM_HOUSE_CRASH    (1 << 12)  /* ����_�������� (��������)  */
#define ROOM_ATRIUM         (1 << 13)  /* (R) The door to a house */
#define ROOM_OLC            (1 << 14)  /* (R) Modifyable/!compress  */
#define ROOM_BFS_MARK       (1 << 15)  /* (R) breath-first srch mrk */
#define ROOM_MAGIC_USER     (1 << 16)  /* ������� ������� ����� */
#define ROOM_WARRIOR        (1 << 17)  /* ������� ������� ������ */
#define ROOM_THIEF          (1 << 18)  /* ������� ������� ����� */
#define ROOM_RANGER         (1 << 19)  /* ������� ������� ��������� */
#define ROOM_NARROW         (1 << 20)  /* ������ ������� */
#define ROOM_PRIEST         (1 << 21)  /* ������� ������� ������ */
#define ROOM_NODECAY     (1 << 22)  /* ������� � ��������� ����� */

#define ROOM_ARENA          (INT_ONE | (1 << 0))  /* ����� */
#define ROOM_NOTELEPORT     (INT_ONE | (1 << 1))  /* !�������� */
#define ROOM_NORECALL       (INT_ONE | (1 << 2))  /* !����������� (�����) */
#define ROOM_NOHORSE        (INT_ONE | (1 << 3)) /* !������� */
#define ROOM_NOWEATHER      (INT_ONE | (1 << 4)) /* ��� ������ */
#define ROOM_FCIRCLE        (INT_ONE | (1 << 5)) /* ���������� */
#define ROOM_CIRCLE         (INT_ONE | (1 << 6)) /* �������� */
#define ROOM_FLYDEATH       (INT_ONE | (1 << 7)) /* ����� ������� */
#define ROOM_DOWN     (INT_ONE | (1 << 8)) /* ������ */
#define ROOM_NOHUMAN        (INT_ONE | (1 << 9))  /* ������ ����� */
#define ROOM_NOORC          (INT_ONE | (1 << 10)) /* ������ ����� */
#define ROOM_NOGNOME        (INT_ONE | (1 << 11)) /* ������ ������ */
#define ROOM_NOELVES        (INT_ONE | (1 << 12)) /* ������ ������ */
#define ROOM_NOHALFELVES    (INT_ONE | (1 << 13)) /* ������ ���������� */
#define ROOM_NOBARIAUR      (INT_ONE | (1 << 14)) /* ������ ��������� */
#define ROOM_NOTIEFLING     (INT_ONE | (1 << 15)) /* ������ ��������� */
#define ROOM_NOAASIMAR      (INT_ONE | (1 << 16)) /* ������ ��������� */
#define ROOM_NOPLAYER       (INT_ONE | (1 << 17)) /* ������ ������� */
#define ROOM_DESECRATE     (INT_ONE | (1 << 18)) /* ����������� ������� */
#define ROOM_SANCTUARY     (INT_ONE | (1 << 19)) /* ��������� ������� */

/* �������� ����������� */
#define DAMAGE_ALL (1 << 0)
#define DAMAGE_PC (1 << 1)
#define DAMAGE_NPC (1 << 2)
#define DAMAGE_STEP (1 << 3)
#define DAMAGE_LEVIT (1 << 4)
#define DAMAGE_FLY (1 << 5)
#define DAMAGE_ONES (1 << 6)

#define AF_BATTLEDEC    (1 << 0)
#define AF_DEADKEEP     (1 << 1)

/* ��� ������� */
#define EXIT_CRASHED (1 << 0) /* ������� */
#define EXIT_CLOSED (1 << 1) /* ������� */
#define EXIT_LOCKED (1 << 2) /* ������� */
#define EXIT_NOPICK     (1 << 3) /* ���������� */
#define EXIT_NOCRASH    (1 << 4) /* ��������� */
#define EXIT_HIDDEN (1 << 5) /* ������� */
#define EXIT_CLOSE (1 << 6) /* ����������� */
#define EXIT_MOVED (1 << 7) /* ��������� */
#define EXIT_NOEXP (1 << 8) /* �� ������ ���� �� ����� */
#define EXIT_REPOP (1 << 9) /* �������c� �� ������ */

/* Sector types: used in room_data.sector_type */
#define SECT_INSIDE          0       /* Indoors     */
#define SECT_CITY            1       /* In a city     */
#define SECT_FIELD           2       /* In a field    */
#define SECT_FOREST          3       /* In a forest   */
#define SECT_HILLS           4       /* In the hills    */
#define SECT_MOUNTAIN        5       /* On a mountain   */
#define SECT_BMOUNTAIN       6       /* On a mountain   */
#define SECT_WATER_SWIM      7       /* Swimmable water   */
#define SECT_WATER_NOSWIM    8       /* Water - need a boat */
#define SECT_FLYING          9       /* Wheee!      */
#define SECT_UNDERWATER      10      /* Underwater    */
#define SECT_SECRET          11      /* ��������� */

/* Added values for weather changes */
#define SECT_FIELD_SNOW      20
#define SECT_FIELD_RAIN      21
#define SECT_FOREST_SNOW     22
#define SECT_FOREST_RAIN     23
#define SECT_HILLS_SNOW      24
#define SECT_HILLS_RAIN      25
#define SECT_MOUNTAIN_SNOW   26
#define SECT_BMOUNTAIN_SNOW  27
#define SECT_THIN_ICE        28
#define SECT_NORMAL_ICE      29
#define SECT_THICK_ICE       30


#define WEATHER_QUICKCOOL   (1 << 0)
#define WEATHER_QUICKHOT    (1 << 1)
#define WEATHER_LIGHTRAIN   (1 << 2)
#define WEATHER_MEDIUMRAIN    (1 << 3)
#define WEATHER_BIGRAIN     (1 << 4)
#define WEATHER_GRAD      (1 << 5)
#define WEATHER_LIGHTSNOW   (1 << 6)
#define WEATHER_MEDIUMSNOW    (1 << 7)
#define WEATHER_BIGSNOW     (1 << 8)
#define WEATHER_LIGHTWIND   (1 << 9)
#define WEATHER_MEDIUMWIND    (1 << 10)
#define WEATHER_BIGWIND     (1 << 11)


#define NUM_GODS 10 //���������� �����
// ������
#define GOD_UNDEFINED 0
// ����
#define GOD_PELOR 1
#define GOD_ELONA 2
#define GOD_KL  3
#define GOD_FARLAN 4
#define GOD_OLIDAM      5
#define GOD_GUBERT 6
#define GOD_GRUUMSH 7
#define GOD_NERULL 8
#define GOD_HEXTOR 9

#define NUM_CLASSES   7  //���������� �������

// ����������� ���������
#define CLASS_UNDEFINED      0
#define CLASS_MAGIC_USER     1 //������ ���
#define CLASS_WARRIOR        2
#define CLASS_THIEF          3
#define CLASS_RANGER         4
#define CLASS_PRIEST         5
#define CLASS_NECRO      6 //���������

#define MASK_MAGIC_USER   (1 << CLASS_MAGIC_USER)
#define MASK_WARRIOR      (1 << CLASS_WARRIOR)
#define MASK_THIEF        (1 << CLASS_THIEF)
#define MASK_RANGER       (1 << CLASS_RANGER)
#define MASK_CASTER       (MASK_MAGIC_USER)

/* PC religions */
#define RELIGION_POLY    0
#define RELIGION_MONO    1

#define ALIGN_NUM       9
#define ALIGN_GOODABS   0
#define ALIGN_GOOD      1
#define ALIGN_GOODNEUT  2
#define ALIGN_NEUTGOOD  3
#define ALIGN_NEUTRAL   4
#define ALIGN_NEUTEVIL  5
#define ALIGN_EVILNEUT  6
#define ALIGN_EVIL      7
#define ALIGN_EVILABS   8


#define MASK_RELIGION_POLY        (1 << RELIGION_POLY)
#define MASK_RELIGION_MONO        (1 << RELIGION_MONO)

#define SAV_NUMBER 14

/* ���� ���� */
#define NUM_EYES 11
#define EYES_NOT 0
#define EYES_BLACK 1 //������
#define EYES_YELLOW 2 //������
#define EYES_RED 3 //�������
#define EYES_GREEN 4 //�������
#define EYES_GRAY 5 //�����
#define EYES_BLUE 6 //�����
#define EYES_CYAN 7 //�������
#define EYES_BROWN 8 //�����
#define EYES_SILVER 9 //�����������
#define EYES_GOLDEN 10 //����������

/* ���� */
#define NUM_RACES         35
#define RACE_UNDEFINED   -1
// ��������� ������
#define RACE_HUMAN        0  // �������
#define RACE_ORC          1  // ���
#define RACE_GNOME        2  // ����
#define RACE_ELVES        3  // ����
#define RACE_HALFELVES    4  // ��������
#define RACE_BARIAUR      5 // �������
#define RACE_TIEFLING     6 // �������
#define RACE_AASIMAR      7 // �������
#define RACE_DROU   8 //����
// ���������
#define RACE_ANIMAL   9 // ��������
#define RACE_DRAGON       10 // ������
#define RACE_HUMANOID     11 // ��������
#define RACE_GENASI   12 // ������
#define RACE_BIRD   13 // �����
#define RACE_WORM  14 //�����
#define RACE_REPTILE  15 //��������
#define RACE_SNAKE  16 //����
#define RACE_INSECT  17 //���������
#define RACE_GIGANT  18 //�������
#define RACE_CONSTRUCTION 19 //�����������
#define RACE_GHOST  20 //�������
#define RACE_PLANT  21 //��������
#define RACE_SLIME  22 //�����
#define RACE_FISH  23
#define RACE_MONSTER  24
#define RACE_GITH  25
#define RACE_SKELET  26
#define RACE_SLAAD  27
#define RACE_ABBERATION  28
#define RACE_FAIRY  29
#define RACE_ELEMENTAL  30
#define RACE_OBJECT  31
#define RACE_MEPHIT  32
#define RACE_HORSE  33
#define RACE_RODENT  34

#define RACE_NONE  255

#define NUM_TMOBS  5
#define TMOB_NONE  0
#define TMOB_LIVE  1
#define TMOB_UNDEAD  2
#define TMOB_DAEMON  3
#define TMOB_AUTSAIDER  4

//����������� ��� ������
#define VMOB_NONE  0
#define VMOB_SKELET  1
#define VMOB_ZOMBIE  2
#define VMOB_MUMIE  3
#define VMOB_GHOLA  4
#define VMOB_   5
#define VMOB_VAMPIRE  6
#define VMOB_WRAITH             7
#define VMOB_DKNIGHT  8
#define VMOB_LICH  9
#define VMOB_GHOST  10
#define VMOB_SPECTUM  11
#define VMOB_SHADOW  12


#define MASK_HUMAN        (1 << RACE_HUMAN)
#define MASK_ORC          (1 << RACE_ORC)
#define MASK_GNOME        (1 << RACE_GNOME)
#define MASK_ELVES        (1 << RACE_ELVES)
#define MASK_HALFELVES    (1 << RACE_HALFELVES)
#define MASK_BARIAUR      (1 << RACE_BARIAUR)
#define MASK_TIEFLING     (1 << RACE_TIEFLING)
#define MASK_AASIMAR      (1 << RACE_AASIMAR)
#define MASK_ANIMAL   (1 << RACE_ANIMAL)
#define MASK_DRAGON       (1 << RACE_DRAGON)
#define MASK_HUMANOID     (1 << RACE_HUMANOID)
#define MASK_GENASI       (1 << RACE_GENASI)

/*���� ����� � ������� */
#define BODY_TYPE_HUMANOID 0 // ����� �������� 
#define BODY_TYPE_LIZARD 1 // ����� �������
#define BODY_TYPE_DRAGON 2 // ����� �������
#define BODY_TYPE_AVIAN  3 // ����� ������� �����
#define BODY_TYPE_ANIMAL 4 // ����� �������� (������, ������, ����)
#define BODY_TYPE_BLANK  5 // ����� ��� ���� (����������)
#define MAX_BODY_TYPE  6

/* ��� */
#define SEX_NEUTRAL         0
#define SEX_MALE            1
#define SEX_FEMALE          2
#define SEX_POLY            3
#define SEX_NONE     255

#define NUM_SEXES           4

#define MASK_SEX_NEUTRAL  (1 << SEX_NEUTRAL)
#define MASK_SEX_MALE     (1 << SEX_MALE)
#define MASK_SEX_FEMALE   (1 << SEX_FEMALE)
#define MASK_SEX_POLY     (1 << SEX_POLY)

/* GODs FLAGS */
#define GF_GODSLIKE   (1 << 0)
#define GF_GODSCURSE  (1 << 1)
#define GF_HIGHGOD    (1 << 2)
#define GF_REMORT     (1 << 3)

#define NUM_ZONES 8
#define ZONE_0  0
#define ZONE_1          1
#define ZONE_2          2
#define ZONE_3          3
#define ZONE_4          4
#define ZONE_5          5
#define ZONE_6          6
#define ZONE_7          7

/* Positions */
#define POS_DEAD       0  /* dead     */
#define POS_MORTALLYW  1  /* mortally wounded */
#define POS_INCAP      2  /* incapacitated  */
#define POS_STUNNED    3  /* stunned    */
#define POS_SLEEPING   4  /* sleeping   */
#define POS_RESTING    5  /* resting    */
#define POS_SITTING    6  /* sitting    */
#define POS_FIGHTING   7  /* fighting   */
#define POS_STANDING   8  /* standing   */
#define POS_FLYING     9  /* ������/����� */
#define POS_NUMBER     10


/* Player flags: used by char_data.char_specials.act */
#define PLR_DROPLINK  (1 << 0)   /* Player is a player-killer   */
#define PLR_SOUL     (1 << 1)   /* Player is a player-thief    */
#define PLR_FROZEN      (1 << 2)   /* Player is frozen      */
#define PLR_DONTSET     (1 << 3)   /* Don't EVER set (ISNPC bit)  */
#define PLR_WRITING     (1 << 4)   /* Player writing (board/mail/olc) */
#define PLR_MAILING     (1 << 5)   /* Player is writing mail    */
#define PLR_CRASH     (1 << 6)   /* Player needs to be crash-saved  */
#define PLR_SITEOK      (1 << 7)   /* Player has been site-cleared  */
#define PLR_MUTE      (1 << 8)   /* Player not allowed to shout/goss/auct */
#define PLR_NOTITLE     (1 << 9)   /* Player not allowed to set title */
#define PLR_DELETED     (1 << 10)  /* Player deleted - space reusable */
#define PLR_LOADROOM  (1 << 11)  /* Player uses nonstandard loadroom  */
#define PLR_NOWIZLIST (1 << 12)  /* Player shouldn't be on wizlist  */
#define PLR_NODELETE  (1 << 13)  /* Player shouldn't be deleted */
#define PLR_INVSTART  (1 << 14)  /* Player should enter game wizinvis */
#define PLR_CRYO      (1 << 15)  /* Player is cryo-saved (purge prog) */
#define PLR_HELLED      (1 << 16)  /* Player is in Hell */
#define PLR_NAMED     (1 << 17)  /* Player is in Names Room */
#define PLR_REGISTERED      (1 << 18)
#define PLR_DUMB      (1 << 19)  /* Player is not allowed to tell/emote/social */
#define PLR_SCRIPTING (1 << 20) /* Player is writing fenia script */
#define PLR_DELETE          (1 << 28) /* RESERVED - ONLY INTERNALLY */
#define PLR_MULTI (INT_ONE | (1 << 0)) /* ��� ������� */
#define PLR_COMPS (INT_ONE | (1 << 1)) /* */

/* Mobile flags: used by char_data.char_specials.act */
#define MOB_SPEC             (1 << 0)  /* Mob has a callable spec-proc  */
#define MOB_SENTINEL         (1 << 1)  /* Mob should not move   */
#define MOB_SCAVENGER        (1 << 2)  /* Mob picks up stuff on the ground  */
#define MOB_ISNPC            (1 << 3)  /* (R) Automatically set on all Mobs */
#define MOB_AWARE      (1 << 4)  /* Mob can't be backstabbed    */
#define MOB_AGGRESSIVE       (1 << 5)  /* Mob hits players in the room  */
#define MOB_STAY_ZONE        (1 << 6)  /* Mob shouldn't wander out of zone  */
#define MOB_AGGR_SLEEP       (1 << 7)  /* Mob flees if severely injured */
#define MOB_AGGR_DAY       (1 << 8)  /* */
#define MOB_AGGR_NIGHT   (1 << 9)  /* */
#define MOB_WALKER       (1 << 10) /* */
#define MOB_MEMORY       (1 << 11) /* remember attackers if attacked  */
#define MOB_HELPER       (1 << 12) /* attack PCs fighting other NPCs  */
#define MOB_NOCHARM      (1 << 13) /* Mob can't be charmed    */
#define MOB_NOSUMMON       (1 << 14) /* Mob can't be summoned   */
#define MOB_NOSLEEP      (1 << 15) /* Mob can't be slept    */
#define MOB_NOBASH       (1 << 16) /* Mob can't be bashed (e.g. trees)  */
#define MOB_NOBLIND      (1 << 17) /* Mob can't be blinded    */
#define MOB_MOUNTING         (1 << 18)
#define MOB_NOHOLD           (1 << 19)
#define MOB_NOSIELENCE       (1 << 20)
#define MOB_AGGRGOOD         (1 << 21)
#define MOB_AGGREVIL         (1 << 22)
#define MOB_AGGRNEUTRAL      (1 << 30)
#define MOB_NOFEAR           (1 << 23)
#define MOB_NOGROUP          (1 << 24)
#define MOB_CORPSE           (1 << 25)
#define MOB_LOOTER           (1 << 26)
#define MOB_FREE1            (1 << 27)
#define MOB_DELETE           (1 << 28) /* RESERVED - ONLY INTERNALLY */
#define MOB_FREE             (1 << 29) /* RESERVED - ONLY INTERBALLY */

#define MOB_SWIMMING      (INT_ONE | (1 << 0))
#define MOB_FLYING        (INT_ONE | (1 << 1))
#define MOB_ONLYSWIMMING  (INT_ONE | (1 << 2))
#define MOB_AGGR_WINTER   (INT_ONE | (1 << 3))
#define MOB_AGGR_SPRING   (INT_ONE | (1 << 4))
#define MOB_AGGR_SUMMER   (INT_ONE | (1 << 5))
#define MOB_AGGR_AUTUMN   (INT_ONE | (1 << 6))
#define MOB_LIKE_DAY      (INT_ONE | (1 << 7))
#define MOB_LIKE_NIGHT    (INT_ONE | (1 << 8))
#define MOB_LIKE_FULLMOON (INT_ONE | (1 << 9))
#define MOB_LIKE_WINTER   (INT_ONE | (1 << 10))
#define MOB_LIKE_SPRING   (INT_ONE | (1 << 11))
#define MOB_LIKE_SUMMER   (INT_ONE | (1 << 12))
#define MOB_LIKE_AUTUMN   (INT_ONE | (1 << 13))
#define MOB_NOFIGHT       (INT_ONE | (1 << 14))
#define MOB_EADECREASE    (INT_ONE | (1 << 15))
#define MOB_HORDE         (INT_ONE | (1 << 16))
#define MOB_CLONE         (INT_ONE | (1 << 17))
#define MOB_ANGEL         (INT_ONE | (1 << 18))
#define MOB_DAEMON        (INT_ONE | (1 << 19))
#define MOB_LEVITING      (INT_ONE | (1 << 20))

#define MOB_FIREBREATH    (INT_TWO | (1 << 0))
#define MOB_GASBREATH     (INT_TWO | (1 << 1))
#define MOB_FROSTBREATH   (INT_TWO | (1 << 2))
#define MOB_ACIDBREATH    (INT_TWO | (1 << 3))
#define MOB_LIGHTBREATH   (INT_TWO | (1 << 4))
#define MOB_GUARD         (INT_TWO | (1 << 5))
#define MOB_XENO    (INT_TWO | (1 << 6))
#define MOB_IQ_FIGHT    (INT_TWO | (1 << 7))
#define MOB_RENT    (INT_TWO | (1 << 8))
#define MOB_POSTMAN    (INT_TWO | (1 << 9))
#define MOB_BANK     (INT_TWO | (1 << 10))
#define MOB_MASTER     (INT_TWO | (1 << 11))
#define MOB_TRAINER     (INT_TWO | (1 << 12))


#define NPC_NORTH         (1 << 0)
#define NPC_EAST          (1 << 1)
#define NPC_SOUTH         (1 << 2)
#define NPC_WEST          (1 << 3)
#define NPC_UP            (1 << 4)
#define NPC_DOWN          (1 << 5)
#define NPC_POISON        (1 << 6)
#define NPC_INVIS         (1 << 7)
#define NPC_FREE4         (1 << 8)
#define NPC_FREE5         (1 << 9)
#define NPC_MOVEFLY       (1 << 11)
#define NPC_MOVECREEP     (1 << 12)
#define NPC_MOVEJUMP      (1 << 13)
#define NPC_MOVESWIM      (1 << 14)
#define NPC_MOVERUN       (1 << 15)
#define NPC_FREE1   (1 << 20)
#define NPC_FREE2   (1 << 21)
#define NPC_NOREST    (1 << 22)
#define NPC_NOREGAIN      (1 << 23)
#define NPC_HELPED        (1 << 24)
#define NPC_NOHELPED   (1 << 25)
#define NPC_ISOBJECT      (1 << 26)
#define NPC_MISLEAD       (1 << 27)
#define NPC_REPOPKILL     (1 << 28)

#define NPC_FREE3         (INT_ONE | (1 << 0))
#define NPC_WIELDING      (INT_ONE | (1 << 1))
#define NPC_ARMORING      (INT_ONE | (1 << 2))
#define NPC_USELIGHT      (INT_ONE | (1 << 3))
#define NPC_TRACK         (INT_ONE | (1 << 4))
#define NPC_PLAGUE   (INT_ONE | (1 << 5))
#define NPC_HIDDEN   (INT_ONE | (1 << 6))

/* Descriptor flags */
#define DESC_CANZLIB  (1 << 0)  /* Client says compression capable.   */

/* Preference flags: used by char_data.player_specials.pref */
#define PRF_BRIEF       (1 << 0)  /* Room descs won't normally be shown */
#define PRF_COMPACT     (1 << 1)  /* No extra CRLF pair before prompts  */
#define PRF_NOHOLLER      (1 << 2)  /* �� ������ ������� "�����"  */
#define PRF_NOTELL      (1 << 3)  /* �� ������ ������� "�������"  */
#define PRF_DISPHP      (1 << 4)  /* Display hit points in prompt */
#define PRF_DISPMANA  (1 << 5)  /* Display mana points in prompt  */
#define PRF_DISPMOVE  (1 << 6)  /* Display move points in prompt  */
#define PRF_AUTOEXIT  (1 << 7)  /* Display exits in a room    */
#define PRF_NOHASSLE  (1 << 8)  /* Aggr mobs won't attack   */
#define PRF_QUEST     (1 << 9)  /* On quest       */
#define PRF_SUMMONABLE  (1 << 10) /* Can be summoned      */
#define PRF_NOREPEAT  (1 << 11) /* No repetition of comm commands */
#define PRF_HOLYLIGHT (1 << 12) /* Can see in dark      */
#define PRF_COLOR_1     (1 << 13) /* Color (low bit)      */
#define PRF_COLOR_2     (1 << 14) /* Color (high bit)     */
#define PRF_NOWIZ     (1 << 15) /* Can't hear wizline     */
#define PRF_LOG1      (1 << 16) /* On-line System Log (low bit) */
#define PRF_LOG2      (1 << 17) /* On-line System Log (high bit)  */
#define PRF_CURSES      (1 << 18) /* Can't hear auction channel   */
#define PRF_NOGOSS      (1 << 19) /* �� ������ ������� "�������"  */
#define PRF_NOGRATZ     (1 << 20) /* Can't hear grats channel   */
#define PRF_ROOMFLAGS (1 << 21) /* Can see room flags (ROOM_x)  */
#define PRF_DISPEXP     (1 << 22)
#define PRF_DISPEXITS   (1 << 23)
#define PRF_DISPLEVEL   (1 << 24)
#define PRF_DISPGOLD    (1 << 25)
#define PRF_BANK_RENT   (1 << 26)
#define PRF_SLOG (1 << 27)
#define PRF_PROMPT      (1 << 28)
#define PRF_CODERINFO   (1 << 29)

#define PRF_AUTOZLIB    (INT_ONE | 1 << 0) /* Automatically do compression.  */
#define PRF_AUTOMEM     (INT_ONE | 1 << 1)
#define PRF_NOSHOUT     (INT_ONE | 1 << 2) /* �� ������ ������� "�������" */
#define PRF_AUTOFRM     (INT_ONE | 1 << 3)
#define PRF_TIPS        (INT_ONE | 1 << 4)
#define PRF_MOBILES     (INT_ONE | 1 << 5)
#define PRF_OBJECTS     (INT_ONE | 1 << 6)
#define PRF_EQ    (INT_ONE | 1 << 7)
#define PRF_ROLL    (INT_ONE | 1 << 8)
#define PRF_DISPBOI    (INT_ONE | 1 << 9)
#define PRF_SHOWKILL (INT_ONE | 1 << 10)
#define PRF_SHOWMEGA (INT_ONE | 1 << 11)
#define PRF_LOGGING (INT_ONE | 1 << 12)
#define PRF_NOFOLLOW (INT_ONE | 1 << 13)
#define PRF_NOGIVE (INT_ONE | 1 << 14)
#define PRF_TRACKON (INT_ONE | 1 << 15) /* �������� */
#define PRF_THEME (INT_ONE | 1 << 16) /* �������� ����� */
#define PRF_NONEW (INT_ONE | 1 << 17) /* ��������� */
#define PRF_EXAMINE     (INT_ONE | 1 << 18)
#define PRF_EXITRUS     (INT_ONE | 1 << 19) /* ����������. �� ������� */
#define PRF_SELFMESS    (INT_ONE | 1 << 20) /* ������ ���� ������ ��������� */
#define PRF_MAPPER 		(INT_ONE | 1 << 21)


#define MAX_SPELL_AFFECTS 25 /* ������������ ���-�� �������� �� ���� (�� � ���������) */

/* Affect bits: used in char_data.char_specials.saved.affected_by */
/* WARNING: In the world files, NEVER set the bits marked "R" ("Reserved") */
#define AFF_BLIND             (1 << 0)     /* (R) Char is blind   */
#define AFF_INVISIBLE         (1 << 1)     /* Char is invisible   */
#define AFF_DETECT_ALIGN      (1 << 2)     /* Char is sensitive to align*/
#define AFF_DETECT_INVIS      (1 << 3)     /* Char can see invis chars  */
#define AFF_DETECT_MAGIC      (1 << 4)     /* Char is sensitive to magic*/
#define AFF_SENSE_LIFE        (1 << 5)     /* Char can sense hidden life*/
#define AFF_WATERWALK       (1 << 6)     /* Char can walk on water  */
#define AFF_SANCTUARY         (1 << 7)     /* Char protected by sanct.  */
#define AFF_GROUP             (1 << 8)     /* (R) Char is grouped */
#define AFF_CURSE             (1 << 9)     /* Char is cursed    */
#define AFF_INFRAVISION       (1 << 10)    /* Char can see in dark  */
#define AFF_POISON            (1 << 11)    /* (R) Char is poisoned  */
#define AFF_PROTECT_EVIL      (1 << 12)    /* Char protected from evil  */
#define AFF_PROTECT_GOOD      (1 << 13)    /* Char protected from good  */
#define AFF_SLEEP             (1 << 14)    /* (R) Char magically asleep */
#define AFF_NOTRACK       (1 << 15)    /* Char can't be tracked */
#define AFF_TETHERED        (1 << 16)    /* Room for future expansion */
#define AFF_NOTHING      (1 << 17)    /* ������ �� �������� ������ */
#define AFF_SNEAK             (1 << 18)    /* Char can move quietly */
#define AFF_HIDE              (1 << 19)    /* Char is hidden    */
#define AFF_COURAGE         (1 << 20)    /* Room for future expansion */
#define AFF_CHARM             (1 << 21)    /* Char is charmed   */
#define AFF_HOLD              (1 << 22)
#define AFF_FLY               (1 << 23)
#define AFF_SIELENCE          (1 << 24)
#define AFF_AWARNESS          (1 << 25)
#define AFF_TIED              (1 << 26) /* ��������� */
#define AFF_HORSE             (1 << 27)    /* NPC - is horse, PC - is horsed */
#define AFF_NOFLEE            (1 << 28)
#define AFF_SINGLELIGHT       (1 << 29)
#define AFF_HOLYLIGHT           (INT_ONE | (1 << 0))
#define AFF_HOLYDARK            (INT_ONE | (1 << 1))
#define AFF_DETECT_POISON       (INT_ONE | (1 << 2))
#define AFF_DRUNKED             (INT_ONE | (1 << 3))
#define AFF_STUNE               (INT_ONE | (1 << 4))
#define AFF_STOPRIGHT           (INT_ONE | (1 << 5))
#define AFF_STOPLEFT            (INT_ONE | (1 << 6))
#define AFF_STOPFIGHT           (INT_ONE | (1 << 7))
#define AFF_HAEMORRAGIA         (INT_ONE | (1 << 8))
#define AFF_CAMOUFLAGE          (INT_ONE | (1 << 9))
#define AFF_WATERBREATH         (INT_ONE | (1 << 10))
#define AFF_SLOW                (INT_ONE | (1 << 11))
#define AFF_FASTER              (INT_ONE | (1 << 12))
#define AFF_BONES_WALL   (INT_ONE | (1 << 13))
#define AFF_FIRECROWN          (INT_ONE | (1 << 14))
#define AFF_FIRESHIELD          (INT_ONE | (1 << 15))
#define AFF_PLAGUE            (INT_ONE | (1 << 16))
#define AFF_MEDITATION     (INT_ONE | (1 << 17))
#define AFF_BANDAGE      (INT_ONE | (1 << 18))
#define AFF_PRISMA_SKIN    (INT_ONE | (1 << 19))
#define AFF_BONES_PICK          (INT_ONE | (1 << 20))
#define AFF_HELPER      (INT_ONE | (1 << 21))
#define AFF_EVILESS             (INT_ONE | (1 << 22))
#define AFF_HOLYAURA        (INT_ONE | (1 << 23))
#define AFF_UNHOLYAURA  (INT_ONE | (1 << 24))
#define AFF_DETECT_UNDEAD       (INT_ONE | (1 << 25))
/* ����� AFFECT� */
#define AFF_STONE_SKIN          (INT_ONE | (1 << 26))
#define AFF_ORENT               (INT_ONE | (1 << 27))
#define AFF_EMPTY_1             (INT_ONE | (1 << 28))
#define AFF_DARKVISION  (INT_ONE | (1 << 29))
#define AFF_BLADES              (INT_TWO | (1 << 0))
#define AFF_DEAFNESS  (INT_TWO | (1 << 1))
#define AFF_SUNBEAM         (INT_TWO | (1 << 2))
#define AFF_HORSE_BUY  (INT_TWO | (1 << 3)) //�������� ������
#define AFF_PROT_UNDEAD  (INT_TWO | (1 << 4)) //����� � ������ ������ ������
#define AFF_LEVIT  (INT_TWO | (1 << 5)) //����� ����������
#define AFF_ILLNESS  (INT_TWO | (1 << 6)) //����� �����
#define AFF_IMPLANT_WEAPON (INT_TWO | (1 << 7)) //������� ������
#define AFF_IS_UNDEAD  (INT_TWO | (1 << 8)) //undead� �� �������
#define AFF_DANCE  (INT_TWO | (1 << 9)) //��������� �����

/* ������� �� ������� */
#define ROOM_AFF_NOTHING                (1 << 0)
#define ROOM_AFF_DESECRATE  (1 << 1)  //������� ��������
#define ROOM_AFF_SANCTUARY  (1 << 2)  //������� ��������
#define ROOM_AFF_NOMAGIC  (1 << 3)  //������� �������
#define ROOM_AFF_PRISMA_FIRE  (1 << 4)
#define ROOM_AFF_PRISMA_COLD            (1 << 5)
#define ROOM_AFF_PRISMA_ELEC            (1 << 6)
#define ROOM_AFF_PRISMA_ACID            (1 << 7)
#define ROOM_AFF_PRISMA_HITS            (1 << 8)
#define ROOM_AFF_PSPHERE  (1 << 9)
#define ROOM_AFF_SNARE   (1 << 10)
#define ROOM_AFF_THRDEATH  (1 << 11)
#define ROOM_AFF_CURE   (1 << 12)
#define ROOM_AFF_POISON_FOG  (1 << 13)


//������������ �����
#define TRANS_ENGINE (1 << 0 ) //����������
#define TRANS_EARTH (1 << 1 ) //����������
#define TRANS_FLY (1 << 2 ) //���������
#define TRANS_WATER (1 << 3 ) //������

#define TARM_NONE 0
#define TARM_LIGHT 1
#define TARM_MEDIUM 2
#define TARM_HARD 3
#define TARM_WEAR 4
#define TARM_JEWS 5

/* Character equipment positions: used as index for char_data.equipment[] */
/* NOTE: Don't confuse these constants with the ITEM_ bitvectors
   which control the valid places you can wear a piece of equipment */
#define WEAR_LIGHT      0 // ����
#define WEAR_FINGER_R   1 // ������ �����
#define WEAR_FINGER_L   2 // ����� �����
#define WEAR_FACE       3 // ����
#define WEAR_EAR_R      4 /* ������ ��� */
#define WEAR_EAR_L      5 /* ����� ��� */
#define WEAR_NECK_1     6  // ��� 1
#define WEAR_NECK_2     7  // ��� 2
#define WEAR_BODY       8  // ����
#define WEAR_HEAD       9  // ������
#define WEAR_LEGS      10  // ����
#define WEAR_FEET      11  // ������
#define WEAR_HANDS     12  // ���� (��������)
#define WEAR_ARMS      13  // ����
#define WEAR_SHIELD    14  // ���
#define WEAR_ABOUT     15  // ����
#define WEAR_WAIST     16  // �����
#define WEAR_WRIST_R   17  // �������� ����
#define WEAR_WRIST_L   18  // �������� ���
#define WEAR_WIELD     19  // ������
#define WEAR_HOLD      20  // ���� ���
#define WEAR_BOTHS     21  // ��� ���
#define WEAR_TAIL      22  /* ����� */
#define WEAR_QUIVER    23  /* ������ */
#define WEAR_DEATH     127  /* ����������� ����� ������ */
#define NUM_WEARS      24 /* This must be the # of eq positions!! */


/* object-related defines ********************************************/

#define NUM_ITEMS 37
/* Item types: used by obj_data.obj_flags.type_flag */
#define ITEM_LIGHT      1   /* Item is a light source */
#define ITEM_SCROLL     2   /* Item is a scroll   */
#define ITEM_WAND       3   /* Item is a wand   */
#define ITEM_STAFF      4   /* Item is a staff    */
#define ITEM_WEAPON     5   /* Item is a weapon   */
#define ITEM_FIREWEAPON 6   /* ��� ������������� ������    */
#define ITEM_MISSILE    7   /* ��� �������     */
#define ITEM_TREASURE   8   /* Item is a treasure, not gold */
#define ITEM_ARMOR      9   /* Item is armor    */
#define ITEM_POTION    10     /* Item is a potion   */
#define ITEM_WORN      11   /* Unimplemented    */
#define ITEM_OTHER     12   /* Misc object      */
#define ITEM_TOOLS     13   /* ���������� */
#define ITEM_TRAP      14   /* Unimplemented    */
#define ITEM_CONTAINER 15   /* Item is a container    */
#define ITEM_NOTE      16   /* Item is note     */
#define ITEM_DRINKCON  17   /* Item is a drink container  */
#define ITEM_KEY       18   /* Item is a key    */
#define ITEM_FOOD      19   /* Item is food     */
#define ITEM_MONEY     20   /* Item is money (gold)   */
#define ITEM_PEN       21   /* Item is a pen    */
#define ITEM_BOAT      22   /* Item is a boat   */
#define ITEM_FOUNTAIN  23   /* Item is a fountain   */
#define ITEM_BOOK      24       /**** Item is book */
#define ITEM_INGRADIENT 25      /**** Item is magical ingradient */
#define ITEM_FURNITURE 26
#define ITEM_CORPSE 27  /* ������� ���� */
#define ITEM_TATOO 28  /* ���������� */
#define ITEM_BOARD 29  /* ����� ���������� */
#define ITEM_FICTION 30  /* ��� ���������� */
#define ITEM_TRANSPORT 31 /* ������ */
#define ITEM_MATERIAL 32 /* �������� � ������ ���� */
#define ITEM_MINE 33 /* ����� */
#define ITEM_FRAGMENT 34 /* ������� */
#define ITEM_SPARE      35      /* �����-������� */
#define ITEM_JURNAL 36 /* ������ ������ ��������� */
#define ITEM_SCRIPTBOARD 37 /* ����� ��� ������ ��������� */

#define TOOL_NONE 0 /* ��� */
#define TOOL_LOCKPICK 1 /* ������� */
#define TOOL_HAMMER 2 /* ������� */
#define TOOL_SHOVEL 3 /* ������ */

/* Take/Wear flags: used by obj_data.obj_flags.wear_flags */
#define ITEM_WEAR_TAKE    (1 << 0)  /* Item can be takes    */
#define ITEM_WEAR_FINGER  (1 << 1)  /* Can be worn on finger  */
#define ITEM_WEAR_NECK    (1 << 2)  /* Can be worn around neck  */
#define ITEM_WEAR_BODY    (1 << 3)  /* Can be worn on body  */
#define ITEM_WEAR_HEAD    (1 << 4)  /* Can be worn on head  */
#define ITEM_WEAR_LEGS    (1 << 5)  /* Can be worn on legs  */
#define ITEM_WEAR_FEET    (1 << 6)  /* Can be worn on feet  */
#define ITEM_WEAR_HANDS   (1 << 7)  /* Can be worn on hands */
#define ITEM_WEAR_ARMS    (1 << 8)  /* Can be worn on arms  */
#define ITEM_WEAR_SHIELD  (1 << 9)  /* Can be used as a shield  */
#define ITEM_WEAR_ABOUT   (1 << 10) /* Can be worn about body   */
#define ITEM_WEAR_WAIST   (1 << 11) /* Can be worn around waist   */
#define ITEM_WEAR_WRIST   (1 << 12) /* Can be worn on wrist   */
#define ITEM_WEAR_WIELD   (1 << 13) /* Can be wielded   */
#define ITEM_WEAR_HOLD    (1 << 14) /* Can be held    */
#define ITEM_WEAR_BOTHS   (1 << 15)
#define ITEM_WEAR_FACE    (1 << 16) /* can be worn face */
#define ITEM_WEAR_TAIL    (1 << 17) /* ����� ����� �� ����� */
#define ITEM_WEAR_EARS    (1 << 18) /* ����� ����� �� ��� */
#define ITEM_WEAR_QUIVER  (1 << 19) /* ����� ����� � ������ */

/* Extra object flags: used by obj_data.obj_flags.extra_flags */
#define ITEM_GLOW          (1 << 0) /* Item is glowing    */
#define ITEM_HUM           (1 << 1) /* Item is humming    */
#define ITEM_NORENT        (1 << 2) /* Item cannot be rented  */
#define ITEM_NODONATE      (1 << 3) /* Item cannot be donated */
#define ITEM_DARKAURA      (1 << 4) /* Item cannot be made invis  */
#define ITEM_INVISIBLE     (1 << 5) /* Item is invisible    */
#define ITEM_MAGIC         (1 << 6) /* Item is magical    */
#define ITEM_NODROP        (1 << 7) /* Item is cursed: can't drop */
#define ITEM_GOODAURA      (1 << 8) /* Item is blessed    */
#define ITEM_NOSELL        (1 << 9) /* Not usable by good people  */
#define ITEM_DECAY         (1 << 10)  /* Not usable by evil people  */
#define ITEM_ZONEDECAY     (1 << 11)  /* Not usable by neutral people */
#define ITEM_NODISARM      (1 << 12)  /* Not usable by mages    */
#define ITEM_NODECAY       (1 << 13)
#define ITEM_POISONED      (1 << 14)
#define ITEM_SHARPEN       (1 << 15)
#define ITEM_ARMORED       (1 << 16)
#define ITEM_DAY           (1 << 17)
#define ITEM_NIGHT         (1 << 18)
#define ITEM_NOIDENT       (1 << 19)
#define ITEM_TAKECRASH     (1 << 20)
#define ITEM_ENCHANT       (1 << 21)
#define ITEM_SOULBIND      (1 << 22)
#define ITEM_DEATHAURA     (1 << 23)
#define ITEM_SWIMMING      (1 << 24)
#define ITEM_FLYING        (1 << 25)
#define ITEM_THROWING      (1 << 26)
#define ITEM_TICKTIMER     (1 << 27)
#define ITEM_FIRE          (1 << 28)
#define ITEM_BURIED    (1 << 29)
#define ITEM_INCOGNITO     (INT_ONE | (1 << 0))
#define ITEM_NOVISIBLE     (INT_ONE | (1 << 1))
#define ITEM_EXTERN        (INT_ONE | (1 << 2))
#define ITEM_HIDDEN        (INT_ONE | (1 << 3))
#define ITEM_REPOPDROP     (INT_ONE | (1 << 4))
#define ITEM_RENT_DELETE   (INT_ONE | (1 << 5)) //������������ �������� � �����
#define ITEM_RENT_DELETE2  (INT_ONE | (1 << 6)) //��������������� �������� ��������
#define ITEM_RENT_DELETE3  (INT_ONE | (1 << 7)) //��������� �� �������
//
#define ITEM_NO_GOOD        (1 << 0)
#define ITEM_NO_EVIL        (1 << 1)
#define ITEM_NO_NEUTRAL     (1 << 2)
//
#define ITEM_NO_MAGIC_USER  (1 << 3)
#define ITEM_NO_WARRIOR     (1 << 4)
#define ITEM_NO_THIEF       (1 << 5)
#define ITEM_NO_RANGER      (1 << 6)
//
#define ITEM_NO_HUMAN     (1 << 7)
#define ITEM_NO_ORC     (1 << 8)
#define ITEM_NO_GNOME     (1 << 9)
#define ITEM_NO_ELVES       (1 << 10)
#define ITEM_NO_HALFELVES   (1 << 11)
#define ITEM_NO_BARIAUR     (1 << 12)
#define ITEM_NO_TIEFLING    (1 << 13)
#define ITEM_NO_AASIMAR     (1 << 14)
#define ITEM_NO_ANIMAL      (1 << 15)
#define ITEM_NO_DRAGON      (1 << 16)
#define ITEM_NO_HUMANOID    (1 << 17)
#define ITEM_NO_GENASI      (1 << 18)
#define ITEM_NO_PRIEST      (1 << 19)

#define ITEM_AN_GOOD        (1 << 0)
#define ITEM_AN_EVIL        (1 << 1)
#define ITEM_AN_NEUTRAL     (1 << 2)
//
#define ITEM_AN_MAGIC_USER  (1 << 3)
#define ITEM_AN_WARRIOR     (1 << 4)
#define ITEM_AN_THIEF       (1 << 5)
#define ITEM_AN_RANGER      (1 << 6)
//
#define ITEM_AN_HUMAN     (1 << 7)
#define ITEM_AN_ORC     (1 << 8)
#define ITEM_AN_GNOME     (1 << 9)
#define ITEM_AN_ELVES       (1 << 10)
#define ITEM_AN_HALFELVES   (1 << 11)
#define ITEM_AN_BARIAUR     (1 << 12)
#define ITEM_AN_TIEFLING    (1 << 13)
#define ITEM_AN_AASIMAR     (1 << 14)
#define ITEM_AN_ANIMAL      (1 << 15)
#define ITEM_AN_DRAGON      (1 << 16)
#define ITEM_AN_HUMANOID    (1 << 17)
#define ITEM_AN_GENASI      (1 << 18)
#define ITEM_AN_PRIEST      (1 << 19)

/* Modifier constants used with obj affects ('A' fields) */
#define APPLY_NONE              0 /* No effect      */
#define APPLY_STR               1 /* Apply to strength    */
#define APPLY_DEX               2 /* Apply to dexterity   */
#define APPLY_INT               3 /* Apply to constitution  */
#define APPLY_WIS               4 /* Apply to wisdom    */
#define APPLY_CON               5 /* Apply to constitution  */
#define APPLY_CHA           6 /* Apply to charisma    */
#define APPLY_CLASS             7 /* Reserved     */
#define APPLY_LEVEL             8 /* Reserved     */
#define APPLY_AGE               9 /* Apply to age     */
#define APPLY_CHAR_WEIGHT      10 /* Apply to weight    */
#define APPLY_CHAR_HEIGHT      11 /* Apply to height    */
#define APPLY_MANA             12 /* Apply to max mana    */
#define APPLY_HIT              13 /* Apply to max hit points  */
#define APPLY_MOVE             14 /* Apply to max move points */
#define APPLY_GOLD             15 /* Reserved     */
#define APPLY_EXP              16 /* Reserved     */
#define APPLY_AC               17 /* Apply to Armor Class   */
#define APPLY_HITROLL          18 /* Apply to hitroll   */
#define APPLY_DAMROLL          19 /* Apply to damage roll   */
#define APPLY_LACKY            20 // ��������� � �����
#define APPLY_MANAREG          21 // ��������� � ����
#define APPLY_SAVING_POISON    22 // ������ �� ���������� (��)
#define APPLY_SAVING_SKILL     23 // ������ �� ������
#define APPLY_SAVING_4         24 // ��������
#define APPLY_HITREG           25
#define APPLY_MOVEREG          26
#define APPLY_DB               27
#define APPLY_SAVING_XAOS      28
#define APPLY_SAVING_ORDER     29
#define APPLY_SIZE             36
#define APPLY_ARMOUR0          37
#define APPLY_POISON           38
#define APPLY_POWER            39
#define APPLY_CAST_SUCCESS     40
#define APPLY_MORALE           41
#define APPLY_SPEED        42
#define APPLY_REMEMORY         43
// saving 3ed
#define APPLY_SAVING_REFL      45 // ������ �������
#define APPLY_SAVING_FORT 46 // ������ ������������
#define APPLY_SAVING_WILL 47 // ������ ����
#define APPLY_SAVING_FIRE 48 // ������ �����
#define APPLY_SAVING_COLD 49 // ������ �����
#define APPLY_SAVING_ELECTRO 50 // ������ �������������
#define APPLY_SAVING_ACID 51 // ������ �������
#define APPLY_SAVING_NEGATIVE 52
#define APPLY_SAVING_POSITIVE 53
#define APPLY_ARMOUR1           54
#define APPLY_ARMOUR2           55
//���������� ����������� �� �����
#define APPLY_INC_HEAL  56 //���������� �����
#define APPLY_INC_MAGIC  57 //��� �����
#define APPLY_INC_FIRE  58 //������ �����
#define APPLY_INC_COLD  59 //������ �����
#define APPLY_INC_ELECTRO 60 //������ �������
#define APPLY_INC_ACID  61 //������ �������
#define APPLY_INC_NEGATIVE 62 //������ �������
#define APPLY_INC_POSITIVE 63 //������ �������

#define ARM_SLASH 0
#define ARM_PICK 1
#define ARM_BLOW 2

//���� ����������
#define TYPE_NONE 0
#define TYPE_METALL 1
#define TYPE_STONE 2
#define TYPE_WOOD 3

#define MAT_NONE                0
#define MAT_CUPRUM              1
#define MAT_BRONZE              2
#define MAT_IRON                3
#define MAT_STEEL               4
#define MAT_SILVER  5
#define MAT_GOLD                6
#define MAT_PLATINUM            7
#define MAT_TITANUM             8
#define MAT_ADAMANT             9
#define MAT_MITHRIL             10
#define MAT_PLASTIC             11
#define MAT_CRYSTALL            12
#define MAT_WOOD                13
#define MAT_SUPERWOOD           14
#define MAT_FARFOR              15
#define MAT_GLASS               16
#define MAT_STONE               17
#define MAT_ROCK                18
#define MAT_SKIN                19
#define MAT_ORGANIC             20
#define MAT_CERAMIC             21
#define MAT_MATERIA             22
#define MAT_PAPER               23
#define MAT_BONE                24
#define MAT_GAZ                 25
#define MAT_DIAMOND             26
#define MAT_METAL               27
#define MAT_PERGAMENT           28


#define TRACK_NPC              (1 << 0)
#define TRACK_HIDE             (1 << 1)


/* Some different kind of liquids for use in values of drink containers */
#define LIQ_WATER      0
#define LIQ_BEER       1
#define LIQ_WINE       2
#define LIQ_ALE        3
#define LIQ_QUAS       4
#define LIQ_BRANDY     5
#define LIQ_MORSE      6
#define LIQ_VODKA      7
#define LIQ_BRAGA      8
#define LIQ_MED        9
#define LIQ_MILK       10
#define LIQ_TEA        11
#define LIQ_COFFE      12
#define LIQ_BLOOD      13
#define LIQ_SALTWATER  14
#define LIQ_CLEARWATER 15


#define EAT_NONE  0 /* �� ��������� ������������� */
#define EAT_MEAT  1 /* ��� ��� ���� */
#define EAT_GRASS  2 /* ��� ��� ����� */

/* other miscellaneous defines *******************************************/


/* Player conditions */
#define DRUNK        0
#define FULL         1
#define THIRST       2
#define SLEEP      3


/* ��������� ������ */
#define SUN_DARK  0
#define SUN_RISE  1
#define SUN_LIGHT 2
#define SUN_SET   3

/* ��� ���� */
#define SKY_CLOUDLESS 0
#define SKY_CLOUDY    1
#define SKY_RAINING   2
#define SKY_LIGHTNING 3

/* ��� ����� */
#define RAIN_NONE    0
#define RAIN_LIGHT   1
#define RAIN_MEDIUM  2
#define RAIN_HARD    3

/* ����� ���� */
#define SEASON_WINTER  0
#define SEASON_SPRING  1
#define SEASON_SUMMER  2
#define SEASON_AUTUMN  3

/* ������ ��������� */
#define MONTH_REGULA     0
#define MONTH_ACCOLDANT   1
#define MONTH_RETRIBUTUS 2
#define MONTH_NARCISS  3
#define MONTH_TITHING  4
#define MONTH_SAVORUS  5
#define MONTH_PIVOT  6
#define MONTH_CATECHISM  7
#define MONTH_SACRILEGION 8
#define MONTH_NIHILUM  9
#define MONTH_MORTIS  10
#define MONTH_DECADRE  11
#define MONTH_CAPRICIOUS 12
#define MONTH_LEAGUCHEIM 13

#define DAYS_PER_WEEK  7



/* Rent codes */

#define EXTRA_FAILHIDE       (1 << 0)
#define EXTRA_FAILSNEAK      (1 << 1)
#define EXTRA_FAILCAMOUFLAGE (1 << 2)


/* other #defined constants **********************************************/

/*
 * **DO**NOT** blindly change the number of levels in your MUD merely by
 * changing these numbers and without changing the rest of the code to match.
 * Other changes throughout the code are required.  See coding.doc for
 * details.
 *
 * LVL_IMPL should always be the HIGHEST possible immortal level, and
 * LVL_IMMORT should always be the LOWEST immortal level.  The number of
 * mortal levels will always be LVL_IMMORT - 1.
 */
#define LVL_MAX     40 //������������ ������� ������
#define LVL_IMPL    65 //��������� ���
#define LVL_GRGOD   64 //������� ���
#define LVL_HIGOD   63 //���
#define LVL_GOD     62 //������� ���
#define LVL_IMMORT  41 //�����
#define LVL_HICHAR  25 //��������
#define LVL_ROLL     3 //������� ��������

#define MAX_MOB_LEVEL 60       /* ������������ ������� ����� */

/* Level of the 'freeze' command */
#define LVL_FREEZE  LVL_GOD

#define MAGIC_NUMBER  (0x06)  /* Arbitrary number that won't be in a string */

#define SEVEN_DAYS 43200
#define OPT_USEC  100000  /* 10 passes per second */
#define PASSES_PER_SEC  (1000000 / OPT_USEC)
#define RL_SEC    * PASSES_PER_SEC
#define RL_PSEC    * PASSES_PER_SEC / 2

#define PULSE_ZONE      (3 RL_SEC)
#define PULSE_MOBILE    (3 RL_SEC)
#define PULSE_VIOLENCE  (3 RL_SEC)
#define PULSE_WEATHER (360 RL_SEC)

/* Variables for the output buffering system */
#define MAX_SOCK_BUF            (12 * 1024) /* Size of kernel's sock buf   */
#define MAX_PROMPT_LENGTH       1024          /* Max length of prompt        */
#define GARBAGE_SPACE     32          /* Space for **OVERFLOW** etc  */
#define SMALL_BUFSIZE     16384        /* Static output buffer size   */
/* Max amount of output that can be buffered */
#define LARGE_BUFSIZE       (MAX_SOCK_BUF - GARBAGE_SPACE - MAX_PROMPT_LENGTH)

#define HISTORY_SIZE      5 /* Keep last 5 commands. */
#define MAX_STRING_LENGTH   8192
#define MAX_EXTEND_LENGTH   0xFFFF
#define MAX_INPUT_LENGTH    256 /* Max length per *line* of input */
#define MAX_RAW_INPUT_LENGTH          512     /* Max size of *raw* input */
#define MAX_NAME_LENGTH     20      /* Used in char_file_u *DO*NOT*CHANGE* */
#define MIN_NAME_LENGTH                 3
#define MAX_PWD_LENGTH      50      /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_TITLE_LENGTH    80      /* Used in char_file_u *DO*NOT*CHANGE* */
#define HOST_LENGTH     30      /* Used in char_file_u *DO*NOT*CHANGE* */
#define EXDSCR_LENGTH     1024     /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_TONGUE      3       /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_SKILLS      211     /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_SPELLS          399    /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_AFFECT      32      /* Used in char_file_u *DO*NOT*CHANGE* */
#define MAX_OBJ_AFFECT      6   /* Used in obj_file_elem *DO*NOT*CHANGE* */
#define MAX_TIMED_SKILLS    16 /* Used in obj_file_elem *DO*NOT*CHANGE* */
#define MAX_ENCHANT 20

/*
 * A MAX_PWD_LENGTH of 10 will cause BSD-derived systems with MD5 passwords
 * and GNU libc 2 passwords to be truncated.  On BSD this will enable anyone
 * with a name longer than 5 character to log in with any password.  If you
 * have such a system, it is suggested you change the limit to 20.
 *
 * Please note that this will erase your player files.  If you are not
 * prepared to do so, simply erase these lines but heed the above warning.
 */
#if defined(HAVE_UNSAFE_CRYPT) && MAX_PWD_LENGTH == 10
#error You need to increase MAX_PWD_LENGTH to at least 20.
#error See the comment near these errors for more explanation.
#endif

/**********************************************************************
* Structures                                                          *
**********************************************************************/
#ifndef _MINGW_
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;
#endif

typedef signed char   sbyte;
typedef unsigned char ubyte;
typedef short int   sh_int;
typedef short int   ush_int;
#if !defined(__cplusplus) /* Anyone know a portable method? */
typedef char      bool;
#endif

#if !defined(CIRCLE_WINDOWS) || defined(LCC_WIN32)  /* Hm, sysdep.h? */
typedef char      byte;
#endif

typedef int room_vnum;  /* A room's vnum type */
typedef int obj_vnum; /* An object's vnum type */
typedef int mob_vnum; /* A mob's vnum type */
typedef int zone_vnum;  /* A virtual zone number. */

typedef int room_rnum;  /* A room's real (internal) number type */
typedef int obj_rnum; /* An object's real (internal) num type */
typedef int mob_rnum; /* A mobile's real (internal) num type */
typedef int zone_rnum;  /* A zone's real (array index) number.  */

#define PATH_SWIM   0x01
#define PATH_WATER   0x02
#define PATH_UNDERWATER  0x04
#define PATH_FLY   0x08

/* used in graph.cpp */
struct CPathRetDir {
    bool Portal;
    int Direction;
};


/************* WARNING **********************************************/
/* This structure describe new bitvector structure                  */
typedef long int bitvector_t;
struct new_flag {
    int flags[4];
};

const struct new_flag clear_flags = { {0, 0, 0, 0} };

#define INT_ZERRO (0 << 30)
#define INT_ONE   (1 << 30)
#define INT_TWO   (2 << 30)
#define INT_THREE (3 << 30)
#define GET_FLAG(value,flag)  ((unsigned long)flag < (unsigned long)INT_ONE   ? value.flags[0] : \
                               (unsigned long)flag < (unsigned long)INT_TWO   ? value.flags[1] : \
                               (unsigned long)flag < (unsigned long)INT_THREE ? value.flags[2] : value.flags[3])


/* Extra description: used in objects, mobiles, and rooms */
struct extra_descr_data {
    char *keyword;                 /* Keyword in look/examine          */
    char *description;             /* What to see                      */
    struct extra_descr_data *next; /* Next in list                     */
};


#define TYP_ROOM 0
#define TYP_MOB  1
#define TYP_OBJ  2

// ��������� ��� �������� ���������/��������
struct db_load_data {
    int vnum; //VNUM ������������
    byte chance; //����������� ��������
    struct db_load_data *next;
};


// ������ ��������� ��������� �� �����
struct mess_p_data {
    int command; //����� �������
    int stoping; // 1 �� ����� ���������
    int script;
    char *sarg;
    char *mess_to_char;//��������� ������ (����� act)
    char *mess_to_vict;
    char *mess_to_other;
    char *mess_to_room;//��������� � ������� (����� act)
    struct mess_p_data *next; //��������� ������� � ������
};

/* object-related structures ******************************************/


/* object flags; used in obj_data */
#define NUM_OBJ_VAL_POSITIONS 4

struct obj_flag_data {
    int  value[NUM_OBJ_VAL_POSITIONS];
    int  cover[NUM_OBJ_VAL_POSITIONS]; //�������� �������
    bitvector_t bvalue[NUM_OBJ_VAL_POSITIONS];
    int    bnotfit[NUM_ITEMS];
    byte   type_flag;          /* Type of item             */
    int    type_tool;  /*  ��� ����������� */
    int    wear_flags;         /* Where you can wear it      */
    struct new_flag extra_flags; /* If it hums, glows, etc.      */
    int    weight;           /* Weigt what else              */
    int   countitems;
    int    cost;               /* Value when sold (gp.)        */
    int    reciept;  //����� ������� ��� ��������
    int    Obj_timer;          /* Timer for object             */
    int    Obj_time_load; /*  ����� �������� �������� */
    struct new_flag bitvector; /* To set chars bits            */

    struct new_flag affects;
    struct new_flag anti_flag;
    struct new_flag no_flag;
    int    Obj_sex;
    int    Obj_max;
    int    Obj_cur;
    int    Obj_mater;
    int    Obj_owner;
    int    Obj_destroyer;
    int    Obj_zone;
    sbyte  Obj_temp; //���������� ������� (-3..0..+3) ����������
    sbyte  Obj_temp_add; //���������� ������� (-3..0..+3) �����������
    byte   quality; //�������� ��������
    byte   type_arm;  /* ��� ����� */
    int light;
    int bright;
};

struct skill_modify_type {
    int skill_no;
    int mod;
};

/* Used in obj_file_elem *DO*NOT*CHANGE* */
struct obj_affected_type {
    int location;      /* Which ability to change (APPLY_XXX) */
    int modifier;     /* How much it changes by              */
};

struct C_obj_affected_type {
    sh_int type;            /* The type of spell that caused this      */
    sh_int duration;        /* For how long its effects will last      */
    int modifier;        /* This is added to apropriate ability     */
    int level;
    byte   location;        /* Tells which ability to change(APPLY_XXX)*/
    bitvector_t bitvector;       /* Tells which bits to set (AFF_XXX) */
    bitvector_t extra;
    bitvector_t no_flag;
    bitvector_t anti_flag;
    bool nomagic; //true ���� ������ �� ����������
    bool main;
    long owner; //ID ��������� �������
    struct C_obj_affected_type *next;

    C_obj_affected_type():
            type(0), duration(0), modifier(0), level(0), location(0), bitvector(0), extra(0), no_flag(0),
            anti_flag(0), nomagic(0), owner(0), next(NULL) {}
};

struct vars_data {
    DLString value; //��������
    int time;     //����� �������� ���������� � �������� (-1 ��� �����)
    vars_data():
            time(0) {}
};

// ���� - ��� ����������
struct vars_storage : public map<string, struct vars_data> {
    void add(const char *key, const char *value, int time, bool override);
    void pulse(int pulse);
    const char * check(const char *key);
};

// ���� - ������, �������� - ���������� �� ������ (DIST_0, DIST_1)
typedef map<const struct char_data *, int> distance_list;

typedef set<const struct char_data *> char_visible_list;

// ���� - unique id ������, ������� �������� �������; �������� - ������� ��� ��������
typedef map<long, int> flee_by_list;

struct material_data_list {
    int number_mat; //��� ���������
    int value;     // ����������
    int main; // ��������
    struct material_data_list *next;
};

//���������� ����������
struct material_data {
    int number;  // ��� ���������
    char *name;  // ��������
    int weight;  // ���
    int price;  // ����
    byte durab;  // ���������
    int ac;  // �����
    int  armor0;  // �������
    int armor1;  // �������
    int armor2;  // �������
    float hitX;  // X (XdY+Z)
    float hitY;  // Y
    float hitZ;  // Z
    int type;
    int include;
    byte save[7];  //�����������

    struct material_data *next;
};

struct transpt_data_people {
//���������� � ������� � ������
    struct char_data *ch;
    struct transpt_data_people *next;
};

struct transpt_data_obj {
    new_flag  transp_flag; //���� �������
    struct char_data * engine; //���������
    struct char_data * driver; //�������
    std::vector<struct char_data *> people;   //������ ������� � ������
    transpt_data_obj():
            transp_flag(clear_flags), engine(NULL), driver(NULL) {}
};

/* ������� �� �������� */
struct obj_trap_data {
    obj_trap_data();

    byte shance;   /* ������� ������� 0-���, 1-������, 100-������� */
    byte damnodice;           /* ����������� �� ������� */
    byte damsizedice;         /* dam = damnodice�damsizedice+damage */
    byte damage;   /*  */
    byte tbreak;   /* ������� �������/����������� */
    byte save;                    /* ���� �������� ������� */
    byte type_hit;  /* ��� ����� */
    char_visible_list tfind; /*������ ���������� ��� ����� ������� */
    char *trap_damage_char; /* ��������� � ����������� - ������ */
    char *trap_damage_room; /* ��������� � ����������� - ������� */
    char *trap_nodamage_char; /* ��������� � ������� ����������� - ������ */
    char *trap_nodamage_room; /* ��������� � ������� ����������� - ������� */
    char *trap_kill_char;  /* ��������� � ������ ������ �� ������� - ������ */
    char *trap_kill_room;         /* ��������� � ������ ������ �� ������� - ������� */
};


struct item_op_data {
    int command;   //�������
    int obj_vnum;  //��������.�������
    char *active;         //���������
    char *active_room; //���������_�������
    char *arg;     //��������.������
    int mob_vnum;  //��������.������
    char *error;   //��������.������
    int script_vnum; //������
    int script;
    int tool; //����������
    int extract;
    int load_room;
    int load_char;
    char *mess_load_char;
    char *mess_load_room;
    void *expr;
    char *error_char;
    char *error_room;

    struct item_op_data *next;

    item_op_data():
            command(0), obj_vnum(0), active(NULL), active_room(NULL), arg(NULL), mob_vnum(0),
            error(NULL), script_vnum(0), tool(0), extract(0), load_room(0), load_char(0),
            mess_load_char(NULL), mess_load_room(NULL), expr(NULL), error_char(NULL), error_room(NULL),
            next(NULL) {}
};


struct weapon_damage_data {
    int type_damage; //��� �����������, (�������, ����� � �.�.)
    int min_damage; //����������� �����������
    int max_damage;  //������������ �����������
    struct weapon_damage_data *next;
    weapon_damage_data():
            type_damage(0), min_damage(0), max_damage(0) {}
};

struct weapon_data {
    struct weapon_damage_data *damages; //������ �����������
    int critics; //����������� ������������ ���������
    int speed; //����������� ��������
    int skill; //������ ������ (���, ���, ������)
    int message; //�������, �������, �������
};

struct spec_weapon_data {
    int shance;
    bool xrace[NUM_RACES];
    bool zrace[NUM_RACES];
    bool xmtype[NUM_TMOBS];
    bool zmtype[NUM_TMOBS];
    char *to_char;
    char *to_vict;
    char *to_room;
    float koef;
    int type_damage;
    int min_damage;
    int max_damage;
    int spell;
    int level;
    int target;
    struct spec_weapon_data *next;
};

struct missile_data {
    int type;
    struct weapon_damage_data *damages; //������ �����������
};

/* ================== Memory Structure for Objects ================== */
struct obj_data : public WrapperTarget {
    obj_vnum item_number;  /* Where in data-base        */
    room_rnum in_room;   /* In what room -1 when conta/carr */

    struct obj_flag_data     obj_flags;/* Object information       */
    struct obj_affected_type affected[MAX_OBJ_AFFECT];  /* ������������ */
    struct C_obj_affected_type *C_affected; /* ����� ����������� */
    int skill_modify[MAX_SKILLS]; /* ������������ ������� */

    struct weapon_data *weapon; //�������� ���������� ������
    struct missile_data  *missile; //�������� ���������� �������
    struct spec_weapon_data *spec_weapon;
    struct item_op_data *operations;
    char *names;   /* �������� ��������� � xformate */
    char *name;                    /* Title of object :get etc.        */
    char *description;         /* When in room                     */
    char *main_description;
    char *short_description;       /* when worn/carry/in cont.         */
    char *action_description;
    struct extra_descr_data *ex_description; /* extra descriptions     */
    struct char_data *carried_by;  /* Carried by :NULL in room/conta   */
    struct char_data *worn_by;   /* Worn by?           */
    short  int worn_on;          /* Worn where?          */
    int  limit;   /* ������ */
    int owner;   /* ������� ������ ID ��������� */

    struct material_data_list *materials; //��������� ��������
    struct P_damage_weapon *damage_weapon; //������ ����������� ������� /* NOTUSED */

    struct obj_data *in_obj;       /* In what object NULL when none    */
    struct obj_data *contains;     /* Contains objects                 */

    new_flag bflag;     // ����� ��� �����������
    new_flag bflag_reset;    // ����� ��� ���������� �����
    struct list_obj_data *load_obj; //������ ��������� ��� ��������
    struct obj_trap_data *trap;   //�������
    int bgold;   //���-�� �����

    long id;                       /* used by DG triggers              */

    struct obj_data *next_content; /* For 'contains' lists             */
    struct obj_data *next;         /* For the object list              */

    struct transpt_data_obj *transpt; /* ��������� */

    struct char_data *trap_victim;  /* ��� ����� � ������ */

    char_visible_list visible_by;   /* ��� ����� ���� ������� */

    int page;  /* ����� �������� ��� ����� */
    int  shance;  /* ����������� �������� �������� � ���� */
    int    room_was_in;
    char   *PNames[NUM_PADS];
    long killer; /* ID ������ ����� */
    ubyte powered; /* �������� �������� */
    struct mess_p_data *mess_data; /* ��������� */
    char lock_code[32];
    int  lock_step;
    std::vector<int> quests;

    long long guid;

    obj_data();
    obj_data(const obj_data &);
    obj_data(int vnum);
    virtual ~obj_data();

    bool isProto() const;
    const struct obj_data * getProto() const;
    struct obj_data * getProto();
    
    void linkWrapper();
    bool hasWrapper() const;

protected:
    void init();
};
/* ======================================================================= */


/* ====================== File Element for Objects ======================= */
/*                 BEWARE: Changing it will ruin rent files      */
struct time_write {
    int year, month, day;
};

struct obj_file_elem {
    obj_vnum item_number;

#if USE_AUTOEQ
    short  int location;
#endif
    int    value[NUM_OBJ_VAL_POSITIONS];
    struct new_flag extra_flags;
    int    weight;
    int    timer;
    int    maxstate;
    int    curstate;
    int    owner;
    int    wear_flags;
    int    mater;
    struct new_flag bitvector;
    struct obj_affected_type affected[MAX_OBJ_AFFECT];
};


/* header block for rent files.  BEWARE: Changing it will ruin rent files  */
struct save_rent_info {
    int  time;
    int  rentcode;
    int  net_cost_per_diem;
    int  gold;
    int  account;
    int  nitems;
    int  oitems;
    int  spare1;
    int  spare2;
    int  spare3;
    int  spare4;
    int  spare5;
    int  spare6;
    int  spare7;
};

struct save_time_info {
    int vnum;
    int timer;
};

struct save_info {
    struct save_rent_info rent;
    struct save_time_info time[2];
};

/* ======================================================================= */


/* room-related structures ************************************************/

struct room_direction_data {
    room_direction_data();

    char *general_description;     /* �������� ������ */

    //��������� ������
    char *exit_name;  /* ��� ������ � ������� */
    char *keyword;  /* �������� ����� */
    struct new_flag exit_data; /* ����� �� ����� */
    struct new_flag exit_data_reset; /* ����� �� ����� �����*/
    int type;   /* ��� ������ */

    byte material;  /* �������� ����� */
    byte quality;   /* �������� ����� 1-�������, 100 �����-������� */
    byte shance;   /* ������� ������� 0-���, 1-������, 100-������� */
    byte damnodice;           /* ����������� �� ������� */
    byte damsizedice;         /* dam = damnodice�damsizedice+damage */
    byte damage;   /*  */
    byte tbreak;   /* ������� �������/����������� */
    byte save;                    /* ���� �������� ������� */
    byte type_hit;  /* ��� ����� */
    char_visible_list tfind; /*������ ���������� ��� ����� ������� */
    char *trap_damage_char; /* ��������� � ����������� - ������ */
    char *trap_damage_room; /* ��������� � ����������� - ������� */
    char *trap_nodamage_char; /* ��������� � ������� ����������� - ������ */
    char *trap_nodamage_room; /* ��������� � ������� ����������� - ������� */
    char *trap_kill_char;  /* ��������� � ������ ������ �� ������� - ������ */
    char *trap_kill_room;         /* ��������� � ������ ������ �� ������� - ������� */
    byte sex;
    char lock_code[32];
    int  lock_step;

    //���������� � ������� - ������
    sh_int  exit_info;             /* Exit info          */
    obj_vnum key;                  /* Key's number (-1 for no key)*/
    room_rnum to_room;           /* Where direction leads (NOWHERE)*/

    // ���������� � ��������
    byte type_port;  /* ��� ������� 0 - ���, 1 - ����������,
                                               2 - ���������,
               3 - ������������ */
    obj_vnum  key_port;  // VNUM ���� �������
    room_vnum room_port;  // VNUM ������� ���� ����� ����

    int time;            // ������� ����� ������������
    int timer;           // ������ �������
    int open;  // ����� ��������
    int close;  // ����� ��������
    int active_room;     // �������� ������� �����������
    byte tStart;         // � ����� ����� �������� ��������
    byte tStop;  // � ����� ����� ���������� ��������
    char *portal_description; // �������� ������ ��� ���.�������
    char *mess_to_open;          // ��������� �� �������� �������
    char *mess_to_close;  // ��������� � �������� �������
    char *mess_char_enter;       // ��������� ������ ��� ��������� �������
    char *mess_room_enter; // ��������� � ������� ��� �������� �������

};

struct room_damage_data {
    byte chance;   //���� ������������
    byte type_hit;  //��� �����������
    long type;   //�������� �����������
    byte save;   // ������
    byte damnodice;           /* ���� ����������� */
    byte damsizedice;         /* dam = damnodice�damsizedice+damage */
    byte damage;   /*  */
    char *damage_mess_char; //��������� ������ � �����������
    char *damage_mess_room;       //��������� � ������� � ����������� ������
    char *nodamage_mess_char;     // ���� ����� �� ��������
    char *nodamage_mess_room;     // ���� ����� �� ��������
};

struct room_trap_data {
    byte direction;
    byte chance;
    byte type_hit;
    byte type;
    byte save;
    byte damnodice;           /* ���� ����������� */
    byte damsizedice;         /* dam = damnodice�damsizedice+damage */
    byte damage;   /*  */
    char *trap_active_char; //��������� �������
    char *trap_active_room; //����������� �������
    char *damage_mess_char; //��������� ������ � �����������
    char *damage_mess_room;       //��������� � ������� � ����������� ������
    char *nodamage_mess_char;     // ���� ����� �� ��������
    char *nodamage_mess_room;     // ���� ����� �� ��������
    char *kill_mess_char;      // ���� - ������
    char *kill_mess_room;          // ���� - �������
};

struct room_forcedir_data {
    int vnum;
    byte dir;
    byte open;
    byte close;
    int timer;
    byte damage_type;
    byte damnodice;           /* ���� ����������� */
    byte damsizedice;         /* dam = damnodice�damsizedice+damage */
    byte damage;   /*  */
    byte save;
    byte save_type;
    char *mess_fail_char;
    char *mess_fail_room;
    char *mess_exit_char;
    char *mess_exit_room;
    char *mess_enter_char;
    char *mess_enter_room;
    char *mess_kill_char;
    char *mess_kill_room;

    struct room_forcedir_data *next;
};

struct   track_data {
    int    track_info;                 /* bitvector */
    long   who;                        /* real_number for NPC, IDNUM for PC */
    int    time_income[6];             /* time bitvector */
    int    time_outgone[6];
    struct track_data *next;
};

struct room_affect_data {
    int type;
    int duration;
    int level;
    sh_int modifier;
    bitvector_t bitvector;
    long owner;
    struct room_affect_data *next;
};

struct weather_control {
    int  rainlevel;
    int  snowlevel;
    int  icelevel;
    int  sky;
    int  weather_type;
    int  duration;
    //����� ���������
    int  light;
    int  fog;
    int  rain;
};

struct hotel_data {
    int  type;
    int  master;
    char *MessChar;
    char *MessRoom;
    char *MessReturn;
};

struct room_period_data {
    int start; //������
    int stop;      //�����
    int weather;
    int object;
    int monster;
    char *start_room;
    char *stop_room;
    char *start_zone;
    char *stop_zone;

    struct room_period_data *next;
};

/* ================== Memory Structure for room ======================= */
struct room_data : public WrapperTarget {
    room_data();
    room_data(int vnum);
    
    void init();
    void linkWrapper();
    bool hasWrapper() const;

    room_vnum number;        /* Rooms number (vnum)              */
    zone_rnum zone;              /* Room zone (for resetting)          */
    int  spellhaunt;             /* Spellhaunt */
    int  sector_type;            /* sector type (move/hide)            */
    int  sector_state;           /**** External, change by weather     */
    char *name;                  /* Rooms name 'You are ...'           */
    char *description;           /* Shown when entered                 */
    char *description_night;     /* ������ �������� */
    struct extra_descr_data *ex_description; /* for examine/look       */
    struct room_direction_data *dir_option[NUM_OF_DIRS]; /* Directions */
    struct new_flag room_flags;  /* DEATH,DARK ... etc */
    struct room_damage_data *damage; /* �������� ����������� */
    struct room_trap_data *trap_option[NUM_OF_DIRS]; /* �������� ������� */
    struct mess_p_data *mess_data; /* ��������� */
    struct room_forcedir_data *forcedir; /* ������� */
    struct db_load_data *mobiles; //������ ��� ��������
    struct db_load_data *objects; //�������� ��� ��������
    struct room_period_data *period; //������
    sbyte    light;                     /* Number of lightsources in room */
    sbyte    glight;                    /* Number of lightness person     */
    sbyte    gdark;                     /* Number of darkness  person     */
    struct  weather_control weather;   /* Weather state for room */

    SPECIAL(*func);
    char   *func_name;         /* name of special function     */

    struct track_data      *track;

    struct obj_data *contents;   /* List of items in room              */
    struct char_data *people;    /* List of NPC / PC in room           */

    struct room_affect_data *affects; /* ������� �� ������� */

    ubyte  fires;                /* Time when fires                    */
    ubyte  forbidden;            /* Time when room forbidden for mobs  */
    ubyte  ices;                 /* Time when ices restore */
    int    portal_room;
    ubyte  portal_time;
    byte blood;                  //���-�� ����� � �������
    char_visible_list p_sphere;   /* ��� ������ � �������������� ����� */
    struct hotel_data *hotel;
    byte path_label;

    long long guid;
};
/* ====================================================================== */


/* char-related structures ************************************************/


/* memory structure for characters */
struct memory_rec_struct {
    long   id;
    long   time;
    struct memory_rec_struct *next;
};

typedef struct memory_rec_struct memory_rec;


/* This structure is purely intended to be an easy way to transfer */
/* and return information about time (real or mudwise).            */
struct time_info_data {
    int hours, day, month;
    sh_int year;
};


/* These data contain information about a players time data */
struct time_data {
    time_t birth;    /* This represents the characters age                */
    time_t logon;    /* Time of the last logon (used to calculate played) */
    int  played;     /* This is the total accumulated time played in secs */
};


/* general player-related info, usually PC's and NPC's */
struct char_player_data {
    void free(const struct char_player_data *proto);

    char passwd[MAX_PWD_LENGTH+1]; /* character's password      */
    char *name;         /* PC / NPC s name (kill ...  )         */
    char *names;
    char *short_descr;  /* for NPC 'actions'                    */
    char *long_descr;   /* for 'look'             */
    char *charm_descr;  /* for look in charm */
    char *charm_descr_me;  /* for look in charm */
    char *description;  /* Extra descriptions                   */
    struct extra_descr_data *ex_description; /* extra descriptions     */
    char *title;        /* PC / NPC's title                     */
    char *title_r; /* ������������� ����� */
    byte sex;           /* PC / NPC's sex                       */
    byte chclass;       /* PC / NPC's class           */
    byte race;
    byte mob_type; //���
    byte mob_vid; //������
    sbyte level;         /* PC / NPC's level                     */
    sbyte extra_level;         /* PC / NPC's level                     */
    byte fraction; /* ������� */
    byte rank;  /* ���� ������� */
    sbyte hlevel[LVL_MAX+1];    /* ������� ������� ������ */
    byte haddhit[LVL_MAX+1];   /* ������� ht-add ������ */
    byte haddmove[LVL_MAX+1];   /* ������� move-add ������ */
    byte haddmana[LVL_MAX+1];   /* ������� mana-add ������ */
    byte gods;
    int  hometown;      /* PC s Hometown (zone)                 */
    struct time_data time;  /* PC's AGE in days                 */
    int weight;       /* PC / NPC's weight                    */
    int height;       /* PC / NPC's height                    */
    int remort;

    char* PNames[NUM_PADS];
    ubyte Religion;
    ubyte Side;
    ubyte Lows;
    struct char_data *current_quest_mob;
    int select_mode;
    long honor;
};


/* Char's abilities.  Used in char_file_u *DO*NOT*CHANGE* */
struct char_ability_data {
    ubyte Skills[MAX_SKILLS+1];  /* array of skills plus skill 0   */
    ubyte SkillsLagRound[MAX_SKILLS+1]; /* ��� � ����� ��� ��������� ������ */
    ubyte ExtraSkills[MAX_SKILLS+1];
    ubyte SplKnw[MAX_SPELLS+1];  /* array of SPELL_KNOW_TYPE         */
    ubyte EntKnw[MAX_ENCHANT+1];  /* ������    */
    long SplMem[MAX_SPELLS+1];  /* ���-�� ������ ����������� ���������� */
    sbyte str;
    sbyte intel;
    sbyte wis;
    sbyte dex;
    sbyte con;
    sbyte cha;
    int size;
    int hitroll;
    int damroll;
    int armor;
    sbyte lck; // ����� ������
    sh_int health; // �������� ������ (0-�����, 100 - ������)
};

/* Char's additional abilities. Used only while work */
struct char_realtime_data {
    int hr_rt;
    int ac_rt;
};

struct char_played_ability_data {
    int  health_add; //���������� � ��������;
    int  lck_add; // ���������� � �����
    int  level_add;
    int  fact_level;
    int  str_add;
    int  intel_add;
    int  wis_add;
    int  dex_add;
    int  con_add;
    int  cha_add;
    int  str_roll;
    int  con_roll;
    int  dex_roll;
    int  intel_roll;
    int  wis_roll;
    int  cha_roll;
    int  weight_add;
    int  height_add;
    int  size_add;
    int  age_add;
    int  hit_add;
    int  powered_add;
    int  move_add;
    int  hitreg;
    int  movereg;
    int  manareg;
    sbyte  slot_add[10];
    int  armour[3];
    int  ac_add;
    int  ac_wear;
    int  hr_add;
    int  dr_add;
    int  absorb;
    int  rememory_add;
    int  cast_success;
    int  speed_add;
    int  poison_add;
    int  pray_add;
    int  mana_add;
    sh_int skills_add[MAX_SKILLS+1]; //���������� � ������
    sh_int apply_saving_throw_3[SAV_NUMBER]; /* Saving throw (3ed) */
    sh_int inc_magic[8];
};


/* Char's points.  Used in char_file_u *DO*NOT*CHANGE* */
struct char_point_data {
    sh_int hit;
    sh_int max_hit;      /* Max hit for PC/NPC                      */
    sh_int move;
    sh_int max_move;     /* Max move for PC/NPC                     */
    sh_int init_hit;
    sh_int init_move;
    sh_int init_mana;
    int  gold;           /* Money carried                           */
    long bank_gold;    /* Gold the char has in a bank account     */
// unsigned long long exp;
    long exp;
    long pk_counter;     /* pk counter list */
};


/*
 * char_special_data_saved: specials which both a PC and an NPC have in
 * common, but which must be saved to the playerfile for PC's.
 *
 * WARNING:  Do not change this structure.  Doing so will ruin the
 * playerfile.  If you want to add to the playerfile, use the spares
 * in player_special_data.
 */
struct char_special_data_saved {
    int  alignment;    /* +-1000 for alignments                */
    long idnum;      /* player's idnum; -1 for mobiles */
    struct new_flag act; /* act flag for NPC's; player flag for PC's */

    struct new_flag affected_by;
    /* Bitvector for spells/skills affected by */

};

struct mem_room_data {
    int vnum_room;
    long times;
    char *desc;
};

/* Special playing constants shared by PCs and NPCs which aren't in pfile */
struct char_special_data {
    struct char_data *fighting;  /* Opponent */
    struct char_data *hunting; /* Char hunted by this char */
    char hunt_name[256];
    int hunt_step;
    struct char_data *guarding; /* Guarding */
    struct obj_data  *obj_locate; //������ �� ������� ������
    struct char_data *char_locate; //������ �� ��������� ������
    int  locate_step; //���-�� �����

    //����������� ���������
    struct mem_room_data memory[40];
    int  lastmemory;

    byte   position;           /* Standing, fighting, sleeping, etc. */
    int    carry_weight;       /* Carried weight */
    int    carry_items;        /* Number of items carried  */
    int    wear_weight;
    int    timer;              /* Timer for update */
    int chage_distance; //��������� ��������� (-1 ����, 0, +1 ������)

    struct char_special_data_saved saved; /* constants saved in plrfile  */
};


/*
 *  If you want to add new values to the playerfile, do it here.  DO NOT
 * ADD, DELETE OR MOVE ANY OF THE VARIABLES - doing so will change the
 * size of the structure and ruin the playerfile.  However, you can change
 * the names of the spares to something more meaningful, and then use them
 * in your new code.  They will automatically be transferred from the
 * playerfile into memory when players log in.
 */
/*
 * Specials needed only by PCs, not NPCs.  Space for this structure is
 * not allocated in memory for NPCs, but it is for PCs and the portion
 * of it labelled 'saved' is saved in the playerfile.  This structure can
 * be changed freely; beware, though, that changing the contents of
 * player_special_data_saved will corrupt the playerfile.
 */
struct player_special_data_saved {
    byte PADDING0;       /* used to be spells_to_learn   */
    bool talks[MAX_TONGUE];  /* PC s Tongues 0 for NPC   */
    int  wimp_level;       /* Below this # of hit points, flee!  */
    int  freeze_level;   /* Level of god who froze char, if any  */
    int  invis_level;    /* level of invisibility    */
    room_vnum load_room;   /* Which room to place char in    */
    struct    new_flag pref; /* preference flags for PC's.   */
    int   bad_pws;   /* number of bad password attemps */
    int   conditions[3];         /* Drunk, full, thirsty     */

    /* spares below for future expansion.  You can change the names from
       'sparen' to something meaningful, but don't change the order.  */

    int Side;              /****/
    int Alignment;
    int Religion;          /****/
    int Race;              /****/
    int Lows;              /****/
    int DrunkState;
    int Prelimit;
    int glory;
    int olc_zone;
    int unique;
    int HouseRank;
    int NameGod;
    int sw;  // player mud-client screen width
    int sh; // player mud-client screen high
    int speaking; // �� ����� ����� ������� ���� player
    int Improove[MAX_SKILLS]; // ���� ���������� �� ����� ��� ��� ���
    int spare15;
    int remort;

    long NameDuration;
    long GodsLike;
    long GodsDuration;
    long MuteDuration;
    long FreezeDuration;
    long HellDuration;
    long HouseUID;
    long LastLogon;
    long DumbDuration;
    long NameIDGod;
    long spare0A;
    long spare0B;
    long spare0C;
    long spare0D;
    long spare0E;
    long spare0F;

    char EMail[128];
    char cAnswer[128];
    char spare001[128];
    char spare002[128];
    char spare003[128];
    char LastIP[128];
    char  remember[MAX_REMEMBER_TELLS][MAX_INPUT_LENGTH];
    int   lasttell;
};

struct player_special_data {
    player_special_data();
    virtual ~player_special_data();

    struct player_special_data_saved saved;
    char  *poofin;       /* Description on arrival of a god. */
    char  *poofout;        /* Description upon a god's exit.   */
    struct alias_data *aliases;      /* Character's aliases    */
    long   last_tell;        /* idnum of last tell from    */
    int    may_rent;                 /* PK control                       */
	bool msg_zero_dsu;       // ����, ������� ����������, ��� ���� �� ���� ������ ��� ���� ��������� � ���, ��� ���� ���� � �������
};

// ������ ������ �����-����������
typedef set<int> helper_data_type;

/* Specials used by NPCs, not PCs */
struct mob_special_data {
    mob_special_data();
    virtual ~mob_special_data();

    byte last_direction;     /* The last direction the monster went     */
    int  attack_type;        /* The Attack Type Bitvector for NPC's     */
    int  attack_type2;        /* The Attack Type Bitvector for NPC's     */
    byte default_pos;        /* Default position for NPC                */
    memory_rec *memory;      /* List of attackers to remember         */  /* NOTUSED */
    byte damnodice;          /* The number of damage dice's             */
    byte damsizedice;        /* The size of the damage dice's           */
    byte damage;
    byte damnodice2;
    byte damsizedice2;
    byte damage2;
    byte armor[3];     /* ���������� �������� ������ */
    room_rnum  move_to; /* ���� �� -1, �� ������ ������ � ��� ������� */
    int  curr_dest;
    int  dest[MAX_DEST];
    int  dest_dir;
    int  dest_pos;
    int  dest_count;
    int  activity;
    struct new_flag npc_flags;
    byte ExtraAttack;        /* ���-�� ���� */
    byte ExtraAttack2;        /* ���-�� ���� */
    byte LikeWork;
    byte MaxFactor;
    int  GoldNoDs;
    int  GoldSiDs;
    int  HorseState;
    int  LastRoom;
    char *Questor;   /* NOTUSED */
    int  SpecType;  /* ��� ���� ����� (1 ����������; 2 ����) */ /* NOTUSED */
    int  SpecSkill; /* id ��������� (���������� ��� ��� �����*/ /* NOTUSED */
    byte SpecDamDice; /* NOTUSED */
    byte SpecDamSize; /* NOTUSED */
    byte SpecDamRoll; /* NOTUSED */
    char *MessToVict; /* ��������� ������*/ /* NOTUSED */
    char *MessToRoom; /* ��������� � ������� */ /* NOTUSED */
    int  LoadRoom; /* ����������� ����������� ������� */
    int  speed;
    int  wimp_level;
    struct obj_data *transpt; /* ��������� ������� ����� ch */
    byte move_type; /* ����������� ��������� � ����������� */
    char *move_str;
    int vnum_horse; /* VNUM ������ ������������� ��� �������� */
    //��������� ����������� � ������ �������
    int vnum_corpse; // ���� ������-�����
    int death_script; //�������� ��� ������
    char *CMessChar; // ��������� ������ ��� ������
    char *CMessRoom; // ��������� � ������� ��� ������
    int d_type_hit;  // ��� ����� ��� ������
    byte DamageDamDice; // ���� �����
    byte DamageDamSize;
    byte DamageDamRoll;
    byte death_flag; //����� �����������
    char *DMessChar; //��������� ������
    char *DMessRoom; //��������� � �������
    ubyte powered; //��������
    ubyte age; //������� ����

    int saved[NUM_SAV]; //����������
    /* ��������� ���������� */
    bool AlrNeed;
    char *AlrMessChar; //C�������� ����������
    char *AlrMessRoom; //C�������� � �������
    int AlrLevel; //������ % �������� ����� ���� ������ �������� �����
    helper_data_type alr_helper; //������ ����������
};


/* An affect structure.  Used in char_file_u *DO*NOT*CHANGE* */
struct affected_type {
    int type;            // The type of spell that caused this
    int duration;        // For how long its effects will last
    int modifier;         // This is added to apropriate ability
    int level;
    byte location;        // Tells which ability to change(APPLY_XXX)
    long bitvector;         // Tells which bits to set (AFF_XXX) */
    long owner;            //ID ������ �� ��������� �������
    bool main;     //������� ���� ��� ���� ������ �������� � ������
    bool battleflag;        //������� ���� ����� ������� ���������� � ����-�������
    bool nomagic; //true ���� ������ �� ����������
    struct affected_type *next;
};

struct timed_type {
    ubyte      skill;               /* Number of used skill/spell */
    int        time;                /* Time for next using        */
    struct     timed_type *next;
};

//��������� �������� ���������
struct P_message {
    P_message();

    bool valid;  // ������� ���� ��� ��������� ���� ��������� ���������
    char *mChar; //������
    char *mVict;
    char *mRoom;

    char *hChar; //�����������
    char *hVict;
    char *hRoom;

    char *kChar; //������
    char *kVict;
    char *kRoom;

    char *pChar; //������������
    char *pVict;
    char *pRoom;

    char *aChar; //���������� (� ���������)
    char *aVict;
    char *aRoom;

    char *bChar; //������.���������� (��� ��������)
    char *bVict;
    char *bRoom;

    char *tChar; //������
    char *tVict;
    char *tRoom;

    char *sChar; //������
    char *sVict;
    char *sRoom;

};

/* ��������� �������� ���������� ����� */
struct P_damage {
    P_damage();
    bool valid; // ��������� ���� ������������������� 
    int type; //��� �����������
    int power; //��������
    int dam; //���� �����������
    int magic; //��� ���������� �����������
    int far_min; //����������� ���-��� �������� �����
    bool deviate; //������������ ���-��� �������� �����
    int check_ac; //��������� AC 0-������ ������, 1-������, 2-������ ������
    bool location; //1 �� ��������� �������
    bool critic;
    bool armor;
    int hLoc;
    struct obj_data *weapObj;
};


// ���������� ����������� � ����� ������ (���� �� ������������, ����������������)
struct P_last_damage {
    long id_ch;
    struct P_damage damage;
    struct P_message message;
    P_last_damage();
};

// ���� - ����� �����, �������� - ��������� �������� �����������
typedef map<int, P_last_damage> LastDamageStorage; 


struct body_part {
    int wear_position;     //������� ���� ����� �����
    string name;            //��������
    int chance;            // ���� ������� (1-100%)
    int kdam;  //������� ���������� �����������
};


/* Structure used for chars following other chars */
struct follow_type {
    struct char_data *follower;
    int type;
    struct follow_type *next;
};

/* Structure used for char PK-memory */
// ���� - unique id ���������, �������� - �� ����� ����� �������� ���� (� �����)
typedef map<long, int> PK_Memory_type;

typedef set<struct char_data *> Killer_list_type;

/* Structure used for extra_attack - bash, kick, diasrm, chopoff, etc */
struct extra_attack_type {
    int    used_skill;
    struct char_data *victim;
};

struct cast_attack_type {
    int    spellnum;
    struct char_data *tch;
    struct obj_data  *tobj;
};

/* Structure used for tracking a mob */
struct track_info {
    int  track_info2;
    int  who;
    int  dirs;
};

/* Structure used for helpers */

/* Structure used for questing */
struct quest_data {
    int count;
    int *quests;
};

struct mob_kill_data {
    int count;
    int *howmany;
    int *vnum;
};

struct script_exp_data {
    int count;
    int *howmany;
    int *vnum;
};


struct list_obj_data {
    int percent;
    int vnum;
    struct list_obj_data *next;
};

struct mob_shop_data {
    bool ok; //�������
    int buy; //�������
    int sell; //�������
    int repair; //������
    byte quality; //����������
    struct list_obj_data *obj_type; //�����
    struct list_obj_data *obj_list; //������
    bitvector_t bitvector; //�������
};

struct mob_spechit_data {
    byte type; //���
    int percent; //����
    int pos[POS_NUMBER]; //���������
    int hit; //����������
    int spell; //����������
    int power; //���� ������
    ubyte damnodice;           /* ����������� �� ���������� */
    ubyte damsizedice;         /* dam = damnodice�damsizedice+damage */
    sh_int damage;   /*  */
    char *to_victim;
    char *to_room;
    int saves[NUM_SAV]; //����������
    struct new_flag property; //�������

    struct mob_spechit_data *next;
};

struct mob_period_data {
    int start;
    int stop;
    int period;
    int target;
    char *to_char;
    char *to_room;

    struct mob_period_data *next;
};

struct req_var_data {
    string title;
    string name;
    string value;
    string current;
};

struct quest_list_data {
    quest_list_data();

    int mob_vnum; //Vnum ������� ������� ��� �����
    int number;   //����� ������ � �������
    bool complite; //������� ���������� ������
//����������
    std::vector<struct req_var_data>   done_req_var; //������ ���������� (���������)
    std::vector<struct bug_abuse_data> done_mobiles; //������ �������� (������)
    std::vector<struct bug_abuse_data> done_objects; //������ ��������� (������)
    std::vector<struct bug_abuse_data> done_followers; //������ ��������������
};

struct mob_qscript_data {
    int number;
    void *expr;
    char *text;
    int script;
    struct mob_qscript_data *next;
};

struct mob_quest_data {
    mob_quest_data();

    int number;
    char *name;
    void *expr;
    char *text;
    char *pretext;
    char *complite;
    bool multi;
    std::vector<struct req_var_data> req_var;
    std::vector<struct bug_abuse_data> mobiles;
    std::vector<struct bug_abuse_data> objects;
    std::vector<struct bug_abuse_data> followers;
    int accept;
    int done;

    struct mob_quest_data *next;
};

struct event_mob_data {
    int command;
    int argument;
    int script;

    struct event_mob_data *next;
};


/* ================== Structure for player/non-player ===================== */
struct Player;
struct Mobile;

struct char_data : public WrapperTarget {
    int pfilepos;       /* playerfile pos              */
    mob_rnum nr;              /* Mob's rnum                */
    room_rnum in_room;        /* Location (real room number)   */
    room_rnum was_in_room;  /* location for linkdead people  */
    int wait;         /* wait for how many loops       */

    struct body_part     *body;     //����� ����
    int       ibody;
    //�����
    int set_number;
    int set_variante;
    int set_change;
    int set_message_var;
    int set_message_num;
    //������������
    int init_classes[NUM_CLASSES];
    int classes[NUM_CLASSES];
    int add_classes[NUM_CLASSES];
    //
    struct item_op_data *operations; /* ����������� */ /* NOTUSED */
    struct event_mob_data *event_mob;
    struct char_player_data         player;          /* Normal data                   */
    struct char_played_ability_data add_abils;       /* Abilities that add to main */
    struct char_realtime_data    realtime_abils;
    struct char_ability_data        real_abils;      /* Abilities without modifiers   */
    struct char_point_data          points;          /* Points                       */
    struct char_special_data        char_specials; /* PC/NPC specials    */
    struct mob_shop_data     *shop_data;
    struct affected_type *affected;       /* affected by what spells       */
    struct timed_type    *timed;          /* use which timed skill/spells  */
    struct obj_data *equipment[NUM_WEARS];/* Equipment array               */
    struct obj_data *tatoo[NUM_WEARS];/* Equipment array               */
    struct list_obj_data *load_death;   /* �������� ����������� ����� ������ */
    struct list_obj_data *load_eq;
    struct list_obj_data *load_inv;
    struct list_obj_data *load_tatoo;
    struct mob_spechit_data *spec_hit; //��������
    int  death_count; //���-�� �������
    int  ress_count; //���-�� ����������
    int  capsoul_count; //���-�� ���������

    struct obj_data *carrying;            /* Head of list                  */
    struct descriptor_data *desc;         /* NULL for mobiles              */

    long ticks_in_mud;   // ���-�� ����� ���������� � ����
    long id;                            /* used by DG triggers             */
    struct script_memory *memory;       /* for mob memory triggers         */ /* NOTUSED */
    struct mess_p_data *mess_data; /* ��������� */
    struct mob_period_data *period; //������

    struct char_data *next_in_room;     /* For room->people - list         */
    struct char_data *next;             /* For either monster or ppl-list  */
    struct char_data *next_fighting;    /* For fighting list               */
    struct obj_data *trap_object;

    char_visible_list visible_by;   /* ��� ����� ����� ��������� */
    flee_by_list flee_by; /* ��� ������� ������� �� ��������� */
    distance_list distance; /* ��������� �� ����� */
    char_visible_list not_attack;   /* ��� �� ����� ��������� ��������� (�����) */
    char_visible_list may_attack;   /* ��� ����� ��������� ��������� (�����) */
    char_visible_list not_moved;    /* ��� �� ����� ����������� (����������) */
    char_visible_list may_moved;    /* ��� ��� ����������� (����������) */
    struct follow_type *followers;        /* List of chars followers       */
    struct follow_type *missed_magic;        /* ������ �� ���� �������� ����� ����� */
    struct char_data *master;             /* Who is char following?        */
    struct follow_type *party;  //�������� ������
    struct char_data *party_leader;      //����� ������, ���� ����� �� �� ��������� ��� �� ����
    struct char_data *invite_char; //������������ ��������
    long invite_time;    //����� ���������

    mob_vnum follow_vnum; //�� ��� ������� ������
    LastDamageStorage Ldamage; //����������� ���������� � ����� ������
    vars_storage saved_vars;

    ubyte                           Memory[MAX_SPELLS+1]; /* ����� ���������� ����������        */
    int                             ManaMemNeeded;        /* ������� ���� ����� ��� ����������� */
    int                             ManaMemStored;        /* ������� ���� ��� ��������          */
    struct quest_data               Questing;
    struct mob_kill_data            MobKill;
    script_exp_data     ScriptExp;
    PK_Memory_type          pk_list;
    Killer_list_type   killer_list; //������ ��� ����� ��� �������� �������� ������� � ��������
    helper_data_type         helpers;
    int                             track_dirs;
    int                             locate_dirs;
    int                             CheckAggressive;
    struct new_flag                 Temporary;
    int                             ExtractTimer;
    int       LastCharm;
    int                             speed;
    int                             BattleCounter;
    struct new_flag                 BattleAffects;
    int                             Poisoner;
    struct extra_attack_type        extra_attack;
    struct cast_attack_type         cast_attack;
    int fskill;
    int sw; //Screen width
    int sh;
    sh_int eyes;
    struct obj_data *is_transpt; //� ����� ���������� ����� ��������
    struct obj_data *fiction; //������ �� ����� ������� ������
    char divd[2];  // �����������
    char divr[2];  // ���������
    bool vdir[NUM_OF_DIRS];
    int damage_rnd;              // ������� ����� �� �����
    int missed_rnd;
    bool doh;

    room_rnum original_location; //������� ������ ��������� �����

    //���������� �� ��������
    int delete_level; //������� ����������
    char delete_name[MAX_STRING_LENGTH]; //��� ����������
    char *registry_code;
    int dir_pick;
    struct obj_data *obj_pick;

    char *welcome;
    char *goodbye;
    struct mob_quest_data *quests;
    struct mob_qscript_data *qscripts;
    struct quest_list_data quest_list[MAX_QUESTS];
    int last_quest;
    int last_room_dmg; //��������� ����������� � �������

    long long guid;

    /*
     * ������������-����������� � ������
     */
    char_data();
    virtual ~char_data();

    virtual Mobile * npc() = 0;
    virtual Player * pc() = 0;
    virtual const Mobile * npc() const = 0;
    virtual const Player * pc() const = 0;

    bool immortal() const;
    
    void linkWrapper();
    virtual bool hasWrapper() const;
    
protected:
    void init();
};


/* ====================================================================== */


/* ==================== File Structure for Player ======================= */


struct descriptor_data;

/* other miscellaneous structures ***************************************/


struct msg_type {
    char *attacker_msg;  /* message to attacker */
    char *victim_msg;    /* message to victim   */
    char *room_msg;      /* message to room     */
};


struct message_type {
    struct msg_type die_msg; /* messages when death      */
    struct msg_type miss_msg;  /* messages when miss     */
    struct msg_type hit_msg; /* messages when hit      */
    struct msg_type god_msg; /* messages when hit on god   */
    struct message_type *next; /* to next messages of this kind. */
};


struct message_list {
    int  a_type;     /* Attack type        */
    int  number_of_attacks;  /* How many attack messages to chose from. */
    struct message_type *msg;  /* List of messages.      */
};

struct extra_affects_type {
    int affect;
    int set_or_clear;
};

struct weather_data {
    int  hours_go;              /* Time life from reboot */

    int  pressure;            /* How is the pressure ( Mb )            */
    int  press_last_day;        /* Average pressure last day             */
    int  press_last_week;       /* Average pressure last week            */

    int  temperature;           /* How is the temperature (C)            */
    int  temp_last_day;         /* Average temperature last day          */
    int  temp_last_week;        /* Average temperature last week         */

    int  rainlevel;             /* Level of water from rains             */
    int  snowlevel;             /* Level of snow                         */
    int  icelevel;              /* Level of ice                          */
    int  foglevel;        /* ������� ������ */

    int  weather_type;          /* bitvector - some values for month     */

    int  change;   /* How fast and what way does it change. */
    int  sky;      /* How is the sky.   */
    int  sunlight; /* And how much sun. */
    int  moon_day;   /* And how much moon */
    int  season;
    int  week_day;
};

struct weapon_affect_types {
    int    aff_pos;
    int    aff_bitvector;
    int    aff_spell;
};

/* element in monster and object index-tables   */
struct index_data {
    int  vnum; /* virtual number of this mob/obj       */
    int  number;   /* number of existing units of this mob/obj */
    int  stored;  /* number of things in rent file            */
    SPECIAL(*func);

    char   *func_name;         /* name of special function     */
};

/* ��������� �������� ����������� � ������ */
struct P_damage_weapon {
    int type; //��� �����������
    int damnodice; //����� ����������� x
    int damsizedice; //����� ����������� y
    int dam; //����� ����������� z
    struct P_damage_weapon *next;
};

struct set_variante_data {
    int count_objects;
    char *score;
    char *start_to_char;
    char *start_to_room;
    char *stop_to_char;
    char *stop_to_room;
    std::vector<struct obj_affected_type> addons;
    std::vector<struct obj_affected_type> skills;
    struct new_flag affects;

    set_variante_data():
            count_objects(0), score(NULL), start_to_char(NULL), start_to_room(NULL), stop_to_char(NULL),
            stop_to_room(0), addons(0), skills(0), affects(clear_flags) {}
};

struct set_items {
    int number;
    std::vector<int> *list_objects;
    std::vector<struct set_variante_data> *variante;
};


struct bug_abuse_data {
    int no;
    int owner;
    int count;
};


struct Player : public char_data {
    Player();
    virtual ~Player();
	
    virtual Mobile * npc();
    virtual Player * pc();
    virtual const Mobile * npc() const;
    virtual const Player * pc() const;

    struct player_special_data specials; // ���� �������� ������ ������
};

struct Mobile : public char_data {
    Mobile();
    Mobile(const Mobile &);
    Mobile(int vnum);
    virtual ~Mobile();

    virtual Mobile * npc();
    virtual Player * pc();
    virtual const Mobile * npc() const;
    virtual const Player * pc() const;

    bool isProto() const;
    const Mobile * getProto() const;
    Mobile * getProto();
    virtual bool hasWrapper() const;
    
    struct mob_special_data specials; // ���� �������� ������ ����
};

#endif
