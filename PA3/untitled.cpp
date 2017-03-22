// class StmtBlock : public Stmt 
// class DeclStmt: public Stmt 
// class ConditionalStmt : public Stmt
class LoopStmt : public ConditionalStmt //
// class ForStmt : public LoopStmt 
// class WhileStmt : public LoopStmt 
// class IfStmt : public ConditionalStmt 
// class BreakStmt : public Stmt
// class ContinueStmt : public Stmt 
// class ReturnStmt : public Stmt  
// class SwitchStmt : public Stmt
// class Case : public SwitchLabel
// class Default : public SwitchLabel


class Decl : public Node //
// class VarDecl : public Decl 
class VarDeclError : public VarDecl//
// class FnDecl : public Decl 
class FormalsError : public FnDecl//


check   class Expr : public Stmt 
// class ExprError : public Expr
check   class EmptyExpr : public Expr
// check   class IntConstant : public Expr 
// check   class FloatConstant: public Expr 
// check   class BoolConstant : public Expr 
// class VarExpr : public Expr
class Operator : public Node //unnecessary
// class ArithmeticExpr : public CompoundExpr //copy
// class RelationalExpr : public CompoundExpr //copy
// class EqualityExpr : public CompoundExpr  //copy
// class LogicalExpr : public CompoundExpr //copy
// class AssignExpr : public CompoundExpr //copy
// class PostfixExpr : public CompoundExpr //copy
// class ConditionalExpr : public Expr //copy
// class ArrayAccess : public LValue 
// class FieldAccess : public LValue 
class Call : public Expr 
// class ActualsError : public Call




不用



ast_type.cc
ast_type.h