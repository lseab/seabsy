#include <iostream>
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
    // Search from innermost scope outward so shadowed bindings are found first
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        Scope* scope = *it;
        if (scope->m_var_map.contains(ident)) {
            return scope->m_var_map[ident];
        }
    }
    return {};
}

void SymbolManager::declareSymbol(std::string ident, size_t stack_position) {
    Scope* currentScope = scopes.back();
    if (currentScope->m_var_map.contains(ident)) {
        std::cerr << "Redefinition of " << ident << std::endl;
        exit(EXIT_FAILURE);
    }
    currentScope->m_var_map[ident] = Var{.ident = ident, .stack_position = stack_position};
}
