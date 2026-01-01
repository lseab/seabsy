$$
\begin{align}
[\text{Prog}] &\to [\text{Stmt}]^* \\
[\text{Stmt}] &\to
    \begin{cases}
        \text{return}\space\ [\text{Expr}]; \\
        \text{let}\space\ \text{ident = [Expr];} \\
        \text{ident = [Expr];} \\
        [\text{Scope}] \\
        \text{if} \space [\text{IfStmt}] \\
    \end{cases} \\
[\text{IfStmt}] &\to
    \begin{cases}
        ([\text{Expr}]) \space [\text{Scope}] \space [\text{IfPred}]
    \end{cases} \\
[\text{IfPred}] &\to
    \begin{cases}
        \text{elif} \space [\text{IfStmt}] \\
        \text{else} \space [\text{Scope}] \\
        \epsilon
    \end{cases} \\
[\text{Scope}] &\to
    \begin{cases}
        \{[\text{Stmt}]^*\} \\
    \end{cases} \\
[\text{Expr}] &\to
    \begin{cases}
        [\text{Term}] \\
        [\text{BinExpr}] \\
    \end{cases} \\
[\text{Term}] &\to
    \begin{cases}
        \text{int\_lit} \\
        \text{ident} \\
        ([\text{Expr}]) \\
    \end{cases} \\
[\text{BinExpr}] &\to
    \begin{cases}
        [\text{Expr}] * [\text{Expr}] & \text{prec}=1 \\
        [\text{Expr}] \space / \space [\text{Expr}] & \text{prec}=1 \\
        [\text{Expr}] + [\text{Expr}] & \text{prec}=0 \\
        [\text{Expr}] - [\text{Expr}] & \text{prec}=0 \\
    \end{cases}
\end{align}
$$