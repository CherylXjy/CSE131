/*
 * Symbol table implementation
 *
 */
#include "symtable.h"
#include <map>
#include <vector>
#include <iostream>
#include <string.h>
#include "errors.h"

/*
 * Insert a new symbol in the current scope
 */

void ScopedTable::insert(Symbol &sym){
    //if symbol already existed
    // Symbol* ifExisted = find(sym.name);
    // if(ifExisted){
    //     // cout<< "inside if Existed" <<endl;
    //     if(found_conflict){
    //         found_conflict=false;
    //         SymbolIterator itrt = symbols.find(sym.name);
    //         symbols.erase(itrt);
    //     }
    //     else{
    //         SymbolIterator itrt = symbols.find(sym.name);
    //         symbols.erase(itrt);
    //         ReportError::DeclConflict(sym.decl,ifExisted->decl);
    //     }
    // }
    symbols.insert(std::pair<const char*, Symbol>(sym.name, sym));
}

Symbol *ScopedTable::find(const char *name){
    if(symbols.count(name)>0){
        return &symbols.at(name);
    }
    return NULL;
}


//here is for SymbolTable
SymbolTable::SymbolTable(){
    size = 0;
    param_body_check = false;
}

void SymbolTable::push(type_of_scope scope_type){
    tables.push_back(new ScopedTable());
    typeStack.push_back(scope_type);
    size++;
}

void SymbolTable::pop(){
    if(size<=0)return;
    tables.pop_back();
    typeStack.pop_back();
    size--;
}

void SymbolTable::insert(Symbol &sym){
    //get the current scoped table and call the insert method on that
    tables.at(size-1)->insert(sym);
    //TODO if sym is function then fun equal to sym
    if(sym.kind==E_FunctionDecl){
        fun = (FnDecl*)sym.decl;
    }
}

// void SymbolTable::remove(int num, Symbol &sym){
//     ScopedTable * scope_table = tables.at(num);
//     SymbolIterator itrt  = scope_table->symbols.find(sym.name);
//     scope_table->symbols.erase(itrt);
// }

Symbol *SymbolTable::find(const char *name){
    Symbol* ret_sym = NULL;
    for(int i=size-1;i>=0;i--){
        ret_sym = tables.at(i)->find(name);
        if(ret_sym!=NULL){
            // if(typeStack.at(i)==Global){
            //     ret_sym->someInfo = Global;
            //     num_table_to_remove = i;
            // }
            break;
        }
    }
    return ret_sym;
}

// void SymbolTable::setConflict(){
//     tables.at(size-1)->found_conflict = true;
// }

type_of_scope SymbolTable::getType(){
    return typeStack.at(size-1);
}

// bool SymbolTable::insideLoop(){
//     for(int i = size-1; i >= 0; i--) {
//         if (typeStack.at(i) == Loop) {
//             return true;
//         }
//     }
//     return false;
// }

// bool SymbolTable::insideSwitch(){
//     for(int i = size-1; i >= 0; i--) {
//         if (typeStack.at(i) == Switch) {
//             return true;
//         }
//     }
//     return false;
// }

