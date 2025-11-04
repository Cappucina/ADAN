//
// ADAN tokenizer, used to translate human-legible words into machine-readable, processable
// language to be compiled later on.
//

pub enum Token {
    Keyword(Keyword),
    Symbols(Symbols),

    Number(String),

    Types(Types),

    Ident(String),
    Error(String),
}

// Individual enum pairs
pub enum Keyword {
    Local,          //  local variables -- Can be used in a specific scope // context.
    Global,         // Global variables -- Can be used in *any* context.
    
    Assign,         // Sign of equality during variable assignment. (local {var} -> {val};)
    SemiColon,      // Used to tell the compiler it's ready to move on to the next line.
    Colon,          // Used when explicitly defining the type of a variable.

    Include,        // Importing binaries or third party packages to your AdaN script.
    Program,        // Creating a new function outside of the main function. (program -> {var})
}

// Less priority symbols unlike Equality & SemiColon.
pub enum Symbols {
    Comment,        // Single-lined comments may be used on a newline *or* *any* line after ";".
    MultiLine,      //  Multi-lined comments make anything after "/*" and before "*/" a comment.
    
    LParen,
    RParen,

    LCurlyBracket,
    RCurlyBracket,

    Quotation,      // ""
    SingleQuote,    // ''
    
    Period,
}

#[allow(non_camel_case_types)]
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
