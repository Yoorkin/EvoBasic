#ifndef CB_VISITOR
#define CB_VISITOR

#include"errorListener.h"

#include<map>
#include<string>
#include<stack>
#include<list>
#include<iostream>

#include<antlr4-runtime/antlr4-runtime.h>
#include"../antlr/BasicLexer.h"
#include"../antlr/BasicParser.h"
#include"../antlr/BasicBaseVisitor.h"

#include<llvm/IR/DerivedTypes.h>
#include<llvm/IR/IRBuilder.h>
#include<llvm/IR/Function.h>
#include<llvm/IR/InstrTypes.h>
#include<llvm/IR/Instruction.h>
#include<llvm/IR/LLVMContext.h>
#include<llvm/IR/Module.h>
#include<llvm/IR/Verifier.h>
#include<llvm/Support/raw_ostream.h>
#include<llvm/Target/TargetMachine.h>
#include<llvm/ExecutionEngine/JITSymbol.h>
#include<llvm/ExecutionEngine/Orc/LLJIT.h>
#include<llvm/ExecutionEngine/Orc/CompileUtils.h>
#include<llvm/ExecutionEngine/Orc/Core.h>
#include<llvm/ExecutionEngine/Orc/JITTargetMachineBuilder.h>
#include<llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include<llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include<llvm/ExecutionEngine/Orc/RTDyldObjectLinkingLayer.h>
#include<llvm/ExecutionEngine/SectionMemoryManager.h>
#include<llvm/IR/DataLayout.h>
#include<llvm/ExecutionEngine/ExecutionEngine.h>

#include"genUtility.h"

namespace classicBasic{
    using namespace llvm;
    using namespace std;
    using namespace antlr4;

    class CodeGenVisitor;
    class StackFrame;
    class TypeTable;
    class CodeGenerator;
    class JIT;

    class CodeGenVisitor: public BasicBaseVisitor{
        GenerateUnit& unit;
        IRBuilder<> builder;
        LLVMContext &context;
        llvm::Module& mod;
        list<StackFrame>& frame;
        Reporter& reporter;
        structure::Scope* scope;
    public:

        CodeGenVisitor(GenerateUnit& unit,structure::Scope* scope);
        //=========================================== utility =================================================
        void visitBlock(vector<BasicParser::LineContext*>& block){
            for(auto& line:block)visit(line);
        }
//    template<typename T>
//    void storeValue(Token* token,T value);
        //=========================================== flow-control =================================================
        virtual antlrcpp::Any visitForStmt(BasicParser::ForStmtContext *ctx) override;
        virtual antlrcpp::Any visitDoWhile(BasicParser::DoWhileContext *ctx) override;
        virtual antlrcpp::Any visitDoUntil(BasicParser::DoUntilContext *ctx) override;
        virtual antlrcpp::Any visitLoopUntil(BasicParser::LoopUntilContext *ctx) override;
        virtual antlrcpp::Any visitLoopWhile(BasicParser::LoopWhileContext *ctx) override;
        virtual antlrcpp::Any visitWhileWend(BasicParser::WhileWendContext *ctx) override;
        virtual antlrcpp::Any visitSingleLineIf(BasicParser::SingleLineIfContext *ctx) override;
        virtual antlrcpp::Any visitMutiLineIf(BasicParser::MutiLineIfContext *ctx) override;
        virtual antlrcpp::Any visitIfBlock(BasicParser::IfBlockContext *ctx) override;
        virtual antlrcpp::Any visitExitStmt(BasicParser::ExitStmtContext *ctx) override;
        //============================================ declare ====================================================
        virtual antlrcpp::Any visitVarDecl(BasicParser::VarDeclContext *ctx) override;
        virtual antlrcpp::Any visitTypeDecl(BasicParser::TypeDeclContext *ctx) override;
        virtual antlrcpp::Any visitVariable(BasicParser::VariableContext *ctx) override;
        virtual antlrcpp::Any visitNecessaryParameter(BasicParser::NecessaryParameterContext *ctx) override;
        virtual antlrcpp::Any visitOptionalParameter(BasicParser::OptionalParameterContext *ctx) override;
        virtual antlrcpp::Any visitFunctionDecl(BasicParser::FunctionDeclContext *ctx) override;
        virtual antlrcpp::Any visitSubDecl(BasicParser::SubDeclContext *ctx) override;
        //====================================== call-statement =========================================
        virtual antlrcpp::Any visitInnerCall(BasicParser::InnerCallContext *ctx) override;
        virtual antlrcpp::Any visitArgPassValue(BasicParser::ArgPassValueContext *ctx) override;
        virtual antlrcpp::Any visitArgIgnore(BasicParser::ArgIgnoreContext *ctx) override;
        virtual antlrcpp::Any visitArgOptional(BasicParser::ArgOptionalContext *ctx) override;
        virtual antlrcpp::Any visitReturnStmt(BasicParser::ReturnStmtContext *ctx) override;
        virtual antlrcpp::Any visitAssignStmt(BasicParser::AssignStmtContext *ctx) override;
        virtual antlrcpp::Any visitPluOp(BasicParser::PluOpContext *ctx) override;
        map<string,CmpInst::Predicate> cmpIntSigned={
                {"=",CmpInst::Predicate::ICMP_EQ},
                {"<>",CmpInst::Predicate::ICMP_NE},
                {"<",CmpInst::Predicate::ICMP_SLT},
                {"<=",CmpInst::Predicate::ICMP_SLE},
                {"=<",CmpInst::Predicate::ICMP_SLE},
                {">",CmpInst::Predicate::ICMP_SGT},
                {">=",CmpInst::Predicate::ICMP_SGE},
                {"=>",CmpInst::Predicate::ICMP_SGE}
        };
        virtual antlrcpp::Any visitCmpOp(BasicParser::CmpOpContext *ctx) override;
        virtual antlrcpp::Any visitLogicNotOp(BasicParser::LogicNotOpContext *ctx) override;
        virtual antlrcpp::Any visitNegOp(BasicParser::NegOpContext *ctx) override;
        virtual antlrcpp::Any visitString(BasicParser::StringContext *ctx) override;
        virtual antlrcpp::Any visitInteger(BasicParser::IntegerContext *ctx) override;
        virtual antlrcpp::Any visitDecimal(BasicParser::DecimalContext *ctx) override;
        virtual antlrcpp::Any visitBucket(BasicParser::BucketContext *ctx) override;
        virtual antlrcpp::Any visitMulOp(BasicParser::MulOpContext *ctx) override;
        virtual antlrcpp::Any visitPowModOp(BasicParser::PowModOpContext *ctx) override;
        virtual antlrcpp::Any visitID(BasicParser::IDContext *ctx) override;
        virtual antlrcpp::Any visitBoolean(BasicParser::BooleanContext *ctx) override;
        virtual antlrcpp::Any visitBitOp(BasicParser::BitOpContext *ctx) override;
        virtual antlrcpp::Any visitLogicOp(BasicParser::LogicOpContext *ctx) override;
    };
}

#endif