#include "xscripts.h"
#include "parser_id.h"
#include "strlib.h"
#include "expr.h"

// ןÐÒÅÄÅÌÅÎÉÅ ÏÓÎÏ×ÎÙÈ ÐÁÒÁÍÅÔÒÏÍ
int SCR_NUMBER;  //הוךףפקיו
int SCR_EXPR;  //ץףלןקיו
int SCR_MESSAGE; //ףןןג‎ומיו
int SCR_MVNUM;  //ףןןג‎ומיו.חהו
int SCR_MCHAR;  //ףןןג‎ומיו.יחעןכץ
int SCR_MROOM;  //ףןןג‎ומיו.לןכבדיס
int SCR_MZONE;  //ףןןג‎ומיו.ןגלבףפר
int SCR_ERROR;  //ןûיגכב
int SCR_ECHAR;  //ןûיגכב.יחעןכץ
int SCR_EROOM;  //ןûיגכב.לןכבדיס
int SCR_EZONE;  //ןûיגכב.ןגלבףפר
int SCR_ESCRIPT; //ןûיגכב.הוךףפקיו
int SCR_LOAD;  //תבחעץתכב
int SCR_LOBJ;  //תבחעץתכב.נעוהםופ
int SCR_LEQ;  //תבחעץתכב.ימקומפבער
int SCR_LRANDOM; //תבחעץתכב.ףלץ‏בךמשך
int SCR_LMOB;  //תבחעץתכב.םןמףפע
int SCR_LEXIT;  //תבחעץתכב.קשטןה
int SCR_LPORTAL; //תבחעץתכב.נןעפבל
int SCR_LCHAR;  //תבחעץתכב.יחעןכץ
int SCR_LROOM;  //תבחעץתכב.לןכבדיס
int SCR_LGOLD;  //תבחעץתכב.הומרחי
int SCR_OBJ_LIMIT; //תבחעץתכב.ליםיפ

int SCR_LE_ROOM; // קשטןה.כןםמבפב
int SCR_LE_DIRECTION; // קשטןה.מבנעבקלומיו (LIST)
int SCR_LE_ROOMNUMBER; // קשטןה.כןםמבפב (INT)
int SCR_LE_DESCRIPTION; // קשטןה.ןניףבמיו (STRING)
int SCR_LE_NAME;  // קשטןה.פינ  (LIST)
int SCR_LE_ALIAS; // קשטןה.ףימןמים (STRING)
int SCR_LE_KEY;  // קשטןה.כלא‏  (INT)
int SCR_LE_PROPERTIES; // קשטןה.ףקןךףפקב (VECTOR)
int SCR_LE_QUALITY; // קשטןה.כב‏וףפקן
int SCR_LE_TRAP;  // קשטןה.לןקץûכב (INT)
int SCR_LE_TRAP_CHANCE; // קשטןה.לןקץûכב.ûבמף (INT)
int SCR_LE_TRAP_TYPEDAMAGE; // קשטןה.לןקץûכב.פקעוה  (LIST)
int SCR_LE_TRAP_FORCEDAMAGE; // קשטןה.לןקץûכב.ףקעוה  (RANDOM)
int SCR_LE_TRAP_SAVE;  // קשטןה.לןקץûכב.נעותיףפ
int SCR_LE_TRAP_MESS_CHAR; // קשטןה.לןקץûכב.יחעןכ
int SCR_LE_TRAP_MESS_ROOM; // קשטןה.לןקץûכב.כןםמבפב
int SCR_LE_TRAP_MESS_SCHAR; // קשטןה.לןקץûכב.עיחעןכ
int SCR_LE_TRAP_MESS_SROOM; // קשטןה.לןקץûכב.עכןםמבפב
int SCR_LE_TRAP_MESS_KCHAR; // קשטןה.לןקץûכב.ץגיפ_יחעןכ
int SCR_LE_TRAP_MESS_KROOM; // קשטןה.לןקץûכב.ץגיפ_לןכבדיס
int SCR_LE_SEX;  // קשטןה.עןה

int SCR_LP_PORTAL; //נןעפבל
int SCR_LP_ROOM;  //נןעפבל.חהו
int SCR_LP_DIRECTION; //נןעפבל.מבנעבקלומיו
int SCR_LP_TYPE;  //נןעפבל.פינ
int SCR_LP_TIME;  //נןעפבל.קעוםס
int SCR_LP_ROOMNUMBER;  //נןעפבל.לןכבדיס
int SCR_LP_KEY;  //נןעפבל.כלא‏
int SCR_LP_DESCRIPTION; //נןעפבל.ןניףבמיו
int SCR_LP_ACTIVE;  //נןעפבל.בכפיקמשך
int SCR_LP_DEACTIVE; //נןעפבל.הובכפיקמשך
int SCR_LP_MESS_CHAR; //נןעפבל.יחעןכ
int SCR_LP_MESS_ROOM; //נןעפבל.ןףפבלרמשם
int SCR_LP_WORK_TIME; //נןעפבל.קעוםס_עבגןפש

int SCR_DELETE;  //ץהבלומיו
int SCR_DOBJ;  //ץהבלומיו.נעוהםופ
int SCR_DEQ;  //ץהבלומיו.ימקומפבער
int SCR_DMOB;  //ץהבלומיו.םןמףפע
int SCR_DGOLD;  //ץהבלומיו.הומרחי
int SCR_DEXIT;  //ץהבלומיו.קשטןה
int SCR_DE_ROOM;  //קשטןה,חהו
int SCR_DE_DIRECTION; //קשטןה.מבנעבקלומיו
int SCR_DPORTAL;  //ץהבלומיו.נןעפבל
int SCR_DP_ROOM;  //נןעפבל.חהו
int SCR_DP_DIRECTION; //נןעפבל.מבנעבקלומיו
int SCR_DCHAR;  //ץהבלומיו.יחעןכ
int SCR_DROOM;  //ץהבלומיו.ןףפבלרמשם

int SCR_SPELL;  //תבכלימבמיו
int SCR_SPELL_NO; //תבכלימבמיו.מןםוע
int SCR_SPELL_LEVEL; //תבכלימבמיו.ץעןקומר
int SCR_SPELL_MCHAR; //תבכלימבמיו.יחעןכ
int SCR_SPELL_MROOM; //תבכלימבמיו.לןכבדיס

int SCR_TELEPORT; //נועוםו‎ומיו
int SCR_TWHERE;  //חהו
int SCR_TROOM;  //כןםמבפב
int SCR_TCHAROUT; //יןפגשפיו
int SCR_TCHARIN; //ינעיגשפיו
int SCR_TROOMOUT; //כןפגשפיו
int SCR_TROOMIN; //כנעיגשפיו


int SCR_VAR;  //נועוםוממבס
int SCR_VAR_NAME; //נועוםוממבס.מבתקבמיו
int SCR_VAR_VALUE; //נועוםוממבס.תמב‏ומיו
int SCR_VAR_TIME; //נועוםוממבס.קעוםס

int SCR_GLB;  //חלןגבלרמבס
int SCR_GLB_NAME; //חלןגבלרמבס.מבתקבמיו
int SCR_GLB_VALUE; //חלןגבלרמבס.תמב‏ומיו
int SCR_GLB_TIME; //חלןגבלרמבס.קעוםס

int SCR_RESET;  //ףגעןף

int SCR_AGRO;  //בחעוףףיס
int SCR_AMOB;  //בחעוףףיס.םןמףפע
int SCR_ATARGET; //בחעוףףיס.דולר

int SCR_DAMAGE;    //קעוה
int SCR_DAM_TARGET; //קעוה.דולר
int SCR_DAM_SHANCE; //קעוה.ûבמף
int SCR_DAM_TYPE;   //קעוה.פקעוה
int SCR_DAM_DAMAGE; //קעוה.ףקעוה
int SCR_DAM_STYPE;  //קעוה.עותיףפ
int SCR_DAM_SAVE;   //קעוה.ףןטעבמומיו
int SCR_DAM_KVICT;  //קעוה.ץצועפקב
int SCR_DAM_KROOM;  //קעוה.ץלןכבדיס
int SCR_DAM_HVICT;  //קעוה.קצועפקב
int SCR_DAM_HROOM;  //קעוה.קלןכבדיס
int SCR_DAM_MVICT;  //קעוה.נצועפקב
int SCR_DAM_MROOM;  //קעוה.נלןכבדיס

int SCR_EXP; //ןנשפ
int SCR_EXP_TYPE; //פינ
int SCR_EXP_SCHAR; //יןהמןעבתןקשך
int SCR_EXP_SGROUP; //חןהמןעבתןקשך
int SCR_EXP_CHAR; //יחעןכץ
int SCR_EXP_GROUP; //חעץננו

int SCR_APPLY; //םןהיזיכבפןע

int SCR_CONTINUE; //נעןהןלצומיו
int SCR_CNT_VNUM; //נעןהןלצומיו.מןםוע
int SCR_CNT_TIMER; //נעןהןלצומיו.פבךםוע


///////////////////////////////////////////////////////////////////////////////
CScr::CScr() {
}

CScr::~CScr() {
}
///////////////////////////////////////////////////////////////////////////////

CScr Scr;

bool CScr::Initialization(void) {
    SCR_NUMBER = Proto->AddItem(TYPE_INT, "הוךףפקיו");
    SCR_EXPR = Proto->AddItem(TYPE_EXPR, "ץףלןקיו", false);
    SCR_MESSAGE = Proto->AddItem(TYPE_STRUCT, "ףןןג‎ומיו", false);
    CItemProto *mess = Proto->GetItem(SCR_MESSAGE);
    SCR_MVNUM = mess->AddItem(TYPE_INT, "חהו", false);
    SCR_MCHAR  = mess->AddItem(TYPE_STRING, "יחעןכץ", false);
    SCR_MROOM  = mess->AddItem(TYPE_STRING, "לןכבדיס", false);
    SCR_MZONE  = mess->AddItem(TYPE_STRING, "ןגלבףפר", false);
    SCR_ERROR = Proto->AddItem(TYPE_STRUCT, "ןûיגכב", false);
    CItemProto *error = Proto->GetItem(SCR_ERROR);
    SCR_ECHAR  = error->AddItem(TYPE_STRING, "יחעןכץ", false);
    SCR_EROOM  = error->AddItem(TYPE_STRING, "לןכבדיס", false);
    SCR_EZONE  = error->AddItem(TYPE_STRING, "ןגלבףפר", false);
    SCR_ESCRIPT = error->AddItem(TYPE_INT, "הוךףפקיו", false);
    SCR_LOAD = Proto->AddItem(TYPE_STRUCT, "תבחעץתכב", false);
    CItemProto *load = Proto->GetItem(SCR_LOAD);
    SCR_LOBJ = load->AddItem(TYPE_STRLIST, "נעוהםופ", false);
    SCR_LEQ = load->AddItem(TYPE_SCRIPT, "ימקומפבער", false);
    SCR_LRANDOM = load->AddItem(TYPE_SCRIPT, "ףלץ‏בךמשך", false);
    SCR_LMOB = load->AddItem(TYPE_STRLIST, "םןמףפע", false);
    SCR_OBJ_LIMIT = load->AddItem(TYPE_INT, "ליםיפ", false);
    SCR_LGOLD = load->AddItem(TYPE_INT, "הומרחי", false);
    SCR_LEXIT = load->AddItem(TYPE_STRUCT, "קשטןה", false);
    CItemProto *exit = load->GetItem(SCR_LEXIT);
    SCR_LE_ROOM = exit->AddItem(TYPE_INT, "חהו", true);
    SCR_LE_DIRECTION = exit->AddItem(TYPE_LIST, "מבנעבקלומיו", true,
                                     ParserConst.GetList(LIST_DIRECTIONS));
    SCR_LE_ROOMNUMBER = exit->AddItem(TYPE_INT, "כןםמבפב");
    SCR_LE_DESCRIPTION = exit->AddItem(TYPE_STRING, "ןניףבמיו", false);
    SCR_LE_NAME       = exit->AddItem(TYPE_STRING, "מבתקבמיו", false);
    SCR_LE_ALIAS = exit->AddItem(TYPE_STRING, "ףימןמיםש", false);
    SCR_LE_KEY = exit->AddItem(TYPE_INT, "כלא‏", false);
    SCR_LE_PROPERTIES = exit->AddItem(TYPE_VECTOR, "ףקןךףפקב", false,
                                      ParserConst.GetVector(VEC_EXIT_PROP));
    SCR_LE_QUALITY   = exit->AddItem(TYPE_INT, "כב‏וףפקן", false);
    SCR_LE_SEX      = exit->AddItem(TYPE_LIST, "עןה", false,
                                    ParserConst.GetList(LIST_SEX));
    SCR_LE_TRAP = exit->AddItem(TYPE_STRUCT, "לןקץûכב", false);
    CItemProto *etrap = exit->GetItem(SCR_LE_TRAP);
    SCR_LE_TRAP_CHANCE = etrap->AddItem(TYPE_INT, "ûבמף");
    SCR_LE_TRAP_TYPEDAMAGE = etrap->AddItem(TYPE_LIST, "פקעוה", true,
                                            ParserConst.GetList(LIST_DAMAGE));
    SCR_LE_TRAP_FORCEDAMAGE = etrap->AddItem(TYPE_RANDOM, "ףקעוה");
    SCR_LE_TRAP_SAVE = etrap->AddItem(TYPE_INT, "נעותיףפ", false);
    SCR_LE_TRAP_MESS_CHAR = etrap->AddItem(TYPE_STRING, "יחעןכ", false);
    SCR_LE_TRAP_MESS_ROOM = etrap->AddItem(TYPE_STRING, "ןףפבלרמשם", false);
    SCR_LE_TRAP_MESS_SCHAR = etrap->AddItem(TYPE_STRING, "עיחעןכ", false);
    SCR_LE_TRAP_MESS_SROOM = etrap->AddItem(TYPE_STRING, "עןףפבלרמשם", false);
    SCR_LE_TRAP_MESS_KCHAR = etrap->AddItem(TYPE_STRING, "ץגיפ_יחעןכ", false);
    SCR_LE_TRAP_MESS_KROOM = etrap->AddItem(TYPE_STRING, "ץגיפ_לןכבדיס", false);

    SCR_LPORTAL = load->AddItem(TYPE_STRUCT, "נןעפבל", false);
    CItemProto *portal = load->GetItem(SCR_LPORTAL);
    SCR_LP_ROOM = portal->AddItem(TYPE_INT, "חהו", true);
    SCR_LP_DIRECTION = portal->AddItem(TYPE_LIST, "מבנעבקלומיו", true,
                                       ParserConst.GetList(LIST_DIRECTIONS));
    SCR_LP_TYPE = portal->AddItem(TYPE_LIST, "פינ", true,
                                  ParserConst.GetList(LIST_PORTAL_TYPES));
    SCR_LP_TIME = portal->AddItem(TYPE_INT, "קעוםס", false);
    SCR_LP_ROOMNUMBER = portal->AddItem(TYPE_INT, "כןםמבפב");
    SCR_LP_KEY = portal->AddItem(TYPE_INT, "כלא‏");
    SCR_LP_DESCRIPTION = portal->AddItem(TYPE_STRING, "ףפעןכב");
    SCR_LP_ACTIVE = portal->AddItem(TYPE_STRING, "בכפיקמשך");
    SCR_LP_DEACTIVE = portal->AddItem(TYPE_STRING, "הובכפיקמשך");
    SCR_LP_MESS_CHAR = portal->AddItem(TYPE_STRING, "יחעןכ");
    SCR_LP_MESS_ROOM = portal->AddItem(TYPE_STRING, "ןףפבלרמשם");
    SCR_LP_WORK_TIME = portal->AddItem(TYPE_STRING, "קעוםס_עבגןפש", false);

    SCR_LCHAR = load->AddItem(TYPE_STRING, "יחעןכץ", false);
    SCR_LROOM = load->AddItem(TYPE_STRING, "לןכבדיס", false);


    SCR_DELETE = Proto->AddItem(TYPE_STRUCT, "ץהבלומיו", false);
    CItemProto *delt = Proto->GetItem(SCR_DELETE);
    SCR_DOBJ = delt->AddItem(TYPE_STRLIST, "נעוהםופ", false);
    SCR_DEQ = delt->AddItem(TYPE_STRLIST, "ימקומפבער", false);
    SCR_DMOB = delt->AddItem(TYPE_STRLIST, "םןמףפע", false);
    SCR_DGOLD = delt->AddItem(TYPE_INT, "הומרחי", false);
    SCR_DEXIT = delt->AddItem(TYPE_STRUCT, "קשטןה", false);
    CItemProto *dexit = delt->GetItem(SCR_DEXIT);
    SCR_DE_ROOM = dexit->AddItem(TYPE_INT, "חהו", true);
    SCR_DE_DIRECTION = dexit->AddItem(TYPE_LIST, "מבנעבקלומיו", true, ParserConst.GetList(LIST_DIRECTIONS));

    SCR_DPORTAL = delt->AddItem(TYPE_STRUCT, "נןעפבל", false);
    CItemProto *dportal = delt->GetItem(SCR_DPORTAL);
    SCR_DP_ROOM = dportal->AddItem(TYPE_INT, "חהו", true);
    SCR_DP_DIRECTION = dportal->AddItem(TYPE_LIST, "מבנעבקלומיו", true, ParserConst.GetList(LIST_DIRECTIONS));

    SCR_DCHAR = delt->AddItem(TYPE_STRING, "יחעןכץ", false);
    SCR_DROOM = delt->AddItem(TYPE_STRING, "לןכבדיס", false);

    SCR_SPELL = Proto->AddItem(TYPE_STRUCT, "תבכלימבמיו", false);
    CItemProto *spell = Proto->GetItem(SCR_SPELL);
    SCR_SPELL_NO = spell->AddItem(TYPE_LIST, "מןםוע", false, ParserConst.GetList(LIST_SPELLS));
    SCR_SPELL_LEVEL = spell->AddItem(TYPE_INT, "ץעןקומר", false);
    SCR_SPELL_MCHAR = spell->AddItem(TYPE_STRING, "יחעןכץ", false);
    SCR_SPELL_MROOM = spell->AddItem(TYPE_STRING, "לןכבדיס", false);

    SCR_TELEPORT = Proto->AddItem(TYPE_STRUCT, "נועוםו‎ומיו", false);
    CItemProto *telep = Proto->GetItem(SCR_TELEPORT);
    SCR_TROOM = telep->AddItem(TYPE_INT, "כןםמבפב", true);
    SCR_TWHERE = telep->AddItem(TYPE_INT, "חהו", false);
    SCR_TCHAROUT = telep->AddItem(TYPE_STRING, "יןפגשפיו", false);
    SCR_TCHARIN  = telep->AddItem(TYPE_STRING, "ינעיגשפיו", false);
    SCR_TROOMOUT  = telep->AddItem(TYPE_STRING, "כןפגשפיו", false);
    SCR_TROOMIN   = telep->AddItem(TYPE_STRING, "כנעיגשפיו", false);

    SCR_VAR = Proto->AddItem(TYPE_STRUCT, "נועוםוממבס", false);
    CItemProto *var = Proto->GetItem(SCR_VAR);
    SCR_VAR_NAME = var->AddItem(TYPE_STRING, "מבתקבמיו", true);
    SCR_VAR_VALUE = var->AddItem(TYPE_STRING, "תמב‏ומיו", true);
    SCR_VAR_TIME = var->AddItem(TYPE_INT, "קעוםס", false);

    SCR_GLB = Proto->AddItem(TYPE_STRUCT, "חלןגבלרמבס", false);
    CItemProto *glb = Proto->GetItem(SCR_GLB);
    SCR_GLB_NAME = glb->AddItem(TYPE_STRING, "מבתקבמיו", true);
    SCR_GLB_VALUE = glb->AddItem(TYPE_STRING, "תמב‏ומיו", true);
    SCR_GLB_TIME = glb->AddItem(TYPE_INT, "קעוםס", false);

    SCR_RESET = Proto->AddItem(TYPE_INT, "ףגעןף", false);

    SCR_AGRO = Proto->AddItem(TYPE_STRUCT, "בחעוףףיס", false);
    CItemProto *agro = Proto->GetItem(SCR_AGRO);
    SCR_AMOB = agro->AddItem(TYPE_INT, "םןמףפע", true);
    SCR_ATARGET = agro->AddItem(TYPE_LIST, "דולר", false, ParserConst.GetList(LIST_TARGET));

    SCR_DAMAGE = Proto->AddItem(TYPE_STRUCT, "קעוה", false);
    CItemProto *damage = Proto->GetItem(SCR_DAMAGE);
    SCR_DAM_TARGET = damage->AddItem(TYPE_LIST, "דולר", false, ParserConst.GetList(LIST_TARGET));
    SCR_DAM_SHANCE = damage->AddItem(TYPE_INT, "ûבמף", true);
    SCR_DAM_TYPE   = damage->AddItem(TYPE_LIST, "פקעוה", true, ParserConst.GetList(LIST_DAMAGE));
    SCR_DAM_DAMAGE = damage->AddItem(TYPE_STRING, "ףקעוה", true);
    SCR_DAM_STYPE  = damage->AddItem(TYPE_LIST, "עותיףפ", true, ParserConst.GetList(LIST_SAVE_TYPES));
    SCR_DAM_SAVE   = damage->AddItem(TYPE_INT, "ףןטעבמומיו", false);
    SCR_DAM_KVICT  = damage->AddItem(TYPE_STRING, "ץצועפקב", false);
    SCR_DAM_KROOM  = damage->AddItem(TYPE_STRING, "ץלןכבדיס", false);
    SCR_DAM_HVICT  = damage->AddItem(TYPE_STRING, "קצועפקב", false);
    SCR_DAM_HROOM  = damage->AddItem(TYPE_STRING, "קלןכבדיס", false);
    SCR_DAM_MVICT  = damage->AddItem(TYPE_STRING, "נצועפקב", false);
    SCR_DAM_MROOM  = damage->AddItem(TYPE_STRING, "נלןכבדיס", false);

    SCR_EXP = Proto->AddItem(TYPE_STRUCT, "ןנשפ", false);
    CItemProto *exp = Proto->GetItem(SCR_EXP);
    SCR_EXP_TYPE = exp->AddItem(TYPE_LIST, "פינ", false, ParserConst.GetList(LIST_EXP_TYPE));
    SCR_EXP_SCHAR = exp->AddItem(TYPE_INT, "יןהמןעבתןקשך", false);
    SCR_EXP_SGROUP = exp->AddItem(TYPE_INT, "חןהמןעבתןקשך", false);
    SCR_EXP_CHAR = exp->AddItem(TYPE_INT, "יחעןכץ", false);
    SCR_EXP_GROUP = exp->AddItem(TYPE_INT, "חעץננו", false);

    SCR_APPLY = Proto->AddItem(TYPE_STRLIST, "קליסמיו", false, ParserConst.GetList(LIST_APPLY));

    SCR_CONTINUE = Proto->AddItem(TYPE_STRUCT, "נעןהןלצומיו", false);
    CItemProto *next = Proto->GetItem(SCR_CONTINUE);
    SCR_CNT_VNUM = next->AddItem(TYPE_INT, "מןםוע", true);
    SCR_CNT_TIMER = next->AddItem(TYPE_INT, "פבךםוע", false);

    return CheckInit();
}

