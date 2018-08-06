/* $Id$
 *
 * ruffina, 2009
 */
#include <sstream>
#include <string.h>

#include "regexp.h"

#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "registration.h"
#include "planescape.h"
#include "mudfile.h"

const char *password_body="Система восстановления пароля.\n\n"
                          "Вы запросили пароль персонажа %s в \"МПМ Грани Мира\".\n"
                          "Ваш код подтверждения: %s\n"
                          "По вопросам поддержки обращайтесь <support@planescape.ru>\n";
const char *password_subj="\"Восстановление пароля для МПМ Грани Мира\"";

const char *register_body="Вы зарегистрировали персонажа с именем '%s' в МПМ \"Грани Мира\".\n"
                          "Для доступа в игру вам необходимо ввести код потверждения.\n"
                          "Ваш код подтверждения: %s\n";
const char *register_subj="\"Регистрация персонажа в МПМ Грани Мира\"";

/*---------------------------------------------------------------------------
 * SpoolFile
 *--------------------------------------------------------------------------*/
const char * SpoolFile::ext = ".ml";

SpoolFile::SpoolFile(struct char_data *ch, const char *suffix) 
     : ch(ch), suffix(suffix) 
{ 
    setFilename();
}

/*
 * Mail files have the following format:
 * email\n
 * subject\n
 * body
 * All files have .ml extention and purpose-specific suffix (.reg, etc).
 */
bool SpoolFile::write() const 
{
    const char *fname = filename.c_str();
    ofstream ostr;

    log("РЕГИСТРАЦИЯ: Посылаю письмо на '%s', файл '%s'.", GET_EMAIL(ch), fname);

    ostr.open(fname);

    if (!ostr) {
        syserr("ОШИБКА: Не могу создать почтовый файл %s", fname);
        return false;
    }
    
    ostr << GET_EMAIL(ch) << endl;
    putSubject(ostr);
    putBody(ostr);
    ostr.close();

    if (ostr.fail()) {
        syserr("ОШИБКА: Не могу записать почтовый файл %s", fname);
        unlink(fname);
        return false;
    }
    
    chmod(fname, 0600); 
    return true;
}

void SpoolFile::setFilename() 
{
    /*
     * make latin character string out of russian name, as in save_char()
     */
    char name[strlen(GET_PC_NAME(ch)) + strlen(suffix) + 1];
    strcpy(name, GET_PC_NAME(ch));
    for (char *ptr = name; *ptr; ptr++)
        *ptr = LOWER(AtoL(*ptr));
    strcat(name, suffix);
    
    MudFile file(mud->mailSpoolDir, name, ext); 
    filename = file.getPath();
}

/*---------------------------------------------------------------------------
 * RegistrationFile 
 *--------------------------------------------------------------------------*/
RegistrationFile::RegistrationFile(struct char_data *ch) 
           : SpoolFile(ch, ".reg") 
{ 
}

void RegistrationFile::putSubject(ostream &buf) const 
{
    buf << register_subj << endl;
}

void RegistrationFile::putBody(ostream &buf) const 
{
    char body[MAX_STRING_LENGTH];

    snprintf(body, sizeof(body), register_body, 
             ch->player.name, ch->registry_code);
    buf << body;
}

/*---------------------------------------------------------------------------
 * LostPasswordFile 
 *--------------------------------------------------------------------------*/
LostPasswordFile::LostPasswordFile(struct char_data *ch) 
      : SpoolFile(ch, ".pass")  
{ 
}

void LostPasswordFile::putSubject(ostream &buf) const 
{
    buf << password_subj << endl;
}

void LostPasswordFile::putBody(ostream &buf) const 
{
    char body[MAX_STRING_LENGTH];

    snprintf(body, sizeof(body), password_body, 
             ch->player.name, ch->pc()->specials.saved.cAnswer);
    buf << body;
}


/*---------------------------------------------------------------------------
 * PlayerMail 
 *--------------------------------------------------------------------------*/
const size_t PlayerMail::max_email_size = 127;
/*
 * Check email address to be a valid user@hostname.tld string
 */
bool PlayerMail::valid() const
{
    static RegExp emailRE("^[-+%a-z0-9_.]+@([-a-z0-9]+\\.){1,}[a-z]{2,4}$");
    return emailRE.match(email);
}

bool PlayerMail::goodsize() const
{
    return email.size() <= max_email_size;
}

void PlayerMail::assign(struct char_data *ch) const
{
    strncpy(GET_EMAIL(ch), email.c_str(), max_email_size);
}

/*---------------------------------------------------------------------------
 * RandomCode 
 *--------------------------------------------------------------------------*/
RandomCode::RandomCode(size_t codelen)
{
    static const char randbuf[] = "1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVQXYZ_";
    static const size_t randlen = sizeof(randbuf);
    code = "";

    for (size_t i = 0; i < codelen; i++)
        code += randbuf[rand() % (randlen - 1)];
}

/*---------------------------------------------------------------------------
 * LostPasswordCode 
 *--------------------------------------------------------------------------*/
void LostPasswordCode::assign(struct char_data *ch) const
{
    strcpy(ch->pc()->specials.saved.cAnswer, code.c_str());
}

/*---------------------------------------------------------------------------
 * RegistrationCode 
 *--------------------------------------------------------------------------*/
void RegistrationCode::assign(struct char_data *ch) const
{
    ch->registry_code = str_dup(code.c_str());
}
