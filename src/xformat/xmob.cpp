/**************************************************************************
    νπν "ηÒΑΞΙ νΙÒΑ" (Σ) 2002-2003 αΞΔÒΕΚ εÒΝΙΫΙΞ
    ϊΑΗÒΥΪΛΑ ΖΑΚΜΟΧ ΙΗÒΟΧΟΗΟ ΝΙÒΑ
 **************************************************************************/

#include "xmob.h"
#include "xbody.h"
#include "xspells.h"
#include "parser_id.h"
#include "strlib.h"
#include "expr.h"


///////////////////////////////////////////////////////////////////////////////
// οΠÒΕΔΕΜΕΞΙΕ ΟΣΞΟΧΞΩΘ ΠΑÒΑΝΕΤÒΟΝ
int MOB_NUMBER;  //νοξστς
int MOB_ALIAS;  //σιξοξινω
int MOB_NAME;  //ινρ
int MOB_LINE;  //στςολα
int MOB_DESCRIPTION; //οπισαξιε
int MOB_ADDITION; //δοπομξιτεμψξο
int MOB_PROPERTIES; //νσχοκστχα
int MOB_ADDONS;  //νδοπομξεξιρ
int MOB_AFFECTS; //νόζζελτω
int MOB_SEX;  //πομ
int MOB_LEVEL;  //υςοχεξψ
int MOB_CLASS;  //πςοζεσσιρ
int MOB_RACE;  //ςασα
int MOB_TYPE;  //τιπ
int MOB_ALIGN;  //ξαλμοξοστι
int MOB_LAWS;  //ϊαλοξξοστψ
int MOB_EXP;  //οπωτ
int MOB_LIMIT;  //πςεδεμ
int MOB_HITPOINTS; //φιϊξψ
int MOB_AC;  //βςοξρ
int MOB_HITROLL; //αταλα
int MOB_HIT1;  //υδας1
int MOB_DAMAGE1; //χςεδ1
int MOB_COUNT1;  //αταλι1
int MOB_HIT2;  //υδας2
int MOB_DAMAGE2; //χςεδ2
int MOB_COUNT2;  //αταλι2
int MOB_STR;  //σιμα
int MOB_CON;  //τεμοσμοφεξιε
int MOB_DEX;  //μοχλοστψ
int MOB_INT;  //ςαϊυν
int MOB_WIS;  //νυδςοστψ
int MOB_CHA;  //οβαρξιε
int MOB_SIZE;  //ςαϊνες
int MOB_HEIGHT;  //ςοστ
int MOB_WEIGHT;  //χεσ
int MOB_POSITION; //πομοφεξιε
int MOB_MOVED;  //πεςενεύεξιε
int MOB_MOVESTR; //πεςεδχιφεξιε
int MOB_GOLD;  //δεξψηι
int MOB_WIMP;  //τςυσοστψ
int MOB_SKILL;  //υνεξιε
int MOB_EQ;  //όλιπιςοχλα
int MOB_INV;  //ιξχεξταςψ
int MOB_DEATH;  //ποσνεςτξο
int MOB_TATOO;  //τατυιςοχλα
int MOB_SAVE;  //ϊαύιτα
int MOB_FOLLOW;  //σμεδυετ
int MOB_HELPED;  //πονούξιλι-ηςυππα
int MOB_DEST;  //πυτψ
int MOB_SCRIPT;  //σλςιπτ
int MOB_SHOP;  //ναηαϊιξ
int MOB_SPECIAL; //σπεγυδας
int MOB_LACKY;  //υδαώα
int MOB_SPEED;  //σλοςοστψ
int MOB_LIKEWORK; //ςαβοτα
int MOB_GOD;  //βοη
int MOB_FRACTION; //ζςαλγιρ
int MOB_RANG;           //ςαξη
int MOB_RANK;           //ςαξλ
int MOB_HORSE;  //μοϋαδψ
int MOB_SDEATH;  //σνεςτψ
int MOB_ALARM;  //οποχεύεξιε
int MOB_POWER;  //υσιμεξιε
int MOB_BODY;  //τεμο
int MOB_CLASSES; //πςοζεσσιρ
int MOB_AGE;  //χοϊςατ

// δΟΠΟΜΞΙΤΕΜΨΞΩΕ ΠΑÒΑΝΕΤÒΩ
int MOB_CLASS_TYPE; //πςοζεσσιρ.λμασσ
int MOB_CLASS_LEVEL; //πςοζεσσιρ.υςοχεξψ

int MOB_BODY_POSITION; //τεμο.ποϊιγιρ
int MOB_BODY_SNAME; //τεμο.σπςξαϊχαξιε
int MOB_BODY_NAME; //τεμο.ξαϊχαξιε
int MOB_BODY_CHANCE; //τεμο.σμοφξοστψ
int MOB_BODY_KDAM; //τεμο,πςοώξοστψ

int MOB_SDT_CORPSE; //σνεςτψ.τςυπ
int MOB_SDT_SCRIPT; //σνεςτψ.δεκστχιε
int MOB_SDT_DCHAR; //σνεςτψ.σιηςολ
int MOB_SDT_DROOM;      //σνεςτψ.σμολαγιρ
int MOB_SDT_DAMAGE; //σνεςτψ.τχςεδ
int MOB_SDT_HIT; //σνεςτψ.σχςεδ
int MOB_SDT_TARGET; //σνεςτψ.γεμψ
int MOB_SDT_CHAR; //σνεςτψ.ιηςολ
int MOB_SDT_ROOM; //σνεςτψ.οσταμψξων

int MOB_ADD_KEY; //δοπομξιτεμψξο.λμΰώ
int MOB_ADD_TEXT; //δοπομξιτεμψξο.τελστ

int MOB_SPEC_TYPE; //σπεγυδας.τιπ
int MOB_SPEC_POS; //σπεγυδας.πομοφεξιε
int MOB_SPEC_PERC; //σπεγυδας.ϋαξσ
int MOB_SPEC_HIT; //σπεγυδας.ςαϊςυϋεξιε
int MOB_SPEC_SPELL; //σπεγυδας.ϊαλμιξαξιε
int MOB_SPEC_POWER; //σπεγυδας.νούξοστψ
int MOB_SPEC_DAMAGE; //σπεγυδας.χςεδ
int MOB_SPEC_VICTIM; //σπεγυδας.ιηςολυ
int MOB_SPEC_ROOM; //σπεγυδας.οσταμψξων
int MOB_SPEC_SAVE; //σπεγυδας.σοθςαξεξιε
int MOB_SPEC_PROP; //σξεγυδας.σχοκστχα

int MOB_SHOP_SELL; //ναηαϊιξ.πςοδαφα (int)
int MOB_SHOP_BUY; //ναηαϊιξ.πολυπλα (int)
int MOB_SHOP_REPAIR; //ναηαϊιξ.ςενοξτ (int)
int MOB_SHOP_QUALITY; //ναηαϊιξ.ναστεςστχο (int)
int MOB_SHOP_TYPE; //ναηαϊιξ.τοχας (list)
int MOB_SHOP_LIST; //ναηαϊιξ.νεξΰ (list)
int MOB_SHOP_ANTI; //ναηαϊιξ.ϊαπςετω (bitvector)

int MOB_ALARM_HELPER; //οποχεύεξιε.πονοϋξιλι
int MOB_ALARM_CHAR;   //οποχεύεξιε.ιηςολ
int MOB_ALARM_ROOM;   //οποχεύεξιε.μολαγιρ
int MOB_ALARM_LEVEL;  //οποχεύεξιε.υςοχεξψ

int MOB_PERIOD;  //πεςιοδιώξοστψ
int MOB_PRD_START; //ξαώαμο
int MOB_PRD_STOP; //λοξεγ
int MOB_PRD_TIME; //πεςιοδ
int MOB_PRD_TARGET; //γεμψ
int MOB_PRD_CHAR; //ιηςολυ
int MOB_PRD_ROOM; //λονξατε

int MOB_WELCOME; //πςιχεστχιε
int MOB_GOODBYE; //πςούαξιε
int MOB_QUEST;  //ϊαδαξιε
int MOB_QNUMBER; //ϊαδαξιε.ξονες
int MOB_QNAME;  //ϊαδαξιε.ξαϊχαξιε
int MOB_QEXPR;  //ϊαδαξιε.υσμοχιε
int MOB_QPRE;  //ϊαδαξιε.λςατλοε
int MOB_QTEXT;  //ϊαδαξιε.τελστ
int MOB_QCOMPLITE; //ϊαδαξιε.χωπομξεξιε
int MOB_QMULTY;  //ϊαδαξιε.νξοηοςαϊοχοε
int MOB_QREQUEST; //ϊαδαξιε.υσμοχιρ
int MOB_QR_VAR;  //ϊαδαξιε.υσμοχιρ.πεςενεξξαρ
int MOB_QRV_TITLE; //ϊαδαξιε.υσμοχξιρ.πεςενεξξαρ.οτοβςαφεξιε
int MOB_QRV_NAME; //ϊαδαξιε.υσμοχξιρ.πεςενεξξαρ.ξαϊχαξιε
int MOB_QRV_VALUE; //ϊαδαξιε.υσμοχξιρ.πεςενεξξαρ.ϊξαώεξιε
int MOB_QR_MOBILES;     //ϊαδαξιε.υσμοχξιρ.νοξστς
int MOB_QR_OBJECTS; //ϊαδαξιε.υσμοχξιρ.πςεδνετ
int MOB_QR_FOLLOWERS; //ϊαδαξιε.υσμοχξιρ.ποσμεδοχατεμψ
int MOB_QACCEPT; //ϊαδαξιε.ξαώαμο
int MOB_QDONE;  //ϊαδαξιε.ϊαχεςϋεξιε
int MOB_QSCRIPT;  //δεκστχιε
int MOB_QSCR_NUMBER; //δεκστχιε.ξονες
int MOB_QSCR_EXPR; //δεκστχιε.υσμοχιε
int MOB_QSCR_TEXT; //δεκστχιε.τελστ
int MOB_QSCR_SCRIPT;  //δεκστχιε.δεκστχιε

//σΠΕΓΛΟΝΑΞΔΩ
int MOB_COMMAND; //σπεγλοναξδα
int MOB_COMMAND_NO; //σπεγλοναξδα.λοναξδα
int MOB_COMMAND_ARG; //σπεγλοναξδα.αςηυνεξτ
int MOB_COMMAND_TOOL; //σπεγλοναξδα.ιξστςυνεξτ
int MOB_COMMAND_ACTIVE;
int MOB_COMMAND_ARG_OBJ; //σπεγλοναξδα.αςηυνεξτ.πςεδνετ
int MOB_COMMAND_ARG_ARG; //σπεγλοναξδα.αςηυνεξτ.στςολα
int MOB_COMMAND_ARG_MOB; //σπεγλοναξδα.αςηυνεξτ.νοξστς
int MOB_COMMAND_ARG_ERR; //σπεγλοναξδα.αςηυνεξτ.οϋιβλα
int MOB_COMMAND_SCRIPT; //σπεγλοναξδα.δεκστχιε
int MOB_COMMAND_EXTRACT; //σπεγλοναξδα.ςασσωπαξιε
int MOB_COMMAND_LOAD;     //σπεγλοναξδα.ϊαηςυϊλα
int MOB_COMMAND_LOAD_ROOM; //σπεγλοναξδα.ϊαηςυϊλα.χμολαγιΰ
int MOB_COMMAND_LOAD_CHAR; //σπεγλοναξδα.ϊαηςυϊλα.χιξχεξταςψ
int MOB_COMMAND_ACTIVE_ROOM;
int MOB_COMMAND_MESS_ROOM;
int MOB_COMMAND_MESS_CHAR;

//πΕÒΕΘΧΑΤ
int MOB_INTERCEPTION; //πεςεθχατ
int MOB_INT_COMMAND; // πεςεθχατ.λοναξδα (LIST)
int MOB_INT_STOP; // πεςεθχατ.στοπ (extern int)
int MOB_INT_MESSPLAYER; // πεςεθχατ.ιηςολ (STRING)
int MOB_INT_MESSVICTIM; // πεςεθχατ.φεςτχα (STRING)
int MOB_INT_MESSOTHER; // πεςεθχατ.οσταμψξων (STRING)
int MOB_INT_MESSROOM; // πεςεθχατ.λονξατα (STRING)
int MOB_INT_SCRIPT; // πεςεθχατ.δεκστχιε (STRING)
int MOB_INT_SARG; // πεςεθχατ.σαςηυνεξτ (STRING)

//σοβωτιε
int MOB_EVENT;  //σοβωτιε
int MOB_EVN_TYPE; //σοβωτιε.τιπ
int MOB_EVN_ARG; //σοβωτιε.αςηυνεξτ
int MOB_EVN_SCRIPT; //σοβωτιε.δεκστχιε

CMob::CMob() {
}

CMob::~CMob() {
}
///////////////////////////////////////////////////////////////////////////////

bool CMob::Initialization(void) {
    MOB_NUMBER = Proto->AddItem(TYPE_INT, "νοξστς");
    MOB_ALIAS = Proto->AddItem(TYPE_STRING, "σιξοξινω");
    MOB_NAME = Proto->AddItem(TYPE_STRING, "ινρ");
    MOB_LINE = Proto->AddItem(TYPE_STRING, "στςολα");
    MOB_DESCRIPTION = Proto->AddItem(TYPE_STRING, "οπισαξιε", false);
    MOB_ADDITION = Proto->AddItem(TYPE_STRUCT, "δοπομξιτεμψξο", false);
    CItemProto *add = Proto->GetItem(MOB_ADDITION);
    MOB_ADD_KEY = add->AddItem(TYPE_STRING, "λμΰώ");
    MOB_ADD_TEXT = add->AddItem(TYPE_STRING, "τελστ");
    MOB_PROPERTIES = Proto->AddItem(TYPE_VECTOR, "νσχοκστχα", false,
                                    ParserConst.GetVector(VEC_MOB_PROP));
    MOB_ADDONS = Proto->AddItem(TYPE_VECTOR, "νδοπομξεξιρ", false,
                                ParserConst.GetVector(VEC_MOB_ADDS));
    MOB_AFFECTS = Proto->AddItem(TYPE_VECTOR, "νόζζελτω", false,
                                 ParserConst.GetVector(VEC_MOB_AFF));
    MOB_SEX = Proto->AddItem(TYPE_LIST, "πομ", true,
                             ParserConst.GetList(LIST_SEX));
    MOB_LEVEL =  Proto->AddItem(TYPE_INT, "υςοχεξψ", false);
    MOB_CLASS = Proto->AddItem(TYPE_LIST, "πςοζεσσιρ", false,
                               ParserConst.GetList(LIST_CLASS));

    MOB_CLASSES =  Proto->AddItem(TYPE_STRUCT, "λμασσ", false);
    CItemProto *classes = Proto->GetItem(MOB_CLASSES);
    MOB_CLASS_TYPE = classes->AddItem(TYPE_LIST, "τιπ", true,
                                      ParserConst.GetList(LIST_CLASS));
    MOB_CLASS_LEVEL = classes->AddItem(TYPE_INT, "υςοχεξψ", true);

    MOB_RACE = Proto->AddItem(TYPE_LIST, "ςασα", true,
                              ParserConst.GetList(LIST_RACE));
    MOB_TYPE = Proto->AddItem(TYPE_LIST, "τιπ", false,
                              ParserConst.GetList(LIST_MOB_TYPE));
    MOB_ALIGN =  Proto->AddItem(TYPE_INT, "ξαλμοξοστψ");
    MOB_LAWS =  Proto->AddItem(TYPE_INT, "ϊαλοξξοστψ", false);
    MOB_EXP = Proto->AddItem(TYPE_INT, "οπωτ");
    MOB_LIMIT = Proto->AddItem(TYPE_INT, "πςεδεμ", false);
    MOB_HITPOINTS = Proto->AddItem(TYPE_RANDOM, "φιϊξψ");
    MOB_AC = Proto->AddItem(TYPE_INT, "βςοξρ");
    MOB_HITROLL =  Proto->AddItem(TYPE_INT, "αταλα");
    MOB_HIT1 = Proto->AddItem(TYPE_LIST, "υδας1", true,
                              ParserConst.GetList(LIST_MOB_HIT));
    MOB_DAMAGE1 = Proto->AddItem(TYPE_RANDOM, "χςεδ1", true);
    MOB_COUNT1 = Proto->AddItem(TYPE_INT, "αταλι1");
    MOB_HIT2 = Proto->AddItem(TYPE_LIST, "υδας2", false,
                              ParserConst.GetList(LIST_MOB_HIT));
    MOB_DAMAGE2 = Proto->AddItem(TYPE_RANDOM, "χςεδ2", false);
    MOB_COUNT2 = Proto->AddItem(TYPE_INT, "αταλι2", false);
    MOB_STR = Proto->AddItem(TYPE_INT, "σιμα");
    MOB_CON =  Proto->AddItem(TYPE_INT, "τεμοσμοφεξιε");
    MOB_DEX = Proto->AddItem(TYPE_INT, "μοχλοστψ");
    MOB_INT = Proto->AddItem(TYPE_INT, "ςαϊυν");
    MOB_WIS = Proto->AddItem(TYPE_INT, "νυδςοστψ");
    MOB_CHA = Proto->AddItem(TYPE_INT, "οβαρξιε");
    MOB_SIZE = Proto->AddItem(TYPE_INT, "ςαϊνες");
    MOB_HEIGHT =  Proto->AddItem(TYPE_INT, "ςοστ", false);
    MOB_WEIGHT =  Proto->AddItem(TYPE_INT, "χεσ", false);
    MOB_POSITION = Proto->AddItem(TYPE_LIST, "πομοφεξιε", true,
                                  ParserConst.GetList(LIST_POSITIONS));
    MOB_AGE = Proto->AddItem(TYPE_LIST, "χοϊςαστ", false,
                             ParserConst.GetList(LIST_AGE));
    MOB_MOVED = Proto->AddItem(TYPE_LIST, "πεςενεύεξιε", false,
                               ParserConst.GetList(LIST_MOB_MOVE));
    MOB_MOVESTR = Proto->AddItem(TYPE_STRING, "πεςεδχιφεξιε", false);
    MOB_GOLD = Proto->AddItem(TYPE_RANDOM, "δεξψηι", false);
    MOB_WIMP = Proto->AddItem(TYPE_INT, "τςυσ", false);
    MOB_SKILL =  Proto->AddItem(TYPE_STRLIST, "υνεξιε", false,
                                ParserConst.GetList(LIST_SKILLS));
    MOB_EQ = Proto->AddItem(TYPE_STRLIST, "όλιπιςοχλα", false,
                            ParserConst.GetList(LIST_EQ_POS));
    MOB_INV = Proto->AddItem(TYPE_STRLIST, "ιξχεξταςψ", false);
    MOB_DEATH = Proto->AddItem(TYPE_STRLIST, "ποσνεςτξο", false);
    MOB_TATOO = Proto->AddItem(TYPE_STRLIST, "τατυιςοχλα", false,
                               ParserConst.GetList(LIST_EQ_POS));
    MOB_SAVE = Proto->AddItem(TYPE_STRLIST, "ϊαύιτα", false,
                              ParserConst.GetList(LIST_SAVE_TYPES));

    MOB_FOLLOW = Proto->AddItem(TYPE_INT, "σμεδυετ", false);
    MOB_HELPED = Proto->AddItem(TYPE_SCRIPT, "ηςυππα", false);
    MOB_DEST = Proto->AddItem(TYPE_SCRIPT, "πυτψ", false);
    MOB_SCRIPT = Proto->AddItem(TYPE_SCRIPT, "σλςιπτ", false);

    MOB_SHOP = Proto->AddItem(TYPE_STRUCT, "ναηαϊιξ", false);
    CItemProto * shop = Proto->GetItem(MOB_SHOP);
    MOB_SHOP_SELL = shop->AddItem(TYPE_INT, "πςοδαφα", true);
    MOB_SHOP_BUY = shop->AddItem(TYPE_INT, "πολυπλα", true);
    MOB_SHOP_REPAIR = shop->AddItem(TYPE_INT, "ςενοξτ", true);
    MOB_SHOP_QUALITY = shop->AddItem(TYPE_INT, "ναστεςστχο", true);
    MOB_SHOP_TYPE = shop->AddItem(TYPE_STRLIST, "τοχας", true,
                                  ParserConst.GetList(LIST_OBJ_TYPES));
    MOB_SHOP_LIST = shop->AddItem(TYPE_SCRIPT, "σπισολ", false);
    MOB_SHOP_ANTI = shop->AddItem(TYPE_VECTOR, "ϊαπςετω", false,
                                  ParserConst.GetVector(VEC_ANTI));

    MOB_SPECIAL = Proto->AddItem(TYPE_STRUCT, "σπεγυδας", false);
    CItemProto *spec = Proto->GetItem(MOB_SPECIAL);
    MOB_SPEC_TYPE = spec->AddItem(TYPE_LIST, "τιπ", true,
                                  ParserConst.GetList(LIST_MOB_SPEC_TYPE));
    MOB_SPEC_POS = spec->AddItem(TYPE_STRLIST, "πομοφεξιε", false, ParserConst.GetList(LIST_POSITIONS));
    MOB_SPEC_PERC = spec->AddItem(TYPE_INT, "ϋαξσ", false);
    MOB_SPEC_HIT = spec->AddItem(TYPE_LIST, "ςαϊςυϋεξιε", false,
                                 ParserConst.GetList(LIST_DAMAGE));
    MOB_SPEC_SPELL = spec->AddItem(TYPE_LIST, "ϊαλμιξαξιε", false,
                                   ParserConst.GetList(LIST_SPELLS));
    MOB_SPEC_POWER = spec->AddItem(TYPE_INT, "νούξοστψ", false);
    MOB_SPEC_DAMAGE = spec->AddItem(TYPE_RANDOM, "χςεδ", false);
    MOB_SPEC_VICTIM = spec->AddItem(TYPE_STRING, "ιηςολυ", false);
    MOB_SPEC_ROOM = spec->AddItem(TYPE_STRING, "οσταμψξων", false);
    MOB_SPEC_SAVE = spec->AddItem(TYPE_STRLIST, "σοθςαξεξιε", false, ParserConst.GetList(LIST_SAVE_TYPES));
    MOB_SPEC_PROP = spec->AddItem(TYPE_VECTOR, "σχοκστχα", false, ParserConst.GetVector(VEC_MOB_SPEC));

    MOB_LACKY = Proto->AddItem(TYPE_INT, "υδαώα", false);
    MOB_SPEED = Proto->AddItem(TYPE_INT, "σλοςοστψ", false);
    MOB_LIKEWORK = Proto->AddItem(TYPE_INT, "ςαβοτα", false);
    MOB_GOD   = Proto->AddItem(TYPE_LIST, "βοη", false,
                               ParserConst.GetList(LIST_GODS));
    MOB_FRACTION = Proto->AddItem(TYPE_LIST, "ζςαλγιρ", false,
                                  ParserConst.GetList(LIST_FRACTION));
    MOB_RANG     = Proto->AddItem(TYPE_LIST, "ςαξη", false,
                                  ParserConst.GetList(LIST_RANK));

    MOB_RANK     = Proto->AddItem(TYPE_INT, "ςαξλ", false);

    MOB_HORSE    = Proto->AddItem(TYPE_INT, "μοϋαδψ", false);

    MOB_SDEATH   = Proto->AddItem(TYPE_STRUCT, "σνεςτψ", false);
    CItemProto *sdt = Proto->GetItem(MOB_SDEATH);
    MOB_SDT_CORPSE = sdt->AddItem(TYPE_INT, "τςυπ", false);
    MOB_SDT_SCRIPT = sdt->AddItem(TYPE_INT, "δεκστχιε", false);
    MOB_SDT_DCHAR  = sdt->AddItem(TYPE_STRING, "σιηςολ", false);
    MOB_SDT_DROOM  = sdt->AddItem(TYPE_STRING, "σμολαγιρ", false);
    MOB_SDT_DAMAGE = sdt->AddItem(TYPE_LIST, "τχςεδ", false,
                                  ParserConst.GetList(LIST_DAMAGE));
    MOB_SDT_HIT    = sdt->AddItem(TYPE_RANDOM, "σχςεδ", false);
    MOB_SDT_TARGET = sdt->AddItem(TYPE_LIST, "γεμψ", false,
                                  ParserConst.GetList(LIST_TARGET));
    MOB_SDT_CHAR   = sdt->AddItem(TYPE_STRING, "ιηςολ", false);
    MOB_SDT_ROOM   = sdt->AddItem(TYPE_STRING, "οσταμψξων", false);

    MOB_ALARM = Proto->AddItem(TYPE_STRUCT, "οποχεύεξιε", false);
    CItemProto *alarm = Proto->GetItem(MOB_ALARM);
    MOB_ALARM_HELPER = alarm->AddItem(TYPE_SCRIPT, "πονοϋξιλι");
    MOB_ALARM_CHAR   = alarm->AddItem(TYPE_STRING, "ιηςολ", false);
    MOB_ALARM_ROOM   = alarm->AddItem(TYPE_STRING, "μολαγιρ", false);
    MOB_ALARM_LEVEL  = alarm->AddItem(TYPE_INT, "φιϊξψ", false);
    MOB_POWER = Proto->AddItem(TYPE_INT, "υσιμεξιε", false);
    MOB_BODY = Proto->AddItem(TYPE_STRUCT, "τεμο", false);
    CItemProto *body = Proto->GetItem(MOB_BODY);
    MOB_BODY_POSITION = body->AddItem(TYPE_LIST, "ποϊιγιρ", true,
                                      ParserConst.GetList(LIST_BODY_POSITION));
    MOB_BODY_SNAME = body->AddItem(TYPE_LIST, "σπςξαϊχαξιε", false, ParserConst.GetList(LIST_BODY_NAME));
    MOB_BODY_NAME = body->AddItem(TYPE_STRING, "ξαϊχαξιε", false);
    MOB_BODY_CHANCE = body->AddItem(TYPE_INT, "σμοφξοστψ", true);
    MOB_BODY_KDAM = body->AddItem(TYPE_INT, "πςοώξοστψ", false);

    MOB_PERIOD = Proto->AddItem(TYPE_STRUCT, "πεςιοδιώξοστψ", false);
    CItemProto *prd = Proto->GetItem(MOB_PERIOD);
    MOB_PRD_START = prd->AddItem(TYPE_INT, "ξαώαμο", false);
    MOB_PRD_STOP = prd->AddItem(TYPE_INT, "λοξεγ", false);
    MOB_PRD_TIME  = prd->AddItem(TYPE_INT, "πεςιοδ", false);
    MOB_PRD_TARGET = prd->AddItem(TYPE_LIST, "γεμψ", false, ParserConst.GetList(LIST_TARGET));
    MOB_PRD_CHAR = prd->AddItem(TYPE_STRING, "ιηςολυ", false);
    MOB_PRD_ROOM = prd->AddItem(TYPE_STRING, "λονξατε", false);


    MOB_WELCOME = Proto->AddItem(TYPE_STRING, "πςιχεστχιε", false);
    MOB_GOODBYE = Proto->AddItem(TYPE_STRING, "πςούαξιε", false);
    MOB_QUEST = Proto->AddItem(TYPE_STRUCT, "ϊαδαξιε", false);
    CItemProto *quest = Proto->GetItem(MOB_QUEST);
    MOB_QNUMBER = quest->AddItem(TYPE_INT, "ξονες", true);
    MOB_QNAME = quest->AddItem(TYPE_STRING, "ξαϊχαξιε", true);
    MOB_QEXPR = quest->AddItem(TYPE_EXPR, "υσμοχιε", false);
    MOB_QPRE = quest->AddItem(TYPE_STRING, "λςατλοε", true);
    MOB_QTEXT = quest->AddItem(TYPE_STRING, "τελστ", true);
    MOB_QCOMPLITE = quest->AddItem(TYPE_STRING, "χωπομξεξιε", false);
    MOB_QMULTY = quest->AddItem(TYPE_INT, "νξοηοςαϊοχοε", false);
    MOB_QREQUEST = quest->AddItem(TYPE_STRUCT, "τςεβοχαξιρ", true);
    CItemProto *request = quest->GetItem(MOB_QREQUEST);
    MOB_QR_VAR = request->AddItem(TYPE_STRUCT, "πεςενεξξαρ", false);
    CItemProto *rvar = request->GetItem(MOB_QR_VAR);
    MOB_QRV_TITLE = rvar->AddItem(TYPE_STRING, "οτοβςαφεξιε", false);
    MOB_QRV_NAME = rvar->AddItem(TYPE_STRING, "ξαϊχαξιε", false);
    MOB_QRV_VALUE = rvar->AddItem(TYPE_STRING, "ϊξαώεξιε", false);
    MOB_QR_MOBILES = request->AddItem(TYPE_STRLIST, "νοξστς", false);
    MOB_QR_OBJECTS = request->AddItem(TYPE_STRLIST, "πςεδνετ", false);
    MOB_QR_FOLLOWERS = request->AddItem(TYPE_STRLIST, "ποσμεδοχατεμψ", false);
    MOB_QACCEPT = quest->AddItem(TYPE_INT, "ξαώαμο", false);
    MOB_QDONE = quest->AddItem(TYPE_INT, "ϊαχεςϋεξιε", false);

    MOB_COMMAND  = Proto->AddItem(TYPE_STRUCT, "σπεγλοναξδα", false);
    CItemProto *command = Proto->GetItem(MOB_COMMAND);
    MOB_COMMAND_NO = command->AddItem(TYPE_LIST, "λοναξδα", true, ParserConst.GetList(LIST_COMMANDS));
    MOB_COMMAND_ARG = command->AddItem(TYPE_STRUCT, "αςηυνεξτ", false);
    MOB_COMMAND_TOOL = command->AddItem(TYPE_LIST, "ιξστςυνεξτ", false, ParserConst.GetList(LIST_TOOLS));
    MOB_COMMAND_ACTIVE = command->AddItem(TYPE_STRING, "αλτιχαγιρ", false);
    MOB_COMMAND_ACTIVE_ROOM = command->AddItem(TYPE_STRING, "αλτιχαγιρ_μολαγιρ", false);

    CItemProto *comarg = command->GetItem(MOB_COMMAND_ARG);
    MOB_COMMAND_ARG_OBJ = comarg->AddItem(TYPE_INT, "πςεδνετ", false);
    MOB_COMMAND_ARG_ARG = comarg->AddItem(TYPE_STRING, "στςολα", false);
    MOB_COMMAND_ARG_MOB = comarg->AddItem(TYPE_INT, "νοξστς", false);
    MOB_COMMAND_ARG_ERR = comarg->AddItem(TYPE_STRING, "οϋιβλα", false);
    MOB_COMMAND_SCRIPT = command->AddItem(TYPE_INT, "δεκστχιε", false);
    MOB_COMMAND_EXTRACT = command->AddItem(TYPE_INT, "ςασσωπαξιε", false);
    MOB_COMMAND_LOAD = command->AddItem(TYPE_STRUCT, "ϊαηςυϊλα", false);
    CItemProto *cload = command->GetItem(MOB_COMMAND_LOAD);
    MOB_COMMAND_LOAD_ROOM = cload->AddItem(TYPE_INT, "χμολαγιΰ", false);
    MOB_COMMAND_LOAD_CHAR = cload->AddItem(TYPE_INT, "χιξχεξταςψ", false);
    MOB_COMMAND_MESS_CHAR = cload->AddItem(TYPE_STRING, "ιηςολ", false);
    MOB_COMMAND_MESS_ROOM = cload->AddItem(TYPE_STRING, "μολαγιρ", false);


    MOB_INTERCEPTION = Proto->AddItem(TYPE_STRUCT, "πεςεθχατ", false);
    CItemProto *intercept = Proto->GetItem(MOB_INTERCEPTION);
    MOB_INT_COMMAND = intercept->AddItem(TYPE_LIST, "λοναξδα", true,
                                         ParserConst.GetList(LIST_COMMANDS));
    MOB_INT_STOP = intercept->AddItem(TYPE_INT, "στοπ", false);
    MOB_INT_MESSPLAYER = intercept->AddItem(TYPE_STRING, "ιηςολ", false);
    MOB_INT_MESSVICTIM = intercept->AddItem(TYPE_STRING, "φεςτχα", false);
    MOB_INT_MESSOTHER  = intercept->AddItem(TYPE_STRING, "οσταμψξων", false);
    MOB_INT_MESSROOM   = intercept->AddItem(TYPE_STRING, "λονξατα", false);
    MOB_INT_SCRIPT   = intercept->AddItem(TYPE_INT, "δεκστχιε", false);
    MOB_INT_SARG   = intercept->AddItem(TYPE_STRING, "σαςηυνεξτ", false);


    MOB_QSCRIPT = Proto->AddItem(TYPE_STRUCT, "δεκστχιε", false);
    CItemProto *qscript = Proto->GetItem(MOB_QSCRIPT);
    MOB_QSCR_NUMBER = qscript->AddItem(TYPE_INT, "ξονες", true);
    MOB_QSCR_EXPR   = qscript->AddItem(TYPE_EXPR, "υσμοχιε", false);
    MOB_QSCR_TEXT   = qscript->AddItem(TYPE_STRING, "τελστ", true);
    MOB_QSCR_SCRIPT = qscript->AddItem(TYPE_INT, "δεκστχιε", true);

    MOB_EVENT = Proto->AddItem(TYPE_STRUCT, "ςεαλγιρ", false);
    CItemProto *evnt = Proto->GetItem(MOB_EVENT);
    MOB_EVN_TYPE = evnt->AddItem(TYPE_LIST, "τιπ", true, ParserConst.GetList(LIST_EVENTS));
    MOB_EVN_ARG = evnt->AddItem(TYPE_INT, "αςηυνεξτ", true);
    MOB_EVN_SCRIPT = evnt->AddItem(TYPE_INT, "δεκστχιε", true);

    return CheckInit();
};

CMob * Mob;

///////////////////////////////////////////////////////////////////////////////
