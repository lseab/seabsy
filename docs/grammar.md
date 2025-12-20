$$
\begin{align}
[\text{Prog}] &\to [\text{Stmt}]^* \\
[\text{Stmt}] &\to
    \begin{cases}
        return\space\ [\text{Expr}]; \\
        let\space\ \text{ident = [Expr];}
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