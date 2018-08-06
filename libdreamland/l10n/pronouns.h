/* $Id: pronouns.h,v 1.1.2.3 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2007
 */
#ifndef L10N_PRONOUNS_H
#define L10N_PRONOUNS_H

#include "dlstring.h"
#include "grammar_entities.h"
#include "noun.h"

namespace Grammar {

class Noun;

class Pronoun {
public:    
    typedef const char * Cases [Case::MAX];
    typedef Cases Persons [Person::MAX];

    virtual ~Pronoun();
};

// (кто? кого? кому?)
class PersonalPronoun : public Pronoun {
public:    
    virtual ~PersonalPronoun();

    DLString decline(const Noun &who, const Person &p, const Case &c) const;

private:
    const Persons &persons;
};

// (чей? чьим?)
class PosessivePronoun : public Pronoun {
public:    
    typedef Persons PosessionGenders [MultiGender::MAX];

    virtual ~PosessivePronoun();

    DLString decline(const Noun &item, const Noun &owner, const Person &p, const Case &c) const;

private:
    const PosessionGenders &posessions;
};

// (некто, нечто)
class IndefinitePronoun : public Pronoun {
public:    
    typedef Cases AnimacyCases [Animacy::MAX];
    
    IndefinitePronoun(const AnimacyCases &acases);
    virtual ~IndefinitePronoun();

    DLString decline(const Case &c, const Animacy &a) const;

private:
    const AnimacyCases &acases;
};

class IndefiniteNoun : public Noun {
public:
    typedef ::Pointer<IndefiniteNoun> Pointer;

    IndefiniteNoun(const IndefinitePronoun::AnimacyCases &acases, const Animacy &a);

    virtual Gender getGender() const;
    virtual Number getNumber() const;
    virtual DLString decline(const Case &c) const;

protected:
    IndefinitePronoun ipron;
    Animacy a;
};

}

#endif
