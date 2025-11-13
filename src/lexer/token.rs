//
// ADAN tokenizer, used to translate human-legible words into machine-readable, processable
// language to be compiled later on.
//

#[derive(Debug, Clone)]
pub enum Token {
    Keyword(Keyword),
    Symbols(Symbols),

    Number(String),

    Types(Types),

    Ident(String),
    Literal(String),
    CharLiteral(char),

    Error(String),
}

// Individual enum pairs
#[derive(Debug, PartialEq, Clone, Copy)]
pub enum Keyword {
    Local,          //  local variables -- Can be used in a specific scope // context.
    Global,         // Global variables -- Can be used in *any* context.

    While,          // Run x task until y condition is met.
    If,
    Else,
    Return,         // Returns a value from a function or a type of loop to be used later on.

    Assign,         // Sign of equality during variable assignment. (local {var} -> {val};)

    Include,        // Importing binaries or third party packages to your AdaN script.
    Program,        // Creating a new function outside of the main function. (program -> {var})
}

// Less priority symbols unlike Equality & SemiColon.
#[derive(Debug, PartialEq, Clone, Copy)]
pub enum Symbols {
    Comment,        // Single-lined comments may be used on a newline *or* *any* line after ";".
    MultiLine,      //  Multi-lined comments make anything after "/*" and before "*/" a comment.
    
    LParen,
    RParen,

    LCurlyBracket,
    RCurlyBracket,

    Equal,          // NOT used for assignment! Used for checking the equality between two values.
    Greater,        // Checks if x is > y, returns a boolean.
    Lesser,         // Checks if x is < y, returns a boolean.
    Gequal,         // Checks if x is < or equal to y, returns a boolean.
    Lequal,         // Checks if x is > or equal to y, returns a boolean.

    Add,            // x + y = z
    Sub,            // x - y = z
    Mul,            // x * y = z
    Div,            // x / y = z
    Mod,            // x % y = remainder of x / y
   
    Not,            // ! (or NOT) can be chained like -> !!x = not not x, or just x, it can also
                    // just be !x, or not x. ! can be chained indefinitely. (!!!!!!x, or just x is passable)

    Quotation,      // ""
    SingleQuote,    // ''
    
    SemiColon,      // Used to tell the compiler it's ready to move on to the next line.
    Colon,          // Used when explicitly defining the type of a variable.
    Period,
    Comma,          // Typically used as a separator for function parameters. (program -> sample(a:
                    // String -> 1, b: i8 -> 2, ...))
}

#[allow(non_camel_case_types)]
#[derive(Debug, Clone, Copy)]
pub enum Types {
    String,

    i8, i32, i64,  // 8-64 bit   signed integers. Works with negative integers. (-n)
    u8, u32, u64,  // 8-64 bit unsigned integers. Only non-negative integers.
    
    f32, f64,      // 32-64 bit floating point values. Must contain decimal.

    Boolean,
    Char,          // Single *char*acter ('a', 'b', ...)
    
    Array,         // Fixed size list of object, where all objects must be of the same type.
    Object,        // Fixed size object where a name (String) is assigned to `x` value.
}
