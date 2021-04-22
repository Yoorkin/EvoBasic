//
// Created by yorkin on 4/16/21.
//

#include"codeGen.h"


string strToLower(string str){
    transform(str.begin(),str.end(),str.begin(),[](unsigned char c){ return std::tolower(c); });
    return str;
}

GenerateUnit::GenerateUnit(CodeGenerator& gen,string path,string name,istream& in,ostream& out)
    :gen(gen),in(in),out(out),context(gen.context),mod(name,gen.context),reporter(out,in,path),errorListener(reporter){
    visitor=new Visitor(*this);
}

void GenerateUnit::generate(){
    gen.reporter=&this->reporter;
    ANTLRInputStream input(in);
    BasicLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    BasicParser parser(&tokens);
    parser.removeErrorListeners();
    parser.addErrorListener(&errorListener);
    tree::ParseTree *tree = parser.moduleBody();
    visitor->visit(tree);
    gen.reporter=nullptr;
    //mod.print(outs(),nullptr,false);
}
void GenerateUnit::printIR(){
    mod.print(outs(),nullptr,false);
}

AllocaInst* GenerateUnit::findInst(Token* id){
    string name = strToLower(id->getText());
    for(auto iter=frame.rbegin();iter!=frame.rend();iter++){
        auto var=iter->varTable.find(name);
        if(var!=iter->varTable.end()){
            return var->second;
        }
    }
    reporter.report(id,"Can not find variable "+name);
    return nullptr;
}

void GenerateUnit::addInst(Token* id,AllocaInst* inst){
    string name = strToLower(id->getText());
    auto p = frame.back().varTable.find(name);
    if(p!=frame.back().varTable.end())
        reporter.report(id,"redefinition of '"+name+"'");
    else
        frame.back().varTable.insert(make_pair(name,inst));
}

CodeGenerator::CodeGenerator(){
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();
    typeTable=new TypeTable(*this);
}

TypeTable::TypeTable(CodeGenerator& generator):gen(generator){
        builtInTypes.operator=({
            {"integer",Type::getInt32Ty(gen.context)},
            {"single",Type::getFloatTy(gen.context)},
            {"double",Type::getDoubleTy(gen.context)},
            {"boolean",Type::getInt1Ty(gen.context)},
            {"long",Type::getInt64Ty(gen.context)},
            {"byte",Type::getInt8Ty(gen.context)}
        });
}


Type* TypeTable::find(Token* type){
    string name = strToLower(type->getText());
    auto builtIn = builtInTypes.find(name);
    if(builtIn!=builtInTypes.end())return builtIn->second;
    if(gen.reporter!=nullptr)
        gen.reporter->report(type,"unexpected type "+type->getText());
    else
        throw "Can not find reporter.\n";
    return Type::getInt32Ty(gen.context);
}

//===================================== visitor =========================================================================

//void Visitor::storeValue(Token* token){
//    string name = strToLower(token->getText());
//    for(auto iter=frame.rbegin();iter!=frame.rend();iter++){
//        auto var=iter->varTable.find(name);
//        if(var!=iter->varTable.end()){
//            return builder.CreateLoad(var->second->getType(),var->second);
//        }
//        auto arg=iter->argTable.find(name);
//        if(arg!=iter->argTable.end()){
//            return arg->second;
//        }
//    }
//    builder.Creat
//    reporter.report(token->getLine(),token->getCharPositionInLine(),"Can not find variable "+name);
//    return nullptr;
//}

Visitor::Visitor(GenerateUnit& unit)
:typeTable(*unit.gen.typeTable),reporter(unit.reporter),frame(unit.frame),context(unit.context),mod(unit.mod),builder(unit.context),unit(unit){
}

antlrcpp::Any Visitor::visitForStmt(BasicParser::ForStmtContext *ctx){
    frame.back().BeginLayer("For");
    Value* iter;
    Type* type;
    string name = strToLower(ctx->iterator->getText());
    if(ctx->Dim()==nullptr){
        iter=unit.findInst(ctx->iterator);
        type=((AllocaInst*)iter)->getAllocatedType();
    }
    else{
        type = visit(ctx->type).as<Type*>();//TODO: 支持数组与结构体
        iter = builder.CreateAlloca(type,nullptr,name);
        unit.addInst(ctx->iterator,(AllocaInst*)iter);
    }

    auto begin=visit(ctx->begin).as<Value*>(),span=visit(ctx->end).as<Value*>();
    auto end=builder.CreateAdd(begin,span);
    Value* step;
    if(ctx->Step() == nullptr) step=ConstantInt::get(Type::getInt32Ty(context),1);
    else step=visit(ctx->step).as<Value*>();
    builder.CreateStore(begin,iter);


    auto condition = BasicBlock::Create(context,frame.back().getBlockName("condition"),frame.back().function);
    auto loop = BasicBlock::Create(context,frame.back().getBlockName("Loop"),frame.back().function);
    auto after = BasicBlock::Create(context,frame.back().getBlockName("After"),frame.back().function);
    builder.CreateBr(loop);

    builder.SetInsertPoint(condition);
    auto val = builder.CreateLoad(type,iter);
    auto added = builder.CreateAdd(val,step);
    builder.CreateStore(added,iter);
    auto cond = builder.CreateCmp(CmpInst::Predicate::ICMP_SLE,added,end);
    builder.CreateCondBr(cond,loop,after);

    builder.SetInsertPoint(loop);
    visitBlock(ctx->block);
    builder.CreateBr(condition);

    builder.SetInsertPoint(after);

    if(ctx->nextFlag!=nullptr){
        string flag=strToLower(ctx->nextFlag->getText());
        if(name!=flag)
            reporter.report(ctx->nextFlag,"Unexpected next flag '"+flag+"',require '"+name+"'");
    }
    frame.back().EndLayer();
    return nullptr;
}

antlrcpp::Any Visitor::visitDoWhile(BasicParser::DoWhileContext *ctx){
    frame.back().BeginLayer("Loop");
    auto condition = visit(ctx->exp()).as<Value*>();
    auto condBlock = BasicBlock::Create(context,frame.back().getBlockName("LoopCondition"),frame.back().function);
    auto loop = BasicBlock::Create(context,frame.back().getBlockName("Loop"),frame.back().function);
    auto loopEnd = BasicBlock::Create(context,frame.back().getBlockName("LoopEnd"),frame.back().function);
    builder.CreateBr(condBlock);
    builder.SetInsertPoint(condBlock);
    builder.CreateCondBr(condition,loop,loopEnd);
    builder.SetInsertPoint(loop);
    visitBlock(ctx->block);
    builder.CreateBr(condBlock);
    builder.SetInsertPoint(loopEnd);
    frame.back().EndLayer();
    return nullptr;
}

antlrcpp::Any Visitor::visitDoUntil(BasicParser::DoUntilContext *ctx){
    frame.back().BeginLayer("Loop");
    auto condition = visit(ctx->exp()).as<Value*>();
    auto condBlock = BasicBlock::Create(context,frame.back().getBlockName("LoopCondition"),frame.back().function);
    auto loop = BasicBlock::Create(context,frame.back().getBlockName("Loop"),frame.back().function);
    auto loopEnd = BasicBlock::Create(context,frame.back().getBlockName("LoopEnd"),frame.back().function);
    builder.CreateBr(condBlock);
    builder.SetInsertPoint(condBlock);
    builder.CreateCondBr(condition,loopEnd,loop);
    builder.SetInsertPoint(loop);
    visitBlock(ctx->block);
    builder.CreateBr(condBlock);
    builder.SetInsertPoint(loopEnd);
    frame.back().EndLayer();
    return nullptr;
}

antlrcpp::Any Visitor::visitLoopUntil(BasicParser::LoopUntilContext *ctx){
    frame.back().BeginLayer("Loop");
    auto condition = visit(ctx->exp()).as<Value*>();
    auto loop = BasicBlock::Create(context,frame.back().getBlockName("Loop"),frame.back().function);
    auto condBlock = BasicBlock::Create(context,frame.back().getBlockName("LoopCondition"),frame.back().function);
    auto loopEnd = BasicBlock::Create(context,frame.back().getBlockName("LoopEnd"),frame.back().function);
    builder.CreateBr(loop);
    builder.SetInsertPoint(condBlock);
    builder.CreateCondBr(condition,loopEnd,loop);
    builder.SetInsertPoint(loop);
    visitBlock(ctx->block);
    builder.CreateBr(condBlock);
    builder.SetInsertPoint(loopEnd);
    frame.back().EndLayer();
    return nullptr;
}

antlrcpp::Any Visitor::visitLoopWhile(BasicParser::LoopWhileContext *ctx){
    frame.back().BeginLayer("Loop");
    auto condition = visit(ctx->exp()).as<Value*>();
    auto loop = BasicBlock::Create(context,frame.back().getBlockName("Loop"),frame.back().function);
    auto condBlock = BasicBlock::Create(context,frame.back().getBlockName("LoopCondition"),frame.back().function);
    auto loopEnd = BasicBlock::Create(context,frame.back().getBlockName("LoopEnd"),frame.back().function);
    builder.CreateBr(loop);
    builder.SetInsertPoint(condBlock);
    builder.CreateCondBr(condition,loop,loopEnd);
    builder.SetInsertPoint(loop);
    visitBlock(ctx->block);
    builder.CreateBr(condBlock);
    builder.SetInsertPoint(loopEnd);
    frame.back().EndLayer();
    return nullptr;
}

antlrcpp::Any Visitor::visitWhileWend(BasicParser::WhileWendContext *ctx){
    frame.back().BeginLayer("Loop");
    auto condition = visit(ctx->exp()).as<Value*>();
    auto condBlock = BasicBlock::Create(context,frame.back().getBlockName("LoopCondition"),frame.back().function);
    auto loop = BasicBlock::Create(context,frame.back().getBlockName("Loop"),frame.back().function);
    auto loopEnd = BasicBlock::Create(context,frame.back().getBlockName("LoopEnd"),frame.back().function);
    builder.CreateBr(condBlock);
    builder.SetInsertPoint(condBlock);
    builder.CreateCondBr(condition,loop,loopEnd);
    builder.SetInsertPoint(loop);
    visitBlock(ctx->block);
    builder.CreateBr(condBlock);
    builder.SetInsertPoint(loopEnd);
    frame.back().EndLayer();
    return nullptr;
}

antlrcpp::Any Visitor::visitSingleLineIf(BasicParser::SingleLineIfContext *ctx){
    return nullptr;
}

antlrcpp::Any Visitor::visitMutiLineIf(BasicParser::MutiLineIfContext *ctx){
    list<BasicBlock*> trueBlocks;
    for(auto c:ctx->ifBlock())trueBlocks.push_back(visit(c).as<BasicBlock*>());
    for(auto b:ctx->elseBlock)visit(b);
    auto endBlock = BasicBlock::Create(context,frame.back().getBlockName("IF_End"),frame.back().function);
    builder.CreateBr(endBlock);
    for(auto b:trueBlocks){
    builder.SetInsertPoint(b);
    builder.CreateBr(endBlock);
    }
    builder.SetInsertPoint(endBlock);
    return nullptr;
}

antlrcpp::Any Visitor::visitIfBlock(BasicParser::IfBlockContext *ctx){
    frame.back().BeginLayer("If");
    Value* cond = visit(ctx->exp()).as<Value*>();
    auto trueBlock = BasicBlock::Create(context,frame.back().getBlockName("True"),frame.back().function);
    auto falseBlock = BasicBlock::Create(context,frame.back().getBlockName("False"),frame.back().function);
    builder.CreateCondBr(cond,trueBlock,falseBlock);
    builder.SetInsertPoint(trueBlock);
    visitBlock(ctx->block);
    builder.SetInsertPoint(falseBlock);
    frame.back().EndLayer();
    return trueBlock;
}

//============================================ declare ====================================================


antlrcpp::Any Visitor::visitVarDecl(BasicParser::VarDeclContext *ctx){
    for(auto arg:ctx->variable()){
        auto info = visit(arg).as<ArgumentInfo>();
        auto ptr = builder.CreateAlloca(info.type,nullptr,info.name);
        unit.addInst(info.token,ptr);
        if(info.initial!=nullptr){
            auto value = visit(info.initial).as<Value*>();
            builder.CreateStore(value,ptr);
        }
    }
    return visitChildren(ctx);
}

antlrcpp::Any Visitor::visitTypeDecl(BasicParser::TypeDeclContext *ctx){
    vector<Type*> paramList;
    vector<ArgumentInfo> arguments;
    for(auto arg:ctx->variable()){
        arguments.push_back(visit(arg).as<ArgumentInfo>());
        paramList.push_back(arguments.back().type);
    }
    return StructType::create(paramList,strToLower(ctx->name->getText()));
}

antlrcpp::Any Visitor::visitVariable(BasicParser::VariableContext *ctx){
    ArgumentInfo info(ctx,typeTable);
    return info;
}

antlrcpp::Any Visitor::visitFunctionDecl(BasicParser::FunctionDeclContext *ctx){
    vector<Type*> paramList;
    vector<ArgumentInfo> arguments;
    for(auto arg:ctx->variable()){
        arguments.push_back(visitVariable(arg).as<ArgumentInfo>());
        paramList.push_back(arguments.back().type);
    }
    Type* retType = typeTable.find(ctx->returnType);
    FunctionType *type = FunctionType::get(retType,paramList,false);
    auto function = Function::Create(type,Function::ExternalLinkage,strToLower(ctx->name->getText()),mod);
    frame.push_back(StackFrame(function));
    auto block = BasicBlock::Create(context, "EntryBlock", function);
    builder.SetInsertPoint(block);
    auto param = function->arg_begin();
    for(auto& arg:arguments){
        param->setName(arg.name);
        auto inst = builder.CreateAlloca(arg.type,nullptr);
        builder.CreateStore(param,inst);
        unit.addInst(arg.token,inst);
        param++;
    }
    visitBlock(ctx->block);
    frame.pop_back();
    return function;
}

antlrcpp::Any Visitor::visitSubDecl(BasicParser::SubDeclContext *ctx){
    vector<Type*> paramList;
    vector<ArgumentInfo> arguments;
    for(auto& arg:ctx->variable()){
        arguments.push_back(visit(arg).as<ArgumentInfo>());
        paramList.push_back(arguments.back().type);
    }
    FunctionType *type = FunctionType::get(Type::getVoidTy(context),paramList,false);
    string funcionName = ctx->name->getText();
    auto function = Function::Create(type,Function::ExternalLinkage,strToLower(ctx->name->getText()),mod);
    frame.push_back(StackFrame(function));
    auto block = BasicBlock::Create(context, "EntryBlock", function);
    builder.SetInsertPoint(block);
    auto param = function->arg_begin();
    for(auto& arg:arguments){
        param->setName(arg.name);
        auto inst = builder.CreateAlloca(arg.type,nullptr);
        builder.CreateStore(param,inst);
        unit.addInst(arg.token,inst);
        param++;
    }
    visitBlock(ctx->block);
    frame.pop_back();
    return function;
}
//====================================== call statement =========================================
antlrcpp::Any Visitor::visitInnerCall(BasicParser::InnerCallContext *ctx){
    AllocaInst* inst = unit.findInst(ctx->ID()->getSymbol());
    return (Value*)builder.CreateLoad(inst->getAllocatedType(),inst);
}

antlrcpp::Any Visitor::visitArgPassValue(BasicParser::ArgPassValueContext *ctx){
    return visitChildren(ctx);

}
antlrcpp::Any Visitor::visitArgIgnore(BasicParser::ArgIgnoreContext *ctx){
    return visitChildren(ctx);
}
antlrcpp::Any Visitor::visitArgOptional(BasicParser::ArgOptionalContext *ctx){
    return visitChildren(ctx);
}


antlrcpp::Any Visitor::visitReturnStmt(BasicParser::ReturnStmtContext *ctx){
    auto val = visit(ctx->exp()).as<Value*>();
    builder.CreateRet(val);
    return nullptr;
}

antlrcpp::Any Visitor::visitAssignStmt(BasicParser::AssignStmtContext *ctx){
    auto val = visit(ctx->right).as<Value*>();
    auto inst = unit.findInst(ctx->left);
    builder.CreateStore(val,inst);
    return (Value*)inst;
}

antlrcpp::Any Visitor::visitPluOp(BasicParser::PluOpContext *ctx){
    auto l = visit(ctx->left).as<llvm::Value*>();
    auto r = visit(ctx->right).as<llvm::Value*>();
    auto op = ctx->op->getText();
    if(op=="+")
    return builder.CreateAdd(l,r);
    else
    return builder.CreateSub(l,r);
}

//map<string,CmpInst::Predicate> cmpIntSigned={
//        {"=",CmpInst::Predicate::ICMP_EQ},
//        {"<>",CmpInst::Predicate::ICMP_NE},
//        {"<",CmpInst::Predicate::ICMP_SLT},
//        {"<=",CmpInst::Predicate::ICMP_SLE},
//        {"=<",CmpInst::Predicate::ICMP_SLE},
//        {">",CmpInst::Predicate::ICMP_SGT},
//        {">=",CmpInst::Predicate::ICMP_SGE},
//        {"=>",CmpInst::Predicate::ICMP_SGE}
//};

antlrcpp::Any Visitor::visitCmpOp(BasicParser::CmpOpContext *ctx){
    auto l = visit(ctx->left).as<llvm::Value*>();
    auto r = visit(ctx->right).as<llvm::Value*>();
    auto op = ctx->op->getText();
    return builder.CreateCmp(cmpIntSigned[op],l,r);
    //TODO:支持不同类型
}

antlrcpp::Any Visitor::visitLogicNotOp(BasicParser::LogicNotOpContext *ctx){
    auto r = visit(ctx->right).as<llvm::Value*>();
    return builder.CreateNot(r);
}

antlrcpp::Any Visitor::visitNegOp(BasicParser::NegOpContext *ctx){
    auto r = visit(ctx->right).as<llvm::Value*>();
    return builder.CreateNeg(r);
}

antlrcpp::Any Visitor::visitString(BasicParser::StringContext *ctx){
    return visitChildren(ctx);//TODO:String
}

antlrcpp::Any Visitor::visitInteger(BasicParser::IntegerContext *ctx){
    //TODO:支持浮点数/整数区分
    int n=std::stoi(ctx->Integer()->getSymbol()->getText());
    return (Value*)ConstantInt::get(Type::getInt32Ty(context),n,true);
}

antlrcpp::Any Visitor::visitDecimal(BasicParser::DecimalContext *ctx){
    cout<<"double\n";
    double n=std::stod(ctx->Decimal()->getSymbol()->getText());
    return (Value*)ConstantFP::get(Type::getDoubleTy(context),n);
}

antlrcpp::Any Visitor::visitBucket(BasicParser::BucketContext *ctx){
    return visit(ctx->exp());
}

antlrcpp::Any Visitor::visitMulOp(BasicParser::MulOpContext *ctx){
    auto l = visit(ctx->left).as<llvm::Value*>();
    auto r = visit(ctx->right).as<llvm::Value*>();
    auto op = ctx->op->getText();
    if(op=="*")return builder.CreateMul(l,r);
    else if(op=="/"||op=="\\")return builder.CreateSDiv(l,r);//TODO: 支持两种除法
}

antlrcpp::Any Visitor::visitPowModOp(BasicParser::PowModOpContext *ctx){
    auto l = visit(ctx->left).as<llvm::Value*>();
    auto r = visit(ctx->right).as<llvm::Value*>();
    auto op = strToLower(ctx->op->getText());
    return visitChildren(ctx);//TODO:pow和mod运算
}

antlrcpp::Any Visitor::visitID(BasicParser::IDContext *ctx){
    //TODO 检查这个函数还有没有用
    //auto p = frame.back().varTable.find(strToLower(ctx->ID()->getText()));
    //Value* val = builder.CreateLoad(p->second->getType(),p->second);
    //return val;//TODO：访问变量
    return nullptr;
}

antlrcpp::Any Visitor::visitBoolean(BasicParser::BooleanContext *ctx){
    char val = toLower(ctx->Boolean()->getText()[0]);
    if(val=='t')return (Value*)ConstantInt::getTrue(context);
    else return (Value*)ConstantInt::getFalse(context);
}

antlrcpp::Any Visitor::visitBitOp(BasicParser::BitOpContext *ctx){
    return visitChildren(ctx);//TODO:位运算
}

antlrcpp::Any Visitor::visitLogicOp(BasicParser::LogicOpContext *ctx){
    auto l = visit(ctx->left).as<llvm::Value*>();
    auto r = visit(ctx->right).as<llvm::Value*>();
    auto op = strToLower(ctx->op->getText());
    if(op=="and")return builder.CreateAnd(l,r);
    else if(op=="or")return builder.CreateOr(l,r);
    else if(op=="xor")return builder.CreateXor(l,r);
}