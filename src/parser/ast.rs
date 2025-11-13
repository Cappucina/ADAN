use crate::lexer::token::Types;

#[derive(Debug, Clone)]
pub enum Expr {
    Binary {
        left: Box<Expr>,
        op: Operation,
        right: Box<Expr>
    },

    Unary {
        op: Operation,
        right: Box<Expr>,
    },

    Assign {
        name: String,
        value: Box<Expr>
    },

    FCall {
        callee: String,
        args: Vec<Expr>,
    },
    Literal(Literal),
    Variable {
        var_name: String,
        var_type: Option<Types>,
    },
    Block(Vec<Expr>),
}

#[derive(Debug, Clone)]
pub enum Operation {
    Add,
    Subtract,
    Multiply,
    Divide,
    Modulo,
    Negate,             // Flips the sign of a Number. (e.g. 2 -> -2, -3 -> 3)
    Not,                // Flips the value of a boolean. (e.g. false -> true, true -> false)
    Greater,
    Lesser,
    Gequal,
    Equal,
    Lequal,
}

#[derive(Debug, Clone)]
pub enum Literal {
    Number(f64),
    String(String),
    Bool(bool),
    Nil,
    Char(char)
}

#[derive(Debug, Clone)]
pub enum Statement {
    Expression(Expr),
    VarDecl {           // local <var> -> <val>;
        name: String,
        var_type: Option<Types>,
        initializer: Option<Expr>,
    },
    Block(Vec<Statement>), // { }
    If {
        condition: Expr,
        then_branch: Box<Statement>,
        else_branch: Option<Box<Statement>>,
    },
    While {
        condition: Expr,
        body: Box<Statement>,
    },
    Function(FunctionDecl),
    Return {
        value: Option<Expr>,
    },
    Include(String),
}

// ADANs Function Declaration
// <function> <name> <params> { <body }
#[derive(Debug, Clone)]
pub struct FunctionDecl {
    pub name: String,
    pub params: Vec<String>, // Params or Arguments
    pub body: Vec<Statement>,
}
