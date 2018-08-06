/**************************************************************************
    νπν "ηÒΑΞΙ νΙÒΑ" (Σ) 2002-2003 αΞΔÒΕΚ εÒΝΙΫΙΞ
    ϊΑΗÒΥΪΛΑ ΖΑΚΜΟΧ ΙΗÒΟΧΟΗΟ ΝΙÒΑ
 **************************************************************************/

#include "xobj.h"
#include "xspells.h"
#include "parser_id.h"
#include "xtempl.h"
#include "strlib.h"
#include "expr.h"

/****/
// οΠÒΕΔΕΜΕΞΙΕ ΟΣΞΟΧΞΩΘ ΠΑÒΑΝΕΤÒΟΝ
int OBJ_NUMBER;  //πςεδνετ
int OBJ_GUID; //ημοβιδ
int OBJ_ALIAS;  //σιξοξινω
int OBJ_NAME;  //ινρ
int OBJ_LINE;  //στςολα
int OBJ_DESCRIPTION; //οπισαξιε
int OBJ_ADDITION; //δοπομξιτεμψξο
int OBJ_PROPERTIES; //πσχοκστχα
int OBJ_SEX;  //ςοδ
int OBJ_MATERIAL; //νατεςιαμ
int OBJ_WEIGHT;  //χεσ
int OBJ_COST;  //γεξα
int OBJ_TYPE;  //τιπ
int OBJ_WEAR;  //ισπομψϊοχαξιε
int OBJ_ANTI;  //ξευδοβστχο
int OBJ_NO;  //ϊαπςετ
int OBJ_AFFECTS; //πόζζελτω
int OBJ_APPLY;  //χμιρξιε
int OBJ_TIMER;  //τακνες
int OBJ_TIMELOAD; //σοϊδαξ
int OBJ_QUALITY; //λαώεστχο
int OBJ_LIMIT;  //πςεδεμ
int OBJ_DURAB;  //πςοώξοστψ
int OBJ_CURRENT_DURAB;  //τελ.πςοώξοστψ
int OBJ_MAGIC;  //ναηιρ
int OBJ_INTERCEPTION; //πεςεθχατ
int OBJ_SCRIPT;  //σλςιπτω
int OBJ_SHANCE;  //ϋαξσ
int OBJ_VAL0;  //ϊξαώ0
int OBJ_VAL1;  //ϊξαώ1
int OBJ_VAL2;  //ϊξαώ2
int OBJ_VAL3;  //ϊξαώ3
int OBJ_LIGHT;  //σχετ
int OBJ_LEVEL;  //υςοχεξψ
int OBJ_SPELL1;  //ϊαλμ1
int OBJ_SPELL2;  //ϊαλμ2
int OBJ_SPELL3;  //ϊαλμ3
int OBJ_ENCHANT; //ζοςνυμα
int OBJ_SLOT_MAX; //νϊαςρδω
int OBJ_SLOT_CUR; //ϊαςρδω
int OBJ_SPELL;  //ϊαλμιξαξιε
int OBJ_AC;  //βςοξρ
int OBJ_ARM0;  //ςεφυύεε
int OBJ_ARM1;  //λομΰύεε
int OBJ_ARM2;  //υδαςξοε
int OBJ_DAMAGE;  //Cχςεδ v1+v2
int OBJ_SKILL;  //οςυφιε
int OBJ_HIT;  //νχςεδ v3
int OBJ_SIZE;  //χνεστινοστψ v0
int OBJ_BAG_PROPERTIES; //λσχοκστχα v1
int OBJ_KEY;  //λμΰώ   v2
int OBJ_BAG_MAGIC; //υδοβστχο  v3
int OBJ_BAG_NOTFIT; //ξεπονεύαετσρ
int OBJ_OLIST;  //σοδεςφινοε
int OBJ_OGOLD;  //δεξψηι
int OBJ_VALUE;  //ενλοστψ v0
int OBJ_CUR_VALUE; //τενλοστψ v1
int OBJ_LIQ;  //φιδλοστψ v2
int OBJ_POISON;  //ρδ v3
int OBJ_FOOD;  //ξασωύεξιε v0
int OBJ_GOLD;  //συννα v0
int OBJ_BRIGHT;  //ρςλοστψ
int OBJ_TR_PROPERTIES; //λςσχοκστχα
int OBJ_TRAP;  //μοχυϋλα (int)
int OBJ_XSAVE;          //σοθςαξεξιε
int OBJ_POWER;  //υσιμεξιε
int OBJ_TEMPLATE; //δϋαβμοξ (ΞΟΝΕÒ ΫΑΒΜΟΞΑ)
int OBJ_ARM_CLASS; //λμασσ
int OBJ_P0;
int OBJ_P1;
int OBJ_P2;
int OBJ_COMMAND; //σπεγλοναξδα
int OBJ_TOOL;  //ιξστςυνεξτ
int OBJ_ADD_HIT; //δυδας
int OBJ_AHIT_TYPE; //δυδας.τιπ
int OBJ_AHIT_DAMAGE; //δυδας.σχςεδ
int OBJ_QUEST;  //λχεστω

//σΠΕΓΛΟΝΑΞΔΩ
int OBJ_COMMAND_NO; //σπεγλοναξδα.λοναξδα
int OBJ_COMMAND_ARG; //σπεγλοναξδα.αςηυνεξτ
int OBJ_COMMAND_TOOL; //σπεγλοναξδα.ιξστςυνεξτ
int OBJ_COMMAND_EXPR; //σπεγλοναξδα.υσμοχιε
int OBJ_COMMAND_ECHAR; //σπεγλοναξδα.οϋιβλα_ιηςολυ
int OBJ_COMMAND_EROOM; //σπεγλοναξδα.οϋιβλα_μολαγιρ
int OBJ_COMMAND_ACTIVE;
int OBJ_COMMAND_ARG_OBJ; //σπεγλοναξδα.αςηυνεξτ.πςεδνετ
int OBJ_COMMAND_ARG_ARG; //σπεγλοναξδα.αςηυνεξτ.στςολα
int OBJ_COMMAND_ARG_MOB; //σπεγλοναξδα.αςηυνεξτ.νοξστς
int OBJ_COMMAND_ARG_ERR; //σπεγλοναξδα.αςηυνεξτ.οϋιβλα
int OBJ_COMMAND_SCRIPT; //σπεγλοναξδα.σλςιπτ
int OBJ_COMMAND_EXTRACT; //σπεγλοναξδα.ςασσωπαξιε
int OBJ_COMMAND_LOAD;     //σπεγλοναξδα.ϊαηςυϊλα
int OBJ_COMMAND_LOAD_ROOM; //σπεγλοναξδα.ϊαηςυϊλα.χμολαγιΰ
int OBJ_COMMAND_LOAD_CHAR; //σπεγλοναξδα.ϊαηςυϊλα.χιξχεξταςψ
int OBJ_COMMAND_ACTIVE_ROOM;
int OBJ_COMMAND_MESS_ROOM;
int OBJ_COMMAND_MESS_CHAR;
int OBJ_COMMAND_XSCRIPT; //σπεγλοναξδα.δεκστχιε

//σΟΘÒΑΞΕΞΙΕ
int OBJ_XSAVE_EQ; //σοθςαξεξιε.όλιπιςοχλα
int OBJ_XSAVE_POS; //σοθςαξεξιε.πομοφεξιε

// δΟΠΟΜΞΙΤΕΜΨΞΟ
int OBJ_ADD_KEY; //δοπομξιτεμψξο.λμΰώ
int OBJ_ADD_TEXT; //δοπομξιτεμψξο.τελστ

// νΑΤΕÒΙΑΜ
int OBJ_MAT_TYPE; //νατεςιαμ.τιπ
int OBJ_MAT_VAL; //λομιώεστχο
int OBJ_MAT_MAIN; //οσξοχξοε

int OBJ_INT_COMMAND; // πεςεθχατ.λοναξδα (LIST)
int OBJ_INT_STOP; // πεςεθχατ.στοπ (int)
int OBJ_INT_EXPR; // πεςεθχατ.υσμοχιε (EXP)
int OBJ_INT_SCRIPT; // πεςεθχατ.δεκστχιε (int)
int OBJ_INT_MESSPLAYER; // πεςεθχατ.ιηςολ (STRING)
int OBJ_INT_MESSVICTIM; // πεςεθχατ.φεςτχα (STRING)
int OBJ_INT_MESSOTHER; // πεςεθχατ.οσταμψξων (STRING)
int OBJ_INT_MESSROOM; // πεςεθχατ.λονξατα (STRING)

//νΑΗΙΡ
int OBJ_MAGIC_SPELL; //ναηιρ.ϊαλμιξαξιε (LIST)
int OBJ_MAGIC_LEVEL; //ναηιρ.υςοχεξψ  (int)
int OBJ_MAGIC_PERCENT; //ναηιρ.ϋαξσ  (int)
int OBJ_MAGIC_MESS_CHAR; //ναηιρ.ιηςολυ (STRING)
int OBJ_MAGIC_MESS_VICT; //ναηιρ.φεςτχε (STRING)
int OBJ_MAGIC_MESS_ROOM; //ναηιρ.οσταμψξων (STRING)

//μΟΧΥΫΛΑ
int OBJ_TRAP_COMMAND;  //μοχυϋλα.λοναξδα
int OBJ_TRAP_CHANCE;  //μοχυϋλα.ϋαξσ (int)
int OBJ_TRAP_TYPEDAMAGE; //μοχυϋλα.τχςεδ  (LIST)
int OBJ_TRAP_FORCEDAMAGE; //μοχυϋλα.σχςεδ  (RANDOM)
int OBJ_TRAP_SAVE;  //μοχυϋλα.πςεϊιστ
int OBJ_TRAP_MESS_CHAR;  //μοχυϋλα.ιηςολ
int OBJ_TRAP_MESS_ROOM;  //μοχυϋλα.λονξατα
int OBJ_TRAP_MESS_SCHAR; //μοχυϋλα.ςιηςολ
int OBJ_TRAP_MESS_SROOM; //μοχυϋλα.ςλονξατα
int OBJ_TRAP_MESS_KCHAR; //μοχυϋλα.υβιτ_ιηςολ
int OBJ_TRAP_MESS_KROOM; //μοχυϋλα.υβιτ_μολαγιρ

//σοθςαξεξιε
int OBJ_AFFECT;
int AFFECT_TYPE;
int AFFECT_LOC;
int AFFECT_MOD;
int AFFECT_VEC;
int AFFECT_EXT;
int AFFECT_DUR;
int AFFECT_NFL;
int AFFECT_AFL;
int OBJ_OWNER;

//σπεγατλα

int OBJ_SPECHIT;  //σπεγαταλα
int OBJ_SPECHIT_SHANCE;  //σπεγαταλα.ϋαξσ
int OBJ_SPECHIT_XRACE;  //σπεγαταλα.ϊςασα
int OBJ_SPECHIT_ZRACE;  //σπεγαταλα.ςςασα
int OBJ_SPECHIT_XTYPE;  //σπεγαταλα.ϊτιπ
int OBJ_SPECHIT_ZTYPE;  //σπεγαταλα.ςτιπ
int OBJ_SPECHIT_CHAR;  //σπεγαταλα.ιηςολυ
int OBJ_SPECHIT_VICTIM;  //σπεγαταλα.φεςτχε
int OBJ_SPECHIT_ROOM;  //σπεγαταλα.μολαγιρ
int OBJ_SPECHIT_KOEF;  //σπεγαταλα.υχεμιώεξιε
int OBJ_SPECHIT_HTYPE;  //σπεγαταλα.υτιπ
int OBJ_SPECHIT_HDAMAGE; //σπεγαταλα.υσχςεδ
int OBJ_SPECHIT_SPELL;  //σπεγαταλα.ϊαλμιξαξιε
int OBJ_SPECHIT_SLEVEL;  //σπεγαταλα.υςοχεξψ
int OBJ_SPECHIT_TARGET;  //σπεγαταλα.γεμι

//σξαςρδω
int OBJ_MISSILE;  //σξαςρδ
int OBJ_MISS_TYPE; //σξρςρδ.τιπ
int OBJ_MISS_COUNT; //σξρςρδ.νλομιώεστχο
int OBJ_MISS_TCOUNT; //σξρςρδ.τλομιώεστχο
int OBJ_MISS_TEMPL; //σξρςρδ.σϋαβμοξ
int OBJ_MADD_HIT; //σξρςρδ.δυδας
int OBJ_MAHIT_TYPE; //σξρςρδ.δυδας.τιπ
int OBJ_MAHIT_DAMAGE; //σξρςρδ.δυδας.σχςεδ

/****/

///////////////////////////////////////////////////////////////////////////////
CObj::CObj() {
}

CObj::~CObj() {
}
///////////////////////////////////////////////////////////////////////////////

bool CObj::Initialization(void) {

    OBJ_NUMBER = Proto->AddItem(TYPE_INT, "πςεδνετ");
    OBJ_GUID = Proto->AddItem(TYPE_LONGLONG, "ημοβιδ", false);
    OBJ_ALIAS = Proto->AddItem(TYPE_STRING, "σιξοξινω");
    OBJ_NAME = Proto->AddItem(TYPE_STRING, "ξαϊχαξιε");
    OBJ_LINE = Proto->AddItem(TYPE_STRING, "στςολα");
    OBJ_DESCRIPTION = Proto->AddItem(TYPE_STRING, "οπισαξιε", false);
    OBJ_ADDITION = Proto->AddItem(TYPE_STRUCT, "δοπομξιτεμψξο", false);
    CItemProto *add = Proto->GetItem(OBJ_ADDITION);
    OBJ_ADD_KEY = add->AddItem(TYPE_STRING, "λμΰώ");
    OBJ_ADD_TEXT = add->AddItem(TYPE_STRING, "τελστ");
    OBJ_PROPERTIES = Proto->AddItem(TYPE_VECTOR, "πσχοκστχα", false, ParserConst.GetVector(VEC_OBJ_PROP));
    OBJ_SEX = Proto->AddItem(TYPE_LIST, "ςοδ", true, ParserConst.GetList(LIST_SEX));
    OBJ_MATERIAL = Proto->AddItem(TYPE_STRUCT, "νατεςιαμ", true);
    CItemProto *mat = Proto->GetItem(OBJ_MATERIAL);
    OBJ_MAT_TYPE = mat->AddItem(TYPE_LIST, "τιπ", true, ParserConst.GetList(LIST_MAT_TYPES));
    OBJ_MAT_VAL = mat->AddItem(TYPE_INT, "λομιώεστχο", true);
    OBJ_MAT_MAIN  = mat->AddItem(TYPE_INT, "οσξοχξοε", false);
    OBJ_WEIGHT = Proto->AddItem(TYPE_INT, "χεσ");
    OBJ_COST = Proto->AddItem(TYPE_INT, "γεξα");
    OBJ_TYPE = Proto->AddItem(TYPE_LIST, "τιπ", true,
                              ParserConst.GetList(LIST_OBJ_TYPES));
    OBJ_WEAR = Proto->AddItem(TYPE_VECTOR, "ισπομψϊοχαξιε", false,
                              ParserConst.GetVector(VEC_OBJ_WEAR));
    OBJ_ANTI = Proto->AddItem(TYPE_VECTOR, "ξευδοβστχο", false,
                              ParserConst.GetVector(VEC_ANTI));
    OBJ_NO  = Proto->AddItem(TYPE_VECTOR, "ϊαπςετ", false,
                             ParserConst.GetVector(VEC_ANTI));
    OBJ_AFFECTS = Proto->AddItem(TYPE_VECTOR, "πόζζελτω", false,
                                 ParserConst.GetVector(VEC_MOB_AFF));
    OBJ_ARM_CLASS  = Proto->AddItem(TYPE_LIST, "λμασσ", false,
                                    ParserConst.GetList(LIST_TYPE_ACLASS));
    OBJ_AFFECT = Proto->AddItem(TYPE_STRUCT, "σοθςαξεξιε", false);
    CItemProto *saffect = Proto->GetItem(OBJ_AFFECT);
    AFFECT_TYPE = saffect->AddItem(TYPE_INT, "τιπ");
    AFFECT_LOC = saffect->AddItem(TYPE_LIST, "παςανετς", false, ParserConst.GetList(LIST_APPLY));
    AFFECT_MOD = saffect->AddItem(TYPE_INT, "νοδιζιλατος");
    AFFECT_VEC = saffect->AddItem(TYPE_INT, "βιτχελτος", false);
    AFFECT_EXT = saffect->AddItem(TYPE_INT, "όλστςα", false);
    AFFECT_DUR = saffect->AddItem(TYPE_INT, "χςενρ");
    AFFECT_NFL = saffect->AddItem(TYPE_INT, "ϊαπςετ", false);
    AFFECT_AFL = saffect->AddItem(TYPE_INT, "ξευδοβστχο", false);
    OBJ_OWNER = Proto->AddItem(TYPE_INT, "χμαδεμεγ", false);

    OBJ_APPLY = Proto->AddItem(TYPE_STRLIST, "χμιρξιε", false,
                               ParserConst.GetList(LIST_APPLY));
    OBJ_TIMER = Proto->AddItem(TYPE_INT, "τακνες", false);
    OBJ_TIMELOAD = Proto->AddItem(TYPE_INT, "σοϊδαξ", false);
    OBJ_QUALITY = Proto->AddItem(TYPE_INT, "λαώεστχο", false);
    OBJ_LIMIT = Proto->AddItem(TYPE_INT, "πςεδεμ", true);
    OBJ_DURAB = Proto->AddItem(TYPE_INT, "πςοώξοστψ", true);
    OBJ_CURRENT_DURAB = Proto->AddItem(TYPE_INT, "τελυύαρ_πςοώξοστψ", false);
    OBJ_MAGIC = Proto->AddItem(TYPE_STRUCT, "ναηιρ", false);
    CItemProto *magic = Proto->GetItem(OBJ_MAGIC);
    OBJ_MAGIC_SPELL = magic->AddItem(TYPE_LIST, "ϊαλμιξαξιε", true,
                                     ParserConst.GetList(LIST_SPELLS));
    OBJ_MAGIC_LEVEL = magic->AddItem(TYPE_INT, "υςοχεξψ");
    OBJ_MAGIC_PERCENT = magic->AddItem(TYPE_INT, "ϋαξσ");
    OBJ_MAGIC_MESS_CHAR = magic->AddItem(TYPE_STRING, "ιηςολυ", false);
    OBJ_MAGIC_MESS_VICT = magic->AddItem(TYPE_STRING, "φεςτχε", false);
    OBJ_MAGIC_MESS_ROOM = magic->AddItem(TYPE_STRING, "οσταμψξων", false);
    OBJ_INTERCEPTION = Proto->AddItem(TYPE_STRUCT, "πεςεθχατ", false);
    CItemProto *intercept = Proto->GetItem(OBJ_INTERCEPTION);
    OBJ_INT_COMMAND = intercept->AddItem(TYPE_LIST, "λοναξδα", true,
                                         ParserConst.GetList(LIST_COMMANDS));
    OBJ_INT_STOP = intercept->AddItem(TYPE_INT, "στοπ", false);
    OBJ_INT_EXPR = intercept->AddItem(TYPE_EXPR, "υσμοχιε", false);
    OBJ_INT_SCRIPT = intercept->AddItem(TYPE_INT, "δεκστχιε", false);
    OBJ_INT_MESSPLAYER = intercept->AddItem(TYPE_STRING, "ιηςολ", false);
    OBJ_INT_MESSVICTIM = intercept->AddItem(TYPE_STRING, "φεςτχα", false);
    OBJ_INT_MESSOTHER  = intercept->AddItem(TYPE_STRING, "οσταμψξων", false);
    OBJ_INT_MESSROOM   = intercept->AddItem(TYPE_STRING, "λονξατα", false);
    OBJ_SHANCE = Proto->AddItem(TYPE_INT, "ϋαξσ", false);
    OBJ_SCRIPT = Proto->AddItem(TYPE_SCRIPT, "σλςιπτ", false);
    OBJ_VAL0 = Proto->AddItem(TYPE_INT, "ϊξαώ0", false);
    OBJ_VAL1 = Proto->AddItem(TYPE_INT, "ϊξαώ1", false);
    OBJ_VAL2 = Proto->AddItem(TYPE_INT, "ϊξαώ2", false);
    OBJ_VAL3 = Proto->AddItem(TYPE_INT, "ϊξαώ3", false);
    OBJ_LIGHT = Proto->AddItem(TYPE_INT, "σχετ", false);
    OBJ_LEVEL = Proto->AddItem(TYPE_INT, "υςοχεξψ", false);
    OBJ_SPELL1 = Proto->AddItem(TYPE_LIST, "ϊαλμ1", false,
                                ParserConst.GetList(LIST_SPELLS));
    OBJ_SPELL2 = Proto->AddItem(TYPE_LIST, "ϊαλμ2", false,
                                ParserConst.GetList(LIST_SPELLS));
    OBJ_SPELL3 = Proto->AddItem(TYPE_LIST, "ϊαλμ3", false,
                                ParserConst.GetList(LIST_SPELLS));
    OBJ_ENCHANT    = Proto->AddItem(TYPE_LIST, "ζοςνυμα", false,
                                    ParserConst.GetList(LIST_ENCHANT));
    OBJ_SLOT_MAX = Proto->AddItem(TYPE_INT, "νϊαςρδω", false);
    OBJ_SLOT_CUR = Proto->AddItem(TYPE_INT, "ϊαςρδω", false);
    OBJ_SPELL = Proto->AddItem(TYPE_LIST, "ϊαλμιξαξιε", false,
                               ParserConst.GetList(LIST_SPELLS));

    OBJ_AC  = Proto->AddItem(TYPE_INT, "βςοξρ", false);
    OBJ_ARM0 = Proto->AddItem(TYPE_INT, "ςεφυύεε", false);
    OBJ_ARM1 = Proto->AddItem(TYPE_INT, "λομΰύεε", false);
    OBJ_ARM2 = Proto->AddItem(TYPE_INT, "υδαςξοε", false);
    OBJ_DAMAGE = Proto->AddItem(TYPE_RANDOM, "σχςεδ", false);
    OBJ_SKILL = Proto->AddItem(TYPE_LIST, "οςυφιε", false,
                               ParserConst.GetList(LIST_WEAPON_SKILLS));
    OBJ_HIT = Proto->AddItem(TYPE_LIST, "τχςεδ", false,
                             ParserConst.GetList(LIST_WEAPON_TYPE));

    OBJ_ADD_HIT = Proto->AddItem(TYPE_STRUCT, "δυδας", false);
    CItemProto *ahit = Proto->GetItem(OBJ_ADD_HIT);
    OBJ_AHIT_TYPE = ahit->AddItem(TYPE_LIST, "τιπ", true, ParserConst.GetList(LIST_DAMAGE));
    OBJ_AHIT_DAMAGE = ahit->AddItem(TYPE_STRING, "σχςεδ", true);

    OBJ_SIZE = Proto->AddItem(TYPE_INT, "χνεστινοστψ", false);
    OBJ_BAG_PROPERTIES = Proto->AddItem(TYPE_VECTOR, "λσχοκστχα", false,
                                        ParserConst.GetVector(VEC_EXIT_PROP));
    OBJ_BAG_NOTFIT = Proto->AddItem(TYPE_STRLIST, "ξεπονεύαετσρ", false,
                                    ParserConst.GetList(LIST_OBJ_TYPES));
    OBJ_KEY = Proto->AddItem(TYPE_INT, "λμΰώ", false);
    OBJ_BAG_MAGIC = Proto->AddItem(TYPE_INT, "υδοβστχο", false);
    OBJ_OLIST = Proto->AddItem(TYPE_STRLIST, "σοδεςφινοε", false);
    OBJ_OGOLD = Proto->AddItem(TYPE_INT, "δεξψηι", false);
    OBJ_VALUE = Proto->AddItem(TYPE_INT, "ενλοστψ", false);
    OBJ_CUR_VALUE = Proto->AddItem(TYPE_INT, "τενλοστψ", false);
    OBJ_LIQ = Proto->AddItem(TYPE_LIST, "φιδλοστψ", false,
                             ParserConst.GetList(LIST_LIQUID_TYPE));
    OBJ_POISON = Proto->AddItem(TYPE_INT, "ρδ", false);
    OBJ_FOOD = Proto->AddItem(TYPE_INT, "ξασωύεξιε", false);
    OBJ_GOLD = Proto->AddItem(TYPE_INT, "συννα", false);
    OBJ_BRIGHT = Proto->AddItem(TYPE_INT, "ρςλοστψ", false);
    OBJ_TR_PROPERTIES = Proto->AddItem(TYPE_VECTOR, "λςσχοκστχα", false,
                                       ParserConst.GetVector(VEC_TRANS_PROP));

    OBJ_TRAP = Proto->AddItem(TYPE_STRUCT, "μοχυϋλα", false);
    CItemProto *etrap = Proto->GetItem(OBJ_TRAP);
    OBJ_TRAP_COMMAND = etrap->AddItem(TYPE_LIST, "λοναξδα", false,
                                      ParserConst.GetList(LIST_COMMANDS));
    OBJ_TRAP_CHANCE = etrap->AddItem(TYPE_INT, "ϋαξσ");
    OBJ_TRAP_TYPEDAMAGE = etrap->AddItem(TYPE_LIST, "τχςεδ", true,
                                         ParserConst.GetList(LIST_DAMAGE));
    OBJ_TRAP_FORCEDAMAGE = etrap->AddItem(TYPE_RANDOM, "σχςεδ");
    OBJ_TRAP_SAVE = etrap->AddItem(TYPE_INT, "πςεϊιστ", false);
    OBJ_TRAP_MESS_CHAR = etrap->AddItem(TYPE_STRING, "ιηςολ", false);
    OBJ_TRAP_MESS_ROOM = etrap->AddItem(TYPE_STRING, "οσταμψξων", false);
    OBJ_TRAP_MESS_SCHAR = etrap->AddItem(TYPE_STRING, "ςιηςολ", false);
    OBJ_TRAP_MESS_SROOM = etrap->AddItem(TYPE_STRING, "ςοσταμψξων", false);
    OBJ_TRAP_MESS_KCHAR = etrap->AddItem(TYPE_STRING, "υβιτ_ιηςολ", false);
    OBJ_TRAP_MESS_KROOM = etrap->AddItem(TYPE_STRING, "υβιτ_μολαγιρ", false);

    OBJ_XSAVE = Proto->AddItem(TYPE_STRUCT, "ϊαπισψ", false);
    CItemProto *xsave = Proto->GetItem(OBJ_XSAVE);
    OBJ_XSAVE_POS = xsave->AddItem(TYPE_INT, "πομοφεξιε", false);
    OBJ_XSAVE_EQ = xsave->AddItem(TYPE_INT, "όλιπιςοχλα", false);

    OBJ_POWER    = Proto->AddItem(TYPE_INT, "υσιμεξιε", false);
    OBJ_TEMPLATE = Proto->AddItem(TYPE_INT, "δϋαβμοξ", false);

    OBJ_P0  = Proto->AddItem(TYPE_INT, "πςεφυύεε", false);
    OBJ_P1  = Proto->AddItem(TYPE_INT, "πλομΰύεε", false);
    OBJ_P2  = Proto->AddItem(TYPE_INT, "πυδαςξοε", false);
    OBJ_TOOL = Proto->AddItem(TYPE_LIST, "ιξστςυνεξτ", false, ParserConst.GetList(LIST_TOOLS));

    OBJ_COMMAND  = Proto->AddItem(TYPE_STRUCT, "σπεγλοναξδα", false);
    CItemProto *command = Proto->GetItem(OBJ_COMMAND);
    OBJ_COMMAND_NO = command->AddItem(TYPE_LIST, "λοναξδα", true, ParserConst.GetList(LIST_COMMANDS));
    OBJ_COMMAND_ARG = command->AddItem(TYPE_STRUCT, "αςηυνεξτ", false);
    OBJ_COMMAND_TOOL = command->AddItem(TYPE_LIST, "ιξστςυνεξτ", false, ParserConst.GetList(LIST_TOOLS));
    OBJ_COMMAND_EXPR = command->AddItem(TYPE_EXPR, "υσμοχιε", false);
    OBJ_COMMAND_ECHAR = command->AddItem(TYPE_STRING, "οϋιβλα", false);
    OBJ_COMMAND_EROOM = command->AddItem(TYPE_STRING, "οϋιβλα_μολαγιρ", false);

    OBJ_COMMAND_ACTIVE = command->AddItem(TYPE_STRING, "αλτιχαγιρ", false);
    OBJ_COMMAND_ACTIVE_ROOM = command->AddItem(TYPE_STRING, "αλτιχαγιρ_μολαγιρ", false);

    CItemProto *comarg = command->GetItem(OBJ_COMMAND_ARG);
    OBJ_COMMAND_ARG_OBJ = comarg->AddItem(TYPE_INT, "πςεδνετ", false);
    OBJ_COMMAND_ARG_ARG = comarg->AddItem(TYPE_STRING, "στςολα", false);
    OBJ_COMMAND_ARG_MOB = comarg->AddItem(TYPE_INT, "νοξστς", false);
    OBJ_COMMAND_ARG_ERR = comarg->AddItem(TYPE_STRING, "οϋιβλα", false);
    OBJ_COMMAND_SCRIPT = command->AddItem(TYPE_INT, "σλςιπτ", false);
    OBJ_COMMAND_XSCRIPT = command->AddItem(TYPE_INT, "δεκστχιε", false);
    OBJ_COMMAND_EXTRACT = command->AddItem(TYPE_INT, "ςασσωπαξιε", false);
    OBJ_COMMAND_LOAD = command->AddItem(TYPE_STRUCT, "ϊαηςυϊλα", false);
    CItemProto *cload = command->GetItem(OBJ_COMMAND_LOAD);
    OBJ_COMMAND_LOAD_ROOM = cload->AddItem(TYPE_INT, "χμολαγιΰ", false);
    OBJ_COMMAND_LOAD_CHAR = cload->AddItem(TYPE_INT, "χιξχεξταςψ", false);
    OBJ_COMMAND_MESS_CHAR = cload->AddItem(TYPE_STRING, "ιηςολ", false);
    OBJ_COMMAND_MESS_ROOM = cload->AddItem(TYPE_STRING, "μολαγιρ", false);

    OBJ_SPECHIT  = Proto->AddItem(TYPE_STRUCT, "σπεγαταλα", false);
    CItemProto *spech = Proto->GetItem(OBJ_SPECHIT);
    OBJ_SPECHIT_SHANCE = spech->AddItem(TYPE_INT, "ϋαξσ", true);
    OBJ_SPECHIT_XRACE = spech->AddItem(TYPE_STRLIST, "ϊςασα", false, ParserConst.GetList(LIST_RACE));
    OBJ_SPECHIT_ZRACE = spech->AddItem(TYPE_STRLIST, "ςςασα", false, ParserConst.GetList(LIST_RACE));
    OBJ_SPECHIT_XTYPE = spech->AddItem(TYPE_STRLIST, "ϊτιπ", false, ParserConst.GetList(LIST_MOB_TYPE));
    OBJ_SPECHIT_ZTYPE = spech->AddItem(TYPE_STRLIST, "ςτιπ", false, ParserConst.GetList(LIST_MOB_TYPE));
    OBJ_SPECHIT_CHAR = spech->AddItem(TYPE_STRING, "ιηςολυ", true);
    OBJ_SPECHIT_VICTIM = spech->AddItem(TYPE_STRING, "φεςτχε", true);
    OBJ_SPECHIT_ROOM = spech->AddItem(TYPE_STRING, "μολαγιρ", true);
    OBJ_SPECHIT_KOEF = spech->AddItem(TYPE_FLOAT, "υχεμιώεξιε", false);
    OBJ_SPECHIT_HTYPE = spech->AddItem(TYPE_LIST, "υτιπ", false, ParserConst.GetList(LIST_DAMAGE));
    OBJ_SPECHIT_HDAMAGE = spech->AddItem(TYPE_STRING, "υσχςεδ", false);
    OBJ_SPECHIT_SPELL = spech->AddItem(TYPE_LIST, "ϊαλμιξαξιε", false, ParserConst.GetList(LIST_SPELLS));
    OBJ_SPECHIT_SLEVEL = spech->AddItem(TYPE_INT, "υςοχεξψ", false);
    OBJ_SPECHIT_TARGET = spech->AddItem(TYPE_LIST, "γεμι", false, ParserConst.GetList(LIST_TARGET));

    OBJ_QUEST = Proto->AddItem(TYPE_SCRIPT, "ϊαδαξιε", false);

    OBJ_MISSILE = Proto->AddItem(TYPE_STRUCT, "σξαςρδ", false);
    CItemProto *missl = Proto->GetItem(OBJ_MISSILE);
    OBJ_MISS_TYPE = missl->AddItem(TYPE_LIST, "τιπ", false, ParserConst.GetList(LIST_MISSILE));
    OBJ_MISS_COUNT = missl->AddItem(TYPE_INT, "νλομιώεστχο", true);
    OBJ_MISS_TCOUNT = missl->AddItem(TYPE_INT, "τλομιώεστχο", false);
    OBJ_MISS_TEMPL = missl->AddItem(TYPE_INT, "σϋαβμοξ", false);
    OBJ_MADD_HIT = missl->AddItem(TYPE_STRUCT, "δυδας", false);
    CItemProto *mah = missl->GetItem(OBJ_MADD_HIT);
    OBJ_MAHIT_TYPE = mah->AddItem(TYPE_LIST, "τιπ", true, ParserConst.GetList(LIST_DAMAGE));
    OBJ_MAHIT_DAMAGE = mah->AddItem(TYPE_STRING, "σχςεδ", true);

    return CheckInit();
}

CObj * Obj;

