std::string function_friendly_name(clang::FunctionDecl& decl) { 
  std::ostringstream s;
  if (auto* mdecl = clang::dyn_cast<clang::CXXMethodDecl>(&decl)) {
    s << mdecl->getThisType().getCanonicalType().getAsString();   
    s << "::";
  }
  s << decl.getQualifiedNameAsString();
  s << "(";
  bool comma = false;
  for (auto* p : decl.parameters()) {
    if (comma) s << ", ";
    s << p->getOriginalType().getCanonicalType().getAsString();   
    comma = true;
  }
  s << ") > ";
  s << decl.getReturnType().getCanonicalType().getAsString();     
  clang::Expr* req = decl.getTrailingRequiresClause();
  if (req) {
    s << " requires ";
    // TODO: dump it.
  }

  if (auto* mdecl = clang::dyn_cast<clang::CXXMethodDecl>(&decl)) {
    if (mdecl->getThisType().isConstQualified()) {
      s << " const";
    }
    if (mdecl->getThisType()->isRValueReferenceType()) {
      s << " &&";
    } else if (mdecl->getThisType()->isLValueReferenceType()) {   
      s << " &";
    }
  }
  return s.str();
}
