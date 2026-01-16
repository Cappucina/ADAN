# ADAN Language Design

### Decide
- Imports
- Switch/Match
- Lambda Syntax
- Variadic access
- Do-while loops, foreach loops, range-based for loop
- Escape characters
- Classes :3
- Postfix increments and logical/bitwise assignment increments (e.g. +=, *=, ^=, |=, etc.)
- C++ style References
- String
- Built-in memory management (new/delete)
- Generics/Templates
- Ternary Operator
- Pointer member access (something->count)

## Includes system/Module system
Officially decide on an implementation
### Rust/Python Like
```adan
import std.io;                          // Import all functions
import std.io:println;                  // Only import println
import std.io:{println, print, puts};   // Multiple functions
```
### C/C++ Like
```adan
include "../std/io.adn";
```
or
```adan
include ..std.io // ?? idk
```

## Variables
```adan
x::float = 1.23;        // Variables
cool = 'c';             // Implicit
arr::int[] = [1, 2, 3]; // Arrays
y::string = "cool";     // Strings
ptr::string* = &y;      // Pointers

custom::UserStruct;     // User-Defined variables
custom.val = 0;
customptr::UserStruct* = new UserStruct; // ?
customptr->val = 0;

cool = 'w';
arr[1] = 90;            // -> [1, 90, 3]

delete customptr;
```

## Loops
```adan
for (i::int = 0; i < 10; i++) {
    do_something();
}

while (0 == NULL) {
    something_cool();
}
// do-while loops?
```

## Control Flow
```adan
if (black == true) {
    racism("nword");
} else if(black == false) {
    black = true;
    racism("nword");
} else {
    racism("nword");
}

// switch/match?
```

## Function Declarations and Function Calls
```pallas
program::void print_something(something::string) {
    println(something);
}

program::int main() {
    print_something("something");
}
```