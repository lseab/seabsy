#include <string>
#include <unordered_map>
#include <vector>


struct Var {
    std::string ident;
    size_t stack_position;
};


struct Scope {
    std::unordered_map<std::string, Var> m_var_map;
};


class SymbolManager {
public:
    SymbolManager() {
        enterScope();
    }

    void enterScope() {
        scopes.push_back(new Scope());
    }

    void exitScope() {
        scopes.pop_back();
    }

    std::optional<Var> findSymbol(std::string ident) {
        for (Scope* scope: scopes) {
            if (scope->m_var_map.contains(ident)) {
                return scope->m_var_map[ident];
            }
        }
        return {};
    };

    void declareSymbol(std::string ident, size_t stack_position) {
        Scope* currentScope = scopes.back();
        currentScope->m_var_map[ident] = Var{.ident = ident, .stack_position = stack_position};
    };

private:
    std::vector<Scope*> scopes;
};