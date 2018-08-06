/* $Id: feniaparser.h,v 1.4.2.4.6.3 2009/10/11 18:35:35 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: feniaparser.h,v 1.4.2.4.6.3 2009/10/11 18:35:35 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#ifndef __FENIAPARSER_H__
#define __FENIAPARSER_H__

#include <sstream>

using namespace std;

#include "register-decl.h"
#include "nodes.h"
#include "exp-tree.h"
#include "ref-tree.h"
#include "stmt-tree.h"
#include "function.h"
#include "codesource.h"

using namespace Scripting;
    
struct SValue {
    Register reg;
    ExpNode::Pointer exp;
    ReferenceNode::Pointer ref;
    StmtNode::Pointer stmt;
    ExpNodeList::Pointer exps;
    StmtNodeList::Pointer stmts;
    ArgNames::Pointer args;
    CodeSourceRef csrc;
    
    DLString str;
    Lex::id_t id;
    int num;
};

class FeniaParser : public yyFlexLexer {
    friend int yyparse(void *dummy, FeniaParser *parser);
    friend int yyparse();
    CodeSource &source;
    ExpNode::Pointer input;
    std::basic_ostringstream<char> ostr;
    std::basic_ostringstream<char> errors;
public:
    FeniaParser(istream &, CodeSource &src);
    
    void compile( );
    Register eval( Register &thiz );

protected:
    int lex( SValue * );
    void error( char* message );
    int parse( ); 
};

extern FeniaParser *parser;

#endif /* __FENIAPARSER_H__ */
