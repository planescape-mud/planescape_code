/* $Id$
 *
 * ruffina, 2009
 */
#ifndef REGISTRATION_H
#define REGISTRATION_H

#include <fstream>
#include <string>

using namespace std;

struct char_data;

/*
 * Basic class to put various emails to the spool.
 * Email files are picked up from the spool asynchronously, by cron, 
 * sent and deleted.
 */
struct SpoolFile {
    SpoolFile(struct char_data *ch, const char *suffix);

    bool write() const;

protected:    
    static const char *ext;
    struct char_data *ch;
    const char *suffix;
    string filename;
    
    void setFilename();
    virtual void putSubject(ostream &buf) const = 0;
    virtual void putBody(ostream &buf) const = 0;
};

/*
 * Writes registration email to the spool.
 */
struct RegistrationFile : public SpoolFile {
    RegistrationFile(struct char_data *ch);

protected:
    virtual void putSubject(ostream &buf) const;
    virtual void putBody(ostream &buf) const;
};

/*
 * Writes email with lost password code.
 */
struct LostPasswordFile : public SpoolFile {
    LostPasswordFile(struct char_data *ch);

protected:
    virtual void putSubject(ostream &buf) const;
    virtual void putBody(ostream &buf) const;
};

/*
 * Utility class to validate and assign player e-mail.
 */
struct PlayerMail {
    PlayerMail(const char *arg) : email(arg) { }
    bool valid() const;
    bool goodsize() const;
    void assign(struct char_data *ch) const;
protected:    
    string email;
    static const size_t max_email_size;
};

/*
 * Basic class which holds a randomly generated code string.
 */
struct RandomCode {
    RandomCode(size_t len);
protected:
    string code;
};

/*
 * Create and assign a code for lost password confirmation
 */
struct LostPasswordCode : public RandomCode {
    LostPasswordCode() : RandomCode(8) { }
    void assign(struct char_data *ch) const;
};

/*
 * Create and assign registration code
 */
struct RegistrationCode : public RandomCode {
    RegistrationCode() : RandomCode(8) { }
    void assign(struct char_data *ch) const;
};

#endif
