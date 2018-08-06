#ifndef XOBJ_H
#define XOBJ_H
#include "parser.h"
#include "parser_const.h"

/*****************************************************************************/
/* οΠÒΕΔΕΜΕΞΙΡ ΔΜΡ ΪΑΗÒΥΪΛΙ ΖΑΚΜΟΧ ΟΠΙΣΑΞΙΡ ΠÒΕΔΝΕΤΟΧ (*.obx)                */
/*****************************************************************************/

class CObj : public CParser {
    public:
        CObj();
        ~CObj();
        bool Initialization(void);
};

// οΠÒΕΔΕΜΕΞΙΕ ΟΣΞΟΧΞΩΘ ΠΑÒΑΝΕΤÒΟΝ
extern int OBJ_NUMBER;  //πςεδνετ
extern int OBJ_GUID; //ημοβιδ
extern int OBJ_ALIAS;  //σιξοξινω
extern int OBJ_NAME;  //ινρ
extern int OBJ_LINE;  //στςολα
extern int OBJ_DESCRIPTION; //οπισαξιε
extern int OBJ_ADDITION; //δοπομξιτεμψξο
extern int OBJ_PROPERTIES; //πσχοκστχα
extern int OBJ_SEX;  //ςοδ
extern int OBJ_MATERIAL; //νατεςιαμ
extern int OBJ_WEIGHT;  //χεσ
extern int OBJ_COST;  //γεξα
extern int OBJ_TYPE;  //τιπ
extern int OBJ_WEAR;  //ισπομψϊοχαξιε
extern int OBJ_ANTI;  //ξευδοβστχο
extern int OBJ_NO;  //ϊαπςετ
extern int OBJ_AFFECTS; //πόζζελτω
extern int OBJ_APPLY;  //χμιρξιε
extern int OBJ_TIMER;  //τακνες
extern int OBJ_TIMELOAD; //σοϊδαξ
extern int OBJ_QUALITY; //λαώεστχο
extern int OBJ_LIMIT;  //πςεδεμ
extern int OBJ_DURAB;  //πςοώξοστψ
extern int OBJ_CURRENT_DURAB;  //τελ.πςοώξοστψ
extern int OBJ_MAGIC;  //ναηιρ
extern int OBJ_INTERCEPTION; //πεςεθχατ
extern int OBJ_SCRIPT;  //σλςιπτω
extern int OBJ_SHANCE;  //ϋαξσ
extern int OBJ_VAL0;  //ϊξαώ0
extern int OBJ_VAL1;  //ϊξαώ1
extern int OBJ_VAL2;  //ϊξαώ2
extern int OBJ_VAL3;  //ϊξαώ3
extern int OBJ_LIGHT;  //σχετ
extern int OBJ_LEVEL;  //υςοχεξψ
extern int OBJ_SPELL1;  //ϊαλμ1
extern int OBJ_SPELL2;  //ϊαλμ2
extern int OBJ_SPELL3;  //ϊαλμ3
extern int OBJ_ENCHANT;  //ϊαώαςοχαξιε
extern int OBJ_SLOT_MAX; //νϊαςρδω
extern int OBJ_SLOT_CUR; //ϊαςρδω
extern int OBJ_SPELL;  //ϊαλμιξαξιε
extern int OBJ_AC;  //βςοξρ
extern int OBJ_ARM0;  //ςεφυύεε
extern int OBJ_ARM1;  //λομΰύεε
extern int OBJ_ARM2;  //υδαςξοε
extern int OBJ_DAMAGE;  //Cχςεδ v1+v2
extern int OBJ_SKILL;  //οςυφιε
extern int OBJ_HIT;  //νχςεδ v3
extern int OBJ_SIZE;  //χνεστινοστψ v0
extern int OBJ_BAG_PROPERTIES; //λσχοκστχα v1
extern int OBJ_BAG_NOTFIT; //ξεπονεύαετσρ
extern int OBJ_KEY;  //λμΰώ   v2
extern int OBJ_BAG_MAGIC; //υδοβστχο  v3
extern int OBJ_OLIST;  //σοδεςφινοε
extern int OBJ_OGOLD;  //δεξψηι
extern int OBJ_VALUE;  //ενλοστψ v0
extern int OBJ_CUR_VALUE; //τενλοστψ v1
extern int OBJ_LIQ;  //φιδλοστψ v2
extern int OBJ_POISON;  //ρδ v3
extern int OBJ_FOOD;  //ξασωύεξιε v0
extern int OBJ_GOLD;  //συννα v0
extern int OBJ_BRIGHT;  //ρςλοστψ
extern int OBJ_TR_PROPERTIES; //λςσχοκστχα
extern int OBJ_TRAP;  //μοχυϋλα (extern int)
extern int OBJ_XSAVE;          //σοθςαξεξιε
extern int OBJ_POWER;  //υσιμεξιε
extern int OBJ_TEMPLATE; //δϋαβμοξ (ΞΟΝΕÒ ΫΑΒΜΟΞΑ)
extern int OBJ_ARM_CLASS; //λμασσ
extern int OBJ_P0;  //πςεφυύεε
extern int OBJ_P1;  //πλομΰύεε
extern int OBJ_P2;  //πυδαςξοε
extern int OBJ_COMMAND;  //σπεγλοναξδα
extern int OBJ_TOOL;  //ιξστςυνεξτ
extern int OBJ_QUEST;  //λχεστω

//σΠΕΓΛΟΝΑΞΔΩ
extern int OBJ_COMMAND_NO; //σπεγλοναξδα.λοναξδα
extern int OBJ_COMMAND_ARG; //σπεγλοναξδα.αςηυνεξτ
extern int OBJ_COMMAND_TOOL; //σπεγλοναξδα.ιξστςυνεξτ
extern int OBJ_COMMAND_EXPR; //σπεγλοναξδα.υσμοχιε
extern int OBJ_COMMAND_ECHAR; //σπεγλοναξδα.οϋιβλα_ιηςολυ
extern int OBJ_COMMAND_EROOM; //σπεγλοναξδα.οϋιβλα_μολαγιρ
extern int OBJ_COMMAND_ACTIVE;
extern int OBJ_COMMAND_ARG_OBJ; //σπεγλοναξδα.αςηυνεξτ.πςεδνετ
extern int OBJ_COMMAND_ARG_ARG; //σπεγλοναξδα.αςηυνεξτ.στςολα
extern int OBJ_COMMAND_ARG_MOB; //σπεγλοναξδα.αςηυνεξτ.νοξστς
extern int OBJ_COMMAND_ARG_ERR; //σπεγλοναξδα.αςηυνεξτ.οϋιβλα
extern int OBJ_COMMAND_SCRIPT; //σπεγλοναξδα.σλςιπτ
extern int OBJ_COMMAND_XSCRIPT; //σπεγλοναξδα.δεκστχιε
extern int OBJ_COMMAND_EXTRACT; //σπεγλοναξδα.ςασσωπαξιε
extern int OBJ_COMMAND_LOAD;     //σπεγλοναξδα.ϊαηςυϊλα
extern int OBJ_COMMAND_LOAD_ROOM; //σπεγλοναξδα.ϊαηςυϊλα.χμολαγιΰ
extern int OBJ_COMMAND_LOAD_CHAR; //σπεγλοναξδα.ϊαηςυϊλα.χιξχεξταςψ
extern int OBJ_COMMAND_ACTIVE_ROOM;
extern int OBJ_COMMAND_MESS_ROOM;
extern int OBJ_COMMAND_MESS_CHAR;


//σΟΘÒΑΞΕΞΙΕ
extern int OBJ_XSAVE_EQ; //σοθςαξεξιε.όλιπιςοχλα
extern int OBJ_XSAVE_POS; //σοθςαξεξιε.πομοφεξιε

// δΟΠΟΜΞΙΤΕΜΨΞΟ
extern int OBJ_ADD_KEY; //δοπομξιτεμψξο.λμΰώ
extern int OBJ_ADD_TEXT; //δοπομξιτεμψξο.τελστ

// νΑΤΕÒΙΑΜ
extern int OBJ_MAT_TYPE; //νατεςιαμ.τιπ
extern int OBJ_MAT_VAL; //λομιώεστχο
extern int OBJ_MAT_MAIN; //οσξοχξοε

extern int OBJ_INT_COMMAND; // πεςεθχατ.λοναξδα (LIST)
extern int OBJ_INT_STOP; // πεςεθχατ.στοπ (extern int)
extern int OBJ_INT_EXPR; // πεςεθχατ.υσμοχιε (expr)
extern int OBJ_INT_SCRIPT; // πεςεθχατ.σλςιπτ (extern int)
extern int OBJ_INT_MESSPLAYER; // πεςεθχατ.ιηςολ (STRING)
extern int OBJ_INT_MESSVICTIM; // πεςεθχατ.φεςτχα (STRING)
extern int OBJ_INT_MESSOTHER; // πεςεθχατ.οσταμψξων (STRING)
extern int OBJ_INT_MESSROOM; // πεςεθχατ.λονξατα (STRING)

//νΑΗΙΡ
extern int OBJ_MAGIC_SPELL; //ναηιρ.ϊαλμιξαξιε (LIST)
extern int OBJ_MAGIC_LEVEL; //ναηιρ.υςοχεξψ  (extern int)
extern int OBJ_MAGIC_PERCENT; //ναηιρ.ϋαξσ  (extern int)
extern int OBJ_MAGIC_MESS_CHAR; //ναηιρ.ιηςολυ (STRING)
extern int OBJ_MAGIC_MESS_VICT; //ναηιρ.φεςτχε (STRING)
extern int OBJ_MAGIC_MESS_ROOM; //ναηιρ.οσταμψξων (STRING)

//μΟΧΥΫΛΑ
extern int OBJ_TRAP_COMMAND;  //μοχυϋλα.λοναξδα
extern int OBJ_TRAP_CHANCE;  //μοχυϋλα.ϋαξσ (extern int)
extern int OBJ_TRAP_TYPEDAMAGE; //μοχυϋλα.τχςεδ  (LIST)
extern int OBJ_TRAP_FORCEDAMAGE; //μοχυϋλα.σχςεδ  (RANDOM)
extern int OBJ_TRAP_SAVE;  //μοχυϋλα.πςεϊιστ
extern int OBJ_TRAP_MESS_CHAR;  //μοχυϋλα.ιηςολ
extern int OBJ_TRAP_MESS_ROOM;  //μοχυϋλα.λονξατα
extern int OBJ_TRAP_MESS_SCHAR; //μοχυϋλα.ςιηςολ
extern int OBJ_TRAP_MESS_SROOM; //μοχυϋλα.ςλονξατα
extern int OBJ_TRAP_MESS_KCHAR; //μοχυϋλα.υβιτ_ιηςολ
extern int OBJ_TRAP_MESS_KROOM; //μοχυϋλα.υβιτ_μολαγιρ

//σοθςαξεξιε
extern int OBJ_AFFECT;
extern int AFFECT_TYPE;
extern int AFFECT_LOC;
extern int AFFECT_MOD;
extern int AFFECT_VEC;
extern int AFFECT_EXT;
extern int AFFECT_DUR;
extern int AFFECT_NFL;
extern int AFFECT_AFL;
extern int OBJ_OWNER;

//οςυφιε

extern int OBJ_WEAP_TEMPL; //οϋαβμοξ

extern int OBJ_ADD_HIT;  //δυδας
extern int OBJ_AHIT_TYPE; //δυδας.τιπ
extern int OBJ_AHIT_DAMAGE; //δυδας.σχςεδ

extern int OBJ_SPECHIT;   //σπεγαταλα
extern int OBJ_SPECHIT_SHANCE;  //σπεγαταλα.ϋαξσ
extern int OBJ_SPECHIT_XRACE;  //σπεγαταλα.ϊςασα
extern int OBJ_SPECHIT_ZRACE;  //σπεγαταλα.ςςασα
extern int OBJ_SPECHIT_XTYPE;  //σπεγαταλα.ϊτιπ
extern int OBJ_SPECHIT_ZTYPE;  //σπεγαταλα.ςτιπ
extern int OBJ_SPECHIT_CHAR;  //σπεγαταλα.ιηςολυ
extern int OBJ_SPECHIT_VICTIM;  //σπεγαταλα.φεςτχε
extern int OBJ_SPECHIT_ROOM;  //σπεγαταλα.μολαγιρ
extern int OBJ_SPECHIT_KOEF;  //σπεγαταλα.υχεμιώεξιε
extern int OBJ_SPECHIT_HTYPE;  //σπεγαταλα.υτιπ
extern int OBJ_SPECHIT_HDAMAGE;  //σπεγαταλα.υσχςεδ
extern int OBJ_SPECHIT_SPELL;  //σπεγαταλα.ϊαλμιξαξιε
extern int OBJ_SPECHIT_SLEVEL;  //σπεγαταλα.υςοχεξψ
extern int OBJ_SPECHIT_TARGET;  //σπεγαταλα.γεμι

extern int OBJ_MISSILE;  //σξαςρδ
extern int OBJ_MISS_TYPE; //σξρςρδ.τιπ
extern int OBJ_MISS_COUNT; //σξρςρδ.νλομιώεστχο
extern int OBJ_MISS_TCOUNT; //σξρςρδ.τλομιώεστχο
extern int OBJ_MISS_TEMPL; //σξρςρδ.σϋαβμοξ
extern int OBJ_MADD_HIT; //σξρςρδ.δυδας
extern int OBJ_MAHIT_TYPE; //σξρςρδ.δυδας.τιπ
extern int OBJ_MAHIT_DAMAGE; //σξρςρδ.δυδας.σχςεδ

extern CObj *Obj;

#endif
