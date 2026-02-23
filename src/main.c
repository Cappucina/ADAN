#include <stdio.h>
#include <stdlib.h>

#include "helper.h"
#include "stm.h"
#include "frontend/scanner/scanner.h"
#include "frontend/parser/parser.h"

int main()
{
	char* source = read_file("./samples/hello.adn");

	SymbolTableStack* global_stack = sts_init();
	Scanner* scanner = scanner_init(source);
	Parser* parser = parser_init(scanner);

	parser_parse_program(parser);

	parser_free(parser);
	scanner_free(scanner);
	free(source);

	return 0;
}