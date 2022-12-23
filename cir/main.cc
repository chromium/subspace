#include <iostream>

#include "clang/Tooling/CommonOptionsParser.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/TargetSelect.h"
#include "subspace/prelude.h"

int main(int argc, const char** argv) {
  llvm::InitLLVM init(argc, argv);
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();

  llvm::cl::OptionCategory option_category("CIR options");
  llvm::Expected<clang::tooling::CommonOptionsParser> options_parser =
      clang::tooling::CommonOptionsParser::create(argc, argv, option_category,
                                                  llvm::cl::ZeroOrMore);

  std::cout << "hello world\n";
}
