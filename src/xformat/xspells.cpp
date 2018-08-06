/**************************************************************************
    νπν "ηÒΑΞΙ νΙÒΑ" (Σ) 2002-2003 αΞΔÒΕΚ εÒΝΙΫΙΞ
    ϊΑΗÒΥΪΛΑ ΖΑΚΜΟΧ ΙΗÒΟΧΟΗΟ ΝΙÒΑ
 **************************************************************************/

#include "xspells.h"
#include "parser_id.h"

// οΠÒΕΔΕΜΕΞΙΕ ΟΣΞΟΧΞΩΘ ΠΑÒΑΝΕΤÒΟΝ
int SPL_NUMBER; //ϊαλμιξαξιε
int SPL_NAME; //ξαϊχαξιε
int SPL_UNAME; //σιξοξιν
int SPL_ALIAS; //ιξδεζιλατος
int SPL_PROCEDURE; //πςογεδυςα
int SPL_SPHERE; //ϋλομα (σζεςα)
int SPL_MANA; //ναξα
int SPL_LEVEL; //υςοχεξψ
int SPL_SKILL; //υνεξιε
int SPL_QUALITY; //ξαχωλ
int SPL_DAMAGE; //σχςεδ
int SPL_TDAMAGE; //τχςεδ
int SPL_AGRO; //αηςεσσιρ
int SPL_TARGET; //γεμψ
int SPL_POSITION; //πομοφεξιε
int SPL_LAG; //ϊαδεςφλα
int SPL_VLAG; //φϊαδεςφλα
int SPL_MEMORY; //ϊαπονιξαξιε
int SPL_CONCENT; //λοξγεξτςαγιρ
int SPL_SAVES; //σοθςαξεξιε
int SPL_SCORE; //σώετ
int SPL_AFF_MESS; //πόζζελτω
int SPL_SAY_MESS; //πςοιϊξοϋεξιε
int SPL_MESSAGES_AFFECT; //ασοοβύεξιρ
int SPL_MESSAGES_DAMAGE;  //βσοοβύεξιρ
int SPL_MESSAGES_OBJECT;  //οσοοβύεξιρ

//ΔΟΠ.ΠΑÒΑΝΕΤÒΩ πόζζελτω
int SPL_AMESS_SCH; //ξαώαμο_ιηςολ
int SPL_AMESS_SRM; //ξαώαμο_μολαγιρ
int SPL_AMESS_ECH; //λοξεγ_ιηςολ
int SPL_AMESS_ERM; //λοξεγ_μολαγιρ

//ΔΟΠ.ΠΑÒΑΝΕΤÒΩ πςοιϊξοϋεξιε
int SPL_SAY_CHAR; //ιηςολ
int SPL_SAY_VICTIM; //φεςτχα
int SPL_SAY_ROOM; //μολαγιρ
int SPL_SAY_ME;  //ριηςολ
int SPL_SAY_MROOM; //ρμολαγιρ

//ΔΟΠ.ΠΑÒΑΝΕΤÒΩ xσοοβύεξιρ
int SPL_MESSAGE_KIL_CHAR;   //υβιτ_ιηςολ
int SPL_MESSAGE_KIL_VICT;   //υβιτ_φεςτχα
int SPL_MESSAGE_KIL_ROOM;   //υβιτ_μολαγιρ
int SPL_MESSAGE_HIT_CHAR;   //υδας_ιηςολ
int SPL_MESSAGE_HIT_VICT;   //υδας_φεςτχα
int SPL_MESSAGE_HIT_ROOM;   //υδας_μολαγιρ
int SPL_MESSAGE_HIT_ME;     //υδας_ριηςολ
int SPL_MESSAGE_HIT_MROM;   //υδας_ρμολαγιρ
int SPL_MESSAGE_MIS_CHAR;   //πςοναθ_ιηςολ
int SPL_MESSAGE_MIS_VICT;   //πςοναθ_φεςτχα
int SPL_MESSAGE_MIS_ROOM;   //πςοναθ_μολαγιρ
int SPL_MESSAGE_MIS_ME;     //πςοναθ_ριηςολ
int SPL_MESSAGE_MIS_MROM;   //πςοναθ_ρμολαγιρ
int SPL_MESSAGE_GOD_CHAR;   //βεσν_ιηςολ
int SPL_MESSAGE_GOD_ME;     //βεσν_ριηςολ
int SPL_MESSAGE_GOD_VICT;   //βεσν_φεςτχα
int SPL_MESSAGE_GOD_ROOM;   //βεσν_μολαγιρ
int SPL_MESSAGE_GOD_MROM;   //βεσν_ρμολαγιρ
int SPL_MESSAGE_ARM_CHAR;
int SPL_MESSAGE_ARM_VICT;
int SPL_MESSAGE_ARM_ROOM;
int SPL_MESSAGE_AR2_CHAR;
int SPL_MESSAGE_AR2_VICT;
int SPL_MESSAGE_AR2_ROOM;

///////////////////////////////////////////////////////////////////////////////
CSpl::CSpl() {
}

CSpl::~CSpl() {
}
///////////////////////////////////////////////////////////////////////////////

CSpl Spl;

bool CSpl::Initialization(void) {
    SPL_NUMBER = Proto->AddItem(TYPE_INT, "ϊαλμιξαξιε");
    SPL_NAME = Proto->AddItem(TYPE_STRING, "ξαϊχαξιε");
    SPL_UNAME = Proto->AddItem(TYPE_STRING, "σιξοξιν");
    SPL_ALIAS = Proto->AddItem(TYPE_STRING, "ιξδεζιλατος");
    SPL_PROCEDURE = Proto->AddItem(TYPE_STRING, "πςογεδυςα");
    SPL_SPHERE = Proto->AddItem(TYPE_LIST, "ϋλομα", true, ParserConst.GetList(LIST_SPH_TYPES));
    SPL_MANA = Proto->AddItem(TYPE_INT, "ναξα");
    SPL_LEVEL = Proto->AddItem(TYPE_INT, "υςοχεξψ");
    SPL_SKILL = Proto->AddItem(TYPE_INT, "υνεξιε");
//SPL_QUALITY = Proto->AddItem(TYPE_INT,"ξαχωλ");
    SPL_DAMAGE = Proto->AddItem(TYPE_RANDOM, "σχςεδ", false);
    SPL_TDAMAGE = Proto->AddItem(TYPE_LIST, "τχςεδ", false, ParserConst.GetList(LIST_DAMAGE));
    SPL_AGRO = Proto->AddItem(TYPE_VECTOR, "αηςεσσιρ", false, ParserConst.GetVector(VEC_AGRO_TYPE));
    SPL_TARGET = Proto->AddItem(TYPE_VECTOR, "γεμψ", true, ParserConst.GetVector(VEC_SPELL_TARGET));
    SPL_POSITION = Proto->AddItem(TYPE_LIST, "πομοφεξιε", false, ParserConst.GetList(LIST_POSITIONS));
    SPL_LAG = Proto->AddItem(TYPE_INT, "ϊαδεςφλα", false);
    SPL_VLAG = Proto->AddItem(TYPE_INT, "φϊαδεςφλα", false);
    SPL_MEMORY = Proto->AddItem(TYPE_INT, "ϊαπονιξαξιε", false);
    SPL_CONCENT = Proto->AddItem(TYPE_INT, "λοξγεξτςαγιρ", false);
    SPL_SAVES = Proto->AddItem(TYPE_INT, "σοθςαξεξιε", false);
    SPL_SCORE = Proto->AddItem(TYPE_STRING, "σώετ", false);
    SPL_AFF_MESS = Proto->AddItem(TYPE_STRUCT, "πόζζελτω", false);
    CItemProto *aff = Proto->GetItem(SPL_AFF_MESS);
    SPL_AMESS_SCH = aff->AddItem(TYPE_STRING, "ξαώαμο_ιηςολ", false);
    SPL_AMESS_SRM = aff->AddItem(TYPE_STRING, "ξαώαμο_μολαγιρ", false);
    SPL_AMESS_ECH = aff->AddItem(TYPE_STRING, "λοξεγ_ιηςολ", false);
    SPL_AMESS_ERM = aff->AddItem(TYPE_STRING, "λοξεγ_μολαγιρ", false);

    SPL_SAY_MESS = Proto->AddItem(TYPE_STRUCT, "πςοιϊξοϋεξιε", false);
    CItemProto *say = Proto->GetItem(SPL_SAY_MESS);
    SPL_SAY_CHAR   = say->AddItem(TYPE_STRING, "ιηςολ", false);
    SPL_SAY_VICTIM = say->AddItem(TYPE_STRING, "φεςτχα", false);
    SPL_SAY_ROOM   = say->AddItem(TYPE_STRING, "μολαγιρ", false);
    SPL_SAY_ME     = say->AddItem(TYPE_STRING, "ριηςολ", false);
    SPL_SAY_MROOM  = say->AddItem(TYPE_STRING, "ρμολαγιρ", false);

    SPL_MESSAGES_AFFECT = Proto->AddItem(TYPE_STRUCT, "ασοοβύεξιρ", false);
    CItemProto *maff = Proto->GetItem(SPL_MESSAGES_AFFECT);
    SPL_MESSAGE_KIL_CHAR = maff->AddItem(TYPE_STRING, "υβιτ_ιηςολ", false);
    SPL_MESSAGE_KIL_VICT = maff->AddItem(TYPE_STRING, "υβιτ_φεςτχα", false);
    SPL_MESSAGE_KIL_ROOM = maff->AddItem(TYPE_STRING, "υβιτ_μολαγιρ", false);
    SPL_MESSAGE_HIT_CHAR = maff->AddItem(TYPE_STRING, "υδας_ιηςολ", false);
    SPL_MESSAGE_HIT_VICT = maff->AddItem(TYPE_STRING, "υδας_φεςτχα", false);
    SPL_MESSAGE_HIT_ROOM = maff->AddItem(TYPE_STRING, "υδας_μολαγιρ", false);
    SPL_MESSAGE_HIT_ME   = maff->AddItem(TYPE_STRING, "υδας_ριηςολ", false);
    SPL_MESSAGE_HIT_MROM = maff->AddItem(TYPE_STRING, "υδας_ρμολαγιρ", false);
    SPL_MESSAGE_MIS_CHAR = maff->AddItem(TYPE_STRING, "πςοναθ_ιηςολ", false);
    SPL_MESSAGE_MIS_VICT = maff->AddItem(TYPE_STRING, "πςοναθ_φεςτχα", false);
    SPL_MESSAGE_MIS_ROOM = maff->AddItem(TYPE_STRING, "πςοναθ_μολαγιρ", false);
    SPL_MESSAGE_MIS_ME   = maff->AddItem(TYPE_STRING, "πςοναθ_ριηςολ", false);
    SPL_MESSAGE_MIS_MROM = maff->AddItem(TYPE_STRING, "πςοναθ_ρμολαγιρ", false);
    SPL_MESSAGE_GOD_CHAR = maff->AddItem(TYPE_STRING, "βεσν_ιηςολ", false);
    SPL_MESSAGE_GOD_ME   = maff->AddItem(TYPE_STRING, "βεσν_ριηςολ", false);
    SPL_MESSAGE_GOD_MROM = maff->AddItem(TYPE_STRING, "βεσν_ρμολαγιρ", false);
    SPL_MESSAGE_GOD_VICT = maff->AddItem(TYPE_STRING, "βεσν_φεςτχα", false);
    SPL_MESSAGE_GOD_ROOM = maff->AddItem(TYPE_STRING, "βεσν_μολαγιρ", false);
    SPL_MESSAGE_ARM_CHAR = maff->AddItem(TYPE_STRING, "ποημ_ιηςολ", false);
    SPL_MESSAGE_ARM_VICT = maff->AddItem(TYPE_STRING, "ποημ_φεςτχα", false);
    SPL_MESSAGE_ARM_ROOM = maff->AddItem(TYPE_STRING, "ποημ_μολαγιρ", false);
    SPL_MESSAGE_AR2_CHAR = maff->AddItem(TYPE_STRING, "αμτς_ιηςολ", false);
    SPL_MESSAGE_AR2_VICT = maff->AddItem(TYPE_STRING, "αμτς_φεςτχα", false);
    SPL_MESSAGE_AR2_ROOM = maff->AddItem(TYPE_STRING, "αμτς_μολαγιρ", false);

    SPL_MESSAGES_DAMAGE = Proto->AddItem(TYPE_STRUCT, "βσοοβύεξιρ", false);
    CItemProto *daff = Proto->GetItem(SPL_MESSAGES_DAMAGE);
    SPL_MESSAGE_KIL_CHAR = daff->AddItem(TYPE_STRING, "υβιτ_ιηςολ", false);
    SPL_MESSAGE_KIL_VICT = daff->AddItem(TYPE_STRING, "υβιτ_φεςτχα", false);
    SPL_MESSAGE_KIL_ROOM = daff->AddItem(TYPE_STRING, "υβιτ_μολαγιρ", false);
    SPL_MESSAGE_HIT_CHAR = daff->AddItem(TYPE_STRING, "υδας_ιηςολ", false);
    SPL_MESSAGE_HIT_VICT = daff->AddItem(TYPE_STRING, "υδας_φεςτχα", false);
    SPL_MESSAGE_HIT_ROOM = daff->AddItem(TYPE_STRING, "υδας_μολαγιρ", false);
    SPL_MESSAGE_HIT_ME   = daff->AddItem(TYPE_STRING, "υδας_ριηςολ", false);
    SPL_MESSAGE_HIT_MROM = daff->AddItem(TYPE_STRING, "υδας_ρμολαγιρ", false);
    SPL_MESSAGE_MIS_CHAR = daff->AddItem(TYPE_STRING, "πςοναθ_ιηςολ", false);
    SPL_MESSAGE_MIS_VICT = daff->AddItem(TYPE_STRING, "πςοναθ_φεςτχα", false);
    SPL_MESSAGE_MIS_ROOM = daff->AddItem(TYPE_STRING, "πςοναθ_μολαγιρ", false);
    SPL_MESSAGE_MIS_ME   = daff->AddItem(TYPE_STRING, "υδας_ριηςολ", false);
    SPL_MESSAGE_MIS_MROM = daff->AddItem(TYPE_STRING, "υδας_ρμολαγιρ", false);
    SPL_MESSAGE_GOD_CHAR = daff->AddItem(TYPE_STRING, "βεσν_ιηςολ", false);
    SPL_MESSAGE_GOD_VICT = daff->AddItem(TYPE_STRING, "βεσν_φεςτχα", false);
    SPL_MESSAGE_GOD_ROOM = daff->AddItem(TYPE_STRING, "βεσν_μολαγιρ", false);
    SPL_MESSAGE_ARM_CHAR = daff->AddItem(TYPE_STRING, "ποημ_ιηςολ", false);
    SPL_MESSAGE_ARM_VICT = daff->AddItem(TYPE_STRING, "ποημ_φεςτχα", false);
    SPL_MESSAGE_ARM_ROOM = daff->AddItem(TYPE_STRING, "ποημ_μολαγιρ", false);
    SPL_MESSAGE_AR2_CHAR = daff->AddItem(TYPE_STRING, "αμτς_ιηςολ", false);
    SPL_MESSAGE_AR2_VICT = daff->AddItem(TYPE_STRING, "αμτς_φεςτχα", false);
    SPL_MESSAGE_AR2_ROOM = daff->AddItem(TYPE_STRING, "αμτς_μολαγιρ", false);

    SPL_MESSAGES_OBJECT = Proto->AddItem(TYPE_STRUCT, "οσοοβύεξιρ", false);
    CItemProto *oaff = Proto->GetItem(SPL_MESSAGES_OBJECT);
    SPL_MESSAGE_KIL_CHAR = oaff->AddItem(TYPE_STRING, "υβιτ_ιηςολ", false);
    SPL_MESSAGE_KIL_VICT = oaff->AddItem(TYPE_STRING, "υβιτ_φεςτχα", false);
    SPL_MESSAGE_KIL_ROOM = oaff->AddItem(TYPE_STRING, "υβιτ_μολαγιρ", false);
    SPL_MESSAGE_HIT_CHAR = oaff->AddItem(TYPE_STRING, "υδας_ιηςολ", false);
    SPL_MESSAGE_HIT_VICT = oaff->AddItem(TYPE_STRING, "υδας_φεςτχα", false);
    SPL_MESSAGE_HIT_ROOM = oaff->AddItem(TYPE_STRING, "υδας_μολαγιρ", false);
    SPL_MESSAGE_HIT_ME   = oaff->AddItem(TYPE_STRING, "υδας_ριηςολ", false);
    SPL_MESSAGE_HIT_MROM = oaff->AddItem(TYPE_STRING, "υδας_ρμολαγιρ", false);
    SPL_MESSAGE_MIS_CHAR = oaff->AddItem(TYPE_STRING, "πςοναθ_ιηςολ", false);
    SPL_MESSAGE_MIS_VICT = oaff->AddItem(TYPE_STRING, "πςοναθ_φεςτχα", false);
    SPL_MESSAGE_MIS_ROOM = oaff->AddItem(TYPE_STRING, "πςοναθ_μολαγιρ", false);
    SPL_MESSAGE_MIS_ME   = oaff->AddItem(TYPE_STRING, "υδας_ριηςολ", false);
    SPL_MESSAGE_MIS_MROM = oaff->AddItem(TYPE_STRING, "υδας_ρμολαγιρ", false);
    SPL_MESSAGE_GOD_CHAR = oaff->AddItem(TYPE_STRING, "βεσν_ιηςολ", false);
    SPL_MESSAGE_GOD_VICT = oaff->AddItem(TYPE_STRING, "βεσν_φεςτχα", false);
    SPL_MESSAGE_GOD_ROOM = oaff->AddItem(TYPE_STRING, "βεσν_μολαγιρ", false);
    SPL_MESSAGE_ARM_CHAR = oaff->AddItem(TYPE_STRING, "ποημ_ιηςολ", false);
    SPL_MESSAGE_ARM_VICT = oaff->AddItem(TYPE_STRING, "ποημ_φεςτχα", false);
    SPL_MESSAGE_ARM_ROOM = oaff->AddItem(TYPE_STRING, "ποημ_μολαγιρ", false);
    SPL_MESSAGE_AR2_CHAR = oaff->AddItem(TYPE_STRING, "αμτς_ιηςολ", false);
    SPL_MESSAGE_AR2_VICT = oaff->AddItem(TYPE_STRING, "αμτς_φεςτχα", false);
    SPL_MESSAGE_AR2_ROOM = oaff->AddItem(TYPE_STRING, "αμτς_μολαγιρ", false);

    return CheckInit();
}

