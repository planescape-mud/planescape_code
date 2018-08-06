#ifndef XTEMPL_H
#define XTEMPL_H
#include "parser.h"
#include "parser_const.h"

/*****************************************************************************/
/* οΠÒΕΔΕΜΕΞΙΡ ΔΜΡ ΪΑΗÒΥΪΛΙ ΖΑΚΜΑ ΟΠΙΣΑΞΙΡ ΫΑΒΜΟΞΑ ΔΟΣΠΕΘΟΧ                  */
/*****************************************************************************/

class CArmTmp : public CParser {
    public:
        CArmTmp();
        ~CArmTmp();
        bool Initialization(void);
};

extern CArmTmp ArmTmp;

// οΠÒΕΔΕΜΕΞΙΕ ΟΣΞΟΧΞΩΘ ΠΑÒΑΝΕΤÒΟΝ
extern int A_NUMBER;  //ϋαβμοξ
extern int A_NAME;    //ξαϊχαξιε
extern int A_DESC;    //
extern int A_ALIAS; //ιδεξτιζιλατος
extern int A_TYPE;    //τιπ
extern int A_PTYPE;   //ποδτιπ
extern int A_WEIGHT;    //χεσ
extern int A_MATERIAL;  //νατεςιαμ
extern int A_PRICE;     //γεξα
extern int A_AC;        //βςοξρ
extern int A_ARM0;      //ϊςεφυύεε
extern int A_ARM1;      //ϊλομΰύεε
extern int A_ARM2;      //ϊυδαςξοε
extern int A_CLASS;     //λμασσ
extern int A_WEAR; //ισπομψϊοχατψ
extern int A_SPEED; //σλοςοστψ

extern int A_MAT_TYPE;  //τιπ
extern int A_MAT_VAL; //λομιώεστχο
extern int A_MAT_MAIN;  //οσξοχξοε

extern int A_P0; //πςεφυύεε
extern int A_P1; //πλομΰύεε
extern int A_P2; //πυδαςξοε


/*****************************************************************************/
/* οΠÒΕΔΕΜΕΞΙΡ ΔΜΡ ΪΑΗÒΥΪΛΙ ΖΑΚΜΑ ΟΠΙΣΑΞΙΡ ΫΑΒΜΟΞΑ ΔΟΣΠΕΘΟΧ                  */
/*****************************************************************************/

class CWeapTmp : public CParser {
    public:
        CWeapTmp();
        ~CWeapTmp();
        bool Initialization(void);
};

extern CWeapTmp WeapTmp;

// οΠÒΕΔΕΜΕΞΙΕ ΟΣΞΟΧΞΩΘ ΠΑÒΑΝΕΤÒΟΝ

extern int W_NUMBER; //ϋαβμοξ
extern int W_NAME; //ξαϊχαξιε
extern int W_DESC; //οπισαξιε
extern int W_ALIAS; //ιδεξτιζιλατος
extern int W_TYPE; //τιπ
extern int W_MESS; //σοοβύεξιε
extern int W_HIT; //υδας
extern int W_HIT_TYPE; //υδας.τιπ
extern int W_HIT_DAM; //υδας,χςεδ
extern int W_WEIGHT; //χεσ
extern int W_PRICE; //γεξα
extern int W_SPEED; //σλοςοστψ
extern int W_CRITIC; //λςιτιώεσλικ
extern int W_MATERIAL; //νατεςιαμ
extern int W_MAT_TYPE;  //νατεςιαμ.τιπ
extern int W_MAT_VAL; //νατεςιαμ.λομιώεστχο
extern int W_MAT_MAIN;  //νατεςιαμ.οσξοχξοε
extern int W_WEAR; //ισπομψϊοχαξιε

class CMissTmp : public CParser {
    public:
        CMissTmp();
        ~CMissTmp();
        bool Initialization(void);
};

extern CMissTmp MissTmp;

extern int M_NUMBER; //ϋαβμοξ
extern int M_NAME;      //ξαϊχαξιε
extern int M_ALIAS; //ιδεξτιζιλατος
extern int M_DESC; //οπισαξιε
extern int M_TYPE; //τιπ
extern int M_HITS; //τιπ
extern int M_HIT_TYPE; //υδας.τιπ
extern int M_HIT_DAM; //υδας,χςεδ
extern int M_MATERIAL; //νατεςιαμ
extern int M_MAT_TYPE;  //νατεςιαμ.τιπ
extern int M_MAT_VAL; //νατεςιαμ.λομιώεστχο
extern int M_MAT_MAIN;  //νατεςιαμ.οσξοχξοε

class CSetTmp : public CParser {
    public:
        CSetTmp();
        ~CSetTmp();
        bool Initialization(void);
};

extern CSetTmp SetTmp;

extern int S_NUMBER;  //ξονες
extern int S_OBJECTS;           //πςεδνετω
extern int S_VARIANTE;  //χαςιαξτ
extern int S_VAR_COUNT; //χαςιαξτ.λομιώεστχο
extern int S_VAR_DESC; //χαςιαξτ.οπισαξιε
extern int S_VAR_AFFECT; //χαςιαξτ.όζζελτ
extern int S_VAR_APPLY; //χαςιαξτ.δοπομξεξιρ
extern int S_VAR_SKILL; //χαςιαξτ.υνεξιε
extern int S_VAR_SCORE; //χαςιαξτ.σώετ
extern int S_VAR_SCHAR; //χαςιαξτ.σιηςολ
extern int S_VAR_SROOM; //χαςιαξτ.σμολαγιρ
extern int S_VAR_ECHAR; //χαςιαξτ.λιηςολ
extern int S_VAR_EROOM; //χαςιαξτ.λμολαγιρ

class CHitMess : public CParser {
    public:
        CHitMess();
        ~CHitMess();
        bool Initialization(void);
};

extern CHitMess HitMess;

extern int MS_TYPE; //υδας
extern int MS_SUB_CHAR; //ποδστςυλτυςα.ιηςολ
extern int MS_SUB_VICT; //ποδστςυλτυςα.φεςτχα
extern int MS_SUB_ROOM; //ποδστςυλτυςα.μολαγιρ
extern int MS_HIT_NONE;  //βεϊοςυφιρ
extern int MS_HIT_WEAP; //σοςυφιεν
extern int MS_HWP_CHAR; //ποδστςυλτυςα.ιηςολ
extern int MS_HWP_VICT; //ποδστςυλτυςα.φεςτχα
extern int MS_HWP_ROOM; //ποδστςυλτυςα.μολαγιρ

extern int MS_MISS_NONE; //πςοναθβεϊ
extern int MS_MISS_WEAP; //πςναθοςυφιε
extern int MS_KILL_NONE; //σνεςτψβεϊ
extern int MS_KILL_WEAP; //σνεςτψοςυφιε
extern int MS_ARM_NONE; //δοσπεθβεϊ
extern int MS_ARM_WEAP; //δοσπεθοςυφιε
extern int MS_SKIN_NONE; //λοφαβεϊ
extern int MS_SKIN_WEAP; //λοφαοςυφιε

#endif
