#ifndef XMOB_H
#define XMOB_H
#include "parser.h"
#include "parser_const.h"

/*****************************************************************************/
/* οΠÒΕΔΕΜΕΞΙΡ ΔΜΡ ΪΑΗÒΥΪΛΙ ΖΑΚΜΟΧ ΟΠΙΣΑΞΙΡ ΝΟΞΣΤÒΟΧ (*.mox)                 */
/*****************************************************************************/

class CMob : public CParser {
    public:
        CMob();
        ~CMob();
        bool Initialization(void);
};

// οΠÒΕΔΕΜΕΞΙΕ ΟΣΞΟΧΞΩΘ ΠΑÒΑΝΕΤÒΟΝ
extern int MOB_NUMBER;  //νοξστς
extern int MOB_ALIAS;  //σιξοξινω
extern int MOB_NAME;  //ινρ
extern int MOB_LINE;  //στςολα
extern int MOB_DESCRIPTION; //οπισαξιε
extern int MOB_ADDITION; //δοπομξιτεμψξο
extern int MOB_PROPERTIES; //νσχοκστχα
extern int MOB_ADDONS;  //νδοπομξεξιρ
extern int MOB_AFFECTS; //νόζζελτω
extern int MOB_SEX;  //πομ
extern int MOB_LEVEL;  //υςοχεξψ
extern int MOB_CLASS;  //πςοζεσσιρ
extern int MOB_RACE;  //ςασα
extern int MOB_TYPE;  //τιπ
extern int MOB_ALIGN;  //ξαλμοξοστι
extern int MOB_LAWS;  //ϊαλοξξοστψ
extern int MOB_EXP;  //οπωτ
extern int MOB_LIMIT;  //πςεδεμ
extern int MOB_HITPOINTS; //φιϊξψ
extern int MOB_AC;  //βςοξρ
extern int MOB_HITROLL; //αταλα
extern int MOB_HIT1;  //υδας1
extern int MOB_DAMAGE1; //χςεδ1
extern int MOB_COUNT1;  //αταλι1
extern int MOB_HIT2;  //υδας2
extern int MOB_DAMAGE2; //χςεδ2
extern int MOB_COUNT2;  //αταλι2
extern int MOB_STR;  //σιμα
extern int MOB_CON;  //τεμοσμοφεξιε
extern int MOB_DEX;  //μοχλοστψ
extern int MOB_INT;  //ςαϊυν
extern int MOB_WIS;  //νυδςοστψ
extern int MOB_CHA;  //οβαρξιε
extern int MOB_SIZE;  //ςαϊνες
extern int MOB_HEIGHT;  //ςοστ
extern int MOB_WEIGHT;  //χεσ
extern int MOB_POSITION; //πομοφεξιε
extern int MOB_MOVED;  //πεςενεύεξιε
extern int MOB_MOVESTR; //πεςεδχιφεξιε
extern int MOB_GOLD;  //δεξψηι
extern int MOB_WIMP;  //τςυσοστψ
extern int MOB_SKILL;  //υνεξιε
extern int MOB_EQ;  //όλιπιςοχλα
extern int MOB_INV;  //ιξχεξταςψ
extern int MOB_DEATH;  //ποσνεςτξο
extern int MOB_TATOO;  //τατυιςοχλα
extern int MOB_SAVE;  //ϊαύιτα
extern int MOB_FOLLOW;  //σμεδυετ
extern int MOB_HELPED;  //πονούξιλι-ηςυππα
extern int MOB_DEST;  //πυτψ
extern int MOB_SCRIPT;  //σλςιπτ
extern int MOB_SHOP;  //ναηαϊιξ
extern int MOB_SPECIAL; //σπεγυδας
extern int MOB_LACKY;  //υδαώα
extern int MOB_SPEED;  //σλοςοστψ
extern int MOB_LIKEWORK; //ςαβοτα
extern int MOB_GOD;  //βοη
extern int MOB_FRACTION; //ζςαλγιρ
extern int MOB_RANG;           //ςαξη
extern int MOB_RANK;           //ςαξλ
extern int MOB_HORSE;  //μοϋαδψ
extern int MOB_SDEATH;  //σνεςτψ
extern int MOB_ALARM;  //οποχεύεξιε
extern int MOB_POWER;  //υσιμεξιε
extern int MOB_BODY;  //τεμο
extern int MOB_CLASSES; //πςοζεσσιρ
extern int MOB_AGE;  //χοϊςατ

// δΟΠΟΜΞΙΤΕΜΨΞΩΕ ΠΑÒΑΝΕΤÒΩ
extern int MOB_CLASS_TYPE; //πςοζεσσιρ.λμασσ
extern int MOB_CLASS_LEVEL; //πςοζεσσιρ.υςοχεξψ

extern int MOB_BODY_POSITION; //τεμο.ποϊιγιρ
extern int MOB_BODY_SNAME; //τεμο.σπςξαϊχαξιε
extern int MOB_BODY_NAME; //τεμο.ξαϊχαξιε
extern int MOB_BODY_CHANCE; //τεμο.σμοφξοστψ
extern int MOB_BODY_KDAM; //τεμο,πςοώξοστψ

extern int MOB_SDT_CORPSE; //σνεςτψ.τςυπ
extern int MOB_SDT_SCRIPT; //σνεςτψ.δεκστχιε
extern int MOB_SDT_DCHAR; //σνεςτψ.σιηςολ
extern int MOB_SDT_DROOM;      //σνεςτψ.σμολαγιρ
extern int MOB_SDT_DAMAGE; //σνεςτψ.τχςεδ
extern int MOB_SDT_HIT; //σνεςτψ.σχςεδ
extern int MOB_SDT_TARGET; //σνεςτψ.γεμψ
extern int MOB_SDT_CHAR; //σνεςτψ.ιηςολ
extern int MOB_SDT_ROOM; //σνεςτψ.οσταμψξων

extern int MOB_ADD_KEY; //δοπομξιτεμψξο.λμΰώ
extern int MOB_ADD_TEXT; //δοπομξιτεμψξο.τελστ

extern int MOB_SPEC_TYPE; //σπεγυδας.τιπ
extern int MOB_SPEC_POS; //σπεγυδας.πομοφεξιε
extern int MOB_SPEC_PERC; //σπεγυδας.ϋαξσ
extern int MOB_SPEC_HIT; //σπεγυδας.ςαϊςυϋεξιε
extern int MOB_SPEC_SPELL; //σπεγυδας.ϊαλμιξαξιε
extern int MOB_SPEC_POWER; //σπεγυδας.νούξοστψ
extern int MOB_SPEC_DAMAGE; //σπεγυδας.χςεδ
extern int MOB_SPEC_VICTIM; //σπεγυδας.ιηςολυ
extern int MOB_SPEC_ROOM; //σπεγυδας.οσταμψξων
extern int MOB_SPEC_SAVE; //σπεγυδας.σοθςαξεξιε
extern int MOB_SPEC_PROP; //σξεγυδας.σχοκστχα

extern int MOB_SHOP_SELL; //ναηαϊιξ.πςοδαφα (int)
extern int MOB_SHOP_BUY; //ναηαϊιξ.πολυπλα (int)
extern int MOB_SHOP_REPAIR; //ναηαϊιξ.ςενοξτ (int)
extern int MOB_SHOP_QUALITY; //ναηαϊιξ.ναστεςστχο (int)
extern int MOB_SHOP_TYPE; //ναηαϊιξ.τοχας (list)
extern int MOB_SHOP_LIST; //ναηαϊιξ.νεξΰ (list)
extern int MOB_SHOP_ANTI; //ναηαϊιξ.ϊαπςετω (bitvector)

extern int MOB_ALARM_HELPER; //οποχεύεξιε.πονοϋξιλι
extern int MOB_ALARM_CHAR;   //οποχεύεξιε.ιηςολ
extern int MOB_ALARM_ROOM;   //οποχεύεξιε.μολαγιρ
extern int MOB_ALARM_LEVEL;  //οποχεύεξιε.υςοχεξψ

extern int MOB_PERIOD;  //πεςιοδιώξοστψ
extern int MOB_PRD_START; //ξαώαμο
extern int MOB_PRD_STOP; //λοξεγ
extern int MOB_PRD_TIME; //πεςιοδ
extern int MOB_PRD_TARGET; //γεμψ
extern int MOB_PRD_CHAR; //ιηςολυ
extern int MOB_PRD_ROOM; //λονξατε

extern int MOB_WELCOME; //πςιχεστχιε
extern int MOB_GOODBYE; //πςούαξιε
extern int MOB_QUEST;  //ϊαδαξιε
extern int MOB_QNUMBER; //ϊαδαξιε.ξονες
extern int MOB_QNAME;  //ϊαδαξιε.ξαϊχαξιε
extern int MOB_QEXPR;  //ϊαδαξιε.υσμοχιε
extern int MOB_QPRE;  //ϊαδαξιε.λςατλοε
extern int MOB_QTEXT;  //ϊαδαξιε.τελστ
extern int MOB_QCOMPLITE; //ϊαδαξιε.χωπομξεξιε
extern int MOB_QMULTY;  //ϊαδαξιε.νξοηοςαϊοχοε
extern int MOB_QREQUEST; //ϊαδαξιε.υσμοχιρ
extern int MOB_QR_VAR;  //ϊαδαξιε.υσμοχιρ.πεςενεξξαρ
extern int MOB_QRV_TITLE; //ϊαδαξιε.υσμοχξιρ.πεςενεξξαρ.οτοβςαφεξιε
extern int MOB_QRV_NAME; //ϊαδαξιε.υσμοχξιρ.πεςενεξξαρ.ξαϊχαξιε
extern int MOB_QRV_VALUE; //ϊαδαξιε.υσμοχξιρ.πεςενεξξαρ.ϊξαώεξιε
extern int MOB_QR_MOBILES;     //ϊαδαξιε.υσμοχξιρ.νοξστς
extern int MOB_QR_OBJECTS; //ϊαδαξιε.υσμοχξιρ.πςεδνετ
extern int MOB_QR_FOLLOWERS; //ϊαδαξιε.υσμοχξιρ.ποσμεδοχατεμψ
extern int MOB_QACCEPT; //ϊαδαξιε.ξαώαμο
extern int MOB_QDONE;  //ϊαδαξιε.ϊαχεςϋεξιε
extern int MOB_QSCRIPT;  //δεκστχιε
extern int MOB_QSCR_NUMBER; //δεκστχιε.ξονες
extern int MOB_QSCR_EXPR; //δεκστχιε.υσμοχιε
extern int MOB_QSCR_TEXT; //δεκστχιε.τελστ
extern int MOB_QSCR_SCRIPT;  //δεκστχιε.δεκστχιε

//σΠΕΓΛΟΝΑΞΔΩ
extern int MOB_COMMAND; //σπεγλοναξδα
extern int MOB_COMMAND_NO; //σπεγλοναξδα.λοναξδα
extern int MOB_COMMAND_ARG; //σπεγλοναξδα.αςηυνεξτ
extern int MOB_COMMAND_TOOL; //σπεγλοναξδα.ιξστςυνεξτ
extern int MOB_COMMAND_ACTIVE;
extern int MOB_COMMAND_ARG_OBJ; //σπεγλοναξδα.αςηυνεξτ.πςεδνετ
extern int MOB_COMMAND_ARG_ARG; //σπεγλοναξδα.αςηυνεξτ.στςολα
extern int MOB_COMMAND_ARG_MOB; //σπεγλοναξδα.αςηυνεξτ.νοξστς
extern int MOB_COMMAND_ARG_ERR; //σπεγλοναξδα.αςηυνεξτ.οϋιβλα
extern int MOB_COMMAND_SCRIPT; //σπεγλοναξδα.δεκστχιε
extern int MOB_COMMAND_EXTRACT; //σπεγλοναξδα.ςασσωπαξιε
extern int MOB_COMMAND_LOAD;     //σπεγλοναξδα.ϊαηςυϊλα
extern int MOB_COMMAND_LOAD_ROOM; //σπεγλοναξδα.ϊαηςυϊλα.χμολαγιΰ
extern int MOB_COMMAND_LOAD_CHAR; //σπεγλοναξδα.ϊαηςυϊλα.χιξχεξταςψ
extern int MOB_COMMAND_ACTIVE_ROOM;
extern int MOB_COMMAND_MESS_ROOM;
extern int MOB_COMMAND_MESS_CHAR;

//πΕÒΕΘΧΑΤ
extern int MOB_INTERCEPTION; //πεςεθχατ
extern int MOB_INT_COMMAND; // πεςεθχατ.λοναξδα (LIST)
extern int MOB_INT_STOP; // πεςεθχατ.στοπ (extern int)
extern int MOB_INT_MESSPLAYER; // πεςεθχατ.ιηςολ (STRING)
extern int MOB_INT_MESSVICTIM; // πεςεθχατ.φεςτχα (STRING)
extern int MOB_INT_MESSOTHER; // πεςεθχατ.οσταμψξων (STRING)
extern int MOB_INT_MESSROOM; // πεςεθχατ.λονξατα (STRING)
extern int MOB_INT_SCRIPT; // πεςεθχατ.δεκστχιε (STRING)
extern int MOB_INT_SARG; // πεςεθχατ.σαςηυνεξτ (STRING)

//σοβωτιε
extern int MOB_EVENT;  //σοβωτιε
extern int MOB_EVN_TYPE; //σοβωτιε.τιπ
extern int MOB_EVN_ARG; //σοβωτιε.αςηυνεξτ
extern int MOB_EVN_SCRIPT; //σοβωτιε.δεκστχιε

extern CMob *Mob;

#endif
