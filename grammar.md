<div align="center">
  <h1>The ADAN Programming Language</h1>
  <h3>Understanding the Grammar and Syntax Rules</h3>
</div>

> [!TIP]
> This tutorial assumes you have _at least_ novice level skills in programming and already know what syntax and scope are and that you already write in C-like languages like JavaScript, C, C++, Java, etc.
>
> If you do not have these skills, just remember that syntax refers to the **way a language is written**.

## Table of Contents 📖

1. [Top Level](#top-level)  
2. [Explaining Programs and the Main Program](#explaining-programs-and-the-main-program)  
3. [Variables: How to Write Them and Why It's Different](#variables-how-to-write-them-and-why-its-different)  
4. [Understanding Conditions and Different Operators](#understanding-conditions-and-different-operators)  
5. [Understanding Loops and Knowing the Difference](#understanding-loops-and-knowing-the-difference)  
6. [Libraries and More Files!!](#libraries-and-more-files)  
7. [Type Casting](#type-casting)  
8. [String Interpolation](#string-interpolation)  
9. [Escape Sequences in Strings](#escape-sequences-in-strings)  
10. [Bitwise Operations](#bitwise-operations)

---

## Top Level
The top level refers to the lowest possible scope and similarly to C, only a few things can legally be placed here.
```adn
program
struct
include
```

---

## Explaining Programs and the Main Program
In ADAN, we refer to functions as programs, hence the "program" keyword. Everything follows a similar standard when defining things, which goes as follows:
```
KEYWORD IDENTIFIER::TYPE
```

Except for variables, which follow that same standard but just excluding the `KEYWORD` aspect.

The main program for every script is **required** and the script will not compile otherwise. Luckily it's very similar to C.
```adn
// In ADAN programs are written as follows:
program IDENTIFIER::TYPE()
{
  // Any code goes here. If you'd like, you can also include parameters in between (),
  // which follow the same standard as regular variables, just without declarations.
}
```

> [!NOTE]
> It's not required to put `{` on a new line, although it is standard to.

---

## Variables: How to Write Them and Why It's Different
Variables are constructed differently than everything else in ADAN and it may appear confusing at first, but there's a good reason why.

In languages such as JavaScript, it can get confusing using `let`, `const`, and `var` and no keyword for reassignment nor creating parameters. _Why?_

Similarly to C, ADAN simplifies this a lot by removing keywords altogether so declaration is consistent between parameters and regular variables. Reassignment is identical with the exception that it lacks the type declaration, as it's redundant.

```adn
program main::int()
{
  my_variable::string = "The best variable ever!!";
  my_variable = "Okay.. maybe not.";

  return 0;
}
```

There are various available types to use in ADAN, which all serve different purposes. However they're practically identical to those in C.
```
int      - A 64-bit signed integer.
string   - An array of characters that can only be wrapped in "
char     - A single character which can only be wrapped in '
bool     - A wrapper for 0 and 1, where 0 is false and 1 is true.
float    - A number value that contains a decimal point.
void     - Represents the lack of a type, no value.
null     - A special type that is both a keyword and a type: Returns a non-existant value.
```

You can even use `[]` to declare it as an array!
```adn
program main::int()
{
  my_array::int[] = [1, 2, 4, 8, 16, 32, 64];
  // Uses 0-based indexing. 1 = 0, 2 = 1, 4 = 2, etc.

  // This dynamically sized array can be changed at runtime!
}
```

If you're paying attention, you've probably noticed variables follow this syntax:
```
Declaration: IDENTIFIER::TYPE = VALUE
Reassignment: IDENTIFIER = VALUE
```

When variables are reassigned, we drop the type declarator due to the compiler already knowing what type that variable was originally assigned as.

**However**, variables can never be put on the top level like programs can.

---

## Understanding Conditions and Different Operators
Conditions are expressions that equate to a `bool` and will only run whats after it if that condition is true.
```adn
program main::int()
{
  users::int = 10;

  // All conditions follow the same syntax: LHS (Left hand side) OPERATOR (==, <, >, etc.) RHS.
  // IF (CONDITION) { CODE }
  if (users > 15)
  {
    // ...
  }
}
```

There are a lot of various operators and keywords you can use to make conditions so much more robust!
```
!    - Negation: Flips the sign of an integer. For booleans it flips between true and false. Can be chained.
==   - Equality is used to validate if the LHS value and RHS value are the same.
>    - Greater than, less than, etc. follow traditional math principals and compare the LHS and RHS values.
<
>=
<=
!=   - The opposite of equality; validating if 2 values are different.
&&   - The "and" operator is used to combine conditions. Can be chained.
||   - The "or" operator allows you to validate something like if X is Y OR Z, etc. Can be chained.
```

"Can be chained" simply means they're stackable. Something like "if X is Y or Z or A or B" is completely legal.

Of course generic math operations are supported and follows the PEMDAS ruleset. (Or BODMAS/BIDMAS)
```
%     - Modulo (The remainder after division)
+     - Addition
-     - Subtraction
/     - Division
*     - Multiplication
**    - Allows for exponentiation of numbers.
(     - Parenthesis can be used as binding agents to give numbers priority.
)
```

There are more equality operators that can't be used in expressions, but instead used for reassignment!
```
+=
-=
*=
/=
%=
```

For example, `+= 5` would add AND reassign the variable to itself + 5!

---

## Understanding Loops and Knowing the Difference
Loops are very very useful when programming as they help avoid redundancy. At the same time, they can cause unintentional issues if used incorrectly.

There are 2 kinds of supported loops in ADAN: While loops and For loops, each of them being unique.

### While Loops
While loops are self-explanatory: A code block is provided and it will continuously run until a certain condition is met.
```adn
program main::int()
{
  count::int = 0;
  while (count <= 360)
  {
    count++; // Once count has reached 360, the loop will stop!
  }

  // Any code after the loop will not be ran until the loop is finished running.
}
```

### For Loops
For loops are a kind of loop that run for a certain amount of times. Although confusing at first glance, when broken down they are one of the easiest components to understand.
```adn
program main::int()
{
  for (i::int = 0; i <= 16; i++)
  {
    // Do whatever you want here.
  }
}
```

The line `for (i::int = 0; i >= 16; i++)` can be broken down into a few instructions:
1. Declare a new variable, `i`, and set it to 0.
2. Run this loop until `i` is greater than or equal to 16.
3. After each iteration, increase the value of `i` by 1.

The semi colons (;) in the loop act as separators, which differ from traditional comma (,) separators like you'll find in a parameters list.

One thing to understand is the grammar rules behind it:
```
for (IDENTIFIER::TYPE = VALUE; CONDITION; INCREMENTOR/DECREMENTOR)
for (INIT; CONDITION; INC./DEC.)
```

Incrementors/Decrementors are special! They increase (+1) or decrease (-1) a variable depending on the token used, which includes `++` and `--`.

Depending on _where_ the `++`/`--` are used can actually change the result of the loop.

Placing them **before** the variable will increment/decrement the variable before it's checked by the condition and placing them **after** will modify the variable after the condition is checked.

---

## Libraries and More Files!!
The `include` keyword is a very unique keyword, giving you the ability to use implementations from other files! In almost every ADAN script ever written you will need at least one include, and that is to print things to console.

There are various core libraries you can include in your script to use programs or access information that's not available otherwise.

The syntax is very simple to understand: `include x.y;` where the identifier after the last period is the file you are including and anything before that are folders to navigate your file system easier.

Core libraries on the other hand are quite special, as they can be included regardless of your file structure.
```adn
include adan.io; // I/O is short for input/output, this library is in charge with
                 // displaying information and gathering input from the user.

program main::int()
{
  my_message::string = "Hello, world!";

  println(my_message); // This function comes from the I/O library we included!
}
```

> [!WARNING]
> Your file names cannot conflict with the core libraries of ADAN or a compiler error will be thrown.

The best part is you can split your projects into various scripts!
<details>
<summary>
  Expand to Show Example  
</summary><br>

```adn
// utils.adn
include adan.array;

program add::int(...) // Accepts any amount of arguments of any type.
{
  total::int = 0;
  // You can treat "..." as an array by wrapping it in [].

  for (i::int = 0; i <= arrlen([...]); i++)
  {
    total += [...][i];
  }

  return total;
}
```

```adn
// example.adn
include utils;
include adan.io;

program main::int()
{
  total::string = (string)add(1, 2, 4, 8, 16);

  println("Total: ${total}"); // Total: 31
}
```

If you're confused about `(string)add(1, 2, 4, 8, 16)` and `${}`, don't worry. These will be covered later on in the tutorial.
</details>

---

## Type Casting
Type casting refers to when you change the type of a variable to use it in a different setting. These changes only apply to that specific line and the variable is not changed anywhere else.
```
(NEW TYPE)OLD VALUE
```

You can use this alongside string interpolation to print cool messages!
```adn
include adan.io;

program main::int()
{
  my_age::int = 17;

  println("Hi! I'm Lily and I am ${(string)my_age} years old!"); // "... I am 17 years old!"
}
```

---

## String Interpolation
String interpolation is one of the simplest and one of my personal favorite features. String interpolation is directly embedded code into a string. You can only embed other strings however, which is where casting can come in handy.
```adn
include adan.io;

program main::int()
{
  total::int = 15;

  println("The total cost of your order was ${(string)total} dollars");
}
```

---

## Escape Sequences in Strings
Escape sequences are special because they allow you to do cool things with strings, such as creating new lines, tab indentation, and more.

Here's every available one:
```
\n         - Inserts a new line (line feed).
\r         - Moves the cursor to the beginning of the current line (carriage return).
\b         - Moves the cursor one position back (backspace).
\t         - Creates a tab indentation.
\0         - Terminates the string (null character); removes everything after it.
\'         - Inserts a single quote character (').
\"         - Inserts a double quote character (").
\\         - Inserts a backslash () itself.
\a         - Triggers a system alert or beep (bell).
\f         - Advances to the next page (form feed).
\v         - Moves the cursor down to the next vertical tab stop.
\nnn       - Represents a character by its octal value (e.g., \141 = 'a').
\xhh       - Represents a character by its hexadecimal value (e.g., \x61 = 'a').
\uhhhh     - Represents a Unicode character using 4 hexadecimal digits (e.g., \u03A9 = Ω).
\Uhhhhhhhh - Represents a Unicode character using 8 hexadecimal digits (e.g., \U0001F600 = 😀).
```

---

## Bitwise Operations
Bitwise operators are fairly more complicated and they are used for directly manipulating the individual bits of integers rather than the number itself.

```
&     -           Bitwise AND: Sets each bit to 1 if both bits are 1.
|     -            Bitwise OR: Sets each bit to 1 if at least one bit is 1.
~     -           Bitwise NOT: Inverts all bits (1 becomes 0, 0 becomes 1).
^     -           Bitwise XOR: Sets each bit to 1 if only one bit is 1.
!&    -          Bitwise NAND: NOT of AND; true unless both bits are 1.
!|    -           Bitwise NOR: NOT of OR; true only if both bits are 0.
!^    -          Bitwise XNOR: NOT of XOR; true if both bits are the same.
<<    -            Left shift: Shifts bits left, filling zeros on the right (multiplies by 2 for each shift).
>>    -    Signed right shift: Shifts bits right, preserving the sign bit (divides by 2 for each shift).
>>>   - Zero-fill right shift: Shifts bits right, filling zeros on the left (unsigned behavior).
```
