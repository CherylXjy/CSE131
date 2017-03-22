/* File: ast_expr.cc
 * -----------------
 * Implementation of expression node classes.
 */

#include <string.h>
#include "ast_expr.h"
#include "ast_type.h"
#include "ast_decl.h"
#include "symtable.h"

/***********************************************************
******************                  ************************
******************    Checking Start     *******************
******************                  ************************
************************************************************/

// We are just setting the type here
// and it will return back to the place 
// where the checked got called

void IntConstant::Check() {
    // set the type to IntType
    this->type = Type::intType;
}

void FloatConstant::Check() {
    // set the type to floatType
    this->type = Type::floatType;
}

void BoolConstant::Check() {
    // set the type to boolType
    this->type = Type::boolType;
}

void VarExpr::Check() {
    // Creating a symbol 
    // this: refer to VarExpr in ast_expr.h
    // id: Identifier in ast.h
    Symbol* sym = st->find((const char *)this->GetIdentifier()->GetName());
    // Case: x = y
    //y is not declared
    if (sym == NULL) {
        this->type = Type::errorType;
        ReportError::IdentifierNotDeclared(this->id, LookingForVariable);
        // Symbol sym((char *)this->id->name, new VarDecl(id, Type::errorType), E_VarDecl);
        // st->insert(sym);
    }
    else {
        Decl* dec = sym->decl;
        this->type = (dynamic_cast<VarDecl*>(dec))->GetType();
    }
}

void ArithmeticExpr::Check() {
    
    // cout << "I am in ArithmeticExpr "<< endl;
    // cout << "left type is: "<<left<< endl;
    // cout <<"right type is: "<< right<< endl;
    // x = 3+5   x is left, 3+5 is right
    // First check if unary is true

    if(left)left->Check();
    if(right)right->Check();


    if(left==NULL) {
        if(right->type->IsError())return;
        // Initlize Type Array
        Type* arr [] = {Type::intType, Type::floatType, 
                        Type::vec2Type, Type::vec3Type, 
                        Type::vec4Type, Type::mat2Type, 
                        Type::mat3Type, Type::mat4Type
                        };
        int arraySize = 8;
        for(int i = 0; i < arraySize; i++) {
            if(right->type->IsEquivalentTo(arr[i])) {
                this->type = right->type;
                return; 
            }
        }
        ReportError::IncompatibleOperand(op, right->type);
        this->type = Type::errorType;
        right->type = Type::errorType;
    }else {
        // not unaray
        // eg: x = 1+2
        // if none of them are errortype
        this->type = Type::errorType;
        if(!left->type->IsError()&& !right->type->IsError()) {
            // case: int x = 1.0+2.0

            //extra credit
            if( left->type->IsEquivalentTo(Type::floatType) || right->type->IsEquivalentTo(Type::floatType)){
                    if( left->type->IsVector() ||  right->type->IsVector() ){
                        if(left->type->IsVector())this->type = left->type;
                        else{
                            this->type = right->type;
                        }
                    }
                    else if(left->type->IsMatrix() || right->type->IsMatrix()){
                        if(left->type->IsMatrix())this->type = left->type;
                        else{
                            this->type = right->type;
                        }
                    }
                    else if(!left->type->IsEquivalentTo(right->type)) {
                       ReportError::IncompatibleOperands(op, left->type, right->type);
                       left->type = Type::errorType;
                       right->type = Type::errorType; 
                    }
                    else{
                        this->type = left->type;
                    }
            }
            else if( left->type->IsVector() || right->type->IsVector()){
                    if( left->type->IsMatrix() ||  right->type->IsMatrix() ){
                        if(left->type->IsMatrix())this->type = left->type;
                        else{
                            this->type = right->type;
                        }
                    }
                    else if(!left->type->IsEquivalentTo(right->type)) {
                       ReportError::IncompatibleOperands(op, left->type, right->type);
                       left->type = Type::errorType;
                       right->type = Type::errorType; 
                    }
                    else{
                        this->type = left->type;
                    }
            }
            else if(!left->type->IsEquivalentTo(right->type)) {

                ReportError::IncompatibleOperands(op, left->type, right->type);
                left->type = Type::errorType;
                right->type = Type::errorType;
            }
            // they are indeed equal, no error occur
            else{
                Type* arr [] = {Type::intType, Type::floatType, 
                                Type::vec2Type, Type::vec3Type, 
                                Type::vec4Type, Type::mat2Type, 
                                Type::mat3Type, Type::mat4Type
                                };
                int arraySize = 8;
                for(int i = 0; i < arraySize; i++) {
                    if(right->type->IsEquivalentTo(arr[i])) {
                        this->type = right->type;
                        return; 
                    }
                }
                ReportError::IncompatibleOperands(op, left->type, right->type);
                left->type = Type::errorType;
                right->type = Type::errorType;
            }
        }
    }
}

void RelationalExpr::Check() {
    
    left->Check(); right->Check();

    if( (left->type->IsEquivalentTo(right->type)) && (left->type->IsNumeric())){
        this->type = Type::boolType;
    }
    else{
        if(!left->type->IsError() && !right->type->IsError()){
            ReportError::IncompatibleOperands(op, left->type, right->type);
            this->type = Type::errorType;
            left->type = Type::errorType;
            right->type = Type::errorType;
        }
    }

}

void EqualityExpr::Check() {

    left->Check(); right->Check();

    if(!right->type->IsEquivalentTo(left->type)) {
        ReportError::IncompatibleOperands(op, left->type, right->type);
        this->type = Type::errorType;
    }else{
        if(left->type->IsError() || right->type->IsError())this->type = Type::errorType;
        else{this->type = Type::boolType;}
    }
}

void LogicalExpr::Check() {
    left->Check(); right->Check();

    if(right->type->IsEquivalentTo(Type::boolType) && left->type->IsEquivalentTo(Type::boolType)){
        this->type = Type::boolType;
    }
    else{
        if(!left->type->IsError() && !right->type->IsError()){
            ReportError::IncompatibleOperands(op, left->type, right->type);
            this->type = Type::errorType;
            left->type = Type::errorType;
            right->type = Type::errorType;
        }
    }

}

void AssignExpr::Check() {
    left->Check(); right->Check();
    if( !(left->type->IsError() || right->type->IsError()) && !left->type->IsEquivalentTo(right->type)) {
        ReportError::IncompatibleOperands(op, left->type, right->type);
    }
    else if(strcmp(op->getString(),"=")!=0){
        if(left->type->IsEquivalentTo(Type::boolType))
        ReportError::IncompatibleOperands(op, left->type, right->type);
        left->type = Type::errorType;
        right->type = Type::errorType;
    }
    this->type = left->type;
}

void PostfixExpr::Check() {
    left->Check();
    if(left->type->IsError()){ 
        this->type = Type::errorType;
        return; 
    }
    // Initlize Type Array
    Type* arr [] = {Type::intType, Type::floatType, 
                    Type::vec2Type, Type::vec3Type, 
                    Type::vec4Type, Type::mat2Type, 
                    Type::mat3Type, Type::mat4Type
                    };
    int arraySize = 8;
    for(int i = 0; i < arraySize; i++) {
        if(left->type->IsEquivalentTo(arr[i])) {
            this->type = left->type;
            return; 
        }
    }
    ReportError::IncompatibleOperand(op, left->type);
    this->type = Type::errorType;
    left->type = Type::errorType;
}

void ConditionalExpr::Check() {
    trueExpr->Check(); falseExpr->Check(); cond->Check(); 
    if(!cond->type->IsEquivalentTo(Type::boolType)){
        // printf("lalal");
        ReportError::TestNotBoolean(cond);
        cond->type = Type::errorType;
    }
    if((trueExpr->type->IsError())|| (falseExpr->type->IsError())){
        this->type = Type::errorType;
        return;
    }
    else{
        this->type = trueExpr->type;
        return;
    }
}

void ArrayAccess::Check() {
    this->type = Type::errorType;
    base->Check(); subscript->Check();
    //cout << "Inside ArrayAccess" <<endl;
    Type* b_typ = base->type;
    //cout << "No seg here" <<endl;
    if(dynamic_cast<ArrayType*>(b_typ)==NULL){
        ReportError::NotAnArray(dynamic_cast<VarExpr*>(base)->GetIdentifier());
        return;
    }
    if(b_typ->IsEquivalentTo(Type::mat2Type)){
        this->type = Type::vec2Type;
    }else if(b_typ->IsEquivalentTo(Type::mat3Type)) {
        this->type = Type::vec3Type;
    }else if(b_typ->IsEquivalentTo(Type::mat4Type)) {
        this->type = Type::vec4Type;
    }else {
        this->type = dynamic_cast<ArrayType*>(b_typ)->GetElemType();
        if(st->find((const char*)dynamic_cast<VarExpr*>(base)->GetIdentifier()->name)==NULL) {
            ReportError::NotAnArray(dynamic_cast<VarExpr*>(base)->GetIdentifier());
        }
    }
}

void FieldAccess::Check() {
    if(base)base->Check();
    if ((!base->type->IsEquivalentTo(Type::vec2Type)) && 
        (!base->type->IsEquivalentTo(Type::vec3Type)) && 
        (!base->type->IsEquivalentTo(Type::vec4Type))) {
        ReportError::InaccessibleSwizzle(field, base);
        this->type = Type::errorType;
        return;
    }

    char* swizzle = field->name;
    int len = strlen(swizzle);
    this->type = Type::floatType;
    
    for(int i = 0; i < len; i++) {
        if(swizzle[i] == 'x' || 
           swizzle[i] == 'y' || 
           swizzle[i] == 'z' || 
           swizzle[i] == 'w') 
        {  continue;
        }
        ReportError::InvalidSwizzle(field, base);
        this->type = Type::errorType;
        return;
    }
    if(base->type->IsEquivalentTo(Type::vec2Type)) {
        for(int i = 0; i < len; i++) {
            if(swizzle[i] == 'z' || 
               swizzle[i] == 'w') {
                ReportError::SwizzleOutOfBound(field, base);
                this->type = Type::errorType;
                return;
            }
        }
        if(len>1){
            this->type = Type::vec2Type;
        }    
    }
    if(base->type->IsEquivalentTo(Type::vec3Type)) {
        for(int i = 0; i < len; i++) {
            if(swizzle[i] == 'w') {
                ReportError::SwizzleOutOfBound(field, base);
                this->type = Type::errorType;
                return;
            }
        }
        if(len==2)
        this->type = Type::vec2Type;
        else if(len>2){
            this->type = Type::vec3Type;
        }
    }
    if(base->type->IsEquivalentTo(Type::vec4Type)){
        if(len==2)
        this->type = Type::vec2Type;
        else if(len==3){
            this->type = Type::vec3Type;
        }
        else
        this->type = Type::vec4Type;
    }
    if (len > 4) {
        ReportError::OversizedVector(field, base);
        this->type = Type::errorType;
    }

}

void Call::Check() {

    if(base) { base->Check(); }

    Symbol* sym = st->find((const char*)(field->name));
    FnDecl* fndecl = NULL;
    if(sym==NULL) {
        ReportError::IdentifierNotDeclared(field, LookingForFunction);
        this->type = Type::errorType;
        // Symbol sym((char *)field->name, new VarDecl(field, Type::errorType), E_VarDecl);
        // st->insert(sym);
        return;
    }

    Decl* decl = (sym)->decl;
    if(decl) { fndecl = dynamic_cast<FnDecl*>(decl); }
    if(fndecl == NULL) {//int x; x()
        ReportError::NotAFunction(field);
        this->type = Type::errorType;
        return;
    }
    int n1 = fndecl->GetFormals()->NumElements();
    int n2 = actuals->NumElements();

    if(n1!=n2){
        if(n1 < n2) {
            ReportError::ExtraFormals(field, n1, n2);
        }else if( n1 > n2) {
            ReportError::LessFormals(field, n1, n2);
        }
        this->type = fndecl->GetType();
        return;
    }


    this->type = fndecl->GetType();
    for(int i = 0; i < n2; i++) {
        actuals->Nth(i)->Check();
        if(!actuals->Nth(i)->type->IsError() && !fndecl->GetFormals()->Nth(i)->GetType()->IsError()){
            if(actuals->Nth(i)->type!= fndecl->GetFormals()->Nth(i)->GetType()) {
                ReportError::FormalsTypeMismatch(field, i+1, fndecl->GetFormals()->Nth(i)->GetType(), actuals->Nth(i)->type);
                return;
            }
        }
    }
    
    
}
/***********************************************************
******************                  ************************
******************    Checking  end    *********************
******************                  ************************
************************************************************/
/***********************************************************
******************                  ************************
******************    Checking  end    *********************
******************                  ************************
************************************************************/
/***********************************************************
******************                  ************************
******************    Checking  end    *********************
******************                  ************************
************************************************************/
/***********************************************************
******************                  ************************
******************    Checking  end    *********************
******************                  ************************
************************************************************/
/***********************************************************
******************                  ************************
******************    Checking  end    *********************
******************                  ************************
************************************************************/
/***********************************************************
******************                  ************************
******************    Checking  end    *********************
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
