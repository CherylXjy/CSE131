/* File: ast_stmt.cc
 * -----------------
 * Implementation of statement node classes.
 */
#include "ast_stmt.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "ast_expr.h"
#include "errors.h"
#include "symtable.h"

/***********************************************************
******************                  ************************
******************    Checking Start     *******************
******************                  ************************
************************************************************/
void Program::Check() {
    /* pp3: here is where the semantic analyzer is kicked off.
     *      The general idea is perform a tree traversal of the
     *      entire program, examining all constructs for compliance
     *      with the semantic rules.  Each node can have its own way of
     *      checking itself, which makes for a great use of inheritance
     *      and polymorphism in the node classes.
     */
    //TODO Create a global scope
    st->push(Global);

    // sample test - not the actual working code
    // replace it with your own implementation
    if ( decls->NumElements() > 0 ) {
      for ( int i = 0; i < decls->NumElements(); ++i ) {
        Decl *d = decls->Nth(i);
        /* !!! YOUR CODE HERE !!!
         * Basically you have to make sure that each declaration is 
         * semantically correct.
         */
        d->Check();
      }
    }
    st->pop();
}

void StmtBlock::Check(){
    int should_pop = 0;
    int i;
    if(!st->param_body_check){
        st->push(Other);
        should_pop = 1;
    }
    else{
        st->param_body_check = false;
    }
    if (decls->NumElements() > 0 ) {
        for (i = 0;i<decls->NumElements();++i) {
            decls->Nth(i)->Check();
        }
    }
    if (stmts->NumElements() > 0 ) {
        for ( i = 0;i<stmts->NumElements();++i) {
            stmts->Nth(i)->Check();
        }
    }

    if(should_pop){
        st->pop();
    }
    
}

void Stmt::Check() {
    st->param_body_check = false;
}

void DeclStmt::Check() {
    decl->Check();
}

void ConditionalStmt::Check() {
    st->push(Other);
    body->Check();
    st->pop();
}


void ForStmt::Check() {
    st->param_body_check = true;
    st->push(Loop);
    init->Check();
    test->Check();

    if(test->type!=(Type::boolType)) {
        // printf("here2\n");
        ReportError::TestNotBoolean(test);
        test->type = Type::errorType;
    }

    if(step) {
        step->Check();
    }

    body->Check();
    st->pop();
}

void WhileStmt::Check() {

    st->push(Loop);
    st->param_body_check = true;
    // cout <<"this is test"<< endl;
    // cout << test << endl;
    test->Check(); //test is expression
    // cout << "test type is: "<<test->type << endl;
    if(!test->type->IsEquivalentTo(Type::boolType)) {
        // printf("here1\n");
        ReportError::TestNotBoolean(test);
        test->type = Type::errorType;
    }

    body->Check();
    st->pop();
}

void IfStmt::Check() {
    st->push(Other);
    st->param_body_check = true;
    test->Check(); //test is expression

    if(!test->type->IsEquivalentTo(Type::boolType)) {
        // printf("here3\n");
        ReportError::TestNotBoolean(test);
        test->type = Type::errorType;
    }
    body->Check(); //body is statement
    st->pop();
    st->push(Other);

    if(elseBody) {
        elseBody->Check();
    }
    st->pop();
}

void ContinueStmt::Check() {
    if(!st->insideLoop()) {
        ReportError::ContinueOutsideLoop(this);
    }
}

void BreakStmt::Check() {
    if(!st->insideLoop() && !st->insideSwitch()) {
        ReportError::BreakOutsideLoop(this);
    }
}


void ReturnStmt::Check() {
    st->return_body_check = true;
    if(expr) {
        expr->Check();
        if((!expr->type->IsError())&&(!st->fun->GetType()->IsEquivalentTo(expr->type))) {
            ReportError::ReturnMismatch(this, expr->type, st->fun->GetType());
        }
    }else {
        if(!st->fun->GetType()->IsEquivalentTo(Type::voidType) ) {
            ReportError::ReturnMismatch(this, Type::voidType, st->fun->GetType());
        }
    }
}

void Case::Check() {
    label->Check(); stmt->Check();
}

void Default::Check() {
    stmt->Check();
}

void SwitchStmt::Check() {
    st->param_body_check = true;
    st->push(Switch);

    expr->Check();

    if (cases){
        for(int i=0;i<cases->NumElements();i++) {
            cases->Nth(i)->Check();
        }
        if(def){
            def->Check();
        }
    }
    st->pop();
}
/***********************************************************
******************                  ************************
******************    Checking  end    *********************
******************                  ************************
************************************************************/
Program::Program(List<Decl*> *d) {
    Assert(d != NULL);
    (decls=d)->SetParentAll(this);
}

void Program::PrintChildren(int indentLevel) {
    decls->PrintAll(indentLevel+1);
    printf("\n");
}

StmtBlock::StmtBlock(List<VarDecl*> *d, List<Stmt*> *s) {
    Assert(d != NULL && s != NULL);
    (decls=d)->SetParentAll(this);
    (stmts=s)->SetParentAll(this);
}

void StmtBlock::PrintChildren(int indentLevel) {
    decls->PrintAll(indentLevel+1);
    stmts->PrintAll(indentLevel+1);
}

DeclStmt::DeclStmt(Decl *d) {
    Assert(d != NULL);
    (decl=d)->SetParent(this);
}

void DeclStmt::PrintChildren(int indentLevel) {
    decl->Print(indentLevel+1);
}

ConditionalStmt::ConditionalStmt(Expr *t, Stmt *b) { 
    Assert(t != NULL && b != NULL);
    (test=t)->SetParent(this); 
    (body=b)->SetParent(this);
}

ForStmt::ForStmt(Expr *i, Expr *t, Expr *s, Stmt *b): LoopStmt(t, b) { 
    Assert(i != NULL && t != NULL && b != NULL);
    (init=i)->SetParent(this);
    step = s;
    if ( s )
      (step=s)->SetParent(this);
}

void ForStmt::PrintChildren(int indentLevel) {
    init->Print(indentLevel+1, "(init) ");
    test->Print(indentLevel+1, "(test) ");
    if ( step )
      step->Print(indentLevel+1, "(step) ");
    body->Print(indentLevel+1, "(body) ");
}

void WhileStmt::PrintChildren(int indentLevel) {
    test->Print(indentLevel+1, "(test) ");
    body->Print(indentLevel+1, "(body) ");
}

IfStmt::IfStmt(Expr *t, Stmt *tb, Stmt *eb): ConditionalStmt(t, tb) { 
    Assert(t != NULL && tb != NULL); // else can be NULL
    elseBody = eb;
    if (elseBody) elseBody->SetParent(this);
}

void IfStmt::PrintChildren(int indentLevel) {
    if (test) test->Print(indentLevel+1, "(test) ");
    if (body) body->Print(indentLevel+1, "(then) ");
    if (elseBody) elseBody->Print(indentLevel+1, "(else) ");
}

ReturnStmt::ReturnStmt(yyltype loc, Expr *e) : Stmt(loc) { 
    expr = e;
    if (e != NULL) expr->SetParent(this);
}

void ReturnStmt::PrintChildren(int indentLevel) {
    if ( expr ) 
      expr->Print(indentLevel+1);
}

SwitchLabel::SwitchLabel(Expr *l, Stmt *s) {
    Assert(l != NULL && s != NULL);
    (label=l)->SetParent(this);
    (stmt=s)->SetParent(this);
}

SwitchLabel::SwitchLabel(Stmt *s) {
    Assert(s != NULL);
    label = NULL;
    (stmt=s)->SetParent(this);
}

void SwitchLabel::PrintChildren(int indentLevel) {
    if (label) label->Print(indentLevel+1);
    if (stmt)  stmt->Print(indentLevel+1);
}

SwitchStmt::SwitchStmt(Expr *e, List<Stmt *> *c, Default *d) {
    Assert(e != NULL && c != NULL && c->NumElements() != 0 );
    (expr=e)->SetParent(this);
    (cases=c)->SetParentAll(this);
    def = d;
    if (def) def->SetParent(this);
}

void SwitchStmt::PrintChildren(int indentLevel) {
    if (expr) expr->Print(indentLevel+1);
    if (cases) cases->PrintAll(indentLevel+1);
    if (def) def->Print(indentLevel+1);
}