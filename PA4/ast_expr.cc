/* File: ast_expr.cc
 * -----------------
 * Implementation of expression node classes.
 */

#include <string.h>
#include "ast_expr.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "symtable.h"
#include "irgen.h"
#include <iostream>
/***********************************************************
******************                  ************************
******************    Emit  Start      ***********************
******************                  ************************
************************************************************/

//TODO
llvm::Value* IntConstant::Emit() {
    type = Type::intType;
    llvm::LLVMContext* context = ir->GetContext();
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(*context), value);
}

//TODO
llvm::Value* FloatConstant::Emit() {
    type = Type::floatType;
    llvm::LLVMContext* context = ir->GetContext();
    return llvm::ConstantFP::get(llvm::Type::getFloatTy(*context), value);
}

//TODO
llvm::Value* BoolConstant::Emit(){
    type = Type::boolType;
    llvm::LLVMContext* context = ir->GetContext();
    return llvm::ConstantInt::get(llvm::Type::getInt1Ty(*context), (int)value);
}

//TODO
llvm::Value* VarExpr::Emit(){
    Symbol* sym = st->find((const char *)this->GetIdentifier()->GetName());

    llvm::Value* tempVal = sym->value;
    Decl* tempDecl = sym->decl;

    VarDecl* dynamcast = dynamic_cast<VarDecl*>(tempDecl);

    this->type = dynamcast->GetType();

    return new llvm::LoadInst(tempVal, GetIdentifier()->GetName(), ir->GetBasicBlock());
}

//TODO
llvm::Value* ArithmeticExpr::Emit() {
    // cout<< "Enter ArithExpr"<<endl;
    if(left == NULL) {
        llvm::Value* r = right->Emit();

        if(dynamic_cast<ArrayAccess*>(right)){
            ArrayAccess* dynamcast = dynamic_cast<ArrayAccess*>(right);

            llvm::Value* ptr = dynamcast->getPtr(dynamcast);

            llvm::LoadInst* vExprInst = new llvm::LoadInst(ptr, "", ir->GetBasicBlock());

            llvm::Value *val = llvm::ConstantInt::get(ir->GetIntType(), 1);
            llvm::Value *operation = NULL;
            if(op->IsOp("--")){
                operation = llvm::BinaryOperator::CreateSub(vExprInst, val, "", ir->GetBasicBlock());
            }
            else if(op->IsOp("++")){
                operation = llvm::BinaryOperator::CreateAdd(vExprInst, val, "", ir->GetBasicBlock());
            }

            new llvm::StoreInst(operation,ptr, "Store Ele", ir->GetBasicBlock());
            return new llvm::LoadInst(ptr, "", ir->GetBasicBlock());

        }
        if(right->type->IsEquivalentTo(Type::intType)) {
            this->type = Type::intType;

            VarExpr* varexpr = dynamic_cast<VarExpr*>(right);
            Symbol* sym = st->find((const char *)varexpr->GetIdentifier()->GetName());
            llvm::Value* llval = sym->value;
            
            llvm::Type *intType = ir->GetIntType();
            llvm::Value *val = llvm::ConstantInt::get(intType, 1);
            if(op->IsOp("++")) {
                llvm::Value *operation = llvm::BinaryOperator::CreateAdd(r, val, "", ir->GetBasicBlock());
                new llvm::StoreInst(operation, llval, ir->GetBasicBlock());
                return new llvm::LoadInst(llval, varexpr->GetIdentifier()->GetName(), ir->GetBasicBlock());
            }
            else if(op->IsOp("--")) {
                llvm::Value *operation = llvm::BinaryOperator::CreateSub(r, val, "", ir->GetBasicBlock());
                new llvm::StoreInst(operation, llval, ir->GetBasicBlock());
                return new llvm::LoadInst(llval, varexpr->GetIdentifier()->GetName(), ir->GetBasicBlock());
            }
            else if(op->IsOp("+")) {
                return r;
            }
            else if(op->IsOp("-")) {
                llvm::Value *val = llvm::ConstantInt::get(ir->GetIntType(), -1);
                return llvm::BinaryOperator::CreateMul(r, val, "", ir->GetBasicBlock());
            }
        }
        else if(right->type->IsEquivalentTo(Type::floatType)){
            this->type = Type::floatType;

            VarExpr* varexpr = dynamic_cast<VarExpr*>(right);
            Symbol* sym = st->find((const char *)varexpr->GetIdentifier()->GetName());
            llvm::Value* llval = sym->value;
            if(op->IsOp("++")) {
                llvm::Value *val = llvm::ConstantFP::get(ir->GetFloatType(), 1.0);
                llvm::Value* operation = llvm::BinaryOperator::CreateFAdd(r, val, "", ir->GetBasicBlock());
                
                new llvm::StoreInst(operation, llval, ir->GetBasicBlock());
                return new llvm::LoadInst(llval, varexpr->GetIdentifier()->GetName(), ir->GetBasicBlock());
            }
            else if(op->IsOp("--")) {
                // std::cerr << "here "<<right<<endl;
                llvm::Value *val = llvm::ConstantFP::get(ir->GetFloatType(), 1.0);
                llvm::Value* operation = llvm::BinaryOperator::CreateFSub(r, val, "", ir->GetBasicBlock());

                new llvm::StoreInst(operation, llval, ir->GetBasicBlock());
                return new llvm::LoadInst(llval, varexpr->GetIdentifier()->GetName(), ir->GetBasicBlock());

            }
            else if(op->IsOp("+")) {
                return r;
            }
            else if(op->IsOp("-")) {
                llvm::Value *val = llvm::ConstantFP::get(ir->GetFloatType(), -1);
                return llvm::BinaryOperator::CreateFMul(r, val, "", ir->GetBasicBlock());
            }
        }
    }
    else if(left) {
        llvm::Value* l = left->Emit();
        llvm::Value* r = right->Emit();

        if(dynamic_cast<ArrayAccess*>(left) && dynamic_cast<ArrayAccess*>(right)){
            ArrayAccess* dynamcast_l = dynamic_cast<ArrayAccess*>(left);
            ArrayAccess* dynamcast_r = dynamic_cast<ArrayAccess*>(right);

            llvm::Value* ptr_l = dynamcast_l->getPtr(dynamcast_l);
            llvm::Value* ptr_r = dynamcast_r->getPtr(dynamcast_r);

            llvm::LoadInst* val_l = new llvm::LoadInst(ptr_l, "Load left", ir->GetBasicBlock());
            llvm::LoadInst* val_r = new llvm::LoadInst(ptr_r, "Load right", ir->GetBasicBlock());

            return getBiOp(val_l, val_r, op, ir);

        }
        else if(dynamic_cast<ArrayAccess*>(left)){
            ArrayAccess* dynamcast_l = dynamic_cast<ArrayAccess*>(left);
            llvm::Value* ptr_l = dynamcast_l->getPtr(dynamcast_l);
            llvm::LoadInst* val_l = new llvm::LoadInst(ptr_l, "Load left", ir->GetBasicBlock());
            return getBiOp(val_l, r, op, ir);
        }
        else if(dynamic_cast<ArrayAccess*>(right)){
            ArrayAccess* dynamcast_r = dynamic_cast<ArrayAccess*>(right);
            llvm::Value* ptr_r = dynamcast_r->getPtr(dynamcast_r);
            llvm::LoadInst* val_r = new llvm::LoadInst(ptr_r, "Load left", ir->GetBasicBlock());
            return getBiOp(l, val_r, op, ir);
        }
        if(l->getType()->isIntegerTy() && r->getType()->isIntegerTy()){
            this->type = Type::intType;
            return getBiOp(l, r, op, ir);
        }
        else if(l->getType()->isVectorTy() && r->getType()->isVectorTy()){
            this->type = Type::floatType;
            return getBiOp(l, r, op, ir);
        }
        else if(l->getType()->isVectorTy() || r->getType()->isVectorTy()){
            if(l->getType()->isVectorTy()){
                    llvm::Value* charVal = NULL;
                    llvm::Value* emptyVec = llvm::UndefValue::get(ir->GetType(left->type, ir->GetContext()));
                    //std::cerr << l->getType()->getVectorNumElements()<< "\n";
                    for(int i=0; i< l->getType()->getVectorNumElements(); i++){
                        charVal = llvm::ConstantInt::get(ir->GetIntType(),i);
                        emptyVec = llvm::InsertElementInst::Create(emptyVec, r, charVal,"Insert Ele",ir->GetBasicBlock());
                    }
                    return getBiOp(l, emptyVec, op, ir);
            }
            else{
                    llvm::Value* charVal = NULL;
                    llvm::Value* emptyVec = llvm::UndefValue::get(ir->GetType(right->type, ir->GetContext()));
                    for(int i=0; i< r->getType()->getVectorNumElements(); i++){
                        charVal = llvm::ConstantInt::get(ir->GetIntType(),i);
                        emptyVec = llvm::InsertElementInst::Create(emptyVec, l, charVal,"Insert Ele",ir->GetBasicBlock());
                    }
                    return getBiOp(emptyVec, r, op, ir);
            }
        }
        else{
            this->type = Type::floatType;
            return getBiOp(l, r, op, ir);
        }

    }

    return NULL;
}

llvm::Value* getBiOp(llvm::Value* val_l, llvm::Value* val_r, Operator* op, IRGenerator* ir){
        if(op->IsOp("+")) {
            return llvm::BinaryOperator::CreateFAdd(val_l, val_r, "", ir->GetBasicBlock());
        }
        else if(op->IsOp("-")) {
            return llvm::BinaryOperator::CreateFSub(val_l, val_r, "", ir->GetBasicBlock());
        }
        else if(op->IsOp("*")) {
            return llvm::BinaryOperator::CreateFMul(val_l, val_r, "", ir->GetBasicBlock());
        }
        else if(op->IsOp("/")) {
            return llvm::BinaryOperator::CreateFDiv(val_l, val_r, "", ir->GetBasicBlock());
        }
        else{
            return NULL;
        }
}

//TODO
llvm::Value* RelationalExpr::Emit(){
    llvm::Value* l = left->Emit();
    llvm::Value* r = right->Emit();

    if(left->type->IsEquivalentTo(Type::intType) && right->type->IsEquivalentTo(Type::intType)) {
        this->type = Type::boolType;
        llvm::CmpInst::Predicate pred = llvm::ICmpInst::ICMP_SGT;

        if(op->IsOp("<")) {
            pred = llvm::ICmpInst::ICMP_SLT;
        }
        else if(op->IsOp(">=")) {
            pred = llvm::ICmpInst::ICMP_SGE;
        }
        else if(op->IsOp("<=")) {
            pred = llvm::ICmpInst::ICMP_SLE;    
        }

        return llvm::CmpInst::Create(llvm::CmpInst::ICmp, pred, l, r, "", ir->GetBasicBlock());
    }
    else if(left->type->IsEquivalentTo(Type::floatType) && right->type->IsEquivalentTo(Type::floatType)) {
        this->type = Type::boolType;
        llvm::CmpInst::Predicate pred = llvm::FCmpInst::FCMP_OLT;

        if(op->IsOp(">")) {
            pred = llvm::FCmpInst::FCMP_OGT;
        }
        else if(op->IsOp(">=")) {
            pred = llvm::FCmpInst::FCMP_OGE;
        }
        else if(op->IsOp("<=")) {
            pred = llvm::FCmpInst::FCMP_OLE;
        }

        return llvm::CmpInst::Create(llvm::CmpInst::FCmp, pred, l, r, "", ir->GetBasicBlock());
    }

    return NULL;
}

//TODO
llvm::Value* EqualityExpr::Emit() {
    llvm::Value* l = left->Emit();
    llvm::Value* r = right->Emit();

    if(l->getType()->isIntegerTy()){
        llvm::CmpInst::Predicate pred = llvm::ICmpInst::ICMP_EQ;
        if(op->IsOp("!=")) {
            pred = llvm::ICmpInst::ICMP_NE; 
        }
        return llvm::CmpInst::Create(llvm::CmpInst::ICmp, pred, l, r, "", ir->GetBasicBlock());
    }
    else{
        llvm::CmpInst::Predicate pred = llvm::FCmpInst::FCMP_OEQ;
        if(op->IsOp("!=")) {
            pred = llvm::FCmpInst::FCMP_ONE;
        }
        return llvm::CmpInst::Create(llvm::CmpInst::FCmp, pred, l, r, "", ir->GetBasicBlock());
    }
}

//TODO
llvm::Value* LogicalExpr::Emit() {
    llvm::Value* l = left->Emit();
    llvm::Value* r = right->Emit();
    this->type == Type::boolType;

    if(op->IsOp("&&")) {
        return llvm::BinaryOperator::CreateAnd(l, r, "Log And", ir->GetBasicBlock());
    }
    else if(op->IsOp("||")) {
        return llvm::BinaryOperator::CreateOr(l, r, "Log Or", ir->GetBasicBlock());
    }

    return NULL;
}

llvm::Value* FindChar(char c, IRGenerator* ir){
    if(c=='x'){
        return llvm::ConstantInt::get(ir->GetIntType(),0);
    }
    else if(c=='y'){
        return llvm::ConstantInt::get(ir->GetIntType(),1);
    }
    else if(c=='z'){
        return llvm::ConstantInt::get(ir->GetIntType(),2);
    }
    else if(c=='w'){
        return llvm::ConstantInt::get(ir->GetIntType(),3);
    }
    return NULL;
}


//TODO
llvm::Value* AssignExpr::Emit(){
    // std::cerr<<"arith begin"<<"\n";

    llvm::Value * l = left->Emit();
    llvm::Value * r = right->Emit();

    // std::cerr<<"arith begin1"<<"\n";

    FieldAccess* dynamcast = dynamic_cast<FieldAccess*>(left);
    if(dynamcast){
        VarExpr* baseExpr = dynamic_cast<VarExpr*>(dynamcast->base);
        
        llvm::Value* charVal = NULL;

        Symbol* sym = st->find((const char *)baseExpr->GetIdentifier()->GetName());

        llvm::Value* tempVal = sym->value;
        string swizzle = string(dynamcast->field->GetName());

        llvm::Value* baseMem = dynamcast->base->Emit();

        if(swizzle.length() == 1) {
            charVal = FindChar(swizzle[0], ir);

            llvm::Value* newbiOp = NULL;
            if(op->IsOp("+=")){
                newbiOp = llvm::BinaryOperator::CreateFAdd(l,r,"", ir->GetBasicBlock());
            }
            else if(op->IsOp("-=")){
                newbiOp = llvm::BinaryOperator::CreateFSub(l,r,"", ir->GetBasicBlock());
            }
            else if(op->IsOp("*=")){
                newbiOp = llvm::BinaryOperator::CreateFMul(l,r,"", ir->GetBasicBlock());
            }
            else if(op->IsOp("/=")){
                newbiOp = llvm::BinaryOperator::CreateFDiv(l, r, "", ir->GetBasicBlock());
            }
            else{
                newbiOp=r;
            }

            llvm::Value* newVec = llvm::InsertElementInst::Create(baseMem, newbiOp, charVal, "", ir->GetBasicBlock());
                
            new llvm::StoreInst(newVec, tempVal, ir->GetBasicBlock());

            llvm::LoadInst* vExprInst = new llvm::LoadInst(tempVal, baseExpr->GetIdentifier()->GetName(), ir->GetBasicBlock());
            return llvm::ExtractElementInst::Create(vExprInst, charVal, "Extract Ele", ir->GetBasicBlock());
        }
        else{
            for(int i = 0; i < swizzle.length(); i++) {
                // std::cerr << "swiz  "<<i<<"\n";
                llvm::Value* fieldIdx = llvm::ConstantInt::get(ir->GetIntType(), i);
                
                charVal = FindChar(swizzle[i],ir);

                llvm::Value* old=llvm::ExtractElementInst::Create(baseMem,fieldIdx,"Extract Ele",ir->GetBasicBlock());

                llvm::Value* newbiOp = NULL;

                if(op->IsOp("+=")){
                    newbiOp = llvm::BinaryOperator::CreateFAdd(old,r,"FAdd",ir->GetBasicBlock());
                }
                else if(op->IsOp("-=")){
                    newbiOp = llvm::BinaryOperator::CreateFSub(old, r, "Fsub", ir->GetBasicBlock());
                }
                else if(op->IsOp("*=")){
                    newbiOp = llvm::BinaryOperator::CreateFMul(old,r,"FMul",ir->GetBasicBlock());
                }
                else if(op->IsOp("/=")){
                    newbiOp = llvm::BinaryOperator::CreateFDiv(old,r,"FMul",ir->GetBasicBlock());
                }
                else{
                    newbiOp = llvm::ExtractElementInst::Create(r,fieldIdx,"Extract Ele",ir->GetBasicBlock());;
                }
                baseMem = llvm::InsertElementInst::Create(baseMem,newbiOp,charVal,"Insert Ele",ir->GetBasicBlock());
            }

            new llvm::StoreInst(baseMem, tempVal, ir->GetBasicBlock());
            return r;
        }
    }
    llvm::Value *ll = llvm::cast<llvm::LoadInst>(l)->getPointerOperand(); 
    llvm::Value *biOp = NULL;
    if(op->IsOp("=")){
        new llvm::StoreInst(r,ll,ir->GetBasicBlock());
    }
    else if(op->IsOp("+=")){
        if(dynamic_cast<ArrayAccess*>(left)){
            ArrayAccess* dynamcast1 = dynamic_cast<ArrayAccess*>(left);
            llvm::Value* ptr = dynamcast1->getPtr(dynamcast1);
            llvm::Value* operation = llvm::BinaryOperator::CreateFAdd(l, r, "FAdd", ir->GetBasicBlock());
            new llvm::StoreInst(operation,ptr, "Store Ele", ir->GetBasicBlock());

            return new llvm::LoadInst(ptr, "Load", ir->GetBasicBlock());
        }
        if( left->type->IsEquivalentTo(Type::intType)){
            biOp=llvm::BinaryOperator::CreateAdd(l,r,"", ir->GetBasicBlock());
            new llvm::StoreInst(biOp,ll,ir->GetBasicBlock());
        }
        if(left->type->IsEquivalentTo(Type::floatType)){
            biOp=llvm::BinaryOperator::CreateFAdd(l,r,"", ir->GetBasicBlock());
            new llvm::StoreInst(biOp,ll,ir->GetBasicBlock());
        }
        if(left->type->IsEquivalentTo(Type::vec2Type)){
            biOp=llvm::BinaryOperator::CreateFAdd(l,r,"", ir->GetBasicBlock());
            new llvm::StoreInst(biOp,ll,ir->GetBasicBlock());
        }
        if(left->type->IsEquivalentTo(Type::vec3Type)){
            biOp=llvm::BinaryOperator::CreateFAdd(l,r,"Vec3", ir->GetBasicBlock());
            new llvm::StoreInst(biOp,ll,ir->GetBasicBlock());
        }
        if(left->type->IsEquivalentTo(Type::vec4Type)){
            biOp=llvm::BinaryOperator::CreateFAdd(l,r,"Vec4", ir->GetBasicBlock());
            new llvm::StoreInst(biOp,ll,ir->GetBasicBlock());
        }
    
    }
    else if(op->IsOp("-=")){
        if(dynamic_cast<ArrayAccess*>(left)){
            ArrayAccess* dynamcast1 = dynamic_cast<ArrayAccess*>(left);
            llvm::Value* ptr = dynamcast1->getPtr(dynamcast1);
            llvm::Value* operation = llvm::BinaryOperator::CreateFSub(l, r, "", ir->GetBasicBlock());
            new llvm::StoreInst(operation,ptr, "Store Element", ir->GetBasicBlock());

            return new llvm::LoadInst(ptr, "Load", ir->GetBasicBlock());
        }
        if(left->type->IsEquivalentTo(Type::intType)){
            biOp=llvm::BinaryOperator::CreateSub(l,r,"", ir->GetBasicBlock());
            new llvm::StoreInst(biOp,ll,ir->GetBasicBlock());
        }
        if(left->type->IsEquivalentTo(Type::floatType)){
            biOp=llvm::BinaryOperator::CreateFSub(l,r,"", ir->GetBasicBlock());
            new llvm::StoreInst(biOp,ll,ir->GetBasicBlock());
        }
        if(left->type->IsEquivalentTo(Type::vec2Type)){
            biOp=llvm::BinaryOperator::CreateFSub(l,r,"Vec2", ir->GetBasicBlock());
            new llvm::StoreInst(biOp,ll,ir->GetBasicBlock());
        }
        if(left->type->IsEquivalentTo(Type::vec3Type)){
            biOp=llvm::BinaryOperator::CreateFSub(l,r,"Vec3", ir->GetBasicBlock());
            new llvm::StoreInst(biOp,ll,ir->GetBasicBlock());
        }
        if(left->type->IsEquivalentTo(Type::vec4Type)){
            biOp=llvm::BinaryOperator::CreateSub(l,r,"Vec4", ir->GetBasicBlock());
            new llvm::StoreInst(biOp,ll,ir->GetBasicBlock());
        }
    }
    else if(op->IsOp("*=")){
        if(dynamic_cast<ArrayAccess*>(left)){
            ArrayAccess* dynamcast1 = dynamic_cast<ArrayAccess*>(left);
            llvm::Value* ptr = dynamcast1->getPtr(dynamcast1);
            llvm::Value* operation = llvm::BinaryOperator::CreateFMul(l, r, "", ir->GetBasicBlock());
            new llvm::StoreInst(operation,ptr, "Store Element", ir->GetBasicBlock());

            return new llvm::LoadInst(ptr, "Load", ir->GetBasicBlock());
        }
        if(left->type->IsEquivalentTo(Type::intType)){
            biOp=llvm::BinaryOperator::CreateMul(l,r,"", ir->GetBasicBlock());
            new llvm::StoreInst(biOp,ll,ir->GetBasicBlock());
        }
        if(left->type->IsEquivalentTo(Type::floatType)){
            biOp=llvm::BinaryOperator::CreateFMul(l,r,"", ir->GetBasicBlock());
            new llvm::StoreInst(biOp,ll,ir->GetBasicBlock());
        }
        if(left->type->IsEquivalentTo(Type::vec2Type)){
            biOp=llvm::BinaryOperator::CreateFMul(l,r,"Vec2", ir->GetBasicBlock());
            new llvm::StoreInst(biOp,ll,ir->GetBasicBlock());
        }
        if(left->type->IsEquivalentTo(Type::vec3Type)){
            biOp=llvm::BinaryOperator::CreateFMul(l,r,"Vec3", ir->GetBasicBlock());
            new llvm::StoreInst(biOp,ll,ir->GetBasicBlock());
        }
        if(left->type->IsEquivalentTo(Type::vec4Type)){
            biOp=llvm::BinaryOperator::CreateFMul(l,r,"Vec4", ir->GetBasicBlock());
            new llvm::StoreInst(biOp,ll,ir->GetBasicBlock());
        }
    }
    else if(op->IsOp("/=")){
        if(dynamic_cast<ArrayAccess*>(left)){
            ArrayAccess* dynamcast1 = dynamic_cast<ArrayAccess*>(left);
            llvm::Value* ptr = dynamcast1->getPtr(dynamcast1);
            llvm::Value* operation = llvm::BinaryOperator::CreateFDiv(l, r, "", ir->GetBasicBlock());
            new llvm::StoreInst(operation,ptr, "Store Element", ir->GetBasicBlock());

            return new llvm::LoadInst(ptr, "Load", ir->GetBasicBlock());
        }
        if(left->type->IsEquivalentTo(Type::intType)){
            biOp=llvm::BinaryOperator::CreateSDiv(l,r,"", ir->GetBasicBlock());
            new llvm::StoreInst(biOp,ll,ir->GetBasicBlock());
        }
        if(left->type->IsEquivalentTo(Type::floatType)){
            biOp=llvm::BinaryOperator::CreateFDiv(l,r,"", ir->GetBasicBlock());
            new llvm::StoreInst(biOp,ll,ir->GetBasicBlock());
        }
        if(left->type->IsEquivalentTo(Type::vec2Type)){
            biOp=llvm::BinaryOperator::CreateFDiv(l,r,"Vec2", ir->GetBasicBlock());
            new llvm::StoreInst(biOp,ll,ir->GetBasicBlock());
        }
        if(left->type->IsEquivalentTo(Type::vec3Type)){
            biOp=llvm::BinaryOperator::CreateFDiv(l,r,"Vec3", ir->GetBasicBlock());
            new llvm::StoreInst(biOp,ll,ir->GetBasicBlock());
        }
        if(left->type->IsEquivalentTo(Type::vec4Type)){
            biOp=llvm::BinaryOperator::CreateFDiv(l,r,"Vec4", ir->GetBasicBlock());
            new llvm::StoreInst(biOp,ll,ir->GetBasicBlock());
        }
    }
    return r;
}

llvm::Value* ArrayAccess::getPtr(ArrayAccess* dynamcast1){
    llvm::Value* baseEmit = dynamcast1->base->Emit();
    VarExpr* baseExpr = dynamic_cast<VarExpr*>(dynamcast1->base);

    Symbol* sym = st->find((const char *)baseExpr->GetIdentifier()->GetName());

    llvm::Value* val = sym->value;
    
    llvm::Value* sub = dynamcast1->subscript->Emit();
    
    vector<llvm::Value*> vecc;
    vecc.push_back(llvm::ConstantInt::get(ir->GetIntType(), 0));
    vecc.push_back(sub);
    llvm::ArrayRef<llvm::Value*> ref = llvm::ArrayRef<llvm::Value*>(vecc);

    return llvm::GetElementPtrInst::Create(val, ref, "Array", ir->GetBasicBlock());

}


//TODO
llvm::Value* PostfixExpr::Emit() {
    // std::cerr<< "PostFixExpr begin"<< "\n";
    llvm::Value* l = left->Emit();

    // std::cerr<< "PostFix here "<<"\n";
    if(dynamic_cast<ArrayAccess*>(left)){
        ArrayAccess* dynamcast = dynamic_cast<ArrayAccess*>(left);

        llvm::Value* baseEmit = dynamcast->base->Emit();
        VarExpr* baseExpr = dynamic_cast<VarExpr*>(dynamcast->base);
        
        Symbol* sym = st->find((const char *)baseExpr->GetIdentifier()->GetName());
        llvm::Value* tempVal = sym->value;
    
        llvm::Value* num = dynamcast->subscript->Emit();
    
        vector<llvm::Value*> indices;
        indices.push_back(llvm::ConstantInt::get(ir->GetIntType(), 0));
        indices.push_back(num);
        llvm::ArrayRef<llvm::Value*> indRef = llvm::ArrayRef<llvm::Value*>(indices);

        llvm::Value* ptr = llvm::GetElementPtrInst::Create(tempVal, indRef, "", ir->GetBasicBlock());

        llvm::LoadInst* vExprInst = new llvm::LoadInst(ptr, "Load", ir->GetBasicBlock());

        llvm::Value *val = llvm::ConstantInt::get(ir->GetIntType(), 1);
        llvm::Value *biOp = NULL;
        if(op->IsOp("--")){
            biOp = llvm::BinaryOperator::CreateSub(vExprInst, val, "", ir->GetBasicBlock());
        }
        else if(op->IsOp("++")){
            biOp = llvm::BinaryOperator::CreateAdd(vExprInst, val, "", ir->GetBasicBlock());
        }

        new llvm::StoreInst(biOp,ptr, "Store Element", ir->GetBasicBlock());
        return l;

    }
    else if (left->type->IsEquivalentTo(Type::intType)) {
        llvm::LoadInst *biOp = llvm::cast<llvm::LoadInst>(l);
        llvm::Type *intType = ir->GetIntType();
        llvm::Value *val = llvm::ConstantInt::get(intType, 1);
        VarExpr* varexpr = dynamic_cast<VarExpr*>(left);
        Symbol* sym = st->find((const char *)varexpr->GetIdentifier()->GetName());
        llvm::Value* llval = sym->value;

        if(op->IsOp("++")){
            llvm::Value *val = llvm::ConstantInt::get(ir->GetIntType(), 1);
            llvm::Value* newbiOp = llvm::BinaryOperator::CreateAdd(l, val, "", ir->GetBasicBlock());
                
            new llvm::StoreInst(newbiOp, llval, ir->GetBasicBlock());
            return new llvm::LoadInst(llval, varexpr->GetIdentifier()->GetName(), ir->GetBasicBlock());
        }
        else if(op->IsOp("--")){
            llvm::Value *val = llvm::ConstantInt::get(ir->GetIntType(), 1);
            llvm::Value* newbiOp = llvm::BinaryOperator::CreateSub(l, val, "", ir->GetBasicBlock());
            
            new llvm::StoreInst(newbiOp, llval, ir->GetBasicBlock());
            return new llvm::LoadInst(llval, varexpr->GetIdentifier()->GetName(), ir->GetBasicBlock());
        }
    }

    else if (left->type->IsEquivalentTo(Type::floatType)) {
        llvm::LoadInst *biOp = llvm::cast<llvm::LoadInst>(l);
        llvm::Type *intType = ir->GetIntType();
        llvm::Value *val = llvm::ConstantInt::get(intType, 1);
        VarExpr* varexpr = dynamic_cast<VarExpr*>(left);
        Symbol* sym = st->find((const char *)varexpr->GetIdentifier()->GetName());
        llvm::Value* llval = sym->value;

        if(op->IsOp("++")){
            llvm::Value *val = llvm::ConstantFP::get(ir->GetFloatType(), 1.0);
            llvm::Value* newbiOp = llvm::BinaryOperator::CreateFAdd(l, val, "", ir->GetBasicBlock());
                
            new llvm::StoreInst(newbiOp, llval, ir->GetBasicBlock());
            return new llvm::LoadInst(llval, varexpr->GetIdentifier()->GetName(), ir->GetBasicBlock());

        }
        else if(op->IsOp("--")){
            llvm::Value *val = llvm::ConstantFP::get(ir->GetFloatType(), 1.0);
            llvm::Value* newbiOp = llvm::BinaryOperator::CreateFSub(l, val, "", ir->GetBasicBlock());

            new llvm::StoreInst(newbiOp, llval, ir->GetBasicBlock());
            return new llvm::LoadInst(llval, varexpr->GetIdentifier()->GetName(), ir->GetBasicBlock());
        }
    } 

    return NULL;
}


//TODO
llvm::Value * ArrayAccess::Emit(){
    // std::cerr<< "ArrayAccess"<< "\n";
    VarExpr* dynamcast = dynamic_cast< VarExpr* >(base);

    if(dynamcast->GetIdentifier()->GetName()){
        // std::cerr<< "ArrayAccess1 "<< dynamcast->GetIdentifier()->GetName()<< "\n";
        Symbol* sym = st->find((const char *)dynamcast->GetIdentifier()->GetName());
        llvm::Value* val = sym->value;

        llvm::Value * subVal = subscript->Emit();

         // std::cerr<< "ArrayAccess2"<< "\n";
        std::vector <llvm::Value*> vec;

        vec.push_back(llvm::ConstantInt::get(ir->GetIntType(),0));
        vec.push_back(subVal);

        // std::cerr<< "ArrayAccess3"<< "\n";
        llvm::ArrayRef<llvm::Value*> valList = llvm::ArrayRef<llvm::Value*>(vec);
        // std::cerr<< "ArrayAccess4"<< "\n";
        llvm::Value *pt= llvm::GetElementPtrInst::Create(val,valList,"Array Access",ir->GetBasicBlock());

        // std::cerr<< "ArrayAccess5"<< "\n";
        llvm::LoadInst* tempVal = new llvm::LoadInst(pt,"Array Access",ir->GetBasicBlock());
        // std::cerr<< "ArrayAccess6"<< "\n";
        return tempVal;
    }
    return NULL;
}

Type* returnType(string swizzle){
    if(swizzle.length() == 1)return Type::floatType;
    else if(swizzle.length() == 2)return Type::vec2Type;
    else if(swizzle.length() == 3)return Type::vec3Type;
    else if(swizzle.length() == 4)return Type::vec4Type;
    else
        return NULL;
}
//TODO
llvm::Value* FieldAccess::Emit() {
    llvm::Value* baseVal = base->Emit();
    llvm::Value* charVal = NULL;
    vector<llvm::Constant*> swizzles;


    string swizzle = string(field->GetName());
    //Sanity check
    this->type = returnType(swizzle);
    if(this->type==NULL)return NULL;
    
    if (swizzle.length() == 1) {
        charVal = FindChar(swizzle[0], ir);
        return llvm::ExtractElementInst::Create(baseVal, charVal, "Field Access", ir->GetBasicBlock());
    }
    else {
        for(int i = 0; i < swizzle.length(); i++) {
            charVal = FindChar(swizzle[0], ir);
            if(swizzle[0] == 'x') {
                swizzles.push_back(llvm::ConstantInt::get(ir->GetIntType(), 0));
            }
            else if(swizzle[0] == 'y') {
                swizzles.push_back(llvm::ConstantInt::get(ir->GetIntType(), 1));
            }
            else if(swizzle[0] == 'z') {
                swizzles.push_back(llvm::ConstantInt::get(ir->GetIntType(), 2));
            }
            else if(swizzle[0] == 'w') {
                swizzles.push_back(llvm::ConstantInt::get(ir->GetIntType(), 3));
            }
        }
        if(dynamic_cast<VarExpr*>(base)) {
            Symbol* sym = st->find((const char *)dynamic_cast<VarExpr*>(base)->GetIdentifier()->GetName());
            llvm::Value* tempVal = sym->value;

            llvm::LoadInst* vecVal = new llvm::LoadInst(tempVal, field->GetName(), ir->GetBasicBlock());
            llvm::UndefValue* un = llvm::UndefValue::get(baseVal->getType());
    
            llvm::ArrayRef<llvm::Constant*> ref(swizzles);
            return new llvm::ShuffleVectorInst(vecVal, un, llvm::ConstantVector::get(ref), "Shuffle", ir->GetBasicBlock());
        }
        else {
            llvm::UndefValue* un = llvm::UndefValue::get(baseVal->getType());
            llvm::ArrayRef<llvm::Constant*> ref(swizzles);
            return new llvm::ShuffleVectorInst(baseVal, un, llvm::ConstantVector::get(ref), "Shuffle", ir->GetBasicBlock());
        }   
    }
}   

//TODO
llvm::Value* Call::Emit() {
    vector<llvm::Value*> vec;
    for(int i = 0; i < actuals->NumElements(); i++) {
        vec.push_back(actuals->Nth(i)->Emit());
    }

    Symbol* sym = st->find((const char *)field->GetName());
    llvm::Value* llval = sym->value;

    return llvm::CallInst::Create(llval, vec, "Call", ir->GetBasicBlock());
}

/***********************************************************
******************                  ************************
******************    Emit     end  ************************
******************                  ************************
************************************************************/

IntConstant::IntConstant(yyltype loc, int val) : Expr(loc) {
    value = val;
}
void IntConstant::PrintChildren(int indentLevel) { 
    printf("%d", value);
}


FloatConstant::FloatConstant(yyltype loc, double val) : Expr(loc) {
    value = val;
}
void FloatConstant::PrintChildren(int indentLevel) { 
    printf("%g", value);
}

BoolConstant::BoolConstant(yyltype loc, bool val) : Expr(loc) {
    value = val;
}
void BoolConstant::PrintChildren(int indentLevel) { 
    printf("%s", value ? "true" : "false");
}

VarExpr::VarExpr(yyltype loc, Identifier *ident) : Expr(loc) {
    Assert(ident != NULL);
    this->id = ident;
}

void VarExpr::PrintChildren(int indentLevel) {
    id->Print(indentLevel+1);
}

Operator::Operator(yyltype loc, const char *tok) : Node(loc) {
    Assert(tok != NULL);
    strncpy(tokenString, tok, sizeof(tokenString));
}

void Operator::PrintChildren(int indentLevel) {
    printf("%s",tokenString);
}

bool Operator::IsOp(const char *op) const {
    return strcmp(tokenString, op) == 0;
}

CompoundExpr::CompoundExpr(Expr *l, Operator *o, Expr *r) 
  : Expr(Join(l->GetLocation(), r->GetLocation())) {
    Assert(l != NULL && o != NULL && r != NULL);
    (op=o)->SetParent(this);
    (left=l)->SetParent(this); 
    (right=r)->SetParent(this);
}

CompoundExpr::CompoundExpr(Operator *o, Expr *r) 
  : Expr(Join(o->GetLocation(), r->GetLocation())) {
    Assert(o != NULL && r != NULL);
    left = NULL; 
    (op=o)->SetParent(this);
    (right=r)->SetParent(this);
}

CompoundExpr::CompoundExpr(Expr *l, Operator *o) 
  : Expr(Join(l->GetLocation(), o->GetLocation())) {
    Assert(l != NULL && o != NULL);
    (left=l)->SetParent(this);
    (op=o)->SetParent(this);
}

void CompoundExpr::PrintChildren(int indentLevel) {
   if (left) left->Print(indentLevel+1);
   op->Print(indentLevel+1);
   if (right) right->Print(indentLevel+1);
}
   
ConditionalExpr::ConditionalExpr(Expr *c, Expr *t, Expr *f)
  : Expr(Join(c->GetLocation(), f->GetLocation())) {
    Assert(c != NULL && t != NULL && f != NULL);
    (cond=c)->SetParent(this);
    (trueExpr=t)->SetParent(this);
    (falseExpr=f)->SetParent(this);
}

void ConditionalExpr::PrintChildren(int indentLevel) {
    cond->Print(indentLevel+1, "(cond) ");
    trueExpr->Print(indentLevel+1, "(true) ");
    falseExpr->Print(indentLevel+1, "(false) ");
}
ArrayAccess::ArrayAccess(yyltype loc, Expr *b, Expr *s) : LValue(loc) {
    (base=b)->SetParent(this); 
    (subscript=s)->SetParent(this);
}

void ArrayAccess::PrintChildren(int indentLevel) {
    base->Print(indentLevel+1);
    subscript->Print(indentLevel+1, "(subscript) ");
}
     
FieldAccess::FieldAccess(Expr *b, Identifier *f) 
  : LValue(b? Join(b->GetLocation(), f->GetLocation()) : *f->GetLocation()) {
    Assert(f != NULL); // b can be be NULL (just means no explicit base)
    base = b; 
    if (base) base->SetParent(this); 
    (field=f)->SetParent(this);
}


void FieldAccess::PrintChildren(int indentLevel) {
    if (base) base->Print(indentLevel+1);
    field->Print(indentLevel+1);
}

Call::Call(yyltype loc, Expr *b, Identifier *f, List<Expr*> *a) : Expr(loc)  {
    Assert(f != NULL && a != NULL); // b can be be NULL (just means no explicit base)
    base = b;
    if (base) base->SetParent(this);
    (field=f)->SetParent(this);
    (actuals=a)->SetParentAll(this);
}

void Call::PrintChildren(int indentLevel) {
   if (base) base->Print(indentLevel+1);
   if (field) field->Print(indentLevel+1);
   if (actuals) actuals->PrintAll(indentLevel+1, "(actuals) ");
}

