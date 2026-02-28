@.str1 = private constant [5 x i8] c"ADAN\00"
define i64* @language() {
entry:
  %v1 = alloca i64**
  store i64* @.str1, i64** %v1
  %v2 = load i64*, i64** %v1
  ret i64* %v2
}

define i64 @main() {
entry:
  %v3 = call i64* @language()
  call void @adn_println(i64* %v3)
  ret i64 0
}

declare void @adn_println(i64* %v4)

