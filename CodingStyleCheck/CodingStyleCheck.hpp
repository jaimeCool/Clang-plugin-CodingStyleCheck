
#ifndef CodingStyleCheck_hpp
#define CodingStyleCheck_hpp

#include <iostream>
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Basic/Diagnostic.h"
#include "CustomPluginUtil.hpp"

using namespace clang;
using namespace std;
using namespace llvm;
using namespace clang::ast_matchers;

namespace CodingStyleCheck
{
  class CodingStyleCheckHandler : public MatchFinder::MatchCallback
  {
  private:
    CompilerInstance &Instance;
    ASTContext *context;
    std::vector<ObjCPropertyDecl *> propertyDeclVector;
    
  public:
    CodingStyleCheckHandler(CompilerInstance &Instance) : Instance(Instance) {}
    void setContext(ASTContext &context);
    virtual void run(const MatchFinder::MatchResult &Result);
    
    void checkInterfaceDecl(const ObjCInterfaceDecl* interfaceDecl);
    void checkPropertyDecl(const ObjCPropertyDecl* propertyDecl);
    void checkAppointedMethod(const BinaryOperator* binaryOperator);
    
    template <unsigned N>
    void diagWaringReport(SourceLocation Loc,
                          const char (&FormatString)[N],
                          FixItHint *Hint);
    
    string getPropertyType(const string propertyName);
  };
  
  
  class CodingStyleCheckASTConsumer: public ASTConsumer
  {
  public:
    CodingStyleCheckASTConsumer(CompilerInstance &Instance);
    
  private:
    MatchFinder matcher;
    CodingStyleCheckHandler handlerForMatchResult;
    
    void HandleTranslationUnit(ASTContext &context);
  };
  
  class CodingStyleCheckASTAction: public PluginASTAction
  {
  public:
    unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &Compiler, StringRef InFile);
    bool ParseArgs(const CompilerInstance &CI, const std::vector<std::string> &args);
  };
}


#endif /* CodingStyleCheck_hpp */
