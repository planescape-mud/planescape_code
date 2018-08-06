/* $Id$
 *
 * ruffina, 2009
 */
#include <sstream>

#include "logstream.h"
#include "exception.h"
#include "dl_ctype.h"

#include "fenia/register-impl.h"
#include "fenia/codesource.h"
#include "feniamanager.h"

#include "structs.h"
#include "utils.h"
#include "interpreter.h"
#include "comm.h"
#include "handler.h"

using namespace Scripting;

#define MAX_CS_SIZE 65565

ACMD(do_write);
ACMD(do_look);

static bool arg_is_number(char *arg, int &num)
{
    skip_spaces(&arg);

    if (!is_number(arg))
        return false;

    num = atoi(arg);
    return true;
}

static bool arg_is_board(char *arg, struct obj_data *board)
{
    char tmp[MAX_STRING_LENGTH];

    one_argument(arg, tmp);

    if (!*tmp || (!isname(tmp, board->name) && !isfullname(tmp, board->names)))
        return false;

    return true;
}

static inline bool fenia_check(struct char_data *ch)
{
    if (!FeniaManager::wrapperManager) {
        send_to_char("Работа со сценариями невозможна - в мире нету фени!\n\r", ch);
        return false;
    }

    return true;
}

static inline bool fenia_security(struct char_data *ch)
{
    return ch->desc && IS_GOD(ch);
}

static void cs_subj_set(struct char_data *ch, const char *arg)
{
    static IdRef ID_CS_SUBJ("csSubj");
    Register thiz = FeniaManager::wrapperManager->getWrapper(ch);
    thiz[ID_CS_SUBJ] = Register(arg);
}

static DLString cs_subj_get(struct char_data *ch)
{
    static IdRef ID_CS_SUBJ("csSubj");
    Register thiz = FeniaManager::wrapperManager->getWrapper(ch);
    return (*thiz[ID_CS_SUBJ]).toString();
}

void cs_header(const CodeSource &cs, ostringstream &buf)
{
    char header[MAX_STRING_LENGTH];

    sprintf(header, "[%5lu] &g%12s&n: %s &K(используется %d функц.)&x\r\n",
                    cs.getId(),
                    cs.author.c_str(),
                    cs.name.c_str(),
                    cs.functions.size());
    buf << header;
}

void cs_content(const CodeSource &cs, ostringstream &buf)
{
    string::const_iterator c;
    
    /* феневый оператор && не должен распознаваться как цвет */
    for (c = cs.content.begin(); c != cs.content.end(); c++) {
        if (*c == '&')
            buf << '&';

        buf << *c;
    }

    buf << endl;
}

void cs_post(struct char_data *ch, char *arg)
{
    if (!fenia_security(ch) || !fenia_check(ch))
        return;

    CodeSource &cs = CodeSource::manager->allocate();

    cs.author = GET_NAME(ch);
    cs.name = cs_subj_get(ch);
    cs.content = arg;

    send_to_char("Пробуем выполнить сценарий...\r\n", ch);
    Register thiz = FeniaManager::wrapperManager->getWrapper(ch);

    try {
        cs.eval(thiz);
        send_to_char("Ок.\r\n", ch);
    }
    catch (const ::Exception &e) {
        send_to_char("Ошибка!\r\n", ch);
        send_to_char(e.what(), ch);
    }
}


void cs_cancel(struct char_data *ch)
{
    // XXX clear subj
    send_to_char("Сценарий удален.\r\n", ch);
}

int cs_write(struct char_data *ch, char *arg, struct obj_data *board)
{
    char **cs_body;
    
    if (!fenia_check(ch))
        return 0;

    skip_spaces(&arg);

    if (!*arg) {
        send_to_char("Укажите тему сценария.\n\r", ch);
        return 1;
    }

    send_to_char("Можете писать сценарий. (/s записать /h помощь)\r\n\r\n", ch);
    act("$n начал$g что-то выцарапывать на $o5.", TRUE, ch, board, 0, TO_ROOM);

    // to destroy buffer on lost connect and to handle /s editor command
    SET_BIT(PLR_FLAGS(ch, PLR_SCRIPTING), PLR_SCRIPTING); 

    cs_subj_set(ch, arg);
    CREATE(cs_body, char *, 1);

    string_write(ch->desc, cs_body, MAX_CS_SIZE, 0, NULL);
    return 1;
}

int cs_list(struct char_data *ch, char *arg, struct obj_data *board)
{
    ostringstream buf;
    CodeSource::Manager::iterator i;
    
    if (!arg_is_board(arg, board))
        return 0;

    if (CodeSource::manager->empty()) {
        act("На $o5 совсем пусто.", TRUE, ch, board, 0, TO_CHAR);
        return 1;
    }

    send_to_charf(ch, "&YСценарии&n (всего %zu): \r\n",
                  CodeSource::manager->size());
    
    for (i = CodeSource::manager->begin(); i != CodeSource::manager->end(); i++)
        cs_header(*i, buf);

    page_string(ch->desc, buf.str().c_str(), 1);
    return 1;
}

int cs_read(struct char_data *ch, char *arg, struct obj_data *board)
{
    ostringstream buf;
    int num;
    CodeSource::Manager::iterator i;
    
    if (!arg_is_number(arg, num)) {
        send_to_char("Укажите номер сценария.\r\n", ch);
        return 1;
    }

    i = CodeSource::manager->find(num);
    if (i == CodeSource::manager->end()) {
        send_to_char("Сценарий с таким номером не найден.\r\n", ch);
        return 1;
    }

    cs_header(*i, buf);
    cs_content(*i, buf);

    page_string(ch->desc, buf.str().c_str(), 1);
    return 1;
}

SPECIAL(scriptboard) 
{
    struct obj_data *board = (obj_data *)me;
    int subcmd = cmd_info[cmd].subcmd;

    if (!fenia_security(ch))
        return 0;

    if (CMD_PTR == do_write) 
        return cs_write(ch, argument, board);

    if (CMD_PTR == do_look && (subcmd == SCMD_LOOK || subcmd == SCMD_EXAMINE)) 
        return cs_list(ch, argument, board);

    if (CMD_PTR == do_look && (subcmd == SCMD_READ)) 
        return cs_read(ch, argument, board);

    return 0;
}

