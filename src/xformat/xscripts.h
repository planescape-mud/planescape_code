#ifndef XSCRIPTS_H
#define XSCRIPTS_H
#include "parser.h"
#include "parser_const.h"
#include "parser_items.h"

/*****************************************************************************/
/* οΠÒΕΔΕΜΕΞΙΡ ΔΜΡ ΪΑΗÒΥΪΛΙ ΖΑΚΜΟΧ ΔΕΚΣΤΧΙΚ (scr) */
/*****************************************************************************/

class CScr : public CParser {
    public:
        CScr();
        ~CScr();
        bool Initialization(void);
};

extern CScr Scr;


// οΠÒΕΔΕΜΕΞΙΕ ΟΣΞΟΧΞΩΘ ΠΑÒΑΝΕΤÒΟΝ
extern int SCR_NUMBER;  //δεκστχιε
extern int SCR_EXPR;  //υσμοχιε
extern int SCR_MESSAGE;  //σοοβύεξιε
extern int SCR_MVNUM;  //σοοβύεξιε.ηδε
extern int SCR_MCHAR;  //σοοβύεξιε.ιηςολυ
extern int SCR_MROOM;  //σοοβύεξιε.μολαγιρ
extern int SCR_MZONE;  //σοοβύεξιε.οβμαστψ

extern int SCR_ERROR;  //οϋιβλα
extern int SCR_ECHAR;  //οϋιβλα.ιηςολυ
extern int SCR_EROOM;  //οϋιβλα.μολαγιρ
extern int SCR_EZONE;  //οϋιβλα.οβμαστψ
extern int SCR_ESCRIPT;  //οϋιβλα.δεκστχιε

extern int SCR_LOAD;  //ϊαηςυϊλα
extern int SCR_LOBJ;  //ϊαηςυϊλα.πςεδνετ
extern int SCR_LEQ;  //ϊαηςυϊλα.ιξχεξταςψ
extern int SCR_LRANDOM;  //ϊαηςυϊλα.σμυώακξωκ
extern int SCR_LMOB;  //ϊαηςυϊλα.νοξστς
extern int SCR_LEXIT;  //ϊαηςυϊλα.χωθοδ
extern int SCR_LPORTAL;  //ϊαηςυϊλα.ποςταμ
extern int SCR_LCHAR;  //ϊαηςυϊλα.ιηςολυ
extern int SCR_LROOM;  //ϊαηςυϊλα.μολαγιρ
extern int SCR_LGOLD;  //ϊαηςυϊλα.δεξψηι

extern int SCR_OBJ_LIMIT; //ϊαηςυϊλα.μινιτ

extern int SCR_LE_ROOM; // χωθοδ.ηδε
extern int SCR_LE_DIRECTION; // χωθοδ.ξαπςαχμεξιε (LIST)
extern int SCR_LE_ROOMNUMBER; // χωθοδ.λονξατα (INT)
extern int SCR_LE_DESCRIPTION; // χωθοδ.οπισαξιε (STRING)
extern int SCR_LE_NAME;  // χωθοδ.τιπ  (LIST)
extern int SCR_LE_ALIAS; // χωθοδ.σιξοξιν (STRING)
extern int SCR_LE_KEY;  // χωθοδ.λμΰώ  (INT)
extern int SCR_LE_PROPERTIES; // χωθοδ.σχοκστχα (VECTOR)
extern int SCR_LE_QUALITY; // χωθοδ.λαώεστχο
extern int SCR_LE_TRAP;  // χωθοδ.μοχυϋλα (INT)
extern int SCR_LE_TRAP_CHANCE; // χωθοδ.μοχυϋλα.ϋαξσ (INT)
extern int SCR_LE_TRAP_TYPEDAMAGE; // χωθοδ.μοχυϋλα.τχςεδ  (LIST)
extern int SCR_LE_TRAP_FORCEDAMAGE; // χωθοδ.μοχυϋλα.σχςεδ  (RANDOM)
extern int SCR_LE_TRAP_SAVE;  // χωθοδ.μοχυϋλα.πςεϊιστ
extern int SCR_LE_TRAP_MESS_CHAR; // χωθοδ.μοχυϋλα.ιηςολ
extern int SCR_LE_TRAP_MESS_ROOM; // χωθοδ.μοχυϋλα.λονξατα
extern int SCR_LE_TRAP_MESS_SCHAR; // χωθοδ.μοχυϋλα.ςιηςολ
extern int SCR_LE_TRAP_MESS_SROOM; // χωθοδ.μοχυϋλα.ςλονξατα
extern int SCR_LE_TRAP_MESS_KCHAR; // χωθοδ.μοχυϋλα.υβιτ_ιηςολ
extern int SCR_LE_TRAP_MESS_KROOM; // χωθοδ.μοχυϋλα.υβιτ_μολαγιρ
extern int SCR_LE_SEX;  // χωθοδ.ςοδ

extern int SCR_LP_PORTAL; //ποςταμ
extern int SCR_LP_ROOM;  //ποςταμ.ηδε
extern int SCR_LP_DIRECTION; //ποςταμ.ξαπςαχμεξιε
extern int SCR_LP_TYPE;  //ποςταμ.τιπ
extern int SCR_LP_TIME;  //ποςταμ.χςενρ
extern int SCR_LP_ROOMNUMBER;  //ποςταμ.λονξατα
extern int SCR_LP_KEY;  //ποςταμ.λμΰώ
extern int SCR_LP_DESCRIPTION; //ποςταμ.οπισαξιε
extern int SCR_LP_ACTIVE;  //ποςταμ.αλτιχξωκ
extern int SCR_LP_DEACTIVE; //ποςταμ.δεαλτιχξωκ
extern int SCR_LP_MESS_CHAR; //ποςταμ.ιηςολ
extern int SCR_LP_MESS_ROOM; //ποςταμ.οσταμψξων
extern int SCR_LP_WORK_TIME; //ποςταμ.χςενρ_ςαβοτω

extern int SCR_DELETE;  //υδαμεξιε
extern int SCR_DOBJ;  //υδαμεξιε.πςεδνετ
extern int SCR_DEQ;  //υδαμεξιε.ιξχεξταςψ
extern int SCR_DMOB;  //υδαμεξιε.νοξστς
extern int SCR_DGOLD;  //υδαμεξιε.δεξψηι
extern int SCR_DEXIT;  //υδαμεξιε.χωθοδ
extern int SCR_DE_ROOM;  //χωθοδ,ηδε
extern int SCR_DE_DIRECTION; //χωθοδ.ξαπςαχμεξιε
extern int SCR_DPORTAL;  //υδαμεξιε.ποςταμ
extern int SCR_DP_ROOM;  //ποςταμ.ηδε
extern int SCR_DP_DIRECTION; //ποςταμ.ξαπςαχμεξιε
extern int SCR_DCHAR;  //υδαμεξιε.ιηςολ
extern int SCR_DROOM;  //υδαμεξιε.οσταμψξων

extern int SCR_TELEPORT; //πεςενεύεξιε
extern int SCR_TWHERE;  //ηδε
extern int SCR_TROOM;  //λονξατα
extern int SCR_TCHAROUT;  //ιοτβωτιε
extern int SCR_TCHARIN;  //ιπςιβωτιε
extern int SCR_TROOMOUT;  //λοτβωτιε
extern int SCR_TROOMIN;  //λπςιβωτιε

extern int SCR_SPELL;  //ϊαλμιξαξιε
extern int SCR_SPELL_NO; //ϊαλμιξαξιε.ξονες
extern int SCR_SPELL_LEVEL; //ϊαλμιξαξιε.υςοχεξψ
extern int SCR_SPELL_MCHAR; //ϊαλμιξαξιε.ιηςολ
extern int SCR_SPELL_MROOM; //ϊαλμιξαξιε.μολαγιρ

extern int SCR_VAR;  //πεςενεξξαρ
extern int SCR_VAR_NAME; //πεςενεξξαρ.ξαϊχαξιε
extern int SCR_VAR_VALUE; //πεςενεξξαρ.ϊξαώεξιε
extern int SCR_VAR_TIME; //πεςενεξξαρ.χςενρ

extern int SCR_GLB;  //ημοβαμψξαρ
extern int SCR_GLB_NAME; //ημοβαμψξαρ.ξαϊχαξιε
extern int SCR_GLB_VALUE; //ημοβαμψξαρ.ϊξαώεξιε
extern int SCR_GLB_TIME; //ημοβαμψξαρ.χςενρ

extern int SCR_RESET; //σβςοσ

extern int SCR_AGRO; //αηςεσσιρ
extern int SCR_AMOB; //αηςεσσιρ.νοξστς
extern int SCR_ATARGET; //αηςεσσιρ.γεμψ

extern int SCR_DAMAGE;    //χςεδ
extern int SCR_DAM_TARGET; //χςεδ.γεμψ
extern int SCR_DAM_SHANCE; //χςεδ.ϋαξσ
extern int SCR_DAM_TYPE;   //χςεδ.τχςεδ
extern int SCR_DAM_DAMAGE; //χςεδ.σχςεδ
extern int SCR_DAM_STYPE;  //χςεδ.ςεϊιστ
extern int SCR_DAM_SAVE;   //χςεδ.σοθςαξεξιε
extern int SCR_DAM_KVICT;  //χςεδ.υφεςτχα
extern int SCR_DAM_KROOM;  //χςεδ.υμολαγιρ
extern int SCR_DAM_HVICT;  //χςεδ.χφεςτχα
extern int SCR_DAM_HROOM;  //χςεδ.χμολαγιρ
extern int SCR_DAM_MVICT;  //χςεδ.πφεςτχα
extern int SCR_DAM_MROOM;  //χςεδ.πμολαγιρ

extern int SCR_EXP; //οπωτ
extern int SCR_EXP_TYPE; //τιπ
extern int SCR_EXP_SCHAR; //ιοδξοςαϊοχωκ
extern int SCR_EXP_SGROUP; //ηοδξοςαϊοχωκ
extern int SCR_EXP_CHAR; //ιηςολυ
extern int SCR_EXP_GROUP; //ηςυππε

extern int SCR_APPLY; //νοδιζιλατος

extern int SCR_CONTINUE; //πςοδομφεξιε
extern int SCR_CNT_VNUM; //πςοδομφεξιε.ξονες
extern int SCR_CNT_TIMER; //πςοδομφεξιε.τακνες

#endif
