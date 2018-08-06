#ifndef XSPELLS_H
#define XSPELLS_H
#include "parser.h"
#include "parser_const.h"
#include "parser_items.h"

/*****************************************************************************/
/* οΠÒΕΔΕΜΕΞΙΡ ΔΜΡ ΪΑΗÒΥΪΛΙ ΖΑΚΜΟΧ ΟΠΙΣΑΞΙΡ ΪΑΛΜΙΞΑΞΙΚ (spells)              */
/*****************************************************************************/

class CSpl : public CParser {
    public:
        CSpl();
        ~CSpl();
        bool Initialization(void);
};


// οΠÒΕΔΕΜΕΞΙΕ ΟΣΞΟΧΞΩΘ ΠΑÒΑΝΕΤÒΟΝ
extern int SPL_NUMBER; //ϊαλμιξαξιε
extern int SPL_NAME; //ξαϊχαξιε
extern int SPL_UNAME; //σιξοξιν
extern int SPL_ALIAS; //ιξδεζιλατος
extern int SPL_PROCEDURE; //πςογεδυςα
extern int SPL_SPHERE; //ϋλομα (σζεςα)
extern int SPL_MANA; //ναξα
extern int SPL_LEVEL; //υςοχεξψ
extern int SPL_SKILL; //υνεξιε
extern int SPL_DAMAGE; //σχςεδ
extern int SPL_TDAMAGE; //τχςεδ
extern int SPL_QUALITY; //ξαχωλ
extern int SPL_AGRO; //αηςεσσιρ
extern int SPL_TARGET; //γεμψ
extern int SPL_POSITION; //πομοφεξιε
extern int SPL_LAG; //ϊαδεςφλα
extern int SPL_VLAG; //φϊαδεςφλα
extern int SPL_MEMORY; //ϊαπονιξαξιε
extern int SPL_CONCENT; //λοξγεξτςαγιρ
extern int SPL_SAVES; //σοθςαξεξιε
extern int SPL_SCORE; //σώετ
extern int SPL_AFF_MESS; //πόζζελτω
extern int SPL_SAY_MESS; //πςοιϊξοϋεξιε
extern int SPL_MESSAGES_AFFECT; //ασοοβύεξιρ
extern int SPL_MESSAGES_DAMAGE;  //βσοοβύεξιρ
extern int SPL_MESSAGES_OBJECT;  //οσοοβύεξιρ

//ΔΟΠ.ΠΑÒΑΝΕΤÒΩ πόζζελτω
extern int SPL_AMESS_SCH; //ξαώαμο_ιηςολ
extern int SPL_AMESS_SRM; //ξαώαμο_μολαγιρ
extern int SPL_AMESS_ECH; //λοξεγ_ιηςολ
extern int SPL_AMESS_ERM; //λοξεγ_μολαγιρ

//ΔΟΠ.ΠΑÒΑΝΕΤÒΩ πςοιϊξοϋεξιε
extern int SPL_SAY_CHAR; //ιηςολ
extern int SPL_SAY_VICTIM; //φεςτχα
extern int SPL_SAY_ROOM; //μολαγιρ
extern int SPL_SAY_ME;  //ριηςολ
extern int SPL_SAY_MROOM; //ρμολαγιρ

//ΔΟΠ.ΠΑÒΑΝΕΤÒΩ xσοοβύεξιρ
extern int SPL_MESSAGE_KIL_CHAR;   //υβιτ_ιηςολ
extern int SPL_MESSAGE_KIL_VICT;   //υβιτ_φεςτχα
extern int SPL_MESSAGE_KIL_ROOM;   //υβιτ_μολαγιρ
extern int SPL_MESSAGE_HIT_CHAR;   //υδας_ιηςολ
extern int SPL_MESSAGE_HIT_VICT;   //υδας_φεςτχα
extern int SPL_MESSAGE_HIT_ROOM;   //υδας_μολαγιρ
extern int SPL_MESSAGE_HIT_ME;     //υδας_ριηςολ
extern int SPL_MESSAGE_HIT_MROM;   //υδας_ρμολαγιρ
extern int SPL_MESSAGE_MIS_CHAR;   //πςοναθ_ιηςολ
extern int SPL_MESSAGE_MIS_VICT;   //πςοναθ_φεςτχα
extern int SPL_MESSAGE_MIS_ROOM;   //πςοναθ_μολαγιρ
extern int SPL_MESSAGE_MIS_ME;     //πςοναθ_ριηςολ
extern int SPL_MESSAGE_MIS_MROM;   //πςοναθ_ρμολαγιρ
extern int SPL_MESSAGE_GOD_CHAR;   //βεσν_ιηςολ
extern int SPL_MESSAGE_GOD_ME;     //βεσν_ριηςολ
extern int SPL_MESSAGE_GOD_VICT;   //βεσν_φεςτχα
extern int SPL_MESSAGE_GOD_ROOM;   //βεσν_μολαγιρ
extern int SPL_MESSAGE_GOD_MROM;   //βεσν_ρμολαγιρ
extern int SPL_MESSAGE_ARM_CHAR;   //ποημούεξιε
extern int SPL_MESSAGE_ARM_VICT;
extern int SPL_MESSAGE_ARM_ROOM;
extern int SPL_MESSAGE_AR2_CHAR;   //αμψτεςξατιχξοε ποημούεξιε
extern int SPL_MESSAGE_AR2_VICT;
extern int SPL_MESSAGE_AR2_ROOM;

extern CSpl Spl;

#endif
