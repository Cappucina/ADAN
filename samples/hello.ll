@.str1 = private constant [28 x i8] c"What is the best language? \00"
define i64 @main() {
entry:
  %v1 = alloca i64**
  %v2 = call i64* @adn_input(i64* @.str1)
  store i64* %v2, i64** %v1
  %v3 = load i64*, i64** %v1
  call void @adn_println(i64* %v3)
  ret i64 0
}

declare i64* @adn_input(i64* %v4)

declare void @adn_println(i64* %v5)

