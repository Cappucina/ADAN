#include <stdio.h>
#include <stdlib.h>

#include "helper.h"
#include "stm.h"

int main() {
    char* hello_sample = "./samples/hello.adn";
    char* hello_source = read_file(hello_sample);

    SymbolTableManager* global_manager = stm_init();
}