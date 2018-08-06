#ifndef XBODY_H
#define XBODY_H
#include "parser.h"
#include "parser_const.h"
#include "parser_items.h"


/*****************************************************************************/
/* οΠÒΕΔΕΜΕΞΙΡ ΔΜΡ ΪΑΗÒΥΪΛΙ ΖΑΚΜΟΧ ΟΠΙΣΑΞΙΡ ΤΕΜ ÒΑΣ (body) */
/*****************************************************************************/

class CBody : public CParser {
    public:
        CBody();
        ~CBody();
        bool Initialization(void);
};

class CProf : public CParser {
    public:
        CProf();
        ~CProf();
        bool Initialization(void);
};

// οΠÒΕΔΕΜΕΞΙΕ ΟΣΞΟΧΞΩΘ ΠΑÒΑΝΕΤÒΟΝ
extern int TBD_RACE;  //ςασα
extern int TBD_HEALTH;  //ϊδοςοχψε
extern int TBD_BODY;  //τεμο
extern int TBD_BODY_POSITION; //τεμο.ποϊιγιρ
extern int TBD_BODY_SNAME; //τεμο.σπςξαϊχαξιε
extern int TBD_BODY_NAME; //τεμο.ξαϊχαξιε
extern int TBD_BODY_CHANCE; //τεμο.σμοφξοστψ
extern int TBD_BODY_KDAM; //τεμο.πςοώξοστψ
extern int TBD_AC; //τεμο.βςοξρ
extern int TBD_ARM0; //τεμο.ϊςεφυύεε
extern int TBD_ARM1; //τεμο.ϊλομΰύεε
extern int TBD_ARM2; //τεμο.ϊυδαςξοε
extern int TBD_FIRE;
extern int TBD_COLD;
extern int TBD_ELECTRO;
extern int TBD_ACID;
extern int TBD_POISON;
extern int TBD_XAOS;
extern int TBD_ORDER;
extern int TBD_POSITIVE;
extern int TBD_NEGATIVE;

extern int TCL_CLASS;  //πςοζεσσιρ
extern int TCL_ADDSTR;  //δοβαχμεξιε σιμω
extern int TCL_ADDCON;
extern int TCL_ADDDEX;
extern int TCL_ADDINT;
extern int TCL_ADDWIS;
extern int TCL_ADDCHA;  //δοβαχμεξιε οβαρξιρ
extern int TCL_HEALTH;
extern int TCL_MANA;
extern int TCL_KFORT;
extern int TCL_KREFL;
extern int TCL_KWILL;
extern int TCL_HROLL;
extern int TCL_AC;
extern int TCL_ARM0;
extern int TCL_ARM1;
extern int TCL_ARM2;

extern int TBD_BODY_WEAPON; //οςυφιε
extern int TBD_BWP_NAME; //οςυφιε.ξαϊχαξιε
extern int TBD_BWP_SEX;  //οςυφιε.ςοδ

extern CBody tBody;
extern CProf tProf;

#endif
