/**
 * File: symtable.h
 * ----------- 
 *  This file defines a class for symbol table and scoped table table.
 *
 *  Scoped table is to hold all declarations in a nested scope. It simply
 *  uses the standard C++ map.
 *
 *  Symbol table is implemented as a vector, where each vector entry holds
 *  a pointer to the scoped table.
 */

#ifndef _H_symtable
#define _H_symtable

#include <map>
#include <vector>
#include <iostream>
#include <string.h>
#include "errors.h"
//TODO
#include "ast_decl.h"

using namespace std;

class Decl;
class Stmt;

typedef enum {
  Other,
  Global,
  Loop,
  Switch
} type_of_scope;

enum EntryKind {
  E_FunctionDecl,
  E_VarDecl,
};

struct Symbol {
  char *name;
  Decl *decl;
  EntryKind kind;
  int someInfo;

  Symbol() : name(NULL), decl(NULL), kind(E_VarDecl), someInfo(10) {}
  Symbol(char *n, Decl *d, EntryKind k, int info = 0) :
        name(n),
        decl(d),
        kind(k),
        someInfo(info) {}
};

// This is just a comparator, it's used when declaring SymbolIterator
struct lessStr {
  bool operator()(const char* s1, const char* s2) const
  { return strcmp(s1, s2) < 0; }
};
 
typedef map<const char *, Symbol, lessStr>::iterator SymbolIterator;

// Store symbol for the current scope
class ScopedTable {
  
  public:
    map<const char *, Symbol, lessStr> symbols;

    bool found_conflict;
    ScopedTable(){}
    ~ScopedTable(){}
    /*
     * Insert a new symbol in the current scope
     */
    void insert(Symbol &sym); 
    // void remove(Symbol &sym);
    Symbol *find(const char *name);
};
  
/*
* Symboltable is in charge of keep tracking of scopetable
* in a stack manner
* but we should use a vector to implement
*/ 
class SymbolTable {
  std::vector<ScopedTable *> tables;
  //TODO check the type of scope
  vector<type_of_scope> typeStack;
  //number of scoppedTable
  int size;
  
  public:
    //TODO
    bool return_body_check;
    /* Check if there's parameter
     * In this case, we need to throw an error
     * fun a(int i){
     *    int i;
     * }
     */ 
    bool param_body_check;
    FnDecl* fun;
    int num_table_to_remove;

    SymbolTable();
    ~SymbolTable(){}

    // push in a current scope
    void push(type_of_scope scope_type);
    // pop a current scope
    void pop();
    // call current scope table to insert
    void insert(Symbol &sym);
    void remove(int num, Symbol &sym);
    void setConflict();
    // call current scope table to remove
    // void remove(Symbol &sym);
    // call curent scope table to find
    Symbol *find(const char *name);

    //TODO get scopeType
    type_of_scope getType();
    bool insideLoop();
    bool insideSwitch();

};    
#endif
