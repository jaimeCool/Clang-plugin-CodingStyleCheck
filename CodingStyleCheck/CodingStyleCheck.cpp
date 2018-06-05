
#include "CodingStyleCheck.hpp"

string globalSrcRootPath;
static string kClassInterfacePrefix = "ESP";

namespace CodingStyleCheck
{
  
  /// CodingStyleCheckHandler
  void CodingStyleCheckHandler::setContext(ASTContext &context)
  {
    this->context = &context;
  }
  
  void CodingStyleCheckHandler::run(const MatchFinder::MatchResult &Result)
  {
    if (const ObjCPropertyDecl *propertyDecl = Result.Nodes.getNodeAs<ObjCPropertyDecl>("objcPropertyDecl")) {
      // 存储 Objective-C 类属性
      checkPropertyDecl(propertyDecl);
    } else if (const ObjCInterfaceDecl *interfaceDecl = Result.Nodes.getNodeAs<ObjCInterfaceDecl>("objcInterfaceDecl")) {
      checkInterfaceDecl(interfaceDecl);
    } else if (const BinaryOperator *binaryOperator = Result.Nodes.getNodeAs<BinaryOperator>("binaryOperator_modelOfClass")) {
      checkAppointedMethod(binaryOperator);
    } else if (const IfStmt *stmtIf = Result.Nodes.getNodeAs<IfStmt>("ifStmt_empty_then_body")) {
      SourceLocation location = stmtIf->getIfLoc();
      diagWaringReport(location, "Don't use empty body in IfStmt", NULL);
    } else if (const IfStmt *stmtIf = Result.Nodes.getNodeAs<IfStmt>("condition_always_true")) {
      SourceLocation location = stmtIf->getIfLoc();
      diagWaringReport(location, "Body will certainly be executed when condition true", NULL);
    } else if (const IfStmt *stmtIf = Result.Nodes.getNodeAs<IfStmt>("condition_always_false")) {
      SourceLocation location = stmtIf->getIfLoc();
      diagWaringReport(location, "Body will never be executed when condition false.", NULL);
    }
  }
  
  void CodingStyleCheckHandler::checkInterfaceDecl(const clang::ObjCInterfaceDecl *interfaceDecl)
  {
    StringRef name = interfaceDecl->getName();
    SourceLocation location = interfaceDecl->getLocation();
    SourceLocation nameEnd = location.getLocWithOffset(name.size()-1);
    
    if (name.find(kClassInterfacePrefix)!=0) {
      string tempName = string(kClassInterfacePrefix)+name.str();
      StringRef replacement(tempName);
      FixItHint fixItHint = FixItHint::CreateReplacement(SourceRange(location, nameEnd), replacement);
      diagWaringReport(location, "Class name should start with prefix ESP", &fixItHint);
      return;
    }
    if (!name.str().compare(kClassInterfacePrefix)) {
      string tempName = kClassInterfacePrefix+string("ClassName");
      StringRef replacement(tempName);
      FixItHint fixItHint = FixItHint::CreateReplacement(SourceRange(location, nameEnd), replacement);
      diagWaringReport(location, "Class name should not be XX", &fixItHint);
      return;
    }
    
    char tmpCh = name.str().at(kClassInterfacePrefix.length());
    if(islower(tmpCh)){
      string tempName = string(name.str());
      tempName.replace(kClassInterfacePrefix.length(), 1, string(1,toupper(tmpCh)));
      StringRef replacement(tempName);
      FixItHint fixItHint = FixItHint::CreateReplacement(SourceRange(location, nameEnd), replacement);
      diagWaringReport(location, "Class name should start with upper case.", &fixItHint);
      return;
    }
    if(name.find("_")!=string::npos) {
      string tempName(name.str());
      remove_char_from_string(tempName,'_');
      StringRef replacement(tempName);
      FixItHint fixItHint = FixItHint::CreateReplacement(SourceRange(location, nameEnd), replacement);
      diagWaringReport(location, "Class name should not contains '_'", &fixItHint);
      return;
    }
  }
  
  void CodingStyleCheckHandler::checkPropertyDecl(const clang::ObjCPropertyDecl *propertyDecl)
  {
    ObjCPropertyDecl::PropertyAttributeKind attrKind = propertyDecl->getPropertyAttributes();
    SourceLocation location = propertyDecl->getLocation();
    string typeStr = propertyDecl->getType().getAsString();
    nslog("typeStr", typeStr);
    
    if (propertyDecl->getTypeSourceInfo()) {
      if(!(attrKind & ObjCPropertyDecl::OBJC_PR_nonatomic)){
        diagWaringReport(location, "Are you sure to use atomic which might reduce the performance.", NULL);
      }
      
      if ((typeStr.find("NSString")!=string::npos)&& !(attrKind & ObjCPropertyDecl::OBJC_PR_copy)) {
        diagWaringReport(location, "NSString should use the attributes copy instead of strong.", NULL);
      } else if ((typeStr.find("NSArray")!=string::npos)&& !(attrKind & ObjCPropertyDecl::OBJC_PR_copy)) {
        diagWaringReport(location, "NSArray should use the attributes copy instead of strong.", NULL);
      }
      
      if(!typeStr.compare("int")){
        diagWaringReport(location, "Use the built-in NSInteger instead of int.", NULL);
      } else if ((typeStr.find("<")!=string::npos && typeStr.find(">")!=string::npos) && !(attrKind & ObjCPropertyDecl::OBJC_PR_weak)) {
        diagWaringReport(location, "Use weak declare Delegate.", NULL);
      }
    }
  }
  
  void CodingStyleCheckHandler::checkAppointedMethod(const clang::BinaryOperator *binaryOperator)
  {
    ObjCPropertyRefExpr *leftExpr = dyn_cast_or_null<ObjCPropertyRefExpr>(binaryOperator->getLHS());
    OpaqueValueExpr *rightExpr = dyn_cast_or_null<OpaqueValueExpr>(binaryOperator->getRHS());
    if (leftExpr && rightExpr) {
      std::string propertyName = leftExpr->getGetterSelector().getAsString();
      ObjCMessageExpr *messageExpr = dyn_cast_or_null<ObjCMessageExpr>(rightExpr->getSourceExpr());
      if (messageExpr) {
        nslog("存在", messageExpr->getMethodDecl()->getSelector().getAsString());
        for (Stmt *stmt : messageExpr->arguments()) {
          ObjCMessageExpr *callClassExpr = dyn_cast_or_null<ObjCMessageExpr>(stmt);
          if (callClassExpr && callClassExpr->getSelector().getAsString() == "class") {
            string leftType = removePtrString(leftExpr->getExplicitProperty()->getType().getAsString());
            string rightType = removePtrString(callClassExpr->getClassReceiver().getAsString());
            DiagnosticsEngine &diag = Instance.getDiagnostics();
            if (leftType.find(rightType) == std::string::npos) {
              FixItHint fixItHint = FixItHint::CreateReplacement(callClassExpr->getReceiverRange(), leftType);
              diag.Report(binaryOperator->getLocStart(), diag.getCustomDiagID(DiagnosticsEngine::Warning, "类型不一致：左边：%0 右边：%1")) << leftType << rightType << fixItHint;
            }
          }
        }
      }
    }
  }
  
  
  template <unsigned N>
  void CodingStyleCheckHandler::diagWaringReport(SourceLocation Loc,
                                                 const char (&FormatString)[N],
                                                 FixItHint *Hint)
  {
    DiagnosticsEngine &diagEngine = Instance.getDiagnostics();
    unsigned DiagID = diagEngine.getCustomDiagID(clang::DiagnosticsEngine::Warning, FormatString);
    (Hint!=NULL) ? diagEngine.Report(Loc, DiagID) << *Hint : diagEngine.Report(Loc, DiagID);
  }
  
  
  /// CodingStyleCheckASTConsumer
  CodingStyleCheckASTConsumer::CodingStyleCheckASTConsumer(CompilerInstance &Instance) : handlerForMatchResult(Instance)
  {
    // just match Main File, up match speed
    matcher.addMatcher(objcInterfaceDecl(isExpansionInMainFile()).bind("objcInterfaceDecl"), &handlerForMatchResult);
    matcher.addMatcher(objcPropertyDecl(isExpansionInMainFile()).bind("objcPropertyDecl"), &handlerForMatchResult);
    matcher.addMatcher(binaryOperator(hasDescendant(opaqueValueExpr(hasSourceExpression(objcMessageExpr(hasSelector("modelOfClass:"))))),isExpansionInMainFile()).bind("binaryOperator_modelOfClass"), &handlerForMatchResult);
    //match ifStmt
    matcher.addMatcher(ifStmt(isExpansionInMainFile(),hasThen(compoundStmt(statementCountIs(0)))).bind("ifStmt_empty_then_body"), &handlerForMatchResult);
    matcher.addMatcher(ifStmt(isExpansionInMainFile(),hasCondition(integerLiteral(equals(1)))).bind("condition_always_true"), &handlerForMatchResult);
    matcher.addMatcher(ifStmt(isExpansionInMainFile(),hasCondition(floatLiteral(equals(0.0)))).bind("condition_always_false"), &handlerForMatchResult);
    matcher.addMatcher(ifStmt(isExpansionInMainFile(),hasCondition(integerLiteral(equals(0)))).bind("condition_always_false"), &handlerForMatchResult);
  }
  
  void CodingStyleCheckASTConsumer::HandleTranslationUnit(ASTContext &context)
  {
    handlerForMatchResult.setContext(context);
    matcher.matchAST(context);
  }
  
  
  /// CodingStyleCheckASTAction
  unique_ptr<ASTConsumer> CodingStyleCheckASTAction::CreateASTConsumer(CompilerInstance &Compiler, StringRef InFile)
  {
    return unique_ptr<CodingStyleCheckASTConsumer>(new CodingStyleCheckASTConsumer(Compiler));
  }
  
  bool CodingStyleCheckASTAction::ParseArgs(const CompilerInstance &CI, const std::vector<std::string> &args)
  {
    return true;
  }
}

static clang::FrontendPluginRegistry::Add<CodingStyleCheck::CodingStyleCheckASTAction>
X("coding-style-check", "check code style");
