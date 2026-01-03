#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>


struct Var {
    std::string ident;
    size_t stack_position;
};

using Scope = std::unordered_map<std::string, Var>;

class SymbolManager {
public:
    SymbolManager();

    void enterScope();
    void exitScope();
    std::optional<Var> findSymbol(std::string ident);
    void declareSymbol(std::string ident, size_t stack_position);

private:
    std::vector<Scope> scopes;
};
