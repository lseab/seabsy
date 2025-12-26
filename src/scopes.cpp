#include "scopes.hpp"


SymbolManager::SymbolManager() {
    enterScope();
}

void SymbolManager::enterScope() {
    scopes.push_back(new Scope());
}

void SymbolManager::exitScope() {
    scopes.pop_back();
}

std::optional<Var> SymbolManager::findSymbol(std::string ident) {
    for (Scope* scope : scopes) {
        if (scope->m_var_map.contains(ident)) {
            return scope->m_var_map[ident];
        }
    }
    return {};
}

void SymbolManager::declareSymbol(std::string ident, size_t stack_position) {
    Scope* currentScope = scopes.back();
    currentScope->m_var_map[ident] = Var{.ident = ident, .stack_position = stack_position};
}
