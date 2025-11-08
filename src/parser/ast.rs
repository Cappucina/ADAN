/*
MIT License

Copyright (c) 2025 Cappucina

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// The ADAN AST Structure
// Authored by: @oopsite
// Dated Nov 4, 2025

// ADANs Expressions
// ADAN supports these Exprs:
//   - Binary Operations (+, -, *, /, %)
//   - Unary Operations (-, !)
//   - Assignments
//   - Function Calls
//   - Literals (numbers, strings, booleans, nil)
//   - Variables (names)
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
    Variable(String),
}

// ADANs Operations
// ADANs operation, which include:
//   - +
//   - -
//   - *
//   - /
//   - %
#[derive(Debug, Clone)]
pub enum Operation {
    Add,
    Subtract,
    Multiply,
    Divide,
    Modulo,
    Negate,             // Flips the sign of a Number. (e.g. 2 -> -2, -3 -> 3)
    Not,                // Flips the value of a boolean. (e.g. false -> true, true -> false)
}

// ADANs Literals
// "Strings"
// 110, 117, 109, 98, 101, 114, 115
#[derive(Debug, Clone)]
pub enum Literal {
    Number(f64),
    String(String),
    Bool(bool),
    Nil,
}

// ADANs Statements
// These include:
//   - Printing
//   - Expressions
//   - Variables Decs
//   - If Statements
//   - While Loops
//   - Function Statements
//   - Returning
#[derive(Debug, Clone)]
pub enum Statement {
    Expression(Expr),
    Print(Expr),
    VarDecl {           // local <var> -> <val>;
        name: String,
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
}

// ADANs Function Declaration
// <function> <name> <params> { <body }
#[derive(Debug, Clone)]
pub struct FunctionDecl {
    pub name: String,
    pub params: Vec<String>, // Params or Arguments
    pub body: Vec<Statement>,
}
