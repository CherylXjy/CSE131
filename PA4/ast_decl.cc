/* File: ast_decl.cc
 * -----------------
 * Implementation of Decl node classes.
 */
#include "ast_decl.h"
#include "ast_type.h"
#include "ast_stmt.h"
#include "symtable.h"   
#include "irgen.h"     
         
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

//TODO
llvm::Value* VarDecl::Emit() {
    llvm::LLVMContext *context = ir->GetContext();
    llvm::Value* val = NULL;
    
    if(assignTo)val = assignTo->Emit();
    else
        val = llvm::Constant::getNullValue(ir->GetType(GetType(), context));

    llvm::Constant* cons = dynamic_cast<llvm::Constant*>(val);
    llvm::Value* temp = NULL;
    if(st->isGlobalScope()) {
        temp = new llvm::GlobalVariable(*ir->GetOrCreateModule("PM.bc"), ir->GetType(GetType(), context), false, llvm::GlobalValue::ExternalLinkage, cons, id->GetName());
        Symbol sym((char*)GetIdentifier()->GetName(), this, E_VarDecl, temp);
        st->insert(sym);
        return temp;
    }
    else {
        temp = new llvm::AllocaInst(ir->GetType(GetType(), context),id->GetName(), ir->GetBasicBlock());

        Symbol sym((char*)GetIdentifier()->GetName(), this, E_VarDecl, temp);
        st->insert(sym);

        //Delete
        if(assignTo) {
            new llvm::StoreInst(val,temp,"",ir->GetBasicBlock());
        }
        return temp;
    }

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

//TODO
llvm::Value* FnDecl::Emit() {
    llvm::LLVMContext *context = ir->GetContext();
    vector<llvm::Type*> formalTypes;
    int num = formals->NumElements();
    //GET formals type
    for(int i=0;i<num;i++) {
        formalTypes.push_back(ir->GetType(formals->Nth(i)->GetType(), context));
    }
    //Cast into array refe type
    llvm::ArrayRef<llvm::Type*> arg(formalTypes);
    llvm::FunctionType* ty = llvm::FunctionType::get(ir->GetType(GetType(), context), arg, false);
    llvm::Function *fun = llvm::cast<llvm::Function>(ir->GetOrCreateModule("PM.bc")->getOrInsertFunction(GetIdentifier()->GetName(), ty));
    
    //insert into symboltable
    Symbol sym((char*)GetIdentifier()->GetName(), this, E_FunctionDecl, fun);
    st->insert(sym);
    st->push(Other);

    llvm::BasicBlock* bb = llvm::BasicBlock::Create(*context, "Function", fun);
    ir->SetBasicBlock(bb);
    ir->SetFunction(fun);

    int ind = 0;
    for(llvm::Function::arg_iterator args = fun->arg_begin();args!=fun->arg_end();args++){
        VarDecl* tempF = formals->Nth(ind++);
        char* tempN = tempF->GetIdentifier()->GetName();
        args->setName(tempN);
        llvm::Value* val = new llvm::AllocaInst(ir->GetType(tempF->GetType(), context),tempN, ir->GetBasicBlock());
        
        Symbol sym((char*)tempN, tempF, E_FunctionDecl, val);
        st->insert(sym);
        //Delete
        new llvm::StoreInst(args, val, ir->GetBasicBlock());
    }

    if(body)body->Emit();

    
    ir->SetBasicBlock(NULL);
    st->pop();

    return fun;

}