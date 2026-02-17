#include <stdio.h>
#include <stdlib.h>

#include "helper.h"
#include "stm.h"
#include "frontend/scanner/scanner.h"

int main()
{
	char* source = read_file("./samples/hello.adn");

	SymbolTableStack* global_stack = sts_init();
	Scanner* scanner = scanner_init(source);

	Token* token_stream = scanner_scan(scanner);

	// print_token_stream(token_stream);

	free(source);
	scanner_free(scanner);
	sts_free(global_stack);
	token_stream_free(token_stream);
}