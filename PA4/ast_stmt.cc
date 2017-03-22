/* File: ast_stmt.cc
 * -----------------
 * Implementation of statement node classes.
 */
#include "ast_stmt.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "ast_expr.h"
#include "symtable.h"
#include "irgen.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Support/raw_ostream.h"                                                   


Program::Program(List<Decl*> *d) {
    Assert(d != NULL);
    (decls=d)->SetParentAll(this);
}

void Program::PrintChildren(int indentLevel) {
    decls->PrintAll(indentLevel+1);
    printf("\n");
}
/***********************************************************
******************                  ************************
******************    Emit  Start      ***********************
******************                  ************************
************************************************************/


// void Program::Emit() {
llvm::Value* Program::Emit() {
    // TODO:
    // This is just a reference for you to get started
    //
    // You can use this as a template and create Emit() function
    // for individual node to fill in the module structure and instructions.
    //
    // IRGenerator irgen;
    llvm::Module *mod = ir->GetOrCreateModule("PM.bc");
    st->push(Global);

    for (int i = 0; i < decls->NumElements(); i++){
        decls->Nth(i)->Emit();
    }

    st->pop();


    // // create a function signature
    // std::vector<llvm::Type *> argTypes;
    // llvm::Type *intTy = irgen.GetIntType();
    // argTypes.push_back(intTy);
    // llvm::ArrayRef<llvm::Type *> argArray(argTypes);
    // llvm::FunctionType *funcTy = llvm::FunctionType::get(intTy, argArray, false);

    // // llvm::Function *f = llvm::cast<llvm::Function>(mod->getOrInsertFunction("foo", intTy, intTy, (Type *)0));
    // llvm::Function *f = llvm::cast<llvm::Function>(mod->getOrInsertFunction("Name_the_function", funcTy));
    // llvm::Argument *arg = f->arg_begin();
    // arg->setName("x");

    // // insert a block into the runction
    // llvm::LLVMContext *context = irgen.GetContext();
    // llvm::BasicBlock *bb = llvm::BasicBlock::Create(*context, "entry", f);

    // // create a return instruction
    // llvm::Value *val = llvm::ConstantInt::get(intTy, 1);
    // llvm::Value *sum = llvm::BinaryOperator::CreateAdd(arg, val, "", bb);
    // llvm::ReturnInst::Create(*context, sum, bb);

    // write the BC into standard output
    llvm::WriteBitcodeToFile(mod, llvm::outs());

    //uncomment the next line to generate the human readable/assembly file
    // mod->dump();

    return NULL;
}


//TODO
llvm::Value * StmtBlock::Emit(){
    st->push(Global);
    for (int i = 0; i < decls->NumElements(); i++){
        decls->Nth(i)->Emit();
    }
    for (int i = 0; i < stmts->NumElements(); i++){
        stmts->Nth(i)->Emit();
        if(ir->GetBasicBlock()->getTerminator()){ 
            return NULL;
        }
    }
    st->pop();
    return NULL;
}


//TODO
llvm::Value * DeclStmt::Emit(){
    decl->Emit();
    return NULL;
}

//TODO
llvm::Value* ForStmt::Emit() {

    st->push(Other);
    llvm::LLVMContext *context = ir->GetContext();
    llvm::Function* func = ir->GetFunction();
    llvm::BasicBlock *footerBB = llvm::BasicBlock::Create(*context, "footer", func);
    llvm::BasicBlock *stepBB = llvm::BasicBlock::Create(*context, "step", func);
    llvm::BasicBlock *bodyBB = llvm::BasicBlock::Create(*context, "body", func);
    llvm::BasicBlock *headBB = llvm::BasicBlock::Create(*context, "head", func);
    
    //Emit for initialization
    init->Emit();
    llvm::BranchInst::Create(headBB,ir->GetBasicBlock());
    ir->SetBasicBlock(headBB);
     
    //Emit for test
    llvm::Value* value = test->Emit();
    if(!ir->GetBasicBlock()->getTerminator())
    llvm::BranchInst::Create(bodyBB, footerBB, value, headBB);

    
    ir->s_breakBB.push(footerBB);
    ir->s_continueBB.push(stepBB);


    ir->SetBasicBlock(bodyBB);
    body->Emit();

    if(!ir->GetBasicBlock()->getTerminator())
    llvm::BranchInst::Create(stepBB, ir->GetBasicBlock());

    ir->SetBasicBlock(stepBB);
    step->Emit();

    if(!ir->GetBasicBlock()->getTerminator())
    llvm::BranchInst::Create(headBB, stepBB);

    
    ir->s_breakBB.pop();
    ir->s_continueBB.pop();

    ir->SetBasicBlock(footerBB);


    st->pop();

    return NULL;

}


//TODO
llvm::Value* WhileStmt::Emit() {
    st->push(Other);

    llvm::LLVMContext *context = ir->GetContext();
    llvm::Function* func = ir->GetFunction();
    llvm::BasicBlock *footerBB = llvm::BasicBlock::Create(*context, "footer", func);
    llvm::BasicBlock *bodyBB = llvm::BasicBlock::Create(*context, "body", func);
    llvm::BasicBlock *headBB = llvm::BasicBlock::Create(*context, "head", func);


    llvm::BranchInst::Create(headBB, ir->GetBasicBlock());
    
    ir->s_breakBB.push(footerBB);
    ir->s_continueBB.push(headBB);

    ir->SetBasicBlock(headBB);
    llvm::Value* testVal = test->Emit();

    if(ir->GetBasicBlock()->getTerminator()==NULL)
        llvm::BranchInst::Create(bodyBB, footerBB, testVal, ir->GetBasicBlock());

    ir->SetBasicBlock(bodyBB);
    body->Emit();

    if(ir->GetBasicBlock()->getTerminator()==NULL)
        llvm::BranchInst::Create(headBB, ir->GetBasicBlock());

    ir->s_breakBB.pop();
    ir->s_continueBB.pop();

    ir->SetBasicBlock(footerBB);

    st->pop();
    return NULL;

}

//TODO
llvm::Value* IfStmt::Emit() {
    llvm::LLVMContext *context = ir->GetContext();
    llvm::BasicBlock *elseB = NULL;

    llvm::Value* testVal = test->Emit();
    llvm::BasicBlock *footerBB = llvm::BasicBlock::Create(*context, "footer", ir->GetFunction());

    if(elseBody) {
        elseB = llvm::BasicBlock::Create(*context, "Else",ir->GetFunction());
    }

    llvm::BasicBlock *basicB = llvm::BasicBlock::Create(*context, "Then",ir->GetFunction());

    if(elseBody) {
        llvm::BranchInst::Create(basicB, elseB, testVal, ir->GetBasicBlock());
    }
    else {
        llvm::BranchInst::Create(basicB, footerBB, testVal, ir->GetBasicBlock());
    }

    ir->SetBasicBlock(basicB);
    llvm::Value* bodyVal = body->Emit();

    if(!ir->GetBasicBlock()->getTerminator()) {
        llvm::BranchInst::Create(footerBB, ir->GetBasicBlock());
    }

    if(elseBody != NULL) {
        ir->SetBasicBlock(elseB);
        llvm::Value* bodyVal = elseBody->Emit();

        if( !ir->GetBasicBlock()->getTerminator() ) {
            llvm::BranchInst::Create(footerBB, ir->GetBasicBlock());
        }
    }

    ir->SetBasicBlock(footerBB);

    if(elseBody) {
        elseB->moveAfter(basicB);
        footerBB->moveAfter(elseB);
    }
    else {
        footerBB->moveAfter(basicB);
    }

    return NULL;
}

//TODO
llvm::Value* BreakStmt::Emit() {
    return llvm::BranchInst::Create(ir->s_breakBB.top(), ir->GetBasicBlock());
}

//TODO
llvm::Value* ContinueStmt::Emit() {
    return llvm::BranchInst::Create(ir->s_continueBB.top(), ir->GetBasicBlock());
}

//TODO
llvm::Value* ReturnStmt::Emit() {
    llvm::LLVMContext *context = ir->GetContext();

    if(expr){
        return llvm::ReturnInst::Create(*context, expr->Emit(), ir->GetBasicBlock());
    }
    return llvm::ReturnInst::Create(*context, ir->GetBasicBlock());
}

//TODO
llvm::Value* SwitchStmt::Emit() {
    llvm::LLVMContext *context = ir->GetContext();
    llvm::BasicBlock *footerBB = llvm::BasicBlock::Create(*context, "Footer", ir->GetFunction());
    //store cases
    vector<llvm::BasicBlock*> caseslis;
    for(int i = 0; i < cases->NumElements(); i++) {
        if ( dynamic_cast<Case*>(cases->Nth(i))) {
            Case* nest = dynamic_cast<Case*>(dynamic_cast<Case*>(cases->Nth(i))->stmt);
            Default* n_default = dynamic_cast<Default*>(dynamic_cast<Case*>(cases->Nth(i))->stmt);
            if(nest){
                cases->InsertAt(nest,i+1);
                dynamic_cast<Case*>(cases->Nth(i))->stmt = NULL;
            }
            if(n_default){
                cases->InsertAt(n_default, i+1);
                dynamic_cast<Case*>(cases->Nth(i))->stmt = NULL;
            }
            caseslis.push_back(llvm::BasicBlock::Create(*context, "Case", ir->GetFunction()));
        }
    }
    //get default
    llvm::BasicBlock *defaul = llvm::BasicBlock::Create(*context, "Default", ir->GetFunction());
    caseslis.push_back(defaul);
    footerBB->moveAfter(defaul);
    
    //emit expr
    llvm::Value* exprVal = expr->Emit();
    int num = cases->NumElements();
    llvm::SwitchInst* sws = NULL;
    if(defaul)sws = llvm::SwitchInst::Create(exprVal, defaul, num, ir->GetBasicBlock()); 
    else
        sws = llvm::SwitchInst::Create(exprVal, footerBB, num, ir->GetBasicBlock()); 

    ir->s_breakBB.push(footerBB);

    int ind = 0;
    for(int i=0;i<num;i++) {
        if(dynamic_cast<BreakStmt*>(cases->Nth(i)))dynamic_cast<BreakStmt*>(cases->Nth(i))->Emit();
        else if(!dynamic_cast<Case*>(cases->Nth(i))&&!dynamic_cast<BreakStmt*>(cases->Nth(i))){
            if(dynamic_cast<Default*>(cases->Nth(i))){
                ir->SetBasicBlock(defaul);
                dynamic_cast<Default*>(cases->Nth(i))->stmt->Emit();
            }
        }
        else {
            ir->SetBasicBlock(caseslis[ind]);
            llvm::Value* l = dynamic_cast<Case*>(cases->Nth(i))->label->Emit();
            llvm::ConstantInt *cons = llvm::cast<llvm::ConstantInt>(l);
            sws->llvm::SwitchInst::addCase(cons, caseslis[ind++]);
            if(dynamic_cast<Case*>(cases->Nth(i))->stmt)dynamic_cast<Case*>(cases->Nth(i))->stmt->Emit();
        }
    }
    int temp = caseslis.size()-1;
    for(int i = 0; i<=temp; i++) {
        if(!caseslis[i]->getTerminator()){
            if(i==temp) llvm::BranchInst::Create(footerBB, caseslis[i]);
            else 
                llvm::BranchInst::Create(caseslis[i+1], caseslis[i]);
        }
    }

    if(defaul&&!defaul->getTerminator())llvm::BranchInst::Create(footerBB, defaul);
    //set footer for BB
    ir->SetBasicBlock(footerBB);
    return NULL;
}


/***********************************************************
******************                  ************************
******************    Emit     end  ************************
******************                  ************************
************************************************************/

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

IfStmt::IfStmt(Expr *t, Stmt *tb, Stmt *elseB): ConditionalStmt(t, tb) { 
    Assert(t != NULL && tb != NULL); // else can be NULL
    elseBody = elseB;
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

