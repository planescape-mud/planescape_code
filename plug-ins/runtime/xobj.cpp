/**************************************************************************
    ÌÌ "Á“¡Œ… Ì…“¡" (”) 2002-2003 ·Œƒ“≈  Â“Õ…€…Œ
    ˙¡«“’⁄À¡ ∆¡ Ãœ◊ …«“œ◊œ«œ Õ…“¡
 **************************************************************************/

#include "sysdep.h"
#include "structs.h"
#include "db.h"
#include "utils.h"
#include "handler.h"
#include "xobj.h"
#include "spells.h"
#include "xspells.h"
#include "case.h"
#include "parser_id.h"
#include "constants.h"
#include "comm.h"
#include "xtempl.h"
#include "xboot.h"
#include "strlib.h"
#include "expr.h"
#include "planescape.h"
#include "mudfilereader.h"

void fenia_mark_guid(long long guid);

/****/


int get_xrec_obj(void)
{
    Obj = new CObj;

    if (!Obj->Initialization())
        exit(1);

    MudFileReader reader(mud->objDir);

    while (reader.hasNext())
        if (!Obj->ReadConfig(reader.next().getCPath()))
            exit(1);


    return Obj->GetNumberItem();
}

///////////////////////////////////////////////////////////////////////////////

void set_affect_obj(struct obj_data *obj, long affect, int modifier)
{
    struct C_obj_affected_type af[1];

    af[0].type = find_spell_num(SPELL_DB);
    af[0].duration = -1;
    af[0].modifier = modifier;
    af[0].location = APPLY_NONE;
    af[0].bitvector = affect;
    af[0].level = GET_OBJ_QUALITY(obj) * 4;
    af[0].extra = 0;
    af[0].no_flag = 0;
    af[0].anti_flag = 0;
    affect_to_object(obj, af);
}

void set_affect_obj(struct obj_data *obj, long affect)
{
    struct C_obj_affected_type af[1];

    af[0].type = find_spell_num(SPELL_DB);
    af[0].duration = -1;
    af[0].level = GET_OBJ_QUALITY(obj) * 4;
    af[0].modifier = 100;
    af[0].location = APPLY_NONE;
    af[0].bitvector = affect;
    af[0].extra = 0;
    af[0].no_flag = 0;
    af[0].anti_flag = 0;
    affect_to_object(obj, af);
}

void add_quest_obj(struct obj_data *obj, int quest)
{
    obj->quests.push_back(quest);
}

//////
void boot_obj(void)
{

    struct extra_descr_data *new_descr;
    struct material_data_list *m;
    int i, j, number, numadd, vnum, t[3], templ = 0;
    struct C_obj_affected_type af[1];
    float f[3];
    char buf[MAX_STRING_LENGTH];
    CItem *Templ;

    number = Obj->GetNumberItem();
    for (i = 0; i < number; i++) {
        templ = 0;
        Templ = NULL;
        CItem *obj = Obj->GetItem(i);

        vnum = obj->GetItem(OBJ_NUMBER)->GetInt();

        if (!mud->modQuietLoad)
            log("˙·ÁÚı˙Î·: “≈ƒÕ≈‘ %d", vnum);

        obj_index[iobj].vnum = vnum;
        obj_index[iobj].number = 0;
        obj_index[iobj].stored = 0;
        obj_index[iobj].func = NULL;
        obj_index[iobj].func_name = NULL;

        new(obj_proto + iobj) obj_data(vnum);
        obj_proto[iobj].item_number = iobj;

        obj_proto[iobj].obj_flags.Obj_max = 100;
        obj_proto[iobj].obj_flags.Obj_cur = 100;
        obj_proto[iobj].obj_flags.Obj_mater = 0;
        obj_proto[iobj].obj_flags.Obj_sex = 1;
        obj_proto[iobj].obj_flags.Obj_timer = SEVEN_DAYS;
        obj_proto[iobj].obj_flags.Obj_time_load = 0;
        obj_proto[iobj].obj_flags.affects = clear_flags;
        obj_proto[iobj].obj_flags.no_flag = clear_flags;
        obj_proto[iobj].obj_flags.anti_flag = clear_flags;
        obj_proto[iobj].obj_flags.Obj_destroyer = 60;
        obj_proto[iobj].obj_flags.Obj_temp = 0;
        obj_proto[iobj].obj_flags.Obj_temp_add = 0;
        obj_proto[iobj].obj_flags.value[0] = 0;
        obj_proto[iobj].obj_flags.value[1] = 0;
        obj_proto[iobj].obj_flags.value[2] = 0;
        obj_proto[iobj].obj_flags.value[3] = 0;
        obj_proto[iobj].obj_flags.reciept = 0;
        obj_proto[iobj].materials = NULL;
        obj_proto[iobj].load_obj = NULL;
        obj_proto[iobj].trap = NULL;
        obj_proto[iobj].transpt = NULL;

        if (obj->GetItem(OBJ_TEMPLATE)->GetInt()) {
            templ = obj->GetItem(OBJ_TEMPLATE)->GetInt();
            Templ = ArmTmp.FindItem(templ);
            if (!Templ) {
                log("˜ÓÈÌ·ÓÈÂ! ˚¡¬ÃœŒ #%d ◊ –“≈ƒÕ≈‘≈ #%d - Œ≈ Œ¡ ƒ≈Œ", templ, vnum);
                //exit(1);
                templ = 0;
                Templ = NULL;
            }
        }

        CREATE(obj_proto[iobj].name, char, strlen(obj->GetItem(OBJ_ALIAS)->GetString()) + 1);

        strcpy(obj_proto[iobj].name, obj->GetItem(OBJ_ALIAS)->GetString());

        //Ó·˙˜·ÓÈÂ
        del_spaces(obj->GetItem(OBJ_NAME)->GetString());

        CREATE(obj_proto[iobj].names, char, strlen(obj->GetItem(OBJ_NAME)->GetString()) + 1);

        strcpy(obj_proto[iobj].names, obj->GetItem(OBJ_NAME)->GetString());

        for (j = 0; j < NUM_PADS; j++) {
            strcpy(buf, get_name_pad((char *) obj->GetItem(OBJ_NAME)->GetString(), j, PAD_OBJECT));
            CREATE(GET_OBJ_PNAME(obj_proto + iobj, j), char, strlen(buf) + 1);

            strcpy(GET_OBJ_PNAME(obj_proto + iobj, j), buf);
        }

        strcpy(buf,
               get_name_pad((char *) obj->GetItem(OBJ_NAME)->GetString(), PAD_IMN, PAD_OBJECT));
        CREATE(obj_proto[iobj].short_description, char, strlen(buf) + 1);

        strcpy(obj_proto[iobj].short_description, buf);

        CREATE(obj_proto[iobj].description, char, strlen(obj->GetItem(OBJ_LINE)->GetString()) + 1);

        strcpy(obj_proto[iobj].description, obj->GetItem(OBJ_LINE)->GetString());

        //ÔÈÛ·ÓÈÂ
        if (obj->GetItem(OBJ_DESCRIPTION)->GetString()) {
            CREATE(obj_proto[iobj].main_description, char,
                   strlen(obj->GetItem(OBJ_DESCRIPTION)->GetString()) + 1);
            strcpy(obj_proto[iobj].main_description, obj->GetItem(OBJ_DESCRIPTION)->GetString());
        }
        //‰ÔÔÏÓÈÙÂÏ¯ÓÔ
        numadd = obj->GetItem(OBJ_ADDITION)->GetNumberItem();
        for (j = 0; j < numadd; j++) {
            CItem *add = obj->GetItem(OBJ_ADDITION)->GetItem(j);
            CREATE(new_descr, struct extra_descr_data, 1);
            CREATE(new_descr->keyword, char, strlen(add->GetItem(OBJ_ADD_KEY)->GetString()) + 1);

            strcpy(new_descr->keyword, add->GetItem(OBJ_ADD_KEY)->GetString());
            CREATE(new_descr->description, char,
                   strlen(add->GetItem(OBJ_ADD_TEXT)->GetString()) + 1);
            strcpy(new_descr->description, add->GetItem(OBJ_ADD_TEXT)->GetString());
            new_descr->next = obj_proto[iobj].ex_description;
            obj_proto[iobj].ex_description = new_descr;
        }

        //Û˜ÔÍÛÙ˜·
        if (obj->GetItem(OBJ_PROPERTIES)->GetString())
            asciiflag_conv((char *) obj->GetItem(OBJ_PROPERTIES)->GetString(),
                           &obj_proto[iobj].obj_flags.extra_flags);

        //ÚÔ‰
        obj_proto[iobj].obj_flags.Obj_sex = obj->GetItem(OBJ_SEX)->GetInt();

        //Ì·ÙÂÚÈ·Ï
        numadd = obj->GetItem(OBJ_MATERIAL)->GetNumberItem();
        for (j = 0; j < numadd; j++) {
            CItem *mat = obj->GetItem(OBJ_MATERIAL)->GetItem(j);

            add_material(obj_proto + iobj, mat->GetItem(OBJ_MAT_TYPE)->GetInt(),
                         mat->GetItem(OBJ_MAT_VAL)->GetInt(), mat->GetItem(OBJ_MAT_MAIN)->GetInt());
        }

        if (templ && Templ && !obj_proto[iobj].materials) {
            log("Ô˚È‚Î·! Ó≈ ’À¡⁄¡ŒŸ Õ¡‘≈“…¡ÃŸ –“≈ƒÕ≈‘¡ #%d", vnum);
            exit(1);
        }
        //Î·˛ÂÛÙ˜Ô
        if (obj->GetItem(OBJ_QUALITY)->GetInt())
            obj_proto[iobj].obj_flags.quality = obj->GetItem(OBJ_QUALITY)->GetInt();
        else
            obj_proto[iobj].obj_flags.quality = 5;

        //˜ÂÛ
        if (obj->GetItem(OBJ_WEIGHT)->GetInt()) {
            if (templ && obj->GetItem(OBJ_WEIGHT)->GetInt() == -1 && obj_proto[iobj].materials) {
                obj_proto[iobj].obj_flags.weight = Templ->GetItem(A_WEIGHT)->GetInt();
                for (m = obj_proto[iobj].materials; m; m = m->next) {
                    //˜≈” ◊ «“¡ÕÕ¡»
                    t[0] = (m->value * get_material_param(m->number_mat)->weight);
                    obj_proto[iobj].obj_flags.weight += t[0];
                }
            } else if (obj->GetItem(OBJ_WEIGHT)->GetInt() == -1 && obj_proto[iobj].materials) {
                for (m = obj_proto[iobj].materials; m; m = m->next) {
                    //˜≈” ◊ «“¡ÕÕ¡»
                    t[0] = (m->value * get_material_param(m->number_mat)->weight);
                    obj_proto[iobj].obj_flags.weight += t[0];
                }
            } else
                obj_proto[iobj].obj_flags.weight = obj->GetItem(OBJ_WEIGHT)->GetInt();
        } else
            obj_proto[iobj].obj_flags.weight = 0;

        //ÚÂ‰ÂÏ
        obj_proto[iobj].limit = obj->GetItem(OBJ_LIMIT)->GetInt();

        //„ÂÓ·
        if (obj->GetItem(OBJ_COST)->GetInt()) {
            if (templ && obj->GetItem(OBJ_COST)->GetInt() == -1 && obj_proto[iobj].materials) {
                obj_proto[iobj].obj_flags.cost = Templ->GetItem(A_PRICE)->GetInt();
                for (m = obj_proto[iobj].materials; m; m = m->next) {
                    //„≈Œ¡
                    t[0] = (m->value * get_material_param(m->number_mat)->price);
                    obj_proto[iobj].obj_flags.cost += t[0];
                }
                //≈“≈“¡”ﬁ≈‘ Œ¡ À¡ﬁ≈”‘◊œ
                if (obj_proto[iobj].obj_flags.cost)
                    obj_proto[iobj].obj_flags.cost =
                        (obj_proto[iobj].obj_flags.cost *
                         quality_at_cost[(int) obj_proto[iobj].obj_flags.quality]) / 100;
                else
                    obj_proto[iobj].obj_flags.cost = 0;
            } else if (obj->GetItem(OBJ_COST)->GetInt() == -1 && obj_proto[iobj].materials) {
                for (m = obj_proto[iobj].materials; m; m = m->next) {
                    //„≈Œ¡
                    if (get_material_param(m->number_mat)->price)
                        t[0] =
                            ((m->value * get_material_param(m->number_mat)->price) *
                             quality_at_cost[(int) obj_proto[iobj].obj_flags.quality]) / 100;
                    else
                        t[0] = 0;
                    obj_proto[iobj].obj_flags.cost += t[0];
                }
            } else
                obj_proto[iobj].obj_flags.cost = obj->GetItem(OBJ_COST)->GetInt();
        } else
            obj_proto[iobj].obj_flags.cost = 0;

        //ÙÈ
        obj_proto[iobj].obj_flags.type_flag = obj->GetItem(OBJ_TYPE)->GetInt();

        //ÈÛÔÏ¯˙Ô˜·ÓÈÂ
        if (obj->GetItem(OBJ_WEAR)->GetString())
            asciiflag_conv(obj->GetItem(OBJ_WEAR)->GetString(),
                           &obj_proto[iobj].obj_flags.wear_flags);

        //ÓÂı‰Ô‚ÛÙ˜Ô
        if (obj->GetItem(OBJ_ANTI)->GetString())
            asciiflag_conv(obj->GetItem(OBJ_ANTI)->GetString(),
                           &obj_proto[iobj].obj_flags.anti_flag);

        //˙·ÚÂÙ
        if (obj->GetItem(OBJ_NO)->GetString())
            asciiflag_conv(obj->GetItem(OBJ_NO)->GetString(), &obj_proto[iobj].obj_flags.no_flag);

        //˜ÏÈÒÓÈÂ
        numadd = obj->GetItem(OBJ_APPLY)->GetStrListNumber();
        for (j = 0; j < numadd; j++) {
            obj->GetItem(OBJ_APPLY)->GetStrList(j, t[0], t[1]);
            af[0].type = 0;
            af[0].duration = 0;
            af[0].modifier = 0;
            af[0].location = 0;
            af[0].bitvector = 0;
            af[0].extra = 0;
            af[0].no_flag = 0;
            af[0].anti_flag = 0;
            af[0].type = find_spell_num(SPELL_DB);
            af[0].location = t[0];
            if (t[0] == APPLY_HITROLL || t[0] == APPLY_DAMROLL)
                af[0].modifier = 1;     //È¬œ ⁄¡≈¬¡Ã… ◊≈›… Àœ‘œ“Ÿ≈ ƒ¡¿‘ +5 damroll
            else
                af[0].modifier = t[1];
            af[0].bitvector = 0;
            af[0].extra = 0;
            af[0].no_flag = 0;
            af[0].anti_flag = 0;
            af[0].duration = -1;
            affect_to_object(obj_proto + iobj, af);
        }
        //¸ÊÊÂÎÙ˘
        if (obj->GetItem(OBJ_AFFECTS)->GetString()) {
            new_flag test;

            test.flags[0] = 0;
            test.flags[1] = 0;
            test.flags[2] = 0;
            test.flags[3] = 0;

            asciiflag_conv((char *) obj->GetItem(OBJ_AFFECTS)->GetString(), &GET_FLAG(test, 0));

            if (IS_SET(GET_FLAG(test, AFF_BLIND), AFF_BLIND))   //ÛÏÂÔÙ·
                set_affect_obj(obj_proto + iobj, AFF_BLIND);

            if (IS_SET(GET_FLAG(test, AFF_INVISIBLE), AFF_INVISIBLE))   //ÓÂ˜È‰ÈÌÔÛÙ¯
                set_affect_obj(obj_proto + iobj, AFF_INVISIBLE);

            if (IS_SET(GET_FLAG(test, AFF_DETECT_ALIGN), AFF_DETECT_ALIGN))     //˙Ó·ÓÈÂ_Ó·ÎÏÔÓÔÛÙÂÍ
                set_affect_obj(obj_proto + iobj, AFF_DETECT_ALIGN);

            if (IS_SET(GET_FLAG(test, AFF_DETECT_INVIS), AFF_DETECT_INVIS))     //˜È‰ÂÙ¯_ÓÂ˜È‰ÈÌÔÂ
                set_affect_obj(obj_proto + iobj, AFF_DETECT_INVIS);

            if (IS_SET(GET_FLAG(test, AFF_DETECT_MAGIC), AFF_DETECT_MAGIC))     //˜È‰ÂÙ¯_Ì·ÁÈ‡
                set_affect_obj(obj_proto + iobj, AFF_DETECT_MAGIC);

            if (IS_SET(GET_FLAG(test, AFF_SENSE_LIFE), AFF_SENSE_LIFE)) //˛ı˜ÛÙ˜Ô˜·Ù¯_ˆÈ˙Ó¯
                set_affect_obj(obj_proto + iobj, AFF_SENSE_LIFE);

            if (IS_SET(GET_FLAG(test, AFF_WATERWALK), AFF_WATERWALK))   //ËÔˆ‰ÂÓÈÂ_Ô_˜Ô‰Â
                set_affect_obj(obj_proto + iobj, AFF_WATERWALK);

            //Û˜ÒÙÔÛÙ¯
            if (IS_SET(GET_FLAG(test, AFF_INFRAVISION), AFF_INFRAVISION))       //ÈÓÊÚ·˙ÚÂÓÈÂ
                set_affect_obj(obj_proto + iobj, AFF_INFRAVISION);

            if (IS_SET(GET_FLAG(test, AFF_PROTECT_EVIL), AFF_PROTECT_EVIL))     //˙·˝ÈÙ·_ÔÙ_˙Ï·
                set_affect_obj(obj_proto + iobj, AFF_PROTECT_EVIL);

            if (IS_SET(GET_FLAG(test, AFF_PROTECT_GOOD), AFF_PROTECT_GOOD))     //˙·˝ÈÙ·_ÔÙ_‰Ô‚Ú·
                set_affect_obj(obj_proto + iobj, AFF_PROTECT_GOOD);

            if (IS_SET(GET_FLAG(test, AFF_NOTRACK), AFF_NOTRACK))       //ÓÂ_˜˘ÛÏÂ‰ÈÙ¯
                set_affect_obj(obj_proto + iobj, AFF_NOTRACK);

            //ÎÚ·‰ÂÙ¯ÛÒ
            /* if (IS_SET(GET_FLAG(test,AFF_SNEAK), AFF_SNEAK))
               set_affect_obj(obj_proto+iobj,AFF_SNEAK); */

            //ÚÒ˛ÂÙ¯ÛÒ
            /* if (IS_SET(GET_FLAG(test,AFF_HIDE), AFF_HIDE))
               set_affect_obj(obj_proto+iobj,AFF_HIDE); */

            //ÒÚÔÛÙ¯
            /* if (IS_SET(GET_FLAG(test,AFF_COURAGE), AFF_COURAGE))
               set_affect_obj(obj_proto+iobj,AFF_COURAGE); */

            if (IS_SET(GET_FLAG(test, AFF_HOLD), AFF_HOLD))     //·Ú·ÏÈ˛
                set_affect_obj(obj_proto + iobj, AFF_HOLD);

            if (IS_SET(GET_FLAG(test, AFF_FLY), AFF_FLY))       //ÔÏÂÙ
                set_affect_obj(obj_proto + iobj, AFF_FLY);

            if (IS_SET(GET_FLAG(test, AFF_SIELENCE), AFF_SIELENCE))     //ÓÂÌÔÍ
                set_affect_obj(obj_proto + iobj, AFF_SIELENCE);

            if (IS_SET(GET_FLAG(test, AFF_AWARNESS), AFF_AWARNESS))     //Ó·ÛÙÔÚÔˆÂÓ
                set_affect_obj(obj_proto + iobj, AFF_AWARNESS);

            if (IS_SET(GET_FLAG(test, AFF_HOLYLIGHT), AFF_HOLYLIGHT))   //Û˜ÂÙ
                set_affect_obj(obj_proto + iobj, AFF_HOLYLIGHT);

            if (IS_SET(GET_FLAG(test, AFF_HOLYDARK), AFF_HOLYDARK))     //Ù¯Ì·
                set_affect_obj(obj_proto + iobj, AFF_HOLYDARK);

            if (IS_SET(GET_FLAG(test, AFF_DETECT_POISON), AFF_DETECT_POISON))   //ÔÚÂ‰ÂÏÂÓÈÂ_Ò‰·
                set_affect_obj(obj_proto + iobj, AFF_DETECT_POISON);

            //Ì·ÛÎÈÚÔ˜Î·
            /* if (IS_SET(GET_FLAG(test,AFF_CAMOUFLAGE), AFF_CAMOUFLAGE))
               set_affect_obj(obj_proto+iobj,AFF_CAMOUFLAGE); */

            if (IS_SET(GET_FLAG(test, AFF_WATERBREATH), AFF_WATERBREATH))       //‰˘Ë·ÓÈÂ_˜Ô‰ÔÍ
                set_affect_obj(obj_proto + iobj, AFF_WATERBREATH);

            //˛ıÌ·
            if (IS_SET(GET_FLAG(test, AFF_PLAGUE), AFF_PLAGUE))
                set_affect_obj(obj_proto + iobj, AFF_PLAGUE);

            if (IS_SET(GET_FLAG(test, AFF_DARKVISION), AFF_DARKVISION)) //ÓÔ˛ÓÔÂ_˙ÚÂÓÈÂ
                set_affect_obj(obj_proto + iobj, AFF_DARKVISION);

            if (IS_SET(GET_FLAG(test, AFF_DEAFNESS), AFF_DEAFNESS))     //ÁÏıËÔÙ·
                set_affect_obj(obj_proto + iobj, AFF_DEAFNESS);

            //Ïı˛_Û˜ÂÙ·
            /* if (IS_SET(GET_FLAG(test,AFF_SUNBEAM), AFF_SUNBEAM))
               set_affect_obj(obj_proto+iobj,AFF_SUNBEAM); */

            if (IS_SET(GET_FLAG(test, AFF_ILLNESS), AFF_ILLNESS))       //‚ÔÏÂ˙Ó¯
                set_affect_obj(obj_proto + iobj, AFF_ILLNESS);

            //SET_BIT(GET_FLAG(test,AFF_LEVIT), AFF_LEVIT);
            if (IS_SET(GET_FLAG(test, AFF_LEVIT), AFF_LEVIT))   //ÏÂ˜ÈÙ·„ÈÒ
                set_affect_obj(obj_proto + iobj, AFF_LEVIT);
        }

        //Ù·ÍÌÂÚ
        t[0] = obj->GetItem(OBJ_TIMER)->GetInt();
        obj_proto[iobj].obj_flags.Obj_timer = t[0] > 0 ? t[0] : SEVEN_DAYS;


        //ÚÔ˛ÓÔÛÙ¯
        if (obj->GetItem(OBJ_DURAB)->GetInt()) {
            if (obj->GetItem(OBJ_DURAB)->GetInt() == -1 && obj_proto[iobj].materials) {
                for (m = obj_proto[iobj].materials; m; m = m->next) {
                    obj_proto[iobj].obj_flags.Obj_max =
                        obj_proto[iobj].obj_flags.Obj_cur =
                        GET_OBJ_DURAB(obj_proto +
                                      iobj) *
                        quality_at_durab[(int) obj_proto[iobj].obj_flags.quality];
                }
            } else {
                obj_proto[iobj].obj_flags.Obj_max =
                    obj_proto[iobj].obj_flags.Obj_cur = obj->GetItem(OBJ_DURAB)->GetInt();
            }
        }
        //ÂÚÂË˜·Ù
        numadd = obj->GetItem(OBJ_INTERCEPTION)->GetNumberItem();
        for (j = 0; j < numadd; j++) {
            CItem *inter = obj->GetItem(OBJ_INTERCEPTION)->GetItem(j);

            add_message(obj_proto + iobj, inter->GetItem(OBJ_INT_COMMAND)->GetInt(),
                        inter->GetItem(OBJ_INT_STOP)->GetInt(),
                        inter->GetItem(OBJ_INT_SCRIPT)->GetInt(),
                        (char *) inter->GetItem(OBJ_INT_MESSPLAYER)->GetString(),
                        (char *) inter->GetItem(OBJ_INT_MESSVICTIM)->GetString(),
                        (char *) inter->GetItem(OBJ_INT_MESSOTHER)->GetString(),
                        (char *) inter->GetItem(OBJ_INT_MESSROOM)->GetString());
        }

        //˚·ÓÛ
        if (obj->GetItem(OBJ_SHANCE)->GetInt())
            GET_OBJ_PERCENT(obj_proto + iobj) = obj->GetItem(OBJ_SHANCE)->GetInt();

        //˙Ó·˛0-3
        obj_proto[iobj].obj_flags.value[0] = obj->GetItem(OBJ_VAL0)->GetInt();
        obj_proto[iobj].obj_flags.value[1] = obj->GetItem(OBJ_VAL1)->GetInt();
        obj_proto[iobj].obj_flags.value[2] = obj->GetItem(OBJ_VAL2)->GetInt();
        obj_proto[iobj].obj_flags.value[3] = obj->GetItem(OBJ_VAL3)->GetInt();

        //Û˜ÂÙ
        if (obj->GetItem(OBJ_LIGHT)->GetInt())
            obj_proto[iobj].obj_flags.light = obj->GetItem(OBJ_LIGHT)->GetInt();
        //ÒÚÎÔÛÙ¯
        if (obj->GetItem(OBJ_BRIGHT)->GetInt())
            obj_proto[iobj].obj_flags.bright = obj->GetItem(OBJ_BRIGHT)->GetInt();

        /* ”◊…‘À…, Œ¡–…‘À…, –¡ÃœﬁÀ…, ÷≈⁄ÃŸ, ÀŒ…«… */
        if (GET_OBJ_TYPE(obj_proto + iobj) == ITEM_SCROLL ||
            GET_OBJ_TYPE(obj_proto + iobj) == ITEM_WAND ||
            GET_OBJ_TYPE(obj_proto + iobj) == ITEM_STAFF ||
            GET_OBJ_TYPE(obj_proto + iobj) == ITEM_BOOK ||
            GET_OBJ_TYPE(obj_proto + iobj) == ITEM_POTION) {
            //ıÚÔ˜ÂÓ¯
            if (obj->GetItem(OBJ_LEVEL)->GetInt())
                obj_proto[iobj].obj_flags.value[0] = obj->GetItem(OBJ_LEVEL)->GetInt();
        }

        /* ”◊…‘À…, Œ¡–…‘À…, ÀŒ…«… */
        if (GET_OBJ_TYPE(obj_proto + iobj) == ITEM_SCROLL ||
            GET_OBJ_TYPE(obj_proto + iobj) == ITEM_BOOK ||
            GET_OBJ_TYPE(obj_proto + iobj) == ITEM_POTION) {
            //˙·ÎÏ1
            if (obj->GetItem(OBJ_SPELL1)->GetInt())
                obj_proto[iobj].obj_flags.value[1] = obj->GetItem(OBJ_SPELL1)->GetInt();

            //˙·ÎÏ2
            if (obj->GetItem(OBJ_SPELL2)->GetInt())
                obj_proto[iobj].obj_flags.value[2] = obj->GetItem(OBJ_SPELL2)->GetInt();

            //˙·ÎÏ3
            if (obj->GetItem(OBJ_SPELL3)->GetInt())
                obj_proto[iobj].obj_flags.value[3] = obj->GetItem(OBJ_SPELL3)->GetInt();
            //ÚÂ„ÂÙ
            if (obj->GetItem(OBJ_ENCHANT)->GetInt())
                obj_proto[iobj].obj_flags.reciept = obj->GetItem(OBJ_ENCHANT)->GetInt();
        }

        /* ÷≈⁄ÃŸ, –¡ÃœﬁÀ… */
        if (GET_OBJ_TYPE(obj_proto + iobj) == ITEM_WAND ||
            GET_OBJ_TYPE(obj_proto + iobj) == ITEM_STAFF) {
            //Ì˙·ÚÒ‰˘
            if (obj->GetItem(OBJ_SLOT_MAX)->GetInt())
                obj_proto[iobj].obj_flags.value[1] = obj->GetItem(OBJ_SLOT_MAX)->GetInt();

            //˙·ÚÒ‰˘
            if (obj->GetItem(OBJ_SLOT_CUR)->GetInt())
                obj_proto[iobj].obj_flags.value[2] = obj->GetItem(OBJ_SLOT_CUR)->GetInt();

            //˙·ÎÏÈÓ·ÓÈÂ
            if (obj->GetItem(OBJ_SPELL)->GetInt())
                obj_proto[iobj].obj_flags.value[3] = obj->GetItem(OBJ_SPELL)->GetInt();
        }


        /* ƒœ”–≈» */
        if (GET_OBJ_TYPE(obj_proto + iobj) == ITEM_ARMOR) {
            /* ÙÈ ‰ÔÛÂË· */
            if (templ)
                GET_ARM_TYPE(obj_proto + iobj) = Templ->GetItem(A_CLASS)->GetInt();
            else
                GET_ARM_TYPE(obj_proto + iobj) = obj->GetItem(OBJ_ARM_CLASS)->GetInt();

            /* ÔÎÚ˘ÙÈÂ */
            if (templ) {
                obj_proto[iobj].obj_flags.cover[0] = Templ->GetItem(A_P0)->GetInt();
                obj_proto[iobj].obj_flags.cover[1] = Templ->GetItem(A_P1)->GetInt();
                obj_proto[iobj].obj_flags.cover[2] = Templ->GetItem(A_P2)->GetInt();
            } else {
                obj_proto[iobj].obj_flags.cover[0] = obj->GetItem(OBJ_P0)->GetInt();
                obj_proto[iobj].obj_flags.cover[1] = obj->GetItem(OBJ_P1)->GetInt();
                obj_proto[iobj].obj_flags.cover[2] = obj->GetItem(OBJ_P2)->GetInt();
            }

            if (templ && obj_proto[iobj].materials) {
                int a[4], b[4];
                int count_material = 0;

                for (int y = 0; y < 4; y++) {
                    a[y] = 0;
                    b[y] = 0;
                }

                //–œƒ”ﬁ≈‘ ÀœÃ-◊¡ ƒœ–œÃŒ…≈‘ÃÿŒœ«œ Õ¡‘≈“…œ◊
                for (m = obj_proto[iobj].materials; m; m = m->next) {
                    if (m->main)
                        continue;
                    count_material++;
                }

                m = NULL;
                for (m = obj_proto[iobj].materials; m; m = m->next)
                    if (m->main == 1) {
                        a[0] = get_material_param(m->number_mat)->ac;
                        a[1] = get_material_param(m->number_mat)->armor0;
                        a[2] = get_material_param(m->number_mat)->armor1;
                        a[3] = get_material_param(m->number_mat)->armor2;
                        break;
                    }


                //ƒœ¬¡◊Ã≈Œ…≈ Œ≈œ”Œœ◊ŒŸ» Õ¡‘≈“…¡Ãœ◊
                for (m = obj_proto[iobj].materials; m; m = m->next) {
                    if (m->main)
                        continue;
                    b[0] += get_material_param(m->number_mat)->ac;
                    b[1] += get_material_param(m->number_mat)->armor0;
                    b[2] += get_material_param(m->number_mat)->armor1;
                    b[3] += get_material_param(m->number_mat)->armor2;
                }

                if (b[0])
                    b[0] = (b[0] / count_material) / 2;
                if (b[1])
                    b[1] = (b[1] / count_material) / 2;
                if (b[2])
                    b[2] = (b[2] / count_material) / 2;
                if (b[3])
                    b[3] = (b[3] / count_material) / 2;

                //”Ãœ÷≈Œ…≈ ” €¡¬ÃœŒ¡Õ… … ƒœ–¡Õ…
                obj_proto[iobj].obj_flags.value[0] = Templ->GetItem(A_AC)->GetInt() + a[0] + b[0];
                obj_proto[iobj].obj_flags.value[1] = Templ->GetItem(A_ARM0)->GetInt() + a[1] + b[1];
                obj_proto[iobj].obj_flags.value[2] = Templ->GetItem(A_ARM1)->GetInt() + a[2] + b[2];
                obj_proto[iobj].obj_flags.value[3] = Templ->GetItem(A_ARM2)->GetInt() + a[3] + b[3];

                //≈“≈“¡”ﬁ≈‘ Œ¡ À¡ﬁ≈”‘◊œ –“≈ƒÕ≈‘¡
                obj_proto[iobj].obj_flags.value[0] =
                    (obj_proto[iobj].obj_flags.value[0] *
                     quality_at_ac[(int) obj_proto[iobj].obj_flags.quality]) / 100;
                obj_proto[iobj].obj_flags.value[1] =
                    (obj_proto[iobj].obj_flags.value[1] *
                     quality_at_ac[(int) obj_proto[iobj].obj_flags.quality]) / 100;
                obj_proto[iobj].obj_flags.value[2] =
                    (obj_proto[iobj].obj_flags.value[2] *
                     quality_at_ac[(int) obj_proto[iobj].obj_flags.quality]) / 100;
                obj_proto[iobj].obj_flags.value[3] =
                    (obj_proto[iobj].obj_flags.value[3] *
                     quality_at_ac[(int) obj_proto[iobj].obj_flags.quality]) / 100;
            } else {
                //˙·˝ÈÙ·
                if (obj->GetItem(OBJ_AC)->GetInt()) {
                    if (obj->GetItem(OBJ_AC)->GetInt() == -1 && obj_proto[iobj].materials) {
                        for (m = obj_proto[iobj].materials; m; m = m->next) {
                            if (!m->main)
                                continue;
                            if (get_material_param(m->number_mat)->ac)
                                f[0] =
                                    (float) (((m->value * get_material_param(m->number_mat)->ac) /
                                              3.0) *
                                             quality_at_ac[(int) obj_proto[iobj].obj_flags.
                                                           quality]) / 100;
                            else
                                f[0] = 0.0;
                            obj_proto[iobj].obj_flags.value[0] = (int) f[0];
                        }
                    } else
                        obj_proto[iobj].obj_flags.value[0] = obj->GetItem(OBJ_AC)->GetInt();
                }
                //ÚÂˆı˝ÂÂ
                if (obj->GetItem(OBJ_ARM0)->GetInt()) {
                    if (obj->GetItem(OBJ_ARM0)->GetInt() == -1 && obj_proto[iobj].materials) {
                        for (m = obj_proto[iobj].materials; m; m = m->next) {
                            if (!m->main)
                                continue;
                            if (get_material_param(m->number_mat)->armor0)
                                f[0] =
                                    (float) (((m->value *
                                               get_material_param(m->number_mat)->armor0) / 5.0) *
                                             quality_at_ac[(int) obj_proto[iobj].obj_flags.
                                                           quality]) / 100;
                            else
                                f[0] = 0.0;
                            obj_proto[iobj].obj_flags.value[1] = (int) f[0];
                        }
                    } else
                        obj_proto[iobj].obj_flags.value[1] = obj->GetItem(OBJ_ARM0)->GetInt();
                }
                //ÎÔÏ‡˝ÂÂ
                if (obj->GetItem(OBJ_ARM1)->GetInt()) {
                    if (obj->GetItem(OBJ_ARM1)->GetInt() == -1 && obj_proto[iobj].materials) {
                        for (m = obj_proto[iobj].materials; m; m = m->next) {
                            if (!m->main)
                                continue;
                            if (get_material_param(m->number_mat)->armor1)
                                f[0] =
                                    (float) (((m->value *
                                               get_material_param(m->number_mat)->armor1) / 5.0) *
                                             quality_at_ac[(int) obj_proto[iobj].obj_flags.
                                                           quality]) / 100;
                            else
                                f[0] = 0.0;
                            obj_proto[iobj].obj_flags.value[2] = (int) f[0];
                        }
                    } else
                        obj_proto[iobj].obj_flags.value[2] = obj->GetItem(OBJ_ARM1)->GetInt();
                }
                //ı‰·ÚÓÔÂ
                if (obj->GetItem(OBJ_ARM2)->GetInt()) {
                    if (obj->GetItem(OBJ_ARM2)->GetInt() == -1 && obj_proto[iobj].materials) {
                        for (m = obj_proto[iobj].materials; m; m = m->next) {
                            if (!m->main)
                                continue;
                            if (get_material_param(m->number_mat)->armor2)
                                f[0] =
                                    (float) (((m->value *
                                               get_material_param(m->number_mat)->armor2) / 5.0) *
                                             quality_at_ac[(int) obj_proto[iobj].obj_flags.
                                                           quality]) / 100;
                            else
                                f[0] = 0.0;
                            obj_proto[iobj].obj_flags.value[3] = (int) f[0];
                        }
                    } else
                        obj_proto[iobj].obj_flags.value[3] = obj->GetItem(OBJ_ARM2)->GetInt();
                }
            }
        }

        /* ÛŒ¡“—ƒ */
        if (GET_OBJ_TYPE(obj_proto + iobj) == ITEM_MISSILE) {
            if (obj->GetItem(OBJ_MISSILE)->GetNumberItem()) {
                CItem *missl = obj->GetItem(OBJ_MISSILE)->GetItem(0);
                CREATE(obj_proto[iobj].missile, struct missile_data, 1);

                obj_proto[iobj].obj_flags.value[0] = missl->GetItem(OBJ_MISS_TYPE)->GetInt();
                obj_proto[iobj].obj_flags.value[2] = obj_proto[iobj].obj_flags.value[1] =
                    missl->GetItem(OBJ_MISS_COUNT)->GetInt();
                //missl->GetItem(OBJ_MISS_TEMPL);
                numadd = missl->GetItem(OBJ_MADD_HIT)->GetNumberItem();
                for (j = 0; j < numadd; j++) {
                    int wtyp = 0, wmin = 0, wmax = 0;
                    CItem *addhit = missl->GetItem(OBJ_MADD_HIT)->GetItem(j);

                    wtyp = addhit->GetItem(OBJ_MAHIT_TYPE)->GetInt();
                    if (sscanf(addhit->GetItem(OBJ_MAHIT_DAMAGE)->GetString(), "%d,%d", t, t + 1) ==
                        2) {
                        wmin = t[0];
                        wmax = t[1];
                    }
                    add_missile_damage(obj_proto[iobj].missile, wtyp, wmin, wmax);
                }
            }
        }

        /* Ô“’÷…≈ */
        if (GET_OBJ_TYPE(obj_proto + iobj) == ITEM_WEAPON) {
            CREATE(obj_proto[iobj].weapon, struct weapon_data, 1);

            obj_proto[iobj].weapon->skill = obj->GetItem(OBJ_SKILL)->GetInt();
            obj_proto[iobj].weapon->message = obj->GetItem(OBJ_HIT)->GetInt();
            int wtyp = real_attack_type(obj_proto[iobj].weapon->message), wmin = 0, wmax = 0;

            if (obj->GetItem(OBJ_DAMAGE)->GetString())
                if (sscanf(obj->GetItem(OBJ_DAMAGE)->GetString(), "%dd%d+%d", t, t + 1, t + 2) == 3) {
                    wmin = t[0] + t[2];
                    wmax = (t[0] * t[1]) + t[2];
                }

            add_weapon_damage(obj_proto[iobj].weapon, wtyp, wmin, wmax);

            numadd = obj->GetItem(OBJ_ADD_HIT)->GetNumberItem();
            for (j = 0; j < numadd; j++) {
                int wtyp = 0, wmin = 0, wmax = 0;
                CItem *addhit = obj->GetItem(OBJ_ADD_HIT)->GetItem(j);

                wtyp = addhit->GetItem(OBJ_AHIT_TYPE)->GetInt();
                if (sscanf(addhit->GetItem(OBJ_AHIT_DAMAGE)->GetString(), "%d,%d", t, t + 1) == 2) {
                    wmin = t[0];
                    wmax = t[1];
                }
                add_weapon_damage(obj_proto[iobj].weapon, wtyp, wmin, wmax);
            }

            numadd = obj->GetItem(OBJ_SPECHIT)->GetNumberItem();
            int numadd2, n;

            for (j = 0; j < numadd; j++) {
                struct spec_weapon_data *k;
                CItem *spechit = obj->GetItem(OBJ_SPECHIT)->GetItem(j);

                CREATE(k, struct spec_weapon_data, 1);

                for (n = 0; n < NUM_RACES; n++)
                    k->xrace[n] = FALSE;
                for (n = 0; n < NUM_TMOBS; n++)
                    k->xmtype[n] = FALSE;

                k->shance = spechit->GetItem(OBJ_SPECHIT_SHANCE)->GetInt();

                numadd2 = spechit->GetItem(OBJ_SPECHIT_XRACE)->GetStrListNumber();
                for (n = 0; n < numadd2; n++) {
                    spechit->GetItem(OBJ_SPECHIT_XRACE)->GetStrList(n, t[0], t[1]);
                    k->xrace[t[0]] = TRUE;
                }
                numadd2 = spechit->GetItem(OBJ_SPECHIT_ZRACE)->GetStrListNumber();
                for (n = 0; n < numadd2; n++) {
                    spechit->GetItem(OBJ_SPECHIT_ZRACE)->GetStrList(n, t[0], t[1]);
                    k->zrace[t[0]] = TRUE;
                }

                numadd2 = spechit->GetItem(OBJ_SPECHIT_XTYPE)->GetStrListNumber();
                for (n = 0; n < numadd2; n++) {
                    spechit->GetItem(OBJ_SPECHIT_XTYPE)->GetStrList(n, t[0], t[1]);
                    k->xmtype[t[0]] = TRUE;
                }
                numadd2 = spechit->GetItem(OBJ_SPECHIT_ZTYPE)->GetStrListNumber();
                for (n = 0; n < numadd2; n++) {
                    spechit->GetItem(OBJ_SPECHIT_ZTYPE)->GetStrList(n, t[0], t[1]);
                    k->zmtype[t[0]] = TRUE;
                }

                if (spechit->GetItem(OBJ_SPECHIT_CHAR)->GetString())
                    k->to_char = str_dup(spechit->GetItem(OBJ_SPECHIT_CHAR)->GetString());
                if (spechit->GetItem(OBJ_SPECHIT_VICTIM)->GetString())
                    k->to_vict = str_dup(spechit->GetItem(OBJ_SPECHIT_VICTIM)->GetString());
                if (spechit->GetItem(OBJ_SPECHIT_ROOM)->GetString())
                    k->to_room = str_dup(spechit->GetItem(OBJ_SPECHIT_ROOM)->GetString());
                k->koef = spechit->GetItem(OBJ_SPECHIT_KOEF)->GetFloat();
                k->type_damage = spechit->GetItem(OBJ_SPECHIT_HTYPE)->GetInt();
                if (spechit->GetItem(OBJ_SPECHIT_HDAMAGE)->GetString()) {
                    if (sscanf
                        (spechit->GetItem(OBJ_SPECHIT_HDAMAGE)->GetString(), "%d,%d", t,
                         t + 1) == 2) {
                        k->min_damage = t[0];
                        k->max_damage = t[1];
                    }
                }
                k->spell = spechit->GetItem(OBJ_SPECHIT_SPELL)->GetInt();
                k->level = spechit->GetItem(OBJ_SPECHIT_SLEVEL)->GetInt();
                k->target = spechit->GetItem(OBJ_SPECHIT_TARGET)->GetInt();

                k->next = obj_proto[iobj].spec_weapon;
                obj_proto[iobj].spec_weapon = k;
            }
        }

        /* ÀœŒ‘≈ Œ≈“ */
        if (GET_OBJ_TYPE(obj_proto + iobj) == ITEM_CONTAINER) {
            //˜ÌÂÛÙÈÌÔÛÙ¯ (◊ À«)
            if (obj->GetItem(OBJ_SIZE)->GetInt())
                obj_proto[iobj].obj_flags.value[0] = obj->GetItem(OBJ_SIZE)->GetInt() * 1000;
        }

        /* ÀœŒ‘≈ Œ≈“, ÀŒ…«¡ */
        if (GET_OBJ_TYPE(obj_proto + iobj) == ITEM_CONTAINER ||
            GET_OBJ_TYPE(obj_proto + iobj) == ITEM_FICTION) {
            //ÎÛ˜ÔÍÛÙ˜·
            if (obj->GetItem(OBJ_BAG_PROPERTIES)->GetString()) {
                asciiflag_conv((char *) obj->GetItem(OBJ_BAG_PROPERTIES)->GetString(),
                               &obj_proto[iobj].bflag);
                obj_proto[iobj].bflag_reset = obj_proto[iobj].bflag;
            }

            for (j = 0; j < NUM_ITEMS; j++)
                obj_proto[iobj].obj_flags.bnotfit[j] = FALSE;
            numadd = obj->GetItem(OBJ_BAG_NOTFIT)->GetStrListNumber();
            for (j = 0; j < numadd; j++) {
                obj->GetItem(OBJ_BAG_NOTFIT)->GetStrList(j, t[0], t[1]);
                obj_proto[iobj].obj_flags.bnotfit[t[0]] = TRUE;
            }
        }

        /* ÀœŒ‘≈ Œ≈“, ÀŒ…«¡ */
        if (GET_OBJ_TYPE(obj_proto + iobj) == ITEM_CONTAINER ||
            GET_OBJ_TYPE(obj_proto + iobj) == ITEM_FICTION) {
            //ÎÏ‡˛
            if (obj->GetItem(OBJ_KEY)->GetInt())
                obj_proto[iobj].obj_flags.value[2] = obj->GetItem(OBJ_KEY)->GetInt();
        }

        /* ÀœŒ‘≈ Œ≈“ */
        if (GET_OBJ_TYPE(obj_proto + iobj) == ITEM_CONTAINER) {
            //ı‰Ô‚ÛÙ˜Ô
            if (obj->GetItem(OBJ_BAG_MAGIC)->GetInt())
                obj_proto[iobj].obj_flags.value[3] = obj->GetItem(OBJ_BAG_MAGIC)->GetInt();
        }


        /* ÀœŒ‘≈ Œ≈“ */
        if (GET_OBJ_TYPE(obj_proto + iobj) == ITEM_CONTAINER) {
            //ÛÔ‰ÂÚˆÈÌÔÂ
            numadd = obj->GetItem(OBJ_OLIST)->GetStrListNumber();
            for (j = 0; j < numadd; j++) {
                obj->GetItem(OBJ_OLIST)->GetStrList(j, t[0], t[1]);
                add_container_obj(obj_proto + iobj, t[0], t[1]);
            }
        }

        /* ÀœŒ‘≈ Œ≈“ */
        if (GET_OBJ_TYPE(obj_proto + iobj) == ITEM_CONTAINER) {
            //‰ÂÓ¯ÁÈ
            if (obj->GetItem(OBJ_OGOLD)->GetInt())
                obj_proto[iobj].bgold = obj->GetItem(OBJ_OGOLD)->GetInt();
        }

        /* ∆œŒ‘¡Œ, ”œ”’ƒ */
        if (GET_OBJ_TYPE(obj_proto + iobj) == ITEM_FOUNTAIN ||
            GET_OBJ_TYPE(obj_proto + iobj) == ITEM_DRINKCON) {
            //ÂÌÎÔÛÙ¯
            if (obj->GetItem(OBJ_VALUE)->GetInt())
                obj_proto[iobj].obj_flags.value[0] =
                    obj_proto[iobj].obj_flags.value[1] =
                    obj->GetItem(OBJ_VALUE)->GetInt() * SECS_PER_MUD_TICK;

            //ˆÈ‰ÎÔÛÙ¯
            if (obj->GetItem(OBJ_LIQ)->GetInt())
                obj_proto[iobj].obj_flags.value[2] = obj->GetItem(OBJ_LIQ)->GetInt();
        }

        /* ∆œŒ‘¡Œ, ”œ”’ƒ, ≈ƒ¡ */
        if (GET_OBJ_TYPE(obj_proto + iobj) == ITEM_FOUNTAIN ||
            GET_OBJ_TYPE(obj_proto + iobj) == ITEM_DRINKCON ||
            GET_OBJ_TYPE(obj_proto + iobj) == ITEM_FOOD) {
            //Ò‰
            if (obj->GetItem(OBJ_POISON)->GetInt())
                obj_proto[iobj].obj_flags.value[3] = obj->GetItem(OBJ_POISON)->GetInt();
        }

        /* ≈ƒ¡ */
        if (GET_OBJ_TYPE(obj_proto + iobj) == ITEM_FOOD) {
            //Ó·Û˘˝ÂÓÈÂ
            if (obj->GetItem(OBJ_FOOD)->GetInt())
                obj_proto[iobj].obj_flags.value[0] =
                    obj->GetItem(OBJ_FOOD)->GetInt() * SECS_PER_MUD_TICK;
        }

        /* ƒ≈Œÿ«… */
        if (GET_OBJ_TYPE(obj_proto + iobj) == ITEM_MONEY) {
            //ÛıÌÌ·
            if (obj->GetItem(OBJ_GOLD)->GetInt())
                obj_proto[iobj].obj_flags.value[0] = obj->GetItem(OBJ_GOLD)->GetInt();
        }

        /* ‘“¡Œ”–œ“‘ */
        if (GET_OBJ_TYPE(obj_proto + iobj) == ITEM_TRANSPORT) {
            //ÙÚÛ˜ÔÍÛÙ˜·
            if (obj->GetItem(OBJ_TR_PROPERTIES)->GetString()) {
                obj_proto[iobj].transpt = new transpt_data_obj();
                asciiflag_conv((char *) obj->GetItem(OBJ_TR_PROPERTIES)->GetString(),
                               &obj_proto[iobj].transpt->transp_flag);
                //obj_proto[iobj].bflag_reset = obj_proto[iobj].bflag;
            }
        }

        /* Û–≈√“œ√≈ƒ’“Ÿ ƒÃ— œ¬ﬂ≈À‘œ◊: ƒœ”À¡ œ¬ﬂ—◊Ã≈Œ…  */
        if (GET_OBJ_TYPE(obj_proto + iobj) == ITEM_BOARD)
            spec_func_assign_obj(iobj, "gen_board");

        /* Û–≈√“œ√≈ƒ’“Ÿ ƒÃ— œ¬ﬂ≈À‘œ◊: ƒœ”À¡ ”√≈Œ¡“…≈◊ */
        if (GET_OBJ_TYPE(obj_proto + iobj) == ITEM_SCRIPTBOARD)
            spec_func_assign_obj(iobj, "scriptboard");

        /* ÏÔ˜ı˚Î· */
        if (obj->GetItem(OBJ_TRAP)->GetNumberItem()) {
            obj_proto[iobj].trap = new obj_trap_data();
            CItem *etrap = obj->GetItem(OBJ_TRAP)->GetItem(0);  //◊”≈«ƒ¡ –≈“◊¡— … ≈ƒ…Œ”‘◊≈ŒŒ¡— ⁄¡–…”ÿ

            obj_proto[iobj].trap->shance = etrap->GetItem(OBJ_TRAP_CHANCE)->GetInt();

            if (etrap->GetItem(OBJ_TRAP_TYPEDAMAGE)->GetInt())
                obj_proto[iobj].trap->type_hit = etrap->GetItem(OBJ_TRAP_TYPEDAMAGE)->GetInt();

            if (etrap->GetItem(OBJ_TRAP_FORCEDAMAGE)->GetString()) {
                int t[3];

                if (sscanf
                    (etrap->GetItem(OBJ_TRAP_FORCEDAMAGE)->GetString(), "%dd%d+%d", t, t + 1,
                     t + 2) == 3) {
                    obj_proto[iobj].trap->damnodice = t[0];
                    obj_proto[iobj].trap->damsizedice = t[1];
                    obj_proto[iobj].trap->damage = t[2];
                }
            }

            if (etrap->GetItem(OBJ_TRAP_SAVE)->GetInt())
                obj_proto[iobj].trap->save = etrap->GetItem(OBJ_TRAP_SAVE)->GetInt();

            if (etrap->GetItem(OBJ_TRAP_MESS_CHAR)->GetString()) {
                CREATE(obj_proto[iobj].trap->trap_damage_char, char,
                       strlen(etrap->GetItem(OBJ_TRAP_MESS_CHAR)->GetString()) + 1);
                strcpy(obj_proto[iobj].trap->trap_damage_char,
                       etrap->GetItem(OBJ_TRAP_MESS_CHAR)->GetString());
            }

            if (etrap->GetItem(OBJ_TRAP_MESS_ROOM)->GetString()) {
                CREATE(obj_proto[iobj].trap->trap_damage_room, char,
                       strlen(etrap->GetItem(OBJ_TRAP_MESS_ROOM)->GetString()) + 1);
                strcpy(obj_proto[iobj].trap->trap_damage_room,
                       etrap->GetItem(OBJ_TRAP_MESS_ROOM)->GetString());
            }

            if (etrap->GetItem(OBJ_TRAP_MESS_SCHAR)->GetString()) {
                CREATE(obj_proto[iobj].trap->trap_nodamage_char, char,
                       strlen(etrap->GetItem(OBJ_TRAP_MESS_SCHAR)->GetString()) + 1);
                strcpy(obj_proto[iobj].trap->trap_nodamage_char,
                       etrap->GetItem(OBJ_TRAP_MESS_SCHAR)->GetString());
            }

            if (etrap->GetItem(OBJ_TRAP_MESS_SROOM)->GetString()) {
                CREATE(obj_proto[iobj].trap->trap_nodamage_room, char,
                       strlen(etrap->GetItem(OBJ_TRAP_MESS_SROOM)->GetString()) + 1);
                strcpy(obj_proto[iobj].trap->trap_nodamage_room,
                       etrap->GetItem(OBJ_TRAP_MESS_SROOM)->GetString());
            }

            if (etrap->GetItem(OBJ_TRAP_MESS_KCHAR)->GetString()) {
                CREATE(obj_proto[iobj].trap->trap_kill_char, char,
                       strlen(etrap->GetItem(OBJ_TRAP_MESS_KCHAR)->GetString()) + 1);
                strcpy(obj_proto[iobj].trap->trap_kill_char,
                       etrap->GetItem(OBJ_TRAP_MESS_KCHAR)->GetString());
            }

            if (etrap->GetItem(OBJ_TRAP_MESS_KROOM)->GetString()) {
                CREATE(obj_proto[iobj].trap->trap_kill_room, char,
                       strlen(etrap->GetItem(OBJ_TRAP_MESS_KROOM)->GetString()) + 1);
                strcpy(obj_proto[iobj].trap->trap_kill_room,
                       etrap->GetItem(OBJ_TRAP_MESS_KROOM)->GetString());
            }
        }                       // ÀœŒ≈√ ÏÔ˜ı˚Î·
        //ıÛÈÏÂÓÈÂ
        if (obj->GetItem(OBJ_POWER)->GetInt())
            obj_proto[iobj].powered = obj->GetItem(OBJ_POWER)->GetInt();
        else
            obj_proto[iobj].powered = 0;

        obj_proto[iobj].obj_flags.type_tool = obj->GetItem(OBJ_TOOL)->GetInt();


        /* Î˜ÂÛÙ˘ */
        const int *quest_list = obj->GetItem(OBJ_QUEST)->GetScript(numadd);

        for (j = 0; j < numadd; j++)
            add_quest_obj(obj_proto + iobj, quest_list[j]);

        /* ÛÂ„ÎÔÌ·Ó‰˘ */
        numadd = obj->GetItem(OBJ_COMMAND)->GetNumberItem();
        for (j = 0; j < numadd; j++) {
            int obj_vnum = 0, mob_vnum = 0, extract = 0, lroom = 0, linv = 0;
            char *arg = 0, *err = 0, *active = 0, *active_room = 0, // prool
                *to_char = 0, *to_room = 0, *error_char = 0, *error_room = 0;

            CItem *command = obj->GetItem(OBJ_COMMAND)->GetItem(j);

            extract = command->GetItem(OBJ_COMMAND_EXTRACT)->GetInt();
            active = command->GetItem(OBJ_COMMAND_ACTIVE)->GetString();
            active_room = command->GetItem(OBJ_COMMAND_ACTIVE_ROOM)->GetString();
            error_char = command->GetItem(OBJ_COMMAND_ECHAR)->GetString();
            error_room = command->GetItem(OBJ_COMMAND_EROOM)->GetString();
            if (command->GetItem(OBJ_COMMAND_ARG)->GetNumberItem()) {
                CItem *carg = command->GetItem(OBJ_COMMAND_ARG)->GetItem(0);

                obj_vnum = carg->GetItem(OBJ_COMMAND_ARG_OBJ)->GetInt();
                mob_vnum = carg->GetItem(OBJ_COMMAND_ARG_MOB)->GetInt();
                arg = carg->GetItem(OBJ_COMMAND_ARG_ARG)->GetString();
                err = carg->GetItem(OBJ_COMMAND_ARG_ERR)->GetString();
            }
            if (command->GetItem(OBJ_COMMAND_LOAD)->GetNumberItem()) {
                CItem *cload = command->GetItem(OBJ_COMMAND_LOAD)->GetItem(0);

                lroom = cload->GetItem(OBJ_COMMAND_LOAD_ROOM)->GetInt();
                linv = cload->GetItem(OBJ_COMMAND_LOAD_CHAR)->GetInt();
                to_char = cload->GetItem(OBJ_COMMAND_MESS_CHAR)->GetString();
                to_room = cload->GetItem(OBJ_COMMAND_MESS_ROOM)->GetString();
            }

            struct item_op_data *k;

            CREATE(k, struct item_op_data, 1);

            k->command = command->GetItem(OBJ_COMMAND_NO)->GetInt();
            k->tool = command->GetItem(OBJ_COMMAND_NO)->GetInt();
            k->obj_vnum = obj_vnum;
            k->mob_vnum = mob_vnum;
            k->load_room = lroom;
            k->load_char = linv;
            k->script_vnum = 0;
            k->extract = command->GetItem(OBJ_COMMAND_EXTRACT)->GetInt();
            k->script = command->GetItem(OBJ_COMMAND_XSCRIPT)->GetInt();
            k->tool = command->GetItem(OBJ_COMMAND_TOOL)->GetInt();

            k->expr = new CExpression;
            command->GetItem(OBJ_COMMAND_EXPR)->ReleaseExpression(EXPR(k->expr));

            if (arg)
                k->arg = str_dup(arg);
            if (err)
                k->error = str_dup(arg);
            if (active)
                k->active = str_dup(active);
            if (active_room)
                k->active_room = str_dup(active_room);
            if (to_char)
                k->mess_load_char = str_dup(to_char);
            if (to_room)
                k->mess_load_room = str_dup(to_room);
            if (error_char)
                k->error_char = str_dup(error_char);
            if (error_room)
                k->error_char = str_dup(error_room);


            k->next = (obj_proto + iobj)->operations;
            (obj_proto + iobj)->operations = k;
        }

        obj_proto[iobj].linkWrapper();
        fenia_mark_guid(obj_proto[iobj].guid);
        top_of_objt = iobj++;
        //log("ÙÔ ÔÊ Ú‰ %d",top_of_objt);
    }

    delete Obj;
}

void add_xobj(CObj * WObj, struct obj_data *tobj, int equip, int position)
{
    int obj_nr, spell_db = find_spell_num(SPELL_DB);
    struct obj_data *cobj = NULL;
    struct C_obj_affected_type *af;

    char ctmp[256];

    if (OBJ_FLAGGED(tobj, ITEM_NORENT) || OBJ_FLAGGED(tobj, ITEM_REPOPDROP) || OBJ_FLAGGED(tobj, ITEM_ZONEDECAY))       //“≈ƒÕ≈‘Ÿ ” ∆Ã¡«œÕ !NORENT –“œ”‘œ Œ≈ ⁄¡–…”Ÿ◊¡≈Õ
        return;

    obj_nr = WObj->NewSubItem();
    CItem *obj = WObj->GetItem(obj_nr);

//log("˙¡–…”Ÿ◊¡¿ –“≈ƒÕ≈‘ [%d] %s",GET_OBJ_VNUM(tobj),tobj->names);

    obj->GetItem(OBJ_NUMBER)->SetParam(GET_OBJ_VNUM(tobj));
    obj->GetItem(OBJ_GUID)->SetParam(tobj->guid);
    obj->GetItem(OBJ_ALIAS)->SetParam(tobj->name);
    obj->GetItem(OBJ_NAME)->SetParam(tobj->names);
    obj->GetItem(OBJ_LINE)->SetParam(tobj->description);
    obj->GetItem(OBJ_TIMER)->SetParam(GET_OBJ_TIMER(tobj));
    obj->GetItem(OBJ_TIMELOAD)->SetParam(GET_OBJ_TIMELOAD(tobj));
    obj->GetItem(OBJ_SEX)->SetParam(tobj->obj_flags.Obj_sex);
    obj->GetItem(OBJ_WEIGHT)->SetParam(tobj->obj_flags.weight);
    obj->GetItem(OBJ_COST)->SetParam(tobj->obj_flags.cost);
    obj->GetItem(OBJ_TYPE)->SetParam(tobj->obj_flags.type_flag);
    obj->GetItem(OBJ_LIMIT)->SetParam(tobj->limit);
    obj->GetItem(OBJ_DURAB)->SetParam(tobj->obj_flags.Obj_max);

    if (GET_OBJ_TYPE(tobj) == ITEM_MISSILE && tobj->missile) {
        int ms_nr;

        ms_nr = obj->GetItem(OBJ_MISSILE)->NewSubItem();
        CItem *missl = obj->GetItem(OBJ_MISSILE)->GetItem(ms_nr);

        missl->GetItem(OBJ_MISS_TCOUNT)->SetParam(tobj->obj_flags.value[2]);
        missl->GetItem(OBJ_MISS_COUNT)->SetParam(tobj->obj_flags.value[1]);
    }

    if (tobj->obj_flags.Obj_cur > tobj->obj_flags.Obj_max)
        obj->GetItem(OBJ_CURRENT_DURAB)->SetParam(tobj->obj_flags.Obj_max);
    else
        obj->GetItem(OBJ_CURRENT_DURAB)->SetParam(tobj->obj_flags.Obj_cur);

    if (GET_OBJ_VNUM(tobj) == -1) {
        if (GET_FLAG(tobj->obj_flags.extra_flags, 0)) {
            *ctmp = '\0';
            tascii(&GET_FLAG(tobj->obj_flags.extra_flags, 0), 4, ctmp);
            obj->GetItem(OBJ_PROPERTIES)->SetParam(ctmp);
        }

        if (tobj->obj_flags.wear_flags) {
            *ctmp = '\0';
            tascii(&tobj->obj_flags.wear_flags, 1, ctmp);
            obj->GetItem(OBJ_WEAR)->SetParam(ctmp);
        }
    }

    /* ˙¡–…”ÿ ‹∆∆≈À‘œ◊ */
    for (af = tobj->C_affected; af; af = af->next)
        if (af->type != spell_db && af->location != APPLY_DB) {
            int aff_i;

            aff_i = WObj->GetItem(obj_nr)->GetItem(OBJ_AFFECT)->NewSubItem();
            CItem *affs = WObj->GetItem(obj_nr)->GetItem(OBJ_AFFECT)->GetItem(aff_i);

            affs->GetItem(AFFECT_TYPE)->SetParam(SPELL_NO(af->type));
            affs->GetItem(AFFECT_LOC)->SetParam(af->location);
            affs->GetItem(AFFECT_MOD)->SetParam(af->modifier);
            affs->GetItem(AFFECT_VEC)->SetParam((int) af->bitvector);
            affs->GetItem(AFFECT_NFL)->SetParam((int) af->no_flag);
            affs->GetItem(AFFECT_AFL)->SetParam((int) af->anti_flag);
            affs->GetItem(AFFECT_EXT)->SetParam((int) af->extra);
            affs->GetItem(AFFECT_DUR)->SetParam(af->duration);
        }


    if (tobj->owner && OBJ_FLAGGED(tobj, ITEM_SOULBIND))
        obj->GetItem(OBJ_OWNER)->SetParam(tobj->owner);

//ÈÓ‰È˜È‰ı·Ï¯Ó˘Â ·Ú·ÌÂÙÚ˘ ÚÂ‰ÌÂÙÔ˜
    switch (GET_OBJ_TYPE(tobj)) {
        case ITEM_CONTAINER:
        case ITEM_FICTION:
            if (GET_FLAG(tobj->bflag, 0)) {
                *ctmp = '\0';
                tascii(&GET_FLAG(tobj->bflag, 0), 4, ctmp);
                obj->GetItem(OBJ_BAG_PROPERTIES)->SetParam(ctmp);
            }
            break;
        case ITEM_FOOD:
            obj->GetItem(OBJ_FOOD)->SetParam(tobj->obj_flags.value[0]);
            break;
        case ITEM_FOUNTAIN:
        case ITEM_DRINKCON:
            obj->GetItem(OBJ_VALUE)->SetParam(tobj->obj_flags.value[0]);
            obj->GetItem(OBJ_CUR_VALUE)->SetParam(tobj->obj_flags.value[1]);
            obj->GetItem(OBJ_LIQ)->SetParam(tobj->obj_flags.value[2]);
            break;
        case ITEM_STAFF:
        case ITEM_WAND:
            obj->GetItem(OBJ_SLOT_CUR)->SetParam(tobj->obj_flags.value[2]);
            break;
        case ITEM_MONEY:
            obj->GetItem(OBJ_GOLD)->SetParam(tobj->obj_flags.value[0]);
            break;
    }

    int x_nr = WObj->GetItem(obj_nr)->GetItem(OBJ_XSAVE)->NewSubItem();
    CItem *xsave = WObj->GetItem(obj_nr)->GetItem(OBJ_XSAVE)->GetItem(x_nr);

    xsave->GetItem(OBJ_XSAVE_POS)->SetParam(position);
    xsave->GetItem(OBJ_XSAVE_EQ)->SetParam(equip);

    for (cobj = tobj->contains; cobj; cobj = cobj->next_content)
        add_xobj(WObj, cobj, (tobj->worn_by) ? tobj->worn_on : -1, GET_OBJ_VNUM(tobj));

}

void save_char_xobj(struct char_data *ch)
{
    CObj *Wobj;
    char fname[256];
    struct obj_data *obj;
    int j;

    Wobj = new CObj;

    if (!Wobj->Initialization()) {
        log("Ô˚È‚Î·: Ó≈–œÃ’ﬁ¡≈‘”— …Œ…√…¡Ã…⁄…“œ◊¡ Wobj ’ %s", GET_NAME(ch));
        return;
    }

    get_filename(GET_NAME(ch), fname, TEXT_CRASH_FILE);

    for (obj = ch->carrying; obj; obj = obj->next_content)
        add_xobj(Wobj, obj, -1, -1);

    for (j = 0; j < NUM_WEARS; j++) {
        if (GET_EQ(ch, j))
            add_xobj(Wobj, GET_EQ(ch, j), j, j);

        if (GET_TATOO(ch, j))
            add_xobj(Wobj, GET_TATOO(ch, j), j, j);
    }

    Wobj->SaveAll(fname, TRUE);
    /*{
       remove(fname);
       rename(fbackup,fname);

       } */
    delete Wobj;
}

/*****************************************************************************/
/* ˛ÙÂÓÈÂ                                                                    */
/*****************************************************************************/
void obj_to_contaner(struct char_data *ch, struct obj_data *object, int equip, int position)
{
    bool fnd = FALSE;
    struct obj_data *cobj;

//ÀœŒ‘≈ Œ≈“ ◊ ‹À…–…“œ◊À≈
    if (equip != -1 && equip < NUM_WEARS)
        if ((cobj = GET_EQ(ch, equip))) {
            obj_to_obj(object, cobj);
            return;
        }
//ÀœŒ‘≈ Œ≈“ ◊ …Œ◊≈Œ‘¡“≈
    for (cobj = ch->carrying; cobj; cobj = cobj->next_content)
        if (GET_OBJ_VNUM(cobj) == position) {
            obj_to_obj(object, cobj);
            fnd = TRUE;
            break;
            return;
        }

    if (!fnd)
        obj_to_char(object, ch);
}

struct obj_data *read_xobj(CObj * Wobj, int i)
{
    int vnum, j, numadd;
    struct obj_data *object = NULL;
    CItem *obj = Wobj->GetItem(i);
    char buf[MAX_STRING_LENGTH];
    struct C_obj_affected_type af;

    vnum = obj->GetItem(OBJ_NUMBER)->GetInt();
    if (vnum == -1) {           //”œ⁄ƒ¡≈Õ Œœ◊Ÿ  –“≈ƒÕ≈‘
        object = create_obj();

        CREATE(object->name, char, strlen(obj->GetItem(OBJ_ALIAS)->GetString()) + 1);

        strcpy(object->name, obj->GetItem(OBJ_ALIAS)->GetString());

        //Ó·˙˜·ÓÈÂ
        CREATE(object->names, char, strlen(obj->GetItem(OBJ_NAME)->GetString()) + 1);

        strcpy(object->names, obj->GetItem(OBJ_NAME)->GetString());

        for (j = 0; j < NUM_PADS; j++) {
            strcpy(buf, get_name_pad((char *) obj->GetItem(OBJ_NAME)->GetString(), j, PAD_OBJECT));
            CREATE(GET_OBJ_PNAME(object, j), char, strlen(buf) + 1);

            strcpy(GET_OBJ_PNAME(object, j), buf);
        }

        strcpy(buf,
               get_name_pad((char *) obj->GetItem(OBJ_NAME)->GetString(), PAD_IMN, PAD_OBJECT));
        CREATE(object->short_description, char, strlen(buf) + 1);

        strcpy(object->short_description, buf);

        CREATE(object->description, char, strlen(obj->GetItem(OBJ_LINE)->GetString()) + 1);

        strcpy(object->description, obj->GetItem(OBJ_LINE)->GetString());

        if (obj->GetItem(OBJ_WEAR)->GetString())
            asciiflag_conv(obj->GetItem(OBJ_WEAR)->GetString(), &object->obj_flags.wear_flags);

        if (obj->GetItem(OBJ_PROPERTIES)->GetString())
            asciiflag_conv((char *) obj->GetItem(OBJ_PROPERTIES)->GetString(),
                           &object->obj_flags.extra_flags);

        GET_OBJ_TYPE(object) = obj->GetItem(OBJ_TYPE)->GetInt();
        GET_OBJ_MAX(object) = obj->GetItem(OBJ_DURAB)->GetInt();
        GET_OBJ_SEX(object) = obj->GetItem(OBJ_SEX)->GetInt();
    } else
        object = read_object(vnum, VIRTUAL, FALSE);

    if (!object)
        return (NULL);

    if (obj->GetItem(OBJ_GUID)->GetLongLong())
        object->guid = obj->GetItem(OBJ_GUID)->GetLongLong();
    object->linkWrapper();

    /*˛‘≈Œ…≈ –¡“¡Õ≈‘“œ◊ –“≈ƒÕ≈‘¡ */
    GET_OBJ_TIMER(object) = obj->GetItem(OBJ_TIMER)->GetInt();
    if (obj->GetItem(OBJ_TIMELOAD)->GetInt())
        GET_OBJ_TIMELOAD(object) = obj->GetItem(OBJ_TIMELOAD)->GetInt();
    else
        GET_OBJ_TIMELOAD(object) = time(0);
    GET_OBJ_CUR(object) = obj->GetItem(OBJ_CURRENT_DURAB)->GetInt();

    if (obj->GetItem(OBJ_PROPERTIES)->GetString())
        asciiflag_conv((char *) obj->GetItem(OBJ_PROPERTIES)->GetString(),
                       &object->obj_flags.extra_flags);

    if (obj->GetItem(OBJ_MISSILE)->GetNumberItem()) {
        object->obj_flags.value[1] =
            obj->GetItem(OBJ_MISSILE)->GetItem(0)->GetItem(OBJ_MISS_COUNT)->GetInt();
        object->obj_flags.value[2] =
            obj->GetItem(OBJ_MISSILE)->GetItem(0)->GetItem(OBJ_MISS_TCOUNT)->GetInt();
        //log("ÚÔ˛ÈÙ·ÓÔ %d | %d",object->missile->count,object->missile->icount);
    }

    numadd = obj->GetItem(OBJ_AFFECT)->GetNumberItem();
    for (j = 0; j < numadd; j++) {
        CItem *affect = obj->GetItem(OBJ_AFFECT)->GetItem(j);

        af.type = 0;
        af.duration = 0;
        af.modifier = 0;
        af.location = 0;
        af.bitvector = 0;
        af.extra = 0;
        af.no_flag = 0;
        af.anti_flag = 0;

        af.type = find_spell_num(affect->GetItem(AFFECT_TYPE)->GetInt());
        af.duration = affect->GetItem(AFFECT_DUR)->GetInt();
        af.modifier = affect->GetItem(AFFECT_MOD)->GetInt();
        af.location = affect->GetItem(AFFECT_LOC)->GetInt();
        af.bitvector = affect->GetItem(AFFECT_VEC)->GetInt();
        af.extra = affect->GetItem(AFFECT_EXT)->GetInt();
        af.no_flag = affect->GetItem(AFFECT_NFL)->GetInt();
        af.anti_flag = affect->GetItem(AFFECT_AFL)->GetInt();

        affect_to_object(object, &af);
    }

    if (obj->GetItem(OBJ_OWNER)->GetInt())
        object->owner = obj->GetItem(OBJ_OWNER)->GetInt();

    //ÈŒƒ…◊…ƒ’¡ÃÿŒŸ≈ ”◊œ ”‘◊¡
    switch (GET_OBJ_TYPE(object)) {
        case ITEM_CONTAINER:
        case ITEM_FICTION:
            object->bflag.flags[0] = 0;
            object->bflag.flags[1] = 0;
            object->bflag.flags[2] = 0;
            object->bflag.flags[3] = 0;

            if (obj->GetItem(OBJ_BAG_PROPERTIES)->GetString())
                asciiflag_conv((char *) obj->GetItem(OBJ_BAG_PROPERTIES)->GetString(),
                               &object->bflag);

            object->bflag_reset = object->bflag;
            break;
        case ITEM_FOOD:
            if (obj->GetItem(OBJ_FOOD)->GetInt())
                object->obj_flags.value[0] = obj->GetItem(OBJ_FOOD)->GetInt();
            break;
        case ITEM_FOUNTAIN:
        case ITEM_DRINKCON:
            object->obj_flags.value[0] = obj->GetItem(OBJ_VALUE)->GetInt();
            object->obj_flags.value[1] = obj->GetItem(OBJ_CUR_VALUE)->GetInt();
            object->obj_flags.value[2] = obj->GetItem(OBJ_LIQ)->GetInt();
            name_from_drinkcon(object);
            if (object->obj_flags.value[1] > 0)
                name_to_drinkcon(object, object->obj_flags.value[2]);
            break;
        case ITEM_STAFF:
        case ITEM_WAND:
            if (obj->GetItem(OBJ_SLOT_CUR)->GetInt())
                object->obj_flags.value[2] = obj->GetItem(OBJ_SLOT_CUR)->GetInt();
            break;
        case ITEM_MONEY:
            if (obj->GetItem(OBJ_GOLD)->GetInt())
                object->obj_flags.value[0] = obj->GetItem(OBJ_GOLD)->GetInt();
            break;
    }

    return (object);
}


void read_char_xobj(struct char_data *ch)
{
    CObj *Wobj;
    char fname[256];
    struct obj_data *object = NULL;
    int number, i, position, equip, rnum;

    Wobj = new CObj;

    if (!Wobj->Initialization())
        return;

    get_filename(GET_NAME(ch), fname, TEXT_CRASH_FILE);

    if (!Wobj->ReadConfig(fname)) {
        log("Ô˚È‚Î·: ≈“”œŒ¡÷ '%s' Œ≈ …Õ≈≈‘ ∆¡ Ã¡ –“≈ƒÕ≈‘œ◊-“≈Œ‘Ÿ", GET_NAME(ch));
        return;
    }

    number = Wobj->GetNumberItem();
    for (i = 0; i < number; i++) {
        object = read_xobj(Wobj, i);

        if (Wobj->GetItem(i)->GetItem(OBJ_XSAVE)->GetNumberItem())
            equip =
                Wobj->GetItem(i)->GetItem(OBJ_XSAVE)->GetItem(0)->GetItem(OBJ_XSAVE_EQ)->GetInt();
        else
            equip = -1;

        if (Wobj->GetItem(i)->GetItem(OBJ_XSAVE)->GetNumberItem())
            position =
                Wobj->GetItem(i)->GetItem(OBJ_XSAVE)->GetItem(0)->GetItem(OBJ_XSAVE_POS)->GetInt();
        else
            position = -1;



        if (object) {           //À’ƒ¡ ¬Ÿ Œ¡Õ ⁄¡–…»¡‘ÿ –“≈ƒÕ≈‘
            if ((rnum = GET_OBJ_RNUM(object)) > -1)
                obj_index[rnum].stored--;

            if (!IS_GOD(ch) && IS_SET(GET_OBJ_EXTRA(object, ITEM_RENT_DELETE), ITEM_RENT_DELETE)) {
                send_to_charf(ch, "%s ÀœŒ∆…”Àœ◊¡Œ%s ⁄¡ –“≈◊Ÿ€≈Œ…≈ ”‘œ…Õœ”‘… –œ”‘œ—.\r\n",
                              CAP(GET_OBJ_PNAME(object, 0)), GET_OBJ_SUF_1(object));
                extract_obj(object, FALSE);
                continue;
            }

            if (!IS_GOD(ch) && IS_SET(GET_OBJ_EXTRA(object, ITEM_RENT_DELETE2), ITEM_RENT_DELETE2)) {
                send_to_charf(ch, "%s ÀœŒ∆…”Àœ◊¡Œ%s ⁄¡ Œ¡“’€≈Œ…≈ –“¡◊…Ã –œ”‘œ—.\r\n",
                              CAP(GET_OBJ_PNAME(object, 0)), GET_OBJ_SUF_1(object));
                extract_obj(object, FALSE);
                continue;
            }

            if (IS_SET(GET_OBJ_EXTRA(object, ITEM_RENT_DELETE3), ITEM_RENT_DELETE3)) {
                if (PRF_FLAGGED(ch, PRF_CODERINFO))
                    act("@1… “¡””Ÿ–¡Ã@1(”—,¡”ÿ,œ”ÿ,…”ÿ) –œ …”‘≈ﬁ≈Œ…¿ ◊“≈Õ≈Œ… »“¡Œ≈Œ…—. (Ûœ⁄ƒ¡Œ: %1)", "Ì–‘", ch, object, ascii_time(GET_OBJ_TIMELOAD(object)));
                else
                    act("@1… “¡””Ÿ–¡Ã@1(”—,¡”ÿ,œ”ÿ,…”ÿ) –œ …”‘≈ﬁ≈Œ…¿ ◊“≈Õ≈Œ… »“¡Œ≈Œ…—.", "Ì–", ch,
                        object);
                extract_obj(object, FALSE);
                continue;
            }

            if (position > 99)
                obj_to_contaner(ch, object, equip, position);
            else if (equip >= 0 && equip < NUM_WEARS) {
                if (GET_OBJ_TYPE(object) == ITEM_TATOO)
                    equip_tatoo(ch, object, equip | 0x80 | 0x40);
                else
                    equip_char(ch, object, equip | 0x80 | 0x40);
            } else
                obj_to_char(object, ch);
        }
    }

    delete Wobj;

}
