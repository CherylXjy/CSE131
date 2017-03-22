/* File: ast_decl.cc
 * -----------------
 * Implementation of Decl node classes.
 */
#include "ast_decl.h"
#include "ast_type.h"
#include "ast_stmt.h"
#include "symtable.h"        
         
/***********************************************************
******************                  ************************
******************    Checking Start     *******************
******************                  ************************
************************************************************/
void VarDecl::Check() {

    Symbol sym((char *)this->id->name, this, E_VarDecl);
    // if(!st->param_body_check){

    Symbol* s = st->find((const char*)this->id->name);
    // cout<< "Vardecl check: "<< this->id->name << "   " << s<< endl;
    if(s){
        if(s->someInfo!=Global){
            ReportError::DeclConflict(sym.decl,s->decl);
            // st->remove(st->num_table_to_remove, *s);
            st->setConflict();
        }
    }
    // }
    st->insert(sym);

    if(assignTo) {
        assignTo->Check();
        if( (!this->type->IsEquivalentTo(assignTo->type)) ){
            if ((!this->type->IsError()) && (!assignTo->type->IsError())){
                ReportError::InvalidInitialization(this->id, this->GetType(), assignTo->type);
                // this->type = Type::errorType;
            }
        }
    }
}

void FnDecl::Check() {
    Symbol sym((char *)this->id->name, this, E_FunctionDecl);
    st->insert(sym);

    if(this->body) {
        st->param_body_check = true;
        st->return_body_check = false;
        st->push(Other);

        for(int i = 0; i < this->GetFormals()->NumElements(); i++) { 
            this->GetFormals()->Nth(i)->Check();
        }
        this->body->Check();

        if(!(st->return_body_check || (this->returnType->IsEquivalentTo(Type::voidType)))) {
            ReportError::ReturnMissing(this);
        }

        st->pop();
    }
}
/***********************************************************
******************                  ************************
******************    Checking  end    *********************
******************                  ************************
************************************************************/
Decl::Decl(Identifier *n) : Node(*n->GetLocation()) {
    Assert(n != NULL);
    (id=n)->SetParent(this); 
}

VarDecl::VarDecl(Identifier *n, Type *t, Expr *e) : Decl(n) {
    Assert(n != NULL && t != NULL);
    (type=t)->SetParent(this);
    if (e) (assignTo=e)->SetParent(this);
    typeq = NULL;
}

VarDecl::VarDecl(Identifier *n, TypeQualifier *tq, Expr *e) : Decl(n) {
    Assert(n != NULL && tq != NULL);
    (typeq=tq)->SetParent(this);
    if (e) (assignTo=e)->SetParent(this);
    type = NULL;
}

VarDecl::VarDecl(Identifier *n, Type *t, TypeQualifier *tq, Expr *e) : Decl(n) {
    Assert(n != NULL && t != NULL && tq != NULL);
    (type=t)->SetParent(this);
    (typeq=tq)->SetParent(this);
    if (e) (assignTo=e)->SetParent(this);
}
  
void VarDecl::PrintChildren(int indentLevel) { 
   if (typeq) typeq->Print(indentLevel+1);
   if (type) type->Print(indentLevel+1);
   if (id) id->Print(indentLevel+1);
   if (assignTo) assignTo->Print(indentLevel+1, "(initializer) ");
}

FnDecl::FnDecl(Identifier *n, Type *r, List<VarDecl*> *d) : Decl(n) {
    Assert(n != NULL && r!= NULL && d != NULL);
    (returnType=r)->SetParent(this);
    (formals=d)->SetParentAll(this);
    body = NULL;
    returnTypeq = NULL;
}

FnDecl::FnDecl(Identifier *n, Type *r, TypeQualifier *rq, List<VarDecl*> *d) : Decl(n) {
    Assert(n != NULL && r != NULL && rq != NULL&& d != NULL);
    (returnType=r)->SetParent(this);
    (returnTypeq=rq)->SetParent(this);
    (formals=d)->SetParentAll(this);
    body = NULL;
}

void FnDecl::SetFunctionBody(Stmt *b) { 
    (body=b)->SetParent(this);
}

void FnDecl::PrintChildren(int indentLevel) {
    if (returnType) returnType->Print(indentLevel+1, "(return type) ");
    if (id) id->Print(indentLevel+1);
    if (formals) formals->PrintAll(indentLevel+1, "(formals) ");
    if (body) body->Print(indentLevel+1, "(body) ");
}