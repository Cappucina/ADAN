mod lexer;
mod parser;
mod std;
mod code_gen;

fn main() {
    let user_input = std::io::Terminal::input();

    std::io::Terminal::output(&user_input, true);
}
