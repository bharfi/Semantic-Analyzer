
#ifndef __PARSE__H__
#define __PARSE__H__

#include <string>

#include "inputbuf.h"
#include "lexer.h"
using namespace std;

//  Node element for each variable
struct Node
{

    // scope of the variable
    std::string scope_name;

    //Variable's name
    string var_name;

    //Access specifier
    string access;

public:
    Node();
    Node(string, string, string);
    void toString();
};

//Symbol table for storing variables in their respective scopes
class Scope
{

    //name of the scope
    string scope_name;

    // list of variables in scope
    vector<Node *> list{};

    //pointer to nested scopes
    Scope *inner;

    //pointer to outer scope
    Scope *outer;

    //Pointer to global node
    Scope *global;

    vector<string> statementList;

public:
    Scope()
    {
        inner = NULL;
        outer = NULL;
        global = NULL;
    }
    Scope(string, Scope *);
    void setGlobal();
    Scope *getGlobal();
    string getScopeName();
    void inScope(string, string);
    void outScope();
    bool find(Scope *, string);
    Scope *findScope(Scope *, string);
    void statement(string, string);
    vector<string> getStatementList();
};

// Class Containing the parser function with all
// its sub-parser functions
class Parser
{
public:
    void parseProgram();

private:
    //Used to get and unGet token from input
    LexicalAnalyzer lexer;

    // Utility Functions

    TokenType peek(int);
    string idPeek();
    void expect(TokenType);
    void syntaxError();

    //Parse Functions

    void parse_global_vars(Scope *);
    void parse_var_list(Scope *, bool);
    void parse_scope(Scope *);
    void parse_public_vars(Scope *);
    void parse_private_vars(Scope *);
    void parse_stmt_list(Scope *, bool);
    void parse_stmt(Scope *);
};

#endif //__PARSE__H__