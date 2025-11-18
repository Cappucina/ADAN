#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum IndexOperator {
    Colon,
    Dot,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum BinaryOperator {
    Add,                // +
    Subtract,           // -
    Multiply,           // *
    Divide,             // /
    Modulo,             // %
    Equal,              // =
    Exponentiate,       // ^
    NotEqual,           // !=
    LessThan,           // <
    LessThanOrEqual,    // <=
    GreaterThan,        // >
    GreaterThanOrEqual, // >=
    And,                // &&
    Or,                 // ||
    Concatenate,        // ..
    Assign,             // ->
}

#[derive(Debug, PartialEq)]
pub enum Type<'alloc> {
    Reference { name: &'alloc str },
}

#[derive(Debug, PartialEq)]
pub struct Binding<'alloc> {
    pub name: &'alloc str,
    pub ty: Type<'alloc>,
}

pub type Bindings<'alloc> = Vec<Binding<'alloc>>;
pub type Expressions<'alloc> = Vec<Expression<'alloc>>;
pub type BoxedExpression<'alloc> = Box<Expression<'alloc>>;

#[derive(Debug, PartialEq)]
pub struct Block<'alloc> {
    pub expressions: Expressions<'alloc>,
}

#[derive(Debug, PartialEq)]
pub enum Expression<'alloc> {
    Identifier { value: &'alloc str },
    BooleanLiteral { value: bool },
    IntegerLiteral { value: i64 },
    StringLiteral { value: &'alloc str },
    FloatLiteral { value: f64 },
    NullLiteral,

    // io.printf(); <-- () is a function call on io.printf
    Call {
        function: Box<Expression<'alloc>>,
        arguments: Expressions<'alloc>,
    },

    IndexName {
        operator: IndexOperator,
        expression: Box<Expression<'alloc>>,
        name: &'alloc str,
    },

    IndexExpression {
        expression: Box<Expression<'alloc>>,
        index: Box<Expression<'alloc>>,
    },

    Block {
        block: Block<'alloc>,
    },

    Program {
        name: Option<String>,
        parameters: Bindings<'alloc>,
        body: Block<'alloc>,
    },

    BinaryOperation {
        operator: BinaryOperator,
        left: Box<Expression<'alloc>>,
        right: Box<Expression<'alloc>>,
    },

    Error,
}