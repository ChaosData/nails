// Declares clang::SyntaxOnlyAction.
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

// Declares llvm::cl::extrahelp.
#include "llvm/Support/CommandLine.h"

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/AST/CXXInheritance.h"

#include <stdio.h>

#include <sstream>
#include <unordered_map>
#include <vector>
#include <fstream>

#include "format_string.h"

using namespace clang::tooling;
using namespace clang::ast_matchers;

static llvm::cl::OptionCategory MyToolCategory("my-tool options");
static llvm::cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

void getExprType(clang::Expr const* sc) {
  if (clang::isa<clang::SubstNonTypeTemplateParmPackExpr>(sc)) { puts("clang::SubstNonTypeTemplateParmPackExpr"); }
  if (clang::isa<clang::CXXDefaultInitExpr>(sc)) { puts("clang::CXXDefaultInitExpr"); }
  if (clang::isa<clang::CXXStdInitializerListExpr>(sc)) { puts("clang::CXXStdInitializerListExpr"); }
  if (clang::isa<clang::DesignatedInitUpdateExpr>(sc)) { puts("clang::DesignatedInitUpdateExpr"); }
  if (clang::isa<clang::AsTypeExpr>(sc)) { puts("clang::AsTypeExpr"); }
  if (clang::isa<clang::CXXBindTemporaryExpr>(sc)) { puts("clang::CXXBindTemporaryExpr"); }
  if (clang::isa<clang::CXXConstructExpr>(sc)) { puts("clang::CXXConstructExpr"); }
  if (clang::isa<clang::ObjCPropertyRefExpr>(sc)) { puts("clang::ObjCPropertyRefExpr"); }
  if (clang::isa<clang::PackExpansionExpr>(sc)) { puts("clang::PackExpansionExpr"); }
  if (clang::isa<clang::ObjCIsaExpr>(sc)) { puts("clang::ObjCIsaExpr"); }
  if (clang::isa<clang::CXXFoldExpr>(sc)) { puts("clang::CXXFoldExpr"); }
  if (clang::isa<clang::IntegerLiteral>(sc)) { puts("clang::IntegerLiteral"); }
  if (clang::isa<clang::CXXNoexceptExpr>(sc)) { puts("clang::CXXNoexceptExpr"); }
  if (clang::isa<clang::CXXNewExpr>(sc)) { puts("clang::CXXNewExpr"); }
  if (clang::isa<clang::ObjCEncodeExpr>(sc)) { puts("clang::ObjCEncodeExpr"); }
  if (clang::isa<clang::AtomicExpr>(sc)) { puts("clang::AtomicExpr"); }
  if (clang::isa<clang::ArrayTypeTraitExpr>(sc)) { puts("clang::ArrayTypeTraitExpr"); }
  if (clang::isa<clang::ObjCProtocolExpr>(sc)) { puts("clang::ObjCProtocolExpr"); }
  if (clang::isa<clang::VAArgExpr>(sc)) { puts("clang::VAArgExpr"); }
  if (clang::isa<clang::BinaryOperator>(sc)) { puts("clang::BinaryOperator"); }
  if (clang::isa<clang::ObjCSelectorExpr>(sc)) { puts("clang::ObjCSelectorExpr"); }
  if (clang::isa<clang::ExpressionTraitExpr>(sc)) { puts("clang::ExpressionTraitExpr"); }
  if (clang::isa<clang::ObjCIndirectCopyRestoreExpr>(sc)) { puts("clang::ObjCIndirectCopyRestoreExpr"); }
  if (clang::isa<clang::StmtExpr>(sc)) { puts("clang::StmtExpr"); }
  if (clang::isa<clang::ObjCIvarRefExpr>(sc)) { puts("clang::ObjCIvarRefExpr"); }
  if (clang::isa<clang::GenericSelectionExpr>(sc)) { puts("clang::GenericSelectionExpr"); }
  if (clang::isa<clang::ConvertVectorExpr>(sc)) { puts("clang::ConvertVectorExpr"); }
  if (clang::isa<clang::OverloadExpr>(sc)) { puts("clang::OverloadExpr"); }
  if (clang::isa<clang::CXXDeleteExpr>(sc)) { puts("clang::CXXDeleteExpr"); }
  if (clang::isa<clang::PredefinedExpr>(sc)) { puts("clang::PredefinedExpr"); }
  if (clang::isa<clang::NoInitExpr>(sc)) { puts("clang::NoInitExpr"); }
  if (clang::isa<clang::ParenExpr>(sc)) { puts("clang::ParenExpr"); }
  if (clang::isa<clang::ObjCStringLiteral>(sc)) { puts("clang::ObjCStringLiteral"); }
  if (clang::isa<clang::FloatingLiteral>(sc)) { puts("clang::FloatingLiteral"); }
  if (clang::isa<clang::CXXUuidofExpr>(sc)) { puts("clang::CXXUuidofExpr"); }
  if (clang::isa<clang::ShuffleVectorExpr>(sc)) { puts("clang::ShuffleVectorExpr"); }
  if (clang::isa<clang::CXXNullPtrLiteralExpr>(sc)) { puts("clang::CXXNullPtrLiteralExpr"); }
  if (clang::isa<clang::ExtVectorElementExpr>(sc)) { puts("clang::ExtVectorElementExpr"); }
  if (clang::isa<clang::CXXPseudoDestructorExpr>(sc)) { puts("clang::CXXPseudoDestructorExpr"); }
  if (clang::isa<clang::SubstNonTypeTemplateParmExpr>(sc)) { puts("clang::SubstNonTypeTemplateParmExpr"); }
  if (clang::isa<clang::CastExpr>(sc)) { puts("clang::CastExpr"); }
  if (clang::isa<clang::BlockExpr>(sc)) { puts("clang::BlockExpr"); }
  if (clang::isa<clang::MaterializeTemporaryExpr>(sc)) { puts("clang::MaterializeTemporaryExpr"); }
  if (clang::isa<clang::UnaryOperator>(sc)) { puts("clang::UnaryOperator"); }
  if (clang::isa<clang::StringLiteral>(sc)) { puts("clang::StringLiteral"); }
  if (clang::isa<clang::CompoundLiteralExpr>(sc)) { puts("clang::CompoundLiteralExpr"); }
  if (clang::isa<clang::CXXBoolLiteralExpr>(sc)) { puts("clang::CXXBoolLiteralExpr"); }
  if (clang::isa<clang::MSPropertySubscriptExpr>(sc)) { puts("clang::MSPropertySubscriptExpr"); }
  if (clang::isa<clang::ArraySubscriptExpr>(sc)) { puts("clang::ArraySubscriptExpr"); }
  if (clang::isa<clang::CharacterLiteral>(sc)) { puts("clang::CharacterLiteral"); }
  if (clang::isa<clang::CallExpr>(sc)) { puts("clang::CallExpr"); }
  if (clang::isa<clang::CXXThisExpr>(sc)) { puts("clang::CXXThisExpr"); }
  if (clang::isa<clang::MSPropertyRefExpr>(sc)) { puts("clang::MSPropertyRefExpr"); }
  if (clang::isa<clang::ObjCBoolLiteralExpr>(sc)) { puts("clang::ObjCBoolLiteralExpr"); }
  if (clang::isa<clang::GNUNullExpr>(sc)) { puts("clang::GNUNullExpr"); }
  if (clang::isa<clang::UnaryExprOrTypeTraitExpr>(sc)) { puts("clang::UnaryExprOrTypeTraitExpr"); }
  if (clang::isa<clang::ImplicitValueInitExpr>(sc)) { puts("clang::ImplicitValueInitExpr"); }
  if (clang::isa<clang::InitListExpr>(sc)) { puts("clang::InitListExpr"); }
  if (clang::isa<clang::ChooseExpr>(sc)) { puts("clang::ChooseExpr"); }
  if (clang::isa<clang::ParenListExpr>(sc)) { puts("clang::ParenListExpr"); }
  if (clang::isa<clang::ObjCBoxedExpr>(sc)) { puts("clang::ObjCBoxedExpr"); }
  if (clang::isa<clang::CXXScalarValueInitExpr>(sc)) { puts("clang::CXXScalarValueInitExpr"); }
  if (clang::isa<clang::CXXTypeidExpr>(sc)) { puts("clang::CXXTypeidExpr"); }
  if (clang::isa<clang::AddrLabelExpr>(sc)) { puts("clang::AddrLabelExpr"); }
  if (clang::isa<clang::OpaqueValueExpr>(sc)) { puts("clang::OpaqueValueExpr"); }
  if (clang::isa<clang::ImaginaryLiteral>(sc)) { puts("clang::ImaginaryLiteral"); }
  if (clang::isa<clang::CXXThrowExpr>(sc)) { puts("clang::CXXThrowExpr"); }
  if (clang::isa<clang::TypoExpr>(sc)) { puts("clang::TypoExpr"); }
  if (clang::isa<clang::AbstractConditionalOperator>(sc)) { puts("clang::AbstractConditionalOperator"); }
  if (clang::isa<clang::ObjCSubscriptRefExpr>(sc)) { puts("clang::ObjCSubscriptRefExpr"); }
  if (clang::isa<clang::CoroutineSuspendExpr>(sc)) { puts("clang::CoroutineSuspendExpr"); }
}

DeclarationMatcher NailsRouteMatcher = functionTemplateDecl(
  allOf(
    hasParent(
      cxxRecordDecl(
        isDerivedFrom(
          hasName("BaseController")
        )
      )
    ),
    anyOf(
      hasDescendant(
        nonTypeTemplateParmDecl(
          hasType(
            references(
              cxxRecordDecl(
                hasName("nails_url")
              )
            )
          ),
          hasName("url")
        ).bind("route_param")
      ),
      hasDescendant(
        nonTypeTemplateParmDecl(
          hasType(
            references(
              cxxRecordDecl(
                hasName("nails_methods")
              )
            )
          ),
          hasName("methods")
        ).bind("route_param")
      )
    ),
    isPublic()
  )
);

DeclarationMatcher NailsMagicMatcher = cxxMethodDecl(
  allOf(
    hasParent(
      cxxRecordDecl(
        isDerivedFrom(
          hasName("BaseController")
        )
      )
    ),
    returns(
      asString("class nails::Response")
    ),
    hasParameter(0,
      hasType(
        references(
          cxxRecordDecl(
            hasName("Request")
          )
        )
      )
    ),
    // hasParameter(1,
    //   hasType(
    //     references(
    //       cxxRecordDecl(
    //         hasName("Response")
    //       )
    //     )
    //   )
    // ),
    isPublic()
  )
).bind("magic_method");


bool endsWith(std::string const& str, std::string const& tail) {
  size_t strL = str.size(), tailL = tail.size();
  if (strL >= tailL) {
    return 0 == str.compare(strL - tailL, tailL, tail);
  }
  return false;
}

std::unordered_map<std::string, std::vector<std::string>> magic_mapper;

class NailsMagicCallback : public MatchFinder::MatchCallback {
public:
  virtual void run(MatchFinder::MatchResult const& result) {
    if (auto const * FS = result.Nodes.getNodeAs<
                            clang::CXXMethodDecl
                          >("magic_method")) {
      clang::CXXRecordDecl const* parent = FS->getParent();
      std::string controller = parent->getNameAsString();
      if (!endsWith(controller, "Controller")) { return; }
      std::vector<std::string>& mv = magic_mapper[controller];
      mv.push_back(FS->getNameAsString());
    }
  }
};

/*
std::unordered_map<std::string,
                   std::tuple<std::string,
                              std::string,
                              std::vector<std::string>>
                   > route_mapper;
*/
std::unordered_map<std::string,
                   std::vector<
                     std::tuple<
                       std::string,
                       std::string,
                       std::vector<std::string>>
                     >
                   > route_mapper;


class NailsRouteCallback : public MatchFinder::MatchCallback {
public:
  virtual void run(MatchFinder::MatchResult const& result) {
    bool failed = true;
    std::string controller;
    std::string action;
    std::string path;
    std::vector<std::string> methods;
    std::string error_info;

    if (auto const * FS = result.Nodes.getNodeAs<
                            clang::NonTypeTemplateParmDecl
                          >("route_param")) {
      clang::DeclContext const* dc = FS->getParentFunctionOrMethod();
      if (dc != nullptr) {
        auto* md = clang::cast<clang::CXXMethodDecl>(dc);
        auto const* parent = md->getParent();
        clang::QualType rt = md->getReturnType();
        if (rt.getAsString().find("app_route<") == 0
            && clang::isa<clang::CXXRecordDecl>(parent)) {
          auto const* rd = clang::cast<clang::CXXRecordDecl>(parent);
          bool found_BaseController = false;

          clang::CXXBasePaths cbp;
          found_BaseController = rd->lookupInBases(
            [](clang::CXXBaseSpecifier const* Specifier,
               clang::CXXBasePath& Path) {
              if(Specifier->getType().getAsString()
                 == "class nails::BaseController"
              && Specifier->getAccessSpecifier()
                 == clang::AS_public) {
                return true;
              }
              return false;
            },
          cbp);

          /*
          for (auto const& base : rd->bases()) {
            if (!found_BaseController
                && base.getType().getAsString() == "class nails::BaseController"
                && base.getAccessSpecifier() == clang::AS_public) {
              found_BaseController = true;
            }
          }*/
          // we're taking this even if it errors out here, for error reporting
          controller = rd->getNameAsString().c_str();
          if (!endsWith(controller, "Controller")) {
            error_info = "Class name does not end with \"Controller\".";
            goto end;
          }
          if (!found_BaseController) {
            error_info = "Could not find BaseController in class ancestry.";
            goto end;
          }

          if (md->getNumParams() != 1
              || md->getParamDecl(0)->getType()
                   .getUnqualifiedType()
                   .getNonReferenceType()
                   .getAsString() != "class nails::Request"
              /*|| md->getParamDecl(1)->getType()
                   .getUnqualifiedType()
                   .getNonReferenceType()
                   .getAsString() != "class Response"
              */
          ) {
            goto end;
          }
          action = md->getNameAsString().c_str();

          clang::Type const* t = rt.getTypePtr();
          if (clang::isa<clang::TemplateSpecializationType>(t)) {
            auto const* tst = clang::cast<clang::TemplateSpecializationType>(t);
            unsigned tst_num_args = tst->getNumArgs();
            if (tst_num_args < 3) {
              for (unsigned i(0); i < tst_num_args; i++) {
                clang::Expr const* ex = tst->getArgs()[i].getAsExpr();
                if (clang::isa<clang::CallExpr>(ex)) {
                  auto const* cex = clang::cast<clang::CallExpr>(ex);
                  unsigned cex_num_args = cex->getNumArgs();
                  if (cex_num_args > 1) {
                    auto const* callable = cex->getArg(0);
                    std::string typestr = callable->getType()
                                            .getUnqualifiedType()
                                            .getAsString();
                    std::vector<std::string> string_literals;
                    for (unsigned j(1); j < cex_num_args; j++) {
                      auto const* arg = cex->getArg(j);
                      if (clang::isa<clang::MaterializeTemporaryExpr>(arg)) {
                        auto const* tmp =
                            clang::cast<clang::MaterializeTemporaryExpr>(arg)
                              ->getTemporary();
                        if(clang::isa<clang::CXXConstructExpr>(tmp)) {
                          auto const* str_const_ctor =
                              clang::cast<clang::CXXConstructExpr>(tmp);
                          if(str_const_ctor->getType().getUnqualifiedType()
                               .getAsString() == "class str_const" &&
                             str_const_ctor->getNumArgs() == 1) {
                            auto const* ctor_arg = str_const_ctor->getArg(0);
                            if(clang::isa<clang::StringLiteral>(ctor_arg)) {
                              string_literals.push_back(
                                clang::cast<clang::StringLiteral>(ctor_arg)
                                  ->getString().str()
                              );
                            } else { goto end; }
                          } else { goto end; }
                        } else { goto end; }
                      } else if (clang::isa<clang::StringLiteral>(arg)) {
                        //somehow the methods(...) ones are StringLiterals
                        //but the url(...) one is a:
                        //  MaterializeTemporaryExpr(CXXConstructExpr(StirngLiteral))
                        string_literals.push_back(
                          clang::cast<clang::StringLiteral>(arg)
                            ->getString()
                            .str()
                        );
                      } else { goto end; }
                    }
                    if(typestr == "struct nails::nails_url") {
                      if(string_literals.size() == 1) {
                        path = std::move(string_literals[0]);
                      } else { goto end; }
                    } else if (typestr == "struct nails::nails_methods") {
                      if(string_literals.size() >= 1) {
                        methods = std::move(string_literals);
                      } else { goto end; }
                    } else {
                      goto end;
                    }
                  }
                }
              }
            } else { goto end; }
          } else { goto end; }
        } else { goto end; }
      } else { goto end; }
      failed = false;
      /*
      puts("======");
      puts(controller.c_str());
      puts(action.c_str());
      puts(path.c_str());
      for (auto const& m : methods) {
        puts(m.c_str());
      }
      puts("======");
      */
      std::vector<decltype(std::make_tuple(action, path, methods))>& routes =
          route_mapper[controller];
      routes.emplace_back(std::make_tuple(std::move(action),
                                          std::move(path),
                                          std::move(methods))
      );
    }

end:
    if (failed) {
      puts("An error occurred. We have the following information:");
      if (controller.size() <= 0) {
        puts("  Something strange happend. Possibly moon wizards.");
      } else {
        printf("  Class: %s\n", controller.c_str());
        if (action.size() > 0) {
          printf("  Method: %s\n", action.c_str());
        }
        if (path.size() > 0) {
          printf("  Path annotation: %s\n", path.c_str());
        }
        if (methods.size() > 0) {
          puts("  Method annotations:");
          for (auto const& m : methods) {
            printf("    - %s\n", m.c_str());
          }
        }
      }
      printf("  Additional error information: %s\n", error_info.c_str());
    }

  }

};


void replaceAll(std::string& h, std::string const& n, std::string const& r) {
  size_t p = 0;
  size_t nl = n.size(), rl = r.size();
  while((p = h.find(n, p)) != std::string::npos) {
     h.replace(p, nl, r);
     p += rl;
  }
}

void replaceLast(std::string& h, std::string const& n, std::string const& r) {
  size_t p = 0;
  size_t p2 = std::string::npos;
  size_t nl = n.size();//, rl = r.size();

  //TODO: replace w/ std::string::rfind
  while((p = h.find(n, p)) != std::string::npos) {
     p2 = p;
     p += nl;
  }
  if (p2 != std::string::npos) {
    h.replace(p2, nl, r);
  }
}


std::string includes =
R"inc(#ifndef _GEN_H_
#define _GEN_H_
#include "controller-helper.h"

{include}

extern "C" {
  void init_controllers();
  void teardown_controllers();
}

{singleton}

#endif)inc";

std::string gen_cc =
R"gen_cc(#include "gen.h"
#include "controller-helper.h"
#include "router.h"

namespace nails {
  ControllerMapper controller_mapper;
  ControllerFactoryMapper controller_factory_mapper;
  ControllerNameMapper controller_name_mapper;
  ControllerFactorySingletonMapper controller_factory_singleton_mapper;
};

using namespace nails;

{singleton}


extern "C" void init_controllers() {
  factory_base* tmp_factory = nullptr;

  {controllerinit}

  {routeinit}
}

extern "C" void teardown_controllers() {

  {controllerteardown}
}
)gen_cc";

std::set<std::string> singletons;

void setupIncludes(std::vector<std::string> const& sources) {
  for (auto const& _s : sources) {
    std::string s{_s};
    if (endsWith(s, ".cpp")) {
      replaceLast(s, ".cpp", ".h");
    } else if (endsWith(s, ".cc")) {
      replaceLast(s, ".cc", ".h");
    }
    std::string includeLine = "#include \"../" + s + "\"\n{include}\n";
    replaceAll(includes, "{include}\n", includeLine);
  }
  replaceAll(includes, "{include}\n", "");

  for (auto const& kv : magic_mapper) {
    singletons.insert(kv.first);
  }
  for (auto const& kv : route_mapper) {
    singletons.insert(kv.first);
  }
  for (auto const& s : singletons) {
    replaceAll(includes, "{singleton}\n",
                         "extern " +s+ " singleton_" +s+ ";\n{singleton}\n");
  }
  replaceAll(includes, "{singleton}\n", "");
}

void setupControllers() {
  for (auto const& s : singletons) {
    char const* sc = s.c_str();
    replaceAll(gen_cc, "{singleton}\n",
                       "%s singleton_%s;\n{singleton}\n"_format(sc,sc)
    );

    std::string cinit = "  controller_mapper.emplace(type_name(singleton_%s), ControllerActionMapper());\n"_format(sc);
    cinit += "  tmp_factory = new ControllerFactory<%s>();\n"_format(sc);
    cinit += "  controller_factory_singleton_mapper.emplace(tmp_factory, &singleton_%s);\n"_format(sc);
        
    cinit += "  controller_factory_mapper.emplace(type_name(singleton_%s), tmp_factory);\n"_format(sc);
    cinit += "  controller_name_mapper.emplace(\"%s\", type_name(singleton_%s));\n"_format(
      std::string(s.begin(),s.begin()+s.rfind("Controller")).c_str(), sc
    );
    cinit += "  auto& tcam_%s = controller_mapper.at(type_name(singleton_%s));\n"_format(sc, sc);
    for (auto const& m : magic_mapper[s]) {
      cinit += "  tcam_%s.add(\"%s\", make_func(&%s));\n"_format(
        sc,
        m.c_str(),
        (s + "::" + m).c_str()
      );
    }
    replaceAll(gen_cc, "  {controllerinit}\n", "%s\n  {controllerinit}\n"_format(cinit.c_str()));

    std::string cteardown = "  auto* cfm_%s = controller_factory_mapper.at(type_name(singleton_%s));\n"_format(sc,sc);
    cteardown += "  controller_factory_mapper.erase(type_name(singleton_%s));\n"_format(sc);
    cteardown += "  delete cfm_%s;\n"_format(sc);
    replaceAll(gen_cc, "  {controllerteardown}\n", "%s\n  {controllerteardown}\n"_format(cteardown.c_str()));
  }
  replaceAll(gen_cc, "{singleton}\n", "");
  replaceAll(gen_cc, "  {controllerinit}\n", "");
  replaceAll(gen_cc, "  {controllerteardown}\n", "");
}

void setupNailsRoutes() {

  for(auto const& kv : route_mapper) {
    std::string const& controller = kv.first;
    auto const& routes = kv.second;
    for (auto const& apms : routes) {
      std::string const& action = std::get<0>(apms);
      std::string const& path = std::get<1>(apms);
      std::vector<std::string> const& methods = std::get<2>(apms);

      std::stringstream routeline;
      routeline << "  RouteNode::add(\"" << path << "\", {";
      for (auto const& m : methods) {
        routeline << "\"" << m << "\"";
        if (&m != &methods.back()) {
          routeline << ", ";
        }
      }
      routeline << "}, make_func(&"
                << controller << "::" << action << "<url,methods>"
                << "), type_name(singleton_" << controller << "));";
      replaceAll(gen_cc, "  {routeinit}\n", "%s\n  {routeinit}\n"_format(routeline.str().c_str()));
    }
  }
  replaceAll(gen_cc, "  {routeinit}\n", "");
}


int main(int argc, const char **argv) {
  CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());


  MatchFinder Finder;
  NailsRouteCallback nrcb;
  Finder.addMatcher(NailsRouteMatcher, &nrcb);
  NailsMagicCallback nmcb;
  Finder.addMatcher(NailsMagicMatcher, &nmcb);
  Tool.run(newFrontendActionFactory(&Finder).get());

  /*
  for(auto const& kv : magic_mapper) {
    std::string const& controller = kv.first;
    for (auto const& m : kv.second) {
      printf("found %s::%s\n", controller.c_str(), m.c_str());
    }
  }
  */
  setupIncludes(OptionsParser.getSourcePathList());
  std::ofstream gen_h_file;
  gen_h_file.open("gen/gen.h");
  gen_h_file.write(includes.data(), includes.size());
  gen_h_file.close();

  setupControllers();
  setupNailsRoutes();
  std::ofstream gen_cc_file;
  gen_cc_file.open("gen/gen.cc");
  gen_cc_file.write(gen_cc.data(), gen_cc.size());
  gen_cc_file.close();

  return 0;
}
