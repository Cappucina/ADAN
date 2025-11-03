//
// ADAN tokenizer, used to translate human-legible words into machine-readable, processable
// language to be compiled later on.
//

pub enum Token {
    Keyword(Keyword),
    Symbols(Symbols),

    Error,
}

// Individual enum pairs
pub enum Keyword {
    Store,          //  local variables -- Can be used in a specific scope // context.
    Global,         // Global variables -- Can be used in *any* context.
    
    Equality,       // Sign of equality during variable assignment. (Store {var} -> {val};)
    SemiColon,      // Used to tell the compiler it's ready to move on to the next line.
}

                    // Less priority symbols unlike Equality & SemiColon.
pub enum Symbols {
    Comment,        // Single-lined comments may be used on a newline *or* *any* line after ";".
    MultiLine,      //  Multi-lined comments make anything after "/*" and before "*/" a comment.
}
