#[derive(Debug, Clone)]
pub enum Expr {
    Literal(Literal),
    Variable {
        name: String,
        var_type: Type,
    },
    Binary {
        left: Box<Expr>,
        op: Operand,
        right: Box<Expr>,
        expr_type: Type,
    },
    Unary {
        op: Operand,
        expr: Box<Expr>,
        expr_type: Type,
    },
    Assign {
        name: String,
        value: Box<Expr>,
        expr_type: Type,
    },
    FunctionCall {
        name: String,
        args: Vec<Expr>,
        expr_type: Type,
    },
}

#[derive(Debug, Clone)]
pub enum Operand {
    Add,
    Subtract,
    Multiply,
    Divide,
    Modulus,
    Power,
    Equals,
    Not,
    And,
    Or,
    LessThan,
    GreaterThan,
    LessEqual,
    GreaterEqual,
    NotEqual,
    Negate,
    NotUnary,
}

#[derive(Debug, Clone)]
pub enum Literal {
    Integer(i64, Type),
    Float(f64, Type),
    Boolean(bool, Type),
    String(String, Type),
    Null(Type),
    Identifier(String, Type),
}

#[derive(Debug, Clone)]
pub enum Statement {
    VarDecl {
        name: String,
        initializer: Option<Expr>,
        var_type: Type,
        is_global: bool,
    },
    Expr(Expr),
    Block(Vec<Statement>),
    If {
        condition: Expr,
        then_branch: Vec<Statement>,
        else_branch: Option<Vec<Statement>>,
    },
    While {
        condition: Expr,
        body: Vec<Statement>,
    },
    For {
        initializer: Option<Box<Statement>>,
        condition: Expr,
        increment: Option<Box<Statement>>,
        body: Vec<Statement>,
    },
    Return(Option<Expr>),
    Break,
    Pass,
    Include(String),
}

#[derive(Debug, Clone)]
pub struct FunctionDecl {
    pub name: String,
    pub params: Vec<String>,
    pub body: Vec<Statement>,
}

#[derive(Debug, Clone)]
pub enum Type {
    Int,
    Float,
    Bool,
    String,
    Null,
    Function {
        params: Vec<Type>,
        return_type: Box<Type>,
    },
    Unknown,
}
