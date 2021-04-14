
// Generated from Basic.g4 by ANTLR 4.9.1

#pragma once


#include "antlr4-runtime.h"
#include "BasicVisitor.h"


/**
 * This class provides an empty implementation of BasicVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  BasicBaseVisitor : public BasicVisitor {
public:

  virtual antlrcpp::Any visitModuleBody(BasicParser::ModuleBodyContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDeclare(BasicParser::DeclareContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitTypeDecl(BasicParser::TypeDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVarDecl(BasicParser::VarDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVariable(BasicParser::VariableContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitFunctionDecl(BasicParser::FunctionDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitSubDecl(BasicParser::SubDeclContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitVarType(BasicParser::VarTypeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLine(BasicParser::LineContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitStatement(BasicParser::StatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitCallStmt(BasicParser::CallStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitArgPassValue(BasicParser::ArgPassValueContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitArgIgnore(BasicParser::ArgIgnoreContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitArgOptional(BasicParser::ArgOptionalContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitAssignStmt(BasicParser::AssignStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitExitStmt(BasicParser::ExitStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitReturnStmt(BasicParser::ReturnStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPluOp(BasicParser::PluOpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitCmpOp(BasicParser::CmpOpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLogicNotOp(BasicParser::LogicNotOpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitInnerCall(BasicParser::InnerCallContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitNegOp(BasicParser::NegOpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitString(BasicParser::StringContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitInteger(BasicParser::IntegerContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDecimal(BasicParser::DecimalContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBucket(BasicParser::BucketContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMulOp(BasicParser::MulOpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitPowModOp(BasicParser::PowModOpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitID(BasicParser::IDContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBoolean(BasicParser::BooleanContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitBitOp(BasicParser::BitOpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLogicOp(BasicParser::LogicOpContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitForStmt(BasicParser::ForStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitForeachStmt(BasicParser::ForeachStmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitSingleLineIf(BasicParser::SingleLineIfContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitMutiLineIf(BasicParser::MutiLineIfContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitIfBlock(BasicParser::IfBlockContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDoWhile(BasicParser::DoWhileContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitDoUntil(BasicParser::DoUntilContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLoopUntil(BasicParser::LoopUntilContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitLoopWhile(BasicParser::LoopWhileContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual antlrcpp::Any visitWhileWend(BasicParser::WhileWendContext *ctx) override {
    return visitChildren(ctx);
  }


};

