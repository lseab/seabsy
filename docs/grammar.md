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
        [\text{Term}]
    \end{cases} \\
[\text{Term}] &\to
    \begin{cases}
        \text{int\_lit} \\
        \text{ident} \\
    \end{cases}
\end{align}
$$