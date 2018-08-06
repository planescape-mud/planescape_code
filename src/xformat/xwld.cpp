/**************************************************************************
    νπν "ηÒΑΞΙ νΙÒΑ" (Σ) 2002-2003 αΞΔÒΕΚ εÒΝΙΫΙΞ
    ϊΑΗÒΥΪΛΑ ΖΑΚΜΟΧ ΙΗÒΟΧΟΗΟ ΝΙÒΑ
 **************************************************************************/

#include "xwld.h"
#include "parser_id.h"


///////////////////////////////////////////////////////////////////////////////
// οΠÒΕΔΕΜΕΞΙΕ ΟΣΞΟΧΞΩΘ ΠΑÒΑΝΕΤÒΟΝ
int WLD_NUMBER;  // λονξατα
int WLD_ZONE;  //οβμαστψ
int WLD_POD;  // πςεδμοη
int WLD_NAME;  // ξαϊχαξιε
int WLD_DESCRIPTION; // οπισαξιε
int WLD_DESCRIPTION_N; // οπισαξιε ξοώψ
int WLD_ADDITION; // δοπομξιτεμψξο
int WLD_PROPERTIES; // λσχοκστχα
int WLD_DISTRICT; // νεστξοστψ
int WLD_INTERCEPTION; // πεςεθχατ
int WLD_EXIT;  // χωθοδ
int WLD_DAMAGE;  // ποχςεφδεξιρ
int WLD_TRAP;  // μοχυϋλα
int WLD_PORTAL;  // ποςταμ
int WLD_SCRIPT;  // σλςιτπ
int WLD_MOBILE;  // νοξστς
int WLD_OBJECT;  // πςεδνετ
int WLD_MAP;  // λαςτα
int WLD_FORCEDIR; // τεώεξιε

// οΠÒΕΔΕΜΕΞΙΕ ΧΣΠΟΝΟΗΑΤΕΜΨΞΩΘ ΠΑÒΑΝΕΤÒΟΧ
int WLD_ADD_KEY; //δοπομξιτεμψξο.λμΰώ
int WLD_ADD_TEXT; //δοπομξιτεμψξο.τελστ

int WLD_INT_COMMAND;  // πεςεθχατ.λοναξδα (LIST)
int WLD_INT_STOP;  // πεςεθχατ.στοπ (INT)
int WLD_INT_MESSPLAYER;  // πεςεθχατ.ιηςολ (STRING)
int WLD_INT_MESSVICTIM;  // πεςεθχατ.φεςτχα (STRING)
int WLD_INT_MESSOTHER;  // πεςεθχατ.οσταμψξων (STRING)
int WLD_INT_MESSROOM;  // πεςεθχατ.λονξατα (STRING)
int WLD_INT_SCRIPT;  // πεςεθχατ.δεκστχιε (INT)

int WLD_EXIT_DIRECTION;  // χωθοδ.ξαπςαχμεξιε (LIST)
int WLD_EXIT_ROOMNUMBER; // χωθοδ.λονξατα (INT)
int WLD_EXIT_DESCRIPTION; // χωθοδ.οπισαξιε (STRING)
int WLD_EXIT_NAME;  // χωθοδ.τιπ  (LIST)
int WLD_EXIT_ALIAS;  // χωθοδ.σιξοξιν (STRING)
int WLD_EXIT_KEY;  // χωθοδ.λμΰώ  (INT)
int WLD_EXIT_PROPERTIES; // χωθοδ.σχοκστχα (VECTOR)
int WLD_EXIT_QUALITY;         // χωθοδ.λαώεστχο
int WLD_EXIT_TRAP;  // χωθοδ.μοχυϋλα (INT)
int WLD_EXIT_TRAP_CHANCE; // χωθοδ.μοχυϋλα.ϋαξσ (INT)
int WLD_EXIT_TRAP_TYPEDAMAGE; // χωθοδ.μοχυϋλα.τχςεδ  (LIST)
int WLD_EXIT_TRAP_FORCEDAMAGE; // χωθοδ.μοχυϋλα.σχςεδ  (RANDOM)
int WLD_EXIT_TRAP_SAVE;  // χωθοδ.μοχυϋλα.πςεϊιστ
int WLD_EXIT_TRAP_MESS_CHAR; // χωθοδ.μοχυϋλα.ιηςολ
int WLD_EXIT_TRAP_MESS_ROOM; // χωθοδ.μοχυϋλα.λονξατα
int WLD_EXIT_TRAP_MESS_SCHAR; // χωθοδ.μοχυϋλα.ςιηςολ
int WLD_EXIT_TRAP_MESS_SROOM; // χωθοδ.μοχυϋλα.ςλονξατα
int WLD_EXIT_TRAP_MESS_KCHAR; // χωθοδ.μοχυϋλα.υβιτ_ιηςολ
int WLD_EXIT_TRAP_MESS_KROOM; // χωθοδ.μοχυϋλα.υβιτ_μολαγιρ
int WLD_EXIT_SEX;  // χωθοδ.ςοδ

int WLD_DAM_CHANCE;  //ποχςεφδεξιρ.ϋαξσ
int WLD_DAM_TYPE;  //ποχςεφδεξιρ.τιπ
int WLD_DAM_TYPEDAMAGE;  //ποχςεφδεξιρ.τχςεδ
int WLD_DAM_SAVE;  //ποχςεφδεξιρ.πςεϊιστ
int WLD_DAM_FORCEDAMAGE; //ποχςεφδεξιρ.σχςεδ
int WLD_DAM_MESS_CHAR;  //ποχςεφδεξιρ.ιηςολ
int WLD_DAM_MESS_ROOM;  //ποχςεφδεξιρ.λονξατα
int WLD_DAM_MESS_SCHAR;  //ποχςεφδεξιρ.ςιηςολ
int WLD_DAM_MESS_SROOM;  //ποχςεφδεξιρ.ςλονξατα

int WLD_TRAP_DIRECTION;  //μοχυϋλα.ξαπςαχμεξιε
int WLD_TRAP_CHANCE;  //μοχυϋλα.ϋαξσ
int WLD_TRAP_TYPE;  //μοχυϋλα.τιπ
int WLD_TRAP_TYPEDAMAGE; //μοχυϋλα.τχςεδ
int WLD_TRAP_SAVE;  //μοχυϋλα.πςεϊιστ
int WLD_TRAP_FORCEDAMAGE; //μοχυϋλα.σχςεδ
int WLD_TRAP_MESS_CHAR;  //μοχυϋλα.ιηςολ
int WLD_TRAP_MESS_ROOM;  //μοχυϋλα.οσταμψξων
int WLD_TRAP_MESS_SCHAR; //μοχυϋλα.ςιηςολ
int WLD_TRAP_MESS_SROOM; //μοχυϋλα.ςοσταμψξων
int WLD_TRAP_MESS_ACT_C; //μοχυϋλα.αιηςολ
int WLD_TRAP_MESS_ACT_R; //μοχυϋλα.αοσταμψξων
int WLD_TRAP_MESS_KCHAR; //μοχυϋλα.υβιτ_ιηςολ
int WLD_TRAP_MESS_KROOM; //μοχυϋλα.υβιτ_μολαγιρ

int WLD_PORTAL_DIRECTION; //ποςταμ.ξαπςαχμεξιε
int WLD_PORTAL_TYPE;  //ποςταμ.τιπ
int WLD_PORTAL_TIME;  //ποςταμ.χςενρ
int WLD_PORTAL_ROOM;  //ποςταμ.μολαγιρ
int WLD_PORTAL_KEY;  //ποςταμ.λμΰώ
int WLD_PORTAL_DESCRIPTION; //ποςταμ.οπισαξιε
int WLD_PORTAL_ACTIVE;  //ποςταμ.αλτιχξωκ
int WLD_PORTAL_DEACTIVE; //ποςταμ.δεαλτιχξωκ
int WLD_PORTAL_MESS_CHAR; //ποςταμ.ιηςολ
int WLD_PORTAL_MESS_ROOM; //ποςταμ.οσταμψξων
int WLD_PORTAL_WORK_TIME; //ποςταμ.χςενρ_ςαβοτω

int WLD_HOTEL;                  //ηοστιξιγα
int WLD_HOTEL_TYPE;             //ηοστιξιγα.τιπ
int WLD_HOTEL_MASTER;           //ηοστιξιγα.θοϊριξ
int WLD_HOTEL_CHAR;             //ηοστιξιγα.ιηςολυ
int WLD_HOTEL_ROOM;             //ηοστιξιγα.μολαγιρ
int WLD_HOTEL_RETURN;           //ηοστιξιγα.χεςξυμσρ

int WLD_FD_ROOM;  //τεώεξιε.μολαγιρ
int WLD_FD_DIR;   //τεώεξιε.ξαπςαχμεξιε
int WLD_FD_TIME;  //τεώεξιε.χςενρ
int WLD_FD_PERIOD;  //τεώεξιε.πεςιοδ
int WLD_FD_DAMAGE;  //τεώεξιε.ποχςεφδεξιρ
int WLD_FD_DAM_TYPE;  //τεώεξιε.ποχςεφδεξιρ.τιπ
int WLD_FD_DAM_FORCEDAMAGE; //τεώεξιε.ποχςεφδεξιρ.σιμα
int WLD_FD_DAM_SAVETYPE; //τεώεξιε.ποχςεφδεξιρ.ςεϊιστ
int WLD_FD_DAM_SAVE;  //τεώεξιε.ποχςεφδεξιρ.σοθςαξεξιε
int WLD_FD_MESS_MCHAR;  //τεώεξιε.πςοναθ_πεςσοξαφ
int WLD_FD_MESS_MROOM;  //τεώεξιε.πςοναθ_μολαγιρ
int WLD_FD_MESS_EXCHAR;  //τεώεξιε.υϋεμ_πεςσοξαφ
int WLD_FD_MESS_EXROOM;  //τεώεξιε.υϋεμ_μολαγιρ
int WLD_FD_MESS_ENCHAR;  //τεώεξιε.πςιϋεμ_πεςσοξαφ
int WLD_FD_MESS_ENROOM;  //τεώεξιε.πςιϋεμ_μολαγιρ
int WLD_FD_MESS_KCHAR;  //τεώεξιε.υβιτ_πεςσοξαφ
int WLD_FD_MESS_KROOM;  //τεώεξιε.υβιτ_μολαγιρ

int WLD_PERIOD;   //πεςιοδιώξοστψ
int WLD_PRD_START;  //πεςιοδιώξοστψ.ξαώαμο
int WLD_PRD_STOP;  //πεςιοδιώξοστψ.λοξεγ
int WLD_PRD_WEATHER;  //πεςιοδιώξοστψ.ποηοδα
int WLD_PRD_OBJECT;  //πεςιοδιώξοστψ.πςεδνετ
int WLD_PRD_MONSTER;  //πεςιοδιώξοστψ.νοξστς
int WLD_PRD_SRLOCATION;  //πεςιοδιώξοστψ.ξμολαγιρ
int WLD_PRD_SPLOCATION;  //πεςιοδιώξοστψ.λμολαγιρ
int WLD_PRD_SRZONE;  //πεςιοδιώξοστψ.ξοποχεύεξιε
int WLD_PRD_SPZONE;  //πεςιοδιώξοστψ.λοποχεύεξιε

CWld::CWld() {
}

CWld::~CWld() {
}
///////////////////////////////////////////////////////////////////////////////

bool CWld::Initialization(void) {

    WLD_NUMBER = Proto->AddItem(TYPE_INT, "λονξατα");
    WLD_ZONE = Proto->AddItem(TYPE_INT, "οβμαστψ", false);
    WLD_POD = Proto->AddItem(TYPE_STRING, "πςεδμοη", false);
    WLD_NAME = Proto->AddItem(TYPE_STRING, "ξαϊχαξιε");
    WLD_DESCRIPTION = Proto->AddItem(TYPE_STRING, "οπισαξιε");
    WLD_DESCRIPTION_N = Proto->AddItem(TYPE_STRING, "οπισαξιε_ξοώψ", false);
    WLD_ADDITION = Proto->AddItem(TYPE_STRUCT, "δοπομξιτεμψξο", false);
    CItemProto *addition = Proto->GetItem(WLD_ADDITION);
    WLD_ADD_KEY = addition->AddItem(TYPE_STRING, "λμΰώ");
    WLD_ADD_TEXT = addition->AddItem(TYPE_STRING, "τελστ");
    WLD_PROPERTIES = Proto->AddItem(TYPE_VECTOR, "λσχοκστχα", false,
                                    ParserConst.GetVector(VEC_PROP_ROOM));
    WLD_DISTRICT = Proto->AddItem(TYPE_LIST, "νεστξοστψ", true,
                                  ParserConst.GetList(LIST_DISTRICT_TYPES));
    WLD_INTERCEPTION = Proto->AddItem(TYPE_STRUCT, "πεςεθχατ", false);
    CItemProto *intercept = Proto->GetItem(WLD_INTERCEPTION);
    WLD_INT_COMMAND = intercept->AddItem(TYPE_LIST, "λοναξδα", true,
                                         ParserConst.GetList(LIST_COMMANDS));
    WLD_INT_STOP = intercept->AddItem(TYPE_LIST, "στοπ", false,
                                      ParserConst.GetList(LIST_YESNO));
    WLD_INT_MESSPLAYER = intercept->AddItem(TYPE_STRING, "ιηςολ");
    WLD_INT_MESSVICTIM = intercept->AddItem(TYPE_STRING, "φεςτχα", false);
    WLD_INT_MESSOTHER  = intercept->AddItem(TYPE_STRING, "οσταμψξων", false);
    WLD_INT_MESSROOM   = intercept->AddItem(TYPE_STRING, "λονξατα", false);
    WLD_INT_SCRIPT     = intercept->AddItem(TYPE_INT, "δεκστχιε", false);
    WLD_EXIT = Proto->AddItem(TYPE_STRUCT, "χωθοδ", false);
    CItemProto *exit = Proto->GetItem(WLD_EXIT);
    WLD_EXIT_DIRECTION = exit->AddItem(TYPE_LIST, "ξαπςαχμεξιε", true,
                                       ParserConst.GetList(LIST_DIRECTIONS));
    WLD_EXIT_ROOMNUMBER = exit->AddItem(TYPE_INT, "λονξατα");
    WLD_EXIT_DESCRIPTION = exit->AddItem(TYPE_STRING, "οπισαξιε", false);
    WLD_EXIT_NAME       = exit->AddItem(TYPE_STRING, "ξαϊχαξιε", false);
    WLD_EXIT_ALIAS = exit->AddItem(TYPE_STRING, "σιξοξινω", false);
    WLD_EXIT_KEY = exit->AddItem(TYPE_INT, "λμΰώ", false);
    WLD_EXIT_PROPERTIES = exit->AddItem(TYPE_VECTOR, "σχοκστχα", false,
                                        ParserConst.GetVector(VEC_EXIT_PROP));
    WLD_EXIT_QUALITY   = exit->AddItem(TYPE_INT, "λαώεστχο", false);
    WLD_EXIT_SEX      = exit->AddItem(TYPE_LIST, "ςοδ", false,
                                      ParserConst.GetList(LIST_SEX));
    WLD_EXIT_TRAP = exit->AddItem(TYPE_STRUCT, "μοχυϋλα", false);
    CItemProto *etrap = exit->GetItem(WLD_EXIT_TRAP);
    WLD_EXIT_TRAP_CHANCE = etrap->AddItem(TYPE_INT, "ϋαξσ");
    WLD_EXIT_TRAP_TYPEDAMAGE = etrap->AddItem(TYPE_LIST, "τχςεδ", true,
                               ParserConst.GetList(LIST_DAMAGE));
    WLD_EXIT_TRAP_FORCEDAMAGE = etrap->AddItem(TYPE_RANDOM, "σχςεδ");
    WLD_EXIT_TRAP_SAVE = etrap->AddItem(TYPE_INT, "πςεϊιστ", false);
    WLD_EXIT_TRAP_MESS_CHAR = etrap->AddItem(TYPE_STRING, "ιηςολ", false);
    WLD_EXIT_TRAP_MESS_ROOM = etrap->AddItem(TYPE_STRING, "οσταμψξων", false);
    WLD_EXIT_TRAP_MESS_SCHAR = etrap->AddItem(TYPE_STRING, "ςιηςολ", false);
    WLD_EXIT_TRAP_MESS_SROOM = etrap->AddItem(TYPE_STRING, "ςοσταμψξων", false);
    WLD_EXIT_TRAP_MESS_KCHAR = etrap->AddItem(TYPE_STRING, "υβιτ_ιηςολ", false);
    WLD_EXIT_TRAP_MESS_KROOM = etrap->AddItem(TYPE_STRING, "υβιτ_μολαγιρ", false);

    WLD_DAMAGE = Proto->AddItem(TYPE_STRUCT, "ποχςεφδεξιρ", false);
    CItemProto *damage = Proto->GetItem(WLD_DAMAGE);
    WLD_DAM_CHANCE = damage->AddItem(TYPE_INT, "ϋαξσ");
    WLD_DAM_TYPE = damage->AddItem(TYPE_VECTOR, "τιπ", false,
                                   ParserConst.GetVector(VEC_TRAP_TYPE));
    WLD_DAM_TYPEDAMAGE = damage->AddItem(TYPE_LIST, "τχςεδ", true,
                                         ParserConst.GetList(LIST_DAMAGE));
    WLD_DAM_SAVE = damage->AddItem(TYPE_INT, "πςεϊιστ", false);
    WLD_DAM_FORCEDAMAGE = damage->AddItem(TYPE_RANDOM, "σχςεδ");
    WLD_DAM_MESS_CHAR = damage->AddItem(TYPE_STRING, "ιηςολ");
    WLD_DAM_MESS_ROOM = damage->AddItem(TYPE_STRING, "οσταμψξων");
    WLD_DAM_MESS_SCHAR = damage->AddItem(TYPE_STRING, "ςιηςολ");
    WLD_DAM_MESS_SROOM = damage->AddItem(TYPE_STRING, "ςοσταμψξων");
    WLD_TRAP = Proto->AddItem(TYPE_STRUCT, "μοχυϋλα", false);
    CItemProto *trap = Proto->GetItem(WLD_TRAP);
    WLD_TRAP_DIRECTION = trap->AddItem(TYPE_LIST, "ξαπςαχμεξιε", true,
                                       ParserConst.GetList(LIST_DIRECTIONS));
    WLD_TRAP_CHANCE = trap->AddItem(TYPE_INT, "ϋαξσ");
    WLD_TRAP_TYPE = trap->AddItem(TYPE_LIST, "τιπ", true,
                                  ParserConst.GetList(LIST_TRAP_TYPE));
    WLD_TRAP_TYPEDAMAGE = trap->AddItem(TYPE_LIST, "τχςεδ", true,
                                        ParserConst.GetList(LIST_DAMAGE));
    WLD_TRAP_SAVE = trap->AddItem(TYPE_INT, "πςεϊιστ", false);
    WLD_TRAP_FORCEDAMAGE = trap->AddItem(TYPE_RANDOM, "σχςεδ");
    WLD_TRAP_MESS_CHAR = trap->AddItem(TYPE_STRING, "ιηςολ");
    WLD_TRAP_MESS_ROOM = trap->AddItem(TYPE_STRING, "οσταμψξων");
    WLD_TRAP_MESS_SCHAR = trap->AddItem(TYPE_STRING, "ςιηςολ", false);
    WLD_TRAP_MESS_SROOM = trap->AddItem(TYPE_STRING, "ςοσταμψξων", false);
    WLD_TRAP_MESS_ACT_C = trap->AddItem(TYPE_STRING, "αιηςολ", false);
    WLD_TRAP_MESS_ACT_R = trap->AddItem(TYPE_STRING, "αοσταμψξων", false);
    WLD_TRAP_MESS_KCHAR = trap->AddItem(TYPE_STRING, "υβιτ_ιηςολ", false);
    WLD_TRAP_MESS_KROOM = trap->AddItem(TYPE_STRING, "υβιτ_μολαγιρ", false);


    WLD_PORTAL = Proto->AddItem(TYPE_STRUCT, "ποςταμ", false);
    CItemProto *portal = Proto->GetItem(WLD_PORTAL);
    WLD_PORTAL_DIRECTION = portal->AddItem(TYPE_LIST, "ξαπςαχμεξιε", true,
                                           ParserConst.GetList(LIST_DIRECTIONS));
    WLD_PORTAL_TYPE = portal->AddItem(TYPE_LIST, "τιπ", true,
                                      ParserConst.GetList(LIST_PORTAL_TYPES));
    WLD_PORTAL_TIME = portal->AddItem(TYPE_INT, "χςενρ", false);
    WLD_PORTAL_ROOM = portal->AddItem(TYPE_INT, "λονξατα");
    WLD_PORTAL_KEY = portal->AddItem(TYPE_INT, "λμΰώ");
    WLD_PORTAL_DESCRIPTION = portal->AddItem(TYPE_STRING, "στςολα");
    WLD_PORTAL_ACTIVE = portal->AddItem(TYPE_STRING, "αλτιχξωκ");
    WLD_PORTAL_DEACTIVE = portal->AddItem(TYPE_STRING, "δεαλτιχξωκ");
    WLD_PORTAL_MESS_CHAR = portal->AddItem(TYPE_STRING, "ιηςολ");
    WLD_PORTAL_MESS_ROOM = portal->AddItem(TYPE_STRING, "οσταμψξων");
    WLD_PORTAL_WORK_TIME = portal->AddItem(TYPE_STRING, "χςενρ_ςαβοτω", false);
    WLD_SCRIPT = Proto->AddItem(TYPE_SCRIPT, "σλςιπτ", false);
    WLD_MOBILE = Proto->AddItem(TYPE_STRLIST, "νοξστς", false);
    WLD_OBJECT = Proto->AddItem(TYPE_STRLIST, "πςεδνετ", false);
    WLD_MAP    = Proto->AddItem(TYPE_INT, "λαςτα", false);

    WLD_HOTEL = Proto->AddItem(TYPE_STRUCT, "ηοστιξιγα", false);
    CItemProto *hotel = Proto->GetItem(WLD_HOTEL);
    WLD_HOTEL_TYPE   = hotel->AddItem(TYPE_INT, "τιπ", true);
    WLD_HOTEL_MASTER = hotel->AddItem(TYPE_INT, "θοϊριξ", false);
    WLD_HOTEL_CHAR   = hotel->AddItem(TYPE_STRING, "ιηςολυ", false);
    WLD_HOTEL_ROOM   = hotel->AddItem(TYPE_STRING, "μολαγιρ", false);
    WLD_HOTEL_RETURN = hotel->AddItem(TYPE_STRING, "χεςξυμσρ", false);

    WLD_FORCEDIR = Proto->AddItem(TYPE_STRUCT, "τεώεξιε", false);
    CItemProto *fd = Proto->GetItem(WLD_FORCEDIR);
    WLD_FD_ROOM = fd->AddItem(TYPE_INT, "μολαγιρ", false);
    WLD_FD_DIR = fd->AddItem(TYPE_LIST, "ξαπςαχμεξιε", false, ParserConst.GetList(LIST_DIRECTIONS));
    WLD_FD_TIME = fd->AddItem(TYPE_STRING, "χςενρ", false);
    WLD_FD_PERIOD = fd->AddItem(TYPE_INT, "πεςιοδιώξοστψ", false);
    WLD_FD_DAMAGE = fd->AddItem(TYPE_STRUCT, "ποχςεφδεξιρ", false);
    CItemProto *fdam = fd->GetItem(WLD_FD_DAMAGE);
    WLD_FD_DAM_TYPE = fdam->AddItem(TYPE_LIST, "τιπ", true, ParserConst.GetList(LIST_DAMAGE));
    WLD_FD_DAM_FORCEDAMAGE = fdam->AddItem(TYPE_RANDOM, "σιμα", false);
    WLD_FD_DAM_SAVETYPE = fdam->AddItem(TYPE_LIST, "ςεϊιστ", true, ParserConst.GetList(LIST_SAVE_TYPES));
    WLD_FD_DAM_SAVE = fdam->AddItem(TYPE_INT, "σοθςαξεξιε");
    WLD_FD_MESS_MCHAR = fd->AddItem(TYPE_STRING, "πςοναθ_πεςσοξαφ", false);
    WLD_FD_MESS_MROOM = fd->AddItem(TYPE_STRING, "πςοναθ_μολαγιρ", false);
    WLD_FD_MESS_EXCHAR = fd->AddItem(TYPE_STRING, "υϋεμ_πεςσοξαφ", false);
    WLD_FD_MESS_EXROOM = fd->AddItem(TYPE_STRING, "υϋεμ_μολαγιρ", false);
    WLD_FD_MESS_ENCHAR = fd->AddItem(TYPE_STRING, "πςιϋεμ_πεςσοξαφ", false);
    WLD_FD_MESS_ENROOM = fd->AddItem(TYPE_STRING, "πςιϋεμ_μολαγιρ", false);
    WLD_FD_MESS_KCHAR = fd->AddItem(TYPE_STRING, "υβιτ_πεςσοξαφ", false);
    WLD_FD_MESS_KROOM = fd->AddItem(TYPE_STRING, "υβιτ_μολαγιρ", false);

    WLD_PERIOD = Proto->AddItem(TYPE_STRUCT, "πεςιοδιώξοστψ", false);
    CItemProto *prd = Proto->GetItem(WLD_PERIOD);
    WLD_PRD_START = prd->AddItem(TYPE_INT, "ξαώαμο", false);
    WLD_PRD_STOP = prd->AddItem(TYPE_INT, "λοξεγ", false);
    WLD_PRD_WEATHER = prd->AddItem(TYPE_LIST, "ποηοδα", false, ParserConst.GetList(LIST_WEATHER));
    WLD_PRD_OBJECT = prd->AddItem(TYPE_INT, "πςεδνετ", false);
    WLD_PRD_MONSTER = prd->AddItem(TYPE_INT, "νοξστς", false);
    WLD_PRD_SRLOCATION = prd->AddItem(TYPE_STRING, "ξμολαγιρ", false);
    WLD_PRD_SPLOCATION = prd->AddItem(TYPE_STRING, "λμολαγιρ", false);
    WLD_PRD_SRZONE = prd->AddItem(TYPE_STRING, "ξοποχεύεξιε", false);
    WLD_PRD_SPZONE = prd->AddItem(TYPE_STRING, "λοποχεύεξιε", false);

    return CheckInit();
}
CWld * Wld;


///////////////////////////////////////////////////////////////////////////////
//world loot ΣΙΣΤΕΝΑ (/lib/misc/world_loot)
int WRL_LOOT;   //νιςοχοκ
int WRL_RACE;   //νιςοχοκ.ςασα LIST(14)
int WRL_TYPE;   //τιπ LIST(49)
int WRL_LEVEL;   //υςοχεξψ STRING
int WRL_SEX;   //πομ LIST(11)
int WRL_CLASS;   //λμασσ STRLIST(13)
int WRL_GOD;   //βοη LIST(28)
int WRL_AGE;   //χοϊςαστ STRLIST(52)
int WRL_OBJECT;   //πςεδνετ INT
int WRL_SHANCE;   //ϋαξσ INT (1...1000)

CWlt::CWlt() {
}

CWlt::~CWlt() {
}
///////////////////////////////////////////////////////////////////////////////

bool CWlt::Initialization(void) {

    WRL_LOOT = Proto->AddItem(TYPE_INT, "νιςοχοκ");
    WRL_RACE  = Proto->AddItem(TYPE_LIST, "ςασα", false, ParserConst.GetList(LIST_RACE));
    WRL_TYPE = Proto->AddItem(TYPE_LIST, "τιπ", false, ParserConst.GetList(LIST_MOB_TYPE));
    WRL_LEVEL = Proto->AddItem(TYPE_STRING, "υςοχεξψ", true);
    WRL_SEX = Proto->AddItem(TYPE_LIST, "πομ", false, ParserConst.GetList(LIST_SEX));
    WRL_CLASS = Proto->AddItem(TYPE_STRLIST, "λμασσ", false, ParserConst.GetList(LIST_CLASS));
    WRL_GOD = Proto->AddItem(TYPE_LIST, "βοη", false, ParserConst.GetList(LIST_GODS));
    WRL_AGE = Proto->AddItem(TYPE_LIST, "χοϊςαστ", false, ParserConst.GetList(LIST_AGE));
    WRL_OBJECT = Proto->AddItem(TYPE_INT, "πςεδνετ", true);
    WRL_SHANCE = Proto->AddItem(TYPE_INT, "ϋαξσ", true);
    return CheckInit();
}

CWlt Wlt;

