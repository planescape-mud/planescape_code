#ifndef XWLD_H
#define XWLD_H
#include "parser.h"
#include "parser_const.h"
#include "parser_items.h"

/*****************************************************************************/
/* οΠÒΕΔΕΜΕΞΙΡ ΔΜΡ ΪΑΗÒΥΪΛΙ ΖΑΚΜΟΧ ΟΠΙΣΑΞΙΡ ΜΟΛΑΓΙΚ (*.wlx)                  */
/*****************************************************************************/

class CWld : public CParser {
    public:
        CWld();
        ~CWld();
        bool Initialization(void);
};

// οΠÒΕΔΕΜΕΞΙΕ ΟΣΞΟΧΞΩΘ ΠΑÒΑΝΕΤÒΟΝ
extern int WLD_NUMBER;  // λονξατα
extern int WLD_ZONE;  //οβμαστψ
extern int WLD_POD;  // πςεδμοη
extern int WLD_NAME;  // ξαϊχαξιε
extern int WLD_DESCRIPTION; // οπισαξιε
extern int WLD_DESCRIPTION_N; // οπισαξιε ξοώψ
extern int WLD_ADDITION; // δοπομξιτεμψξο
extern int WLD_PROPERTIES; // λσχοκστχα
extern int WLD_DISTRICT; // νεστξοστψ
extern int WLD_INTERCEPTION; // πεςεθχατ
extern int WLD_EXIT;  // χωθοδ
extern int WLD_DAMAGE;  // ποχςεφδεξιρ
extern int WLD_TRAP;  // μοχυϋλα
extern int WLD_PORTAL;  // ποςταμ
extern int WLD_SCRIPT;  // σλςιτπ
extern int WLD_MOBILE;  // νοξστς
extern int WLD_OBJECT;  // πςεδνετ
extern int WLD_MAP;  // λαςτα
extern int WLD_FORCEDIR; // τεώεξιε

// οΠÒΕΔΕΜΕΞΙΕ ΧΣΠΟΝΟΗΑΤΕΜΨΞΩΘ ΠΑÒΑΝΕΤÒΟΧ
extern int WLD_ADD_KEY; //δοπομξιτεμψξο.λμΰώ
extern int WLD_ADD_TEXT; //δοπομξιτεμψξο.τελστ

extern int WLD_INT_COMMAND;  // πεςεθχατ.λοναξδα (LIST)
extern int WLD_INT_STOP;  // πεςεθχατ.στοπ (INT)
extern int WLD_INT_MESSPLAYER;  // πεςεθχατ.ιηςολ (STRING)
extern int WLD_INT_MESSVICTIM;  // πεςεθχατ.φεςτχα (STRING)
extern int WLD_INT_MESSOTHER;  // πεςεθχατ.οσταμψξων (STRING)
extern int WLD_INT_MESSROOM;  // πεςεθχατ.λονξατα (STRING)
extern int WLD_INT_SCRIPT;  // πεςεθχατ.δεκστχιε (INT)

extern int WLD_EXIT_DIRECTION;  // χωθοδ.ξαπςαχμεξιε (LIST)
extern int WLD_EXIT_ROOMNUMBER; // χωθοδ.λονξατα (INT)
extern int WLD_EXIT_DESCRIPTION; // χωθοδ.οπισαξιε (STRING)
extern int WLD_EXIT_NAME;  // χωθοδ.τιπ  (LIST)
extern int WLD_EXIT_ALIAS;  // χωθοδ.σιξοξιν (STRING)
extern int WLD_EXIT_KEY;  // χωθοδ.λμΰώ  (INT)
extern int WLD_EXIT_PROPERTIES; // χωθοδ.σχοκστχα (VECTOR)
extern int WLD_EXIT_QUALITY;         // χωθοδ.λαώεστχο
extern int WLD_EXIT_TRAP;  // χωθοδ.μοχυϋλα (INT)
extern int WLD_EXIT_TRAP_CHANCE; // χωθοδ.μοχυϋλα.ϋαξσ (INT)
extern int WLD_EXIT_TRAP_TYPEDAMAGE; // χωθοδ.μοχυϋλα.τχςεδ  (LIST)
extern int WLD_EXIT_TRAP_FORCEDAMAGE; // χωθοδ.μοχυϋλα.σχςεδ  (RANDOM)
extern int WLD_EXIT_TRAP_SAVE;  // χωθοδ.μοχυϋλα.πςεϊιστ
extern int WLD_EXIT_TRAP_MESS_CHAR; // χωθοδ.μοχυϋλα.ιηςολ
extern int WLD_EXIT_TRAP_MESS_ROOM; // χωθοδ.μοχυϋλα.λονξατα
extern int WLD_EXIT_TRAP_MESS_SCHAR; // χωθοδ.μοχυϋλα.ςιηςολ
extern int WLD_EXIT_TRAP_MESS_SROOM; // χωθοδ.μοχυϋλα.ςλονξατα
extern int WLD_EXIT_TRAP_MESS_KCHAR; // χωθοδ.μοχυϋλα.υβιτ_ιηςολ
extern int WLD_EXIT_TRAP_MESS_KROOM; // χωθοδ.μοχυϋλα.υβιτ_μολαγιρ
extern int WLD_EXIT_SEX;  // χωθοδ.ςοδ

extern int WLD_DAM_CHANCE;  //ποχςεφδεξιρ.ϋαξσ
extern int WLD_DAM_TYPE;  //ποχςεφδεξιρ.τιπ
extern int WLD_DAM_TYPEDAMAGE;  //ποχςεφδεξιρ.τχςεδ
extern int WLD_DAM_SAVE;  //ποχςεφδεξιρ.πςεϊιστ
extern int WLD_DAM_FORCEDAMAGE; //ποχςεφδεξιρ.σχςεδ
extern int WLD_DAM_MESS_CHAR;  //ποχςεφδεξιρ.ιηςολ
extern int WLD_DAM_MESS_ROOM;  //ποχςεφδεξιρ.λονξατα
extern int WLD_DAM_MESS_SCHAR;  //ποχςεφδεξιρ.ςιηςολ
extern int WLD_DAM_MESS_SROOM;  //ποχςεφδεξιρ.ςλονξατα

extern int WLD_TRAP_DIRECTION;  //μοχυϋλα.ξαπςαχμεξιε
extern int WLD_TRAP_CHANCE;  //μοχυϋλα.ϋαξσ
extern int WLD_TRAP_TYPE;  //μοχυϋλα.τιπ
extern int WLD_TRAP_TYPEDAMAGE; //μοχυϋλα.τχςεδ
extern int WLD_TRAP_SAVE;  //μοχυϋλα.πςεϊιστ
extern int WLD_TRAP_FORCEDAMAGE; //μοχυϋλα.σχςεδ
extern int WLD_TRAP_MESS_CHAR;  //μοχυϋλα.ιηςολ
extern int WLD_TRAP_MESS_ROOM;  //μοχυϋλα.οσταμψξων
extern int WLD_TRAP_MESS_SCHAR; //μοχυϋλα.ςιηςολ
extern int WLD_TRAP_MESS_SROOM; //μοχυϋλα.ςοσταμψξων
extern int WLD_TRAP_MESS_ACT_C; //μοχυϋλα.αιηςολ
extern int WLD_TRAP_MESS_ACT_R; //μοχυϋλα.αοσταμψξων
extern int WLD_TRAP_MESS_KCHAR; //μοχυϋλα.υβιτ_ιηςολ
extern int WLD_TRAP_MESS_KROOM; //μοχυϋλα.υβιτ_μολαγιρ

extern int WLD_PORTAL_DIRECTION; //ποςταμ.ξαπςαχμεξιε
extern int WLD_PORTAL_TYPE;  //ποςταμ.τιπ
extern int WLD_PORTAL_TIME;  //ποςταμ.χςενρ
extern int WLD_PORTAL_ROOM;  //ποςταμ.μολαγιρ
extern int WLD_PORTAL_KEY;  //ποςταμ.λμΰώ
extern int WLD_PORTAL_DESCRIPTION; //ποςταμ.οπισαξιε
extern int WLD_PORTAL_ACTIVE;  //ποςταμ.αλτιχξωκ
extern int WLD_PORTAL_DEACTIVE; //ποςταμ.δεαλτιχξωκ
extern int WLD_PORTAL_MESS_CHAR; //ποςταμ.ιηςολ
extern int WLD_PORTAL_MESS_ROOM; //ποςταμ.οσταμψξων
extern int WLD_PORTAL_WORK_TIME; //ποςταμ.χςενρ_ςαβοτω

extern int WLD_HOTEL;                  //ηοστιξιγα
extern int WLD_HOTEL_TYPE;             //ηοστιξιγα.τιπ
extern int WLD_HOTEL_MASTER;           //ηοστιξιγα.θοϊριξ
extern int WLD_HOTEL_CHAR;             //ηοστιξιγα.ιηςολυ
extern int WLD_HOTEL_ROOM;             //ηοστιξιγα.μολαγιρ
extern int WLD_HOTEL_RETURN;           //ηοστιξιγα.χεςξυμσρ

extern int WLD_FD_ROOM;  //τεώεξιε.μολαγιρ
extern int WLD_FD_DIR;   //τεώεξιε.ξαπςαχμεξιε
extern int WLD_FD_TIME;  //τεώεξιε.χςενρ
extern int WLD_FD_PERIOD;  //τεώεξιε.πεςιοδ
extern int WLD_FD_DAMAGE;  //τεώεξιε.ποχςεφδεξιρ
extern int WLD_FD_DAM_TYPE;  //τεώεξιε.ποχςεφδεξιρ.τιπ
extern int WLD_FD_DAM_FORCEDAMAGE; //τεώεξιε.ποχςεφδεξιρ.σιμα
extern int WLD_FD_DAM_SAVETYPE; //τεώεξιε.ποχςεφδεξιρ.ςεϊιστ
extern int WLD_FD_DAM_SAVE;  //τεώεξιε.ποχςεφδεξιρ.σοθςαξεξιε
extern int WLD_FD_MESS_MCHAR;  //τεώεξιε.πςοναθ_πεςσοξαφ
extern int WLD_FD_MESS_MROOM;  //τεώεξιε.πςοναθ_μολαγιρ
extern int WLD_FD_MESS_EXCHAR;  //τεώεξιε.υϋεμ_πεςσοξαφ
extern int WLD_FD_MESS_EXROOM;  //τεώεξιε.υϋεμ_μολαγιρ
extern int WLD_FD_MESS_ENCHAR;  //τεώεξιε.πςιϋεμ_πεςσοξαφ
extern int WLD_FD_MESS_ENROOM;  //τεώεξιε.πςιϋεμ_μολαγιρ
extern int WLD_FD_MESS_KCHAR;  //τεώεξιε.υβιτ_πεςσοξαφ
extern int WLD_FD_MESS_KROOM;  //τεώεξιε.υβιτ_μολαγιρ

extern int WLD_PERIOD;   //πεςιοδιώξοστψ
extern int WLD_PRD_START;  //πεςιοδιώξοστψ.ξαώαμο
extern int WLD_PRD_STOP;  //πεςιοδιώξοστψ.λοξεγ
extern int WLD_PRD_WEATHER;  //πεςιοδιώξοστψ.ποηοδα
extern int WLD_PRD_OBJECT;  //πεςιοδιώξοστψ.πςεδνετ
extern int WLD_PRD_MONSTER;  //πεςιοδιώξοστψ.νοξστς
extern int WLD_PRD_SRLOCATION;  //πεςιοδιώξοστψ.ξμολαγιρ
extern int WLD_PRD_SPLOCATION;  //πεςιοδιώξοστψ.λμολαγιρ
extern int WLD_PRD_SRZONE;  //πεςιοδιώξοστψ.ξοποχεύεξιε
extern int WLD_PRD_SPZONE;  //πεςιοδιώξοστψ.λοποχεύεξιε

extern CWld *Wld;

/* οΠÒΕΔΕΜΕΞΙΡ ΔΜΡ ΝΙÒΟΧΟΗΟ ΜΥΤΑ Σ ΝΟΞΣΤÒΟΧ (/lib/misc/world_loot)           */
/*****************************************************************************/

class CWlt : public CParser {
    public:
        CWlt();
        ~CWlt();
        bool Initialization(void);
};

//world loot ΣΙΣΤΕΝΑ (/lib/misc/world_loot)
extern int WRL_LOOT;   //νιςοχοκ
extern int WRL_RACE;   //νιςοχοκ.ςασα LIST(14)
extern int WRL_TYPE;   //τιπ LIST(49)
extern int WRL_LEVEL;   //υςοχεξψ STRING
extern int WRL_SEX;   //πομ LIST(11)
extern int WRL_CLASS;   //λμασσ STRLIST(13)
extern int WRL_GOD;   //βοη LIST(28)
extern int WRL_AGE;   //χοϊςαστ STRLIST(52)
extern int WRL_OBJECT;   //πςεδνετ INT
extern int WRL_SHANCE;   //ϋαξσ INT (1...1000)

extern CWlt Wlt;

#endif
