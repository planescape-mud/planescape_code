/* $Id: ceval.cpp,v 1.1.4.4.6.4 2008/03/26 14:55:50 rufina Exp $
 *
 * ruffina, 2004
 */
#include "regexp.h"
#include "dlfilestream.h"
#include "exception.h"
#include "characterwrapper.h"
#include "wrappermanager.h"
#include "sysdep.h"
#include "structs.h"
#include "../runtime/utils.h"
#include "comm.h"

using Scripting::CodeSource;

void cmd_plugin(struct char_data *ch, DLString &args) 
{

}

void cmd_ftp(struct char_data *ch, DLString &args)
{
    DLString fileName = args.getOneArgument();
    DLString subj = args;

    if (fileName.empty() || subj.empty()) {
        send_to_char("Формат: ftp <имя_файла> <тема_сценария>\r\n", ch);
        return;
    }

    static RegExp fileRE("^[a-z0-9_]+$");
    if (!fileRE.match(fileName)) {
        send_to_char("В имени файла допустимы только символы англ алфавита, цифры и _\r\n", ch);
        return;
    }

    DLFileStream file("/home/fenia/ftp_root", fileName, ".f++");
    ostringstream buf;

    try {
        file.toStream(buf);
    } catch (const ExceptionDBIO &ex) {
        send_to_char("Ошибка открытия файла.\r\n", ch);
        return;
    }

    CodeSource &cs = CodeSource::manager->allocate();

    cs.author = GET_NAME(ch);
    cs.name = subj;
    cs.content = buf.str();

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

void cmd_eval(struct char_data *ch, DLString &args) 
{
    if (args.empty( )) {
        send_to_char("Синтаксис: eval <expression>\r\n", ch);
        return;
    }

    Register thiz = WrapperManager::getThis( )->getWrapper( ch );
    
    try {
        CodeSource &cs = CodeSource::manager->allocate();
        
        cs.author = GET_NAME(ch);
        cs.name = "<eval command>";

        cs.content = args;
        cs.eval(thiz);
    
    } catch (::Exception e) {
        DLString what = e.what();
        send_to_char( what.substitute('{', '&').c_str(), ch );
    }
}

bool cmd_hook(struct char_data *ch, char *ccmd, char *carg)
{
    DLString args(carg), cmd(ccmd);
    
    if (!IS_GOD(ch)) 
        return false;

    if (cmd == "eval") {
        cmd_eval(ch, args);
        return true;
    }

    if (cmd == "plugin") {
        cmd_plugin(ch, args);
        return true;
    }

    if (cmd == "ftp") {
        cmd_ftp(ch, args);
        return true;
    }

    return false;
}
