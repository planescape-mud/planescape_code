/* $Id$
 *
 * ruffina, 2009
 */
#include "sysdep.h"
#include "memutils.h"
#include "structs.h"
#include "interp-decl.h"
#include "db-decl.h"
#include "spells-decl.h"
#include "comm-decl.h"

#include "planescape.h"
#include "mudstats.h"
#include "feniamanager.h"
#include "wrapperbase.h"

#define EQUALS_TO_PROTO(x) (proto && proto->x == x)

#define FREE_BY_PROTO(x) \
    do { \
        if (!EQUALS_TO_PROTO(x)) FREEPTR(x); \
        x = 0; \
    } while (0)

static void free_extra_description(struct extra_descr_data *&ed) 
{
    while (ed) {
        struct extra_descr_data *temp = ed->next;
        str_free(ed->keyword);
        str_free(ed->description);
        ::free(ed);
        ed = temp;
    }
}

player_special_data::player_special_data()
{
    bzero(&saved, sizeof(saved));
    saved.load_room = NOWHERE;
    poofin = 0;
    poofout = 0;
    aliases = 0;
    last_tell = NOBODY;
    may_rent = 0;
}

player_special_data::~player_special_data()
{
    str_free(poofin);
    str_free(poofout);

    while (aliases) {
        struct alias_data *temp = aliases->next;
        str_free(aliases->alias);
        str_free(aliases->replacement);
        free(aliases);
        aliases = temp;
    }
}

mob_special_data::mob_special_data()
{
    last_direction = 0;
    attack_type = 0;
    attack_type2 = 0;
    default_pos = POS_STANDING;
    memory = 0;
    damnodice = 0;
    damsizedice = 0;
    damage = 0;
    damnodice2 = 0;
    damsizedice2 = 0;
    damage2 = 0;
    bzero(armor, sizeof(armor));
    move_to = NOWHERE;
    curr_dest = 0;
    bzero(dest, sizeof(dest));
    dest_dir = 0;
    dest_pos = 0;
    dest_count = 0;
    activity = 0;
    npc_flags = clear_flags;
    ExtraAttack = 0;
    ExtraAttack2 = 0;
    LikeWork = 0;
    MaxFactor = 0;
    GoldNoDs = 0;
    GoldSiDs = 0;
    HorseState = 0;
    LastRoom = 0;
    Questor = 0;
    SpecType = 0;
    SpecSkill = 0;
    SpecDamDice = 0;
    SpecDamSize = 0;
    SpecDamRoll = 0;
    MessToVict = 0;
    MessToRoom = 0;
    LoadRoom = 0;
    speed = 0;
    wimp_level = 0;
    transpt = 0;
    move_type = 0;
    move_str = 0;
    vnum_horse = 0;

    vnum_corpse = 0;
    death_script = 0;
    CMessChar = 0;
    CMessRoom = 0;
    d_type_hit = 0;
    DamageDamDice = 0;
    DamageDamSize = 0;
    DamageDamRoll = 0;
    death_flag = 0;
    DMessChar = 0;
    DMessRoom = 0;
    powered = 0;
    age = 0;

    bzero(saved, sizeof(saved));

    AlrNeed = false;
    AlrMessChar = 0;
    AlrMessRoom = 0;
    AlrLevel = 0;
}

mob_special_data::~mob_special_data()
{
    // the following fields aren't used:
    // memory, Questor, MessToVict, MessToRoom

    // currently these cannot be overriden from prototype:
    // move_str, CMessChar, CMessRoom,
    // DMessChar, DMessRoom,
    // AlrMessChar, AlrMessRoom
}

void char_player_data::free(const struct char_player_data *proto)
{
    FREE_BY_PROTO(name);
    FREE_BY_PROTO(names);
    FREE_BY_PROTO(short_descr);
    FREE_BY_PROTO(long_descr);
    FREE_BY_PROTO(charm_descr);
    FREE_BY_PROTO(charm_descr_me);
    FREE_BY_PROTO(description);

    if (!EQUALS_TO_PROTO(ex_description)) 
        free_extra_description(ex_description);

    FREE_BY_PROTO(title);
    FREE_BY_PROTO(title_r);

    for (int i = 0; i < NUM_PADS; i++)
        FREE_BY_PROTO(PNames[i]);
}

char_data::char_data()
{
    init();
}

void char_data::init()
{
    pfilepos = -1;
    nr = NOBODY;
    in_room = NOWHERE;
    was_in_room = NOWHERE;
    wait = 0;

    body = 0;
    ibody = 0;

    set_number = 0;
    set_variante = -1;
    set_change = -1;
    set_message_var = -1;
    set_message_num = -1;

    memset(init_classes, 0, sizeof(init_classes));
    memset(classes, 0, sizeof(classes));
    memset(add_classes, 0, sizeof(add_classes));

    operations = 0;
    event_mob = 0;
    bzero(&player, sizeof(struct char_player_data));
    bzero(&add_abils, sizeof(struct char_played_ability_data));
    bzero(&realtime_abils, sizeof(struct char_realtime_data));
    bzero(&real_abils, sizeof(struct char_ability_data));
    bzero(&points, sizeof(struct char_point_data));
    points.hit = 1;
    points.move = 1;
    bzero(&char_specials, sizeof(struct char_special_data));
    char_specials.position = POS_STANDING;
    shop_data = 0;
    affected = 0;
    timed = 0;
    memset(equipment, 0, sizeof(equipment));
    memset(tatoo, 0, sizeof(tatoo));
    load_death = 0;
    load_eq = 0;
    load_inv = 0;
    load_tatoo = 0;
    spec_hit = 0;
    death_count = 0;
    ress_count = 0;
    capsoul_count = 0;

    carrying = 0;
    desc = 0;

    ticks_in_mud = 0;
    id = 0;
    memory = 0;
    mess_data = 0;
    period = 0;

    next_in_room = 0;
    next = 0;
    next_fighting = 0;
    trap_object = 0;

    followers = 0;
    missed_magic = 0;
    master = 0;
    party = 0;
    party_leader = 0;
    invite_char = 0;
    invite_time = 0;

    follow_vnum = 0;

    memset(Memory, 0, sizeof(Memory));
    ManaMemNeeded = 0;
    ManaMemStored = 0;
    
    bzero(&Questing, sizeof(struct quest_data));
    bzero(&MobKill, sizeof(struct mob_kill_data));
    bzero(&ScriptExp, sizeof(script_exp_data));
    track_dirs = 0;
    locate_dirs = 0;
    CheckAggressive = 0;
    Temporary = clear_flags;
    ExtractTimer = 0;
    LastCharm = 0;
    speed = 0;
    BattleCounter = 0;
    BattleAffects = clear_flags;
    Poisoner = 0;
    bzero(&extra_attack, sizeof(struct extra_attack_type));
    bzero(&cast_attack, sizeof(struct cast_attack_type));
    fskill = 0;
    sw = 0;
    sh = 0;
    eyes = 0;
    is_transpt = 0;
    fiction = 0;
    memset(divd, 0, sizeof(divd));
    memset(divr, 0, sizeof(divr));
    memset(vdir, false, sizeof(vdir));
    damage_rnd = 0;
    missed_rnd = 0;
    doh = false;

    original_location = 0;

    delete_level = 0;
    delete_name[0] = 0;
    registry_code = 0;
    dir_pick = -1;
    obj_pick = 0;

    welcome = 0;
    goodbye = 0;
    quests = 0;
    qscripts = 0;
    last_quest = 0;
    last_room_dmg = 0;

    guid = 0;
    wrapper = NULL;
}

char_data::~char_data()
{
    // body freed in subclasses, requires proto
    // operations aren't used
    // event_mob never differs from proto
    // player freed in subclass, requires proto

    for (int i = 0; i <= char_specials.lastmemory; i++) 
        str_free(char_specials.memory[i].desc);

    // shop_data never differs from proto

    if (affected) {
        struct affected_type *temp;
        DESTROY_LIST(affected, next, temp);
    }

    if (timed) {
        struct timed_type *temp;
        DESTROY_LIST(timed, next, temp);
    }
    
    // load_death, load_eq, load_inv, load_tattoo never differ from proto
    // spec_hit never differs from proto
    // memory not used
    // mess_data never differs from proto
    // period never differs from proto
    
    if (followers) {
        struct follow_type *temp;
        DESTROY_LIST(followers, next, temp);
    }

    if (missed_magic) {
        struct follow_type *temp;
        DESTROY_LIST(missed_magic, next, temp);
    }

    if (party) {
        struct follow_type *temp;
        DESTROY_LIST(party, next, temp);
    }

    FREEPTR(Questing.quests);
    FREEPTR(MobKill.howmany);
    FREEPTR(MobKill.vnum);
    FREEPTR(ScriptExp.howmany);
    FREEPTR(ScriptExp.vnum);
    str_free(registry_code);
    // welcome, goodbye never differ from proto
    // quests, qscripts never differ from proto
}

void char_data::linkWrapper()
{
    if (FeniaManager::wrapperManager)
	FeniaManager::wrapperManager->linkWrapper(this);
}

bool char_data::immortal() const
{
    if (pc() && player.level >= LVL_IMMORT)
        return true;

    if (desc && desc->original)
        return desc->original->immortal();

    return false;
}

Player::Player()
{
    guid = mud->getStats()->genID();
}

Player::~Player()
{
    if (body) delete [] body;
    player.free(0);
}

Mobile * Player::npc() 
{ 
    return NULL;
}
Player * Player::pc() 
{
    return this;
}
const Mobile * Player::npc() const 
{
    return NULL;
}
const Player * Player::pc() const 
{
    return this;
}

Mobile::Mobile()
{
    guid = mud->getStats()->genID();
}

Mobile::Mobile(int vnum)
{
    init();
    guid = ((long long)vnum << 4) | 3; 
}

Mobile::Mobile(const Mobile &proto)
{
    *this = proto;
    guid = mud->getStats()->genID();
    wrapper = NULL;
}

Mobile::~Mobile()
{
    const Mobile *proto = getProto();

    // body never differs from proto
    player.free(proto ? &(proto->player) : NULL);
}

Mobile * Mobile::npc() 
{ 
    return this;
}
Player * Mobile::pc() 
{
    return NULL;
}
const Mobile * Mobile::npc() const 
{
    return this;
}
const Player * Mobile::pc() const 
{
    return NULL;
}

const Mobile * Mobile::getProto() const
{
    return nr >= 0 ? &mob_proto[nr] : NULL;
}

Mobile * Mobile::getProto() 
{
    return nr >= 0 ? &mob_proto[nr] : NULL;
}

bool Mobile::isProto() const
{
    return getProto() == this;
}

/*---------------------------------------------------------------------------*
 * struct obj_data
 *---------------------------------------------------------------------------*/
obj_data::obj_data()
{
    init();
    guid = mud->getStats()->genID();
}

void obj_data::init()
{
    item_number = NOTHING;
    in_room = NOWHERE;

    bzero(&obj_flags, sizeof(struct obj_flag_data));
    obj_flags.Obj_zone = NOWHERE;
    bzero(&affected, sizeof(affected));
    C_affected = 0;

    weapon = 0;
    missile = 0;
    spec_weapon = 0;
    operations = 0;
    names = 0;
    name = 0;
    description = 0;
    main_description = 0;
    short_description = 0;
    action_description = 0;
    ex_description = 0;
    carried_by = 0;
    worn_by = 0;
    worn_on = NOWHERE;
    limit = 0;
    owner = 0;

    materials = 0;
    damage_weapon = 0;

    in_obj = 0;
    contains = 0;

    bflag = clear_flags;
    bflag_reset = clear_flags;
    load_obj = 0;
    trap = 0;
    bgold = 0;

    id = 0;

    next_content = 0;
    next = 0;

    transpt = 0;

    trap_victim = 0;

    page = 0;
    shance = 0;
    room_was_in = NOWHERE;
    bzero(PNames, sizeof(PNames));
    killer = 0;
    powered = 0;
    mess_data = 0;
    bzero(lock_code, sizeof(lock_code));
    lock_step = 0;

    guid = 0;
    wrapper = NULL;
}

obj_data::obj_data(int vnum)
{
    init();
    guid = ((long long)vnum << 4) | 2; 
}

obj_data::obj_data(const obj_data &proto)
{
    *this = proto;
    guid = mud->getStats()->genID();
    wrapper = NULL;
}

obj_data::~obj_data()
{
    const obj_data *proto = getProto();

    // deallocate only non-proto affects
    if (C_affected) {
        struct C_obj_affected_type *af, *af_next, *temp;

        for (af = C_affected; af; af = af_next) {
            af_next = af->next;
            if (af->type != spellnum_db) {
                REMOVE_FROM_LIST(af, C_affected, next);
                free(af);
            }
        }
    }

    // weapon,missile,spec_weapon,operations never differ from proto

    FREE_BY_PROTO(names);
    FREE_BY_PROTO(name);
    FREE_BY_PROTO(description);
    FREE_BY_PROTO(short_description);
    // main_description never differs from proto
    FREE_BY_PROTO(action_description);

    if (!EQUALS_TO_PROTO(ex_description)) 
        free_extra_description(ex_description);

    if (!EQUALS_TO_PROTO(materials)) {
        struct material_data_list *temp;
        DESTROY_LIST(materials, next, temp);
    }

    // damage_weapon not used
    // load_obj,trap never differ from proto

    FREE_BY_PROTO(transpt); /* transport system is one big XXX */

    for (int i = 0; i < NUM_PADS; i++) 
        FREE_BY_PROTO(PNames[i]);
    
    // mess_data never differs from proto 
}

const struct obj_data * obj_data::getProto() const
{
    return item_number >= 0 ? &obj_proto[item_number] : NULL;
}

struct obj_data * obj_data::getProto()
{
    return item_number >= 0 ? &obj_proto[item_number] : NULL;
}

bool obj_data::isProto() const
{
    return getProto() == this;
}

void obj_data::linkWrapper()
{
    if (FeniaManager::wrapperManager)
	FeniaManager::wrapperManager->linkWrapper(this);
}

mob_quest_data::mob_quest_data()
{
    number = 0;
    name = 0;
    expr = 0;
    text = 0;
    pretext = 0;
    complite = 0;
    multi = false;
    accept = 0;
    done = 0;

    next = 0;
}

obj_trap_data::obj_trap_data()
{
    shance = 0;
    damnodice = 0;
    damsizedice = 0;
    damage = 0;
    tbreak = 0;
    save = 0;
    type_hit = 0;

    trap_damage_char = 0;
    trap_damage_room = 0;
    trap_nodamage_char = 0;
    trap_nodamage_room = 0;
    trap_kill_char = 0;
    trap_kill_room = 0;
}

room_direction_data::room_direction_data()
{
    general_description = 0;

    exit_name = 0;
    keyword = 0;
    exit_data = clear_flags;
    exit_data_reset = clear_flags;
    type = 0;

    material = 0;
    quality = 0;
    shance = 0;
    damnodice = 0;
    damsizedice = 0;
    damage = 0;
    tbreak = 0;
    save = 0;
    type_hit = 0;
    
    trap_damage_char = 0;
    trap_damage_room = 0;
    trap_nodamage_char = 0;
    trap_nodamage_room = 0;
    trap_kill_char = 0;
    trap_kill_room = 0;
    
    sex = 0;
    lock_code[0] = 0;
    lock_step = 0;

    exit_info = 0;
    key = 0;
    to_room = 0;

    type_port = 0;
    key_port = 0;
    room_port = 0;

    time = 0;
    timer = 0;
    open = 0;
    close = 0;
    active_room = 0;
    tStart = 0;
    tStop = 0;
    portal_description = 0;
    mess_to_open = 0;
    mess_to_close = 0;
    mess_char_enter = 0;
    mess_room_enter = 0;
}

/*---------------------------------------------------------------------------*
 * struct room_data
 *---------------------------------------------------------------------------*/
room_data::room_data()
{
    init();
}

void room_data::init()
{
    number = 0;
    zone = 0;
    spellhaunt = 0;
    sector_type = 0;
    sector_state = 0;
    name = 0;
    description = 0;
    description_night = 0;
    ex_description = 0;
    bzero(dir_option, sizeof(dir_option));
    room_flags = clear_flags;
    damage = 0;
    bzero(trap_option, sizeof(trap_option));
    mess_data = 0;
    forcedir = 0;
    mobiles = 0;
    objects = 0;
    period = 0;
    light = 0;
    glight = 0;
    gdark = 0;
    bzero(&weather, sizeof(weather));
    func = 0;
    func_name = 0;

    track = 0;

    contents = 0;
    people = 0;

    affects = 0;

    fires = 0;
    forbidden = 0;
    ices = 0;
    portal_room = 0;
    portal_time = 0;
    blood = 0;
    hotel = 0;
    path_label = 0;
    p_sphere.clear();

    guid = 0;
    wrapper = NULL;
}

room_data::room_data(int vnum)
{
    init();
    guid = ((long long)vnum << 4) | 1; 
}

void room_data::linkWrapper()
{
    if (FeniaManager::wrapperManager)
	FeniaManager::wrapperManager->linkWrapper(this);
}

P_message::P_message()
{
    valid = false;
    mChar = mVict = mRoom = NULL;
    hChar = hVict = hRoom = NULL;
    kChar = kVict = kRoom = NULL;
    pChar = pVict = pRoom = NULL;
    aChar = aVict = aRoom = NULL;
    bChar = bVict = bRoom = NULL;
    tChar = tVict = tRoom = NULL;
    sChar = sVict = sRoom = NULL;
}

P_last_damage::P_last_damage()
        : id_ch(0)
{
}

P_damage::P_damage()
        : valid(false), type(0), power(0), dam(0), magic(0), far_min(0), deviate(0), check_ac(0),
          location(0), critic(0), armor(0), hLoc(0), weapObj(NULL) 
{
}

quest_list_data::quest_list_data()
{
    mob_vnum = 0;
    number = 0;
    complite = false;
}

