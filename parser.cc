//Student Name: Bharath Prabakaran
// Class: CSE 340
// ASSIGNMENT 2
// Due: July 30 2021

#include <iostream>
#include <istream>
#include <vector>
#include "lexer.h"
#include "inputbuf.h"
#include "parse.h"
using namespace std;

//*Node member functions

//Node constuctor
// @param string var
// @param string scope
Node::Node(string var, string scope, string access_spec)
{
    var_name = var;
    scope_name = scope;
    access = access_spec;
}

//*Scope Member Functions

// Constructor for local scope
// @param string Scope Name
// @param Scope* pointer to Outer Scope
Scope::Scope(string name, Scope *outerScope)
{
    scope_name = name;
    Scope *globalPoint;
    globalPoint = outerScope->getGlobal();
    global = globalPoint;
    if (globalPoint->inner != NULL)
    {
        outer = outerScope;
        outer->inner = this;
        inner = NULL;
    }
    else
    {
        outer = global;
        global->inner = this;
        inner = NULL;
    }
}

//Sets the global scope
void Scope::setGlobal()
{
    this->list.clear();
    this->global = this;
    this->scope_name = "::";
    this->outer = NULL;
}

// Gets the global scope
Scope *Scope::getGlobal()
{
    return global;
}

// returns the ScopeName
string Scope::getScopeName()
{
    return scope_name;
}

//Returns the Statement List
vector<string> Scope::getStatementList()
{
    return global->statementList;
}

// For when the variable is declared in a scope/globally
//@param string Variable Name
//@param string access
void Scope::inScope(string variable, string access_spec)
{
    //add new variable to symbole table
    Node *newNode = new Node(variable, this->scope_name, access_spec);
    list.push_back(newNode);
}

// For when the variable goes out of scope
void Scope::outScope()
{
    vector<Node *>::iterator i;
    for (i = list.begin(); i != list.end(); i++)
    {
        delete *i;
    }
    list.clear();
}

// find the variable in the scope
// @param Scope* name of the statment scope
//@param string Variable Name
bool Scope::find(Scope *scope, string name) // Doesnt go into For Loop
{
    vector<Node *> vars;
    vars = this->list;
    vector<Node *>::iterator i;
    for (i = vars.begin(); i != vars.end(); i++)
    {
        if ((*i)->var_name == name)
        {
            if ((*i)->scope_name == scope->scope_name)
            {
                return true;
            }
            else if ((*i)->access.compare("private") != 0)
            {
                return true;
            }
            else if ((*i)->scope_name.compare("::") == 0)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    return false;
}

//Search all scopes greater than or equal to given scope to find given variable
// @param Scope* Scope Name of statement scope
// @param string Variable Name
// @return Scope pointer to the innermost scope with given variable
Scope *Scope::findScope(Scope *scopeName, string variable)
{
    Scope *tempScope;
    Scope *found;
    found = NULL;
    tempScope = global;
    if (tempScope->find(scopeName, variable))
    {
        found = global;
    }
    tempScope = global->inner;
    while (tempScope != NULL && tempScope != this->inner) // change the 'this' part of the program if neccesary
    {
        if (tempScope->find(scopeName, variable))
        {
            found = tempScope;
        }
        try
        {
            if (tempScope->inner != NULL)
            {
                tempScope = tempScope->inner;
            }
            else
                break;
        }
        catch (...)
        {
            break;
        }
    }
    return found;
}

// statment function to resolve lhs and rhs
// @param string lhs
// @param string rhs
void Scope::statement(string lhs, string rhs)
{

    Scope *lScope;
    Scope *rScope;
    string lScopeName;
    string rScopeName;
    lScope = findScope(this, lhs);
    rScope = findScope(this, rhs);
    if (lScope != NULL)
    {
        lScopeName = lScope->getScopeName();
    }
    else
        lScopeName = "?";
    if (rScope != NULL)
    {
        rScopeName = rScope->getScopeName();
    }
    else
        rScopeName = "?";
    if (lScopeName.compare("::") != 0)
    {
        lScopeName += ".";
    }
    if (rScopeName.compare("::") != 0)
    {
        rScopeName += ".";
    }
    string statement;
    statement = lScopeName + lhs + " = " + rScopeName + rhs;
    this->global->statementList.push_back(statement);
    //cout <<"\n"<<lScopeName<< lhs << " = " << rScopeName << rhs << endl;
}

//*Parser Functions

// * parser Utility Functions

/**
 * Checks the type of the token n spaces in front of the current Token.
 * @tparam Utility  
 * @param int Number of indexes you want to peek forward.
 * @return TokenType of the token peeked at.
 */
TokenType Parser::peek(int n)
{

    Token t[n];
    int i;
    for (i = 0; i < n; i++)
    {
        t[i] = lexer.GetToken();
    }
    for (--i; i >= 0; i--)
    {
        lexer.UngetToken(t[i]);
    }
    return t[n - 1].token_type;
}

/**
 * Gets the string vale of the peeked token
 * @tparam Utility  
 * @return String of ID
 */
string Parser::idPeek()
{
    Token temp = lexer.GetToken();
    lexer.UngetToken(temp);
    return temp.lexeme;
}

/**
 * 
 * Prints "Syntax Error".
 * @tparam Utility 
 * 
 */
void Parser::syntaxError()
{
    cout << "Syntax Error\n";
    exit(0); // the exit is required
}

/**
 * Consumes the token if it is of the correct type, else calls syntaxError().
 * Utility Function.
 * @tparam Utility 
 * @param type The type of the token to be consumed.
*
 */
void Parser::expect(TokenType type)
{
    Token tmp;
    tmp = lexer.GetToken();
    if (tmp.token_type != type)
    {
        syntaxError();
    }
}

//* Sub Parser functions

/**
 * 
 * Calls var_ list with global vars if any.
 * @note Exhausts SEMICOLON after var_list 
 * @note Exhausts ID if ONLY one global_var
 * @param Scope* Pointer to the gobal scope
 * 
 */
void Parser::parse_global_vars(Scope *globalPoint)
{
    TokenType t1, t2;
    t1 = peek(1);
    t2 = peek(2);
    if (t1 == ID && t2 == SEMICOLON) // Only if there is only one global_var
    {

        parse_var_list(globalPoint, false);
        expect(SEMICOLON);
    }
    else if (t1 == ID && t2 == COMMA) // if there are multiple global_vars
    {
        parse_var_list(globalPoint, false);
        expect(SEMICOLON);
    }
    else if (t1 == ID && t2 == LBRACE) // when there arre no global variables
    {
        return;
    }
    else
    {
        syntaxError();
    }
}

//
//  Adds each variable to the scope's list using inScope()
// @note Exhausts ID and COMMA
// @note the ID's are the Global/Local Variables
// @param Scope* Pointer to Current Scope
// @param bool true if private variable list
//
void Parser::parse_var_list(Scope *currentScope, bool isPrivate)
{

    TokenType t1, t2;
    t1 = peek(1);
    t2 = peek(2);

    if (t1 == ID && t2 == COMMA) //* For each ID in the list, except the last one
    {
        string newVar;
        newVar = idPeek();
        if (isPrivate) //private variable
        {
            currentScope->inScope(newVar, "private");
        }
        else //public variable
        {
            currentScope->inScope(newVar, "public");
        }
        expect(ID); //! var_name
        expect(COMMA);
        parse_var_list(currentScope, isPrivate);
    }
    else if (t1 == ID && t2 == SEMICOLON) //* For Last ID in the var_list or only one ID
    {
        string newVar;
        newVar = idPeek();
        if (isPrivate) //private variable
        {
            currentScope->inScope(newVar, "private");
        }
        else //public variable
        {
            currentScope->inScope(newVar, "public");
        }
        expect(ID); //! var_name
    }
    else
    {
        syntaxError();
    }
}

//
// Creates a new scope which is nested in the given Scope* , and calls outScope when done
// @note Exhausts ID, LBRACE and RBRACE- the ID is the scope name
// @param Scope* the Scope in which this scope in nested in
//
void Parser::parse_scope(Scope *outerScope)
{
    TokenType t1, t2;
    t1 = peek(1);
    t2 = peek(2);
    if (t1 == ID && t2 == LBRACE)
    {

        string nameOfScope;
        nameOfScope = idPeek();
        Scope tempScope(nameOfScope, outerScope); // Declaring a new Scope

        expect(ID);
        expect(LBRACE);
        parse_public_vars(&tempScope);
        parse_private_vars(&tempScope);
        parse_stmt_list(&tempScope, true);
        expect(RBRACE);
        tempScope.outScope();
    }
    else
    {
        syntaxError();
    }
}

/**
 * calls parse_var_list()  with isPrivate false for its variables
 * @note Exhausts PUBLIC, COLON, and a SEMICOLON 
 * @param Scope* the current scope
 */
void Parser::parse_public_vars(Scope *currentScope)
{
    TokenType t1, t2;
    t1 = peek(1);
    t2 = peek(2);
    if (t1 == PUBLIC && t2 == COLON)
    {
        expect(PUBLIC);
        expect(COLON);
        parse_var_list(currentScope, false); // ! public variables
        expect(SEMICOLON);
    }
    else if (t1 == PRIVATE)
    { // when there are no public variables
        return;
    }
    else if (t1 == ID)
    { // when there are no public or private  variables
        return;
    }
    else
    {
        syntaxError();
    }
}

//
// calls parse_var_list()  with isPrivate true for its variables
// @note Exhausts PRIVATE, COLON, and a SEMICOLON
// @param Scope* the current scope
//
void Parser::parse_private_vars(Scope *currentScope)
{
    TokenType t1, t2;
    t1 = peek(1);
    t2 = peek(2);
    if (t1 == PRIVATE && t2 == COLON)
    {
        expect(PRIVATE);
        expect(COLON);
        parse_var_list(currentScope, true); // ! private variables
        expect(SEMICOLON);
    }
    else if (t1 == ID)
    { // when there are no private  variables
        return;
    }
    else
    {
        syntaxError();
    }
}

//
// Just parses over the statement list
// @note Exhausts Nothing
// @param Scope* the current scope
// @param bool to check if it is the first statement
void Parser::parse_stmt_list(Scope *currentScope, bool firstTime)
{
    TokenType t1, t2, t3, t4, t5; // ! the large number of peeks might cause a problem
    t1 = peek(1);
    t2 = peek(2);
    t3 = peek(3);
    t4 = peek(4);
    if (t1 == ID && t2 == EQUAL && t3 == ID && t4 == SEMICOLON) // Multiple statements
    {
        parse_stmt(currentScope);
        parse_stmt_list(currentScope, false);
    }
    // else if (t1 == ID && t2 == EQUAL && t3 == ID && t4 == SEMICOLON) // single statement left (or only stament)
    // {
    //     parse_stmt(currentScope);
    // }
    else if (t1 == ID && t2 == LBRACE) //new scope
    {
        TokenType t6, t7;
        parse_stmt(currentScope);
        t6 = peek(1);
        t7 = peek(2);
        while ((t6 == ID && t7 == LBRACE) || (t6 == ID && t7 == EQUAL))
        {
            parse_stmt(currentScope);
            t6 = peek(1);
            t7 = peek(2);
        }
    }
    else if (t1 == RBRACE && firstTime) // stmt list doesnt contain epsilon
    {
        syntaxError();
    }
    else if (t1 == RBRACE && !firstTime) // stmt list doesnt contain epsilon
    {
        return;
    }
    else
    {
        syntaxError();
    }
}

//
// Parses each statement and calls scope.statement() for each
// @note Exhausts ID, EQUAL, ID, SEMICOLON - IDs are the variables(ID1 = ID2;)
//@param Scope* the current scope
//
void Parser::parse_stmt(Scope *currentScope)
{

    TokenType t1, t2, t3, t4;
    t1 = peek(1);
    t2 = peek(2);
    t3 = peek(3);
    t4 = peek(4);
    if (t1 == ID && t2 == EQUAL && t3 == ID && t4 == SEMICOLON) // statement
    {
        string lhs, rhs;
        lhs = idPeek();
        expect(ID); // ! this ID is LHS
        expect(EQUAL);
        rhs = idPeek();
        expect(ID); // ! this ID is the RHS
        expect(SEMICOLON);
        currentScope->statement(lhs, rhs);
    }
    else if (t1 == ID && t2 == LBRACE) // when there is a new scope
    {
        parse_scope(currentScope); //! new nested scope
    }
    else
    {
        syntaxError();
    }
}

//* Main Parser Function

//
// The main parser program called from lexer.cc
//
void Parser::parseProgram()
{

    TokenType tmp;
    tmp = peek(1);
    Scope globalPoint;
    globalPoint.setGlobal();

    //globalPoint = globalPoint.getGlobal();
    if (tmp == ID)
    {
        parse_global_vars(&globalPoint); // parses variables and adds them to the list of the Scope* linked list
        parse_scope(&globalPoint);
        expect(ERROR);
        vector<string> allStatements;
        allStatements = globalPoint.getStatementList();
        vector<string>::iterator i;
        for (i = allStatements.begin(); i != allStatements.end(); i++)
        {
            cout << (*i) << endl;
        }
    }
    else
    {
        syntaxError();
    }
}
