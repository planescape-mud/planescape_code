#ifndef XBOOT_H
#define XBOOT_H
/*-------------------------------------------------------
 * �������� � ������������� �������� � xformat�
 *-------------------------------------------------------*/

/*
 * ����� ���
 */
void boot_bodys(void);

/*
 * ������  
 */
void boot_skills(void);

/*
 * �������
 */
void boot_enchant(void);

int find_enchant_num(char *name);
int find_enchant_num(int enchant_no);

#define ENCHANT_NUMBERS  xEnchant.GetNumberItem()
#define GET_ENCHANT(num) xEnchant.GetItem(num)
#define ENCHANT_NO(num)  xEnchant.GetItem(num)->GetItem(ECH_NUMBER)->GetInt()
#define ENCHANT_WEAR(num) xEnchant.GetItem(num)->GetItem(ECH_WEAR_TYPE)->GetString()
#define ENCHANT_TYPE(num) xEnchant.GetItem(num)->GetItem(ECH_OBJECT_TYPE)->GetInt()
#define ENCHANT_NAME(num) xEnchant.GetItem(num)->GetItem(ECH_NAME)->GetString()
#define ENCHANT_DESC(num) xEnchant.GetItem(num)->GetItem(ECH_DESCRIPTION)->GetString()
#define ENCHANT_LEVEL(num) xEnchant.GetItem(num)->GetItem(ECH_MINIMAL)->GetInt()

/*
 * ���������
 */
void boot_materials(void);
void new_material(int number, char *name, int weight, int price, int durab, int ac, int arm0,
                  int arm1, int arm2, int type, int hits, int fire, int cold, int electro, int acid,
                  int include);

extern struct material_data *material_table;

/*
 * �������
 */
void boot_mob(void);
int get_xrec_mob(void);


/*
 * ��������
 */
void boot_obj(void);
int get_xrec_obj(void);
void set_affect_obj(struct obj_data *obj, long affect);
void set_affect_obj(struct obj_data *obj, long affect, int modifier);
void read_char_xobj(struct char_data *ch);
void save_char_xobj(struct char_data *ch);


/* 
 * ������� ��������� ������� 
 */
/* Moradin (c) 2006          */

int get_current_quest(struct char_data *ch, int qvnumber);
void go_show_quest(struct char_data *ch, struct char_data *victim, int pos, int number);
int go_show_quests(struct char_data *ch, struct char_data *victim);
int add_quest(struct char_data *ch, int vnum, int number);
int add_quest(struct char_data *ch, struct char_data *victim, int number);
void del_quest(struct char_data *ch, int mob_vnum, int number);
void set_mob_quest(struct char_data *ch, int vnum);
void set_var_quest(struct char_data *ch, char *name, char *value);
void set_obj_quest(struct char_data *ch, int vnum);
void set_flw_quest(struct char_data *ch, int vnum);
void unset_flw_quest(struct char_data *ch, int vnum);
void unset_obj_quest(struct char_data *ch, int vnum);
int get_mob_quest(struct char_data *ch, int number, int mob_vnum, int vnum);
int get_obj_quest(struct char_data *ch, int number, int mob_vnum, int vnum, int multi);
int get_flw_quest(struct char_data *ch, int number, int mob_vnum, int vnum);
const char *get_var_quest(struct char_data *ch, int number, int vnum, const char *name);
int check_quest(struct char_data *ch, int vnum, int number);
int check_quest(struct char_data *ch, int qrnum);
void check_quest_complite(struct char_data *ch, int vnum, int number);
int check_mutli_quest(struct char_data *ch, struct char_data *victim, int vnum, int number);
void go_complite_quest(struct char_data *ch, int vnum, int number);
void set_quested(struct char_data *ch, int vnum, int number);
void set_quested(struct char_data *ch, int quest);
int get_quested(struct char_data *ch, int quest);
int get_quested(struct char_data *ch, int vnum, int number);

void load_quests(struct char_data *ch);
void save_quests(struct char_data *ch);

/*
 * �������
 */
int boot_scripts(void);
void go_script(int vnum, struct char_data *ch, struct char_data *victim, struct obj_data *obj,
               int roomnum);
void go_script(int vnum, struct char_data *ch);
void go_script(int vnum, struct char_data *ch, struct char_data *victim);
void go_script(int vnum, struct char_data *ch, struct char_data *victim, struct obj_data *obj);
void go_script(int vnum, struct char_data *ch, struct obj_data *obj);
void go_script(int vnum, struct char_data *ch, struct char_data *victim, struct obj_data *obj,
               int roomnum, char *arg);
void go_script(int vnum, struct char_data *ch, char *arg);
void go_script(int vnum, struct char_data *ch, struct char_data *victim, char *arg);
void go_script(int vnum, struct char_data *ch, struct char_data *victim, struct obj_data *obj,
               char *arg);
void go_script(int vnum, struct char_data *ch, struct obj_data *obj, char *arg);
void pulse_saved_var(int pulse);
const char *check_saved_var(struct char_data *ch, const char *key);
void add_saved_var(struct char_data *ch, char *key, char *value, int time);
int del_saved_var(struct char_data *ch, char *key);

extern vars_storage global_vars;

/*
 * ������
 */
void boot_skills(void);
int find_skill_num(int skill_no);
void GetSkillMessage(int skill, int hTyp, struct P_message &pMess);

/*
 * ����������
 */
void GetSpellMessage(int spellnum, struct P_message &pMess);
void boot_spells(void);

#define SPELL_NUMBERS  Spl.GetNumberItem()
#define SPELL_NO(num)  Spl.GetItem(num)->GetItem(SPL_NUMBER)->GetInt()
#define SPELL_SPHERE(num) Spl.GetItem(num)->GetItem(SPL_SPHERE)->GetInt()
#define SPELL_LSPHERE(num) Spl.GetItem(num)->GetItem(SPL_SPHERE)->GetInt()+190
#define SPELL_LEVEL(num) Spl.GetItem(num)->GetItem(SPL_LEVEL)->GetInt()
#define SPELL_LSKILL(num) Spl.GetItem(num)->GetItem(SPL_SKILL)->GetInt()
#define SPELL_DANGER(num) Spl.GetItem(num)->GetItem(SPL_AGRO)->GetInt()
#define SPELL_VLAG(num)  Spl.GetItem(num)->GetItem(SPL_VLAG)->GetInt()
#define SPELL_LAG(num)  Spl.GetItem(num)->GetItem(SPL_LAG)->GetInt()
#define SPELL_MEMORY(num) Spl.GetItem(num)->GetItem(SPL_MEMORY)->GetInt()
#define SPELL_POS(num)  Spl.GetItem(num)->GetItem(SPL_POSITION)->GetInt()
#define SPELL_MANA(num)  Spl.GetItem(num)->GetItem(SPL_MANA)->GetInt()
#define SPELL_NAME(num)  Spl.GetItem(num)->GetItem(SPL_NAME)->GetString()
#define SPELL_SYN(num)  Spl.GetItem(num)->GetItem(SPL_UNAME)->GetString()
#define SPELL_TARGET(num) Spl.GetItem(num)->GetItem(SPL_TARGET)->GetString()
#define SPELL_AGRO(num)  Spl.GetItem(num)->GetItem(SPL_AGRO)->GetString()
#define SPELL_CONCENT(num) Spl.GetItem(num)->GetItem(SPL_CONCENT)->GetInt()
#define SPELL_PROCEDURE(num) Spl.GetItem(num)->GetItem(SPL_PROCEDURE)->GetString()

/*
 * ������� ��������, ������.
 * ��������� � ��������
 */
void boot_armtempl(void);
void boot_weaptempl(void);
void boot_misstempl(void);
void boot_hit_messages(void);

/*
 * ������ ���������
 */
void boot_sets(void);
extern struct set_items *set_table;

/*
 * �������
 */
void boot_wld(void);
int get_xrec_wld(void);
void GetWldMessage(int room, struct P_message &pMess);
void count_key_use(void);
void count_maxfactor_mob(void);
void create_virtual_rooms(void);

/*
 * ������� ���
 */
void load_world_loot(void);
std::vector < int >check_world_loot(int vnum_mob);

/*
 * ����
 */
void boot_zon(void);
int get_xrec_zon(void);

/*
 * �������� �������
 */
void load_pets(struct char_data *ch);
void save_pets(struct char_data *ch);
void load_vars(struct char_data *ch);
void save_vars(struct char_data *ch);
void xrent_check(int act);
void xload_rent(struct char_data *ch, int copyover);
void xsave_rent(struct char_data *ch, int type, int extract);
void xrent_save_all(int type);
void xrent_start_check(int index, bool start);

#endif
