#ifndef FLAGS_H
#define FLAGS_H

#include <stdbool.h>

typedef enum {
    WINDOWS,
    DARWIN,     // macOS, iOS, and NeXTSTEP family
    LINUX,      // Linux and Android family
    FREE_BSD,
    OPEN_BSD,
    NET_BSD,
    SOLARIS,
    AIX,
    CLASSIC_MAC // pre-OS X, non-Unix
} OS;

typedef enum {
    X86_64,
    X86_32,
    X86,        // as in real 16 bit mode
    ARM64,
    ARM32,
    MIPS64,
    MIPS32,
    POWERPC64,
    POWERPC32,
    RISCV64,
    RISCV32,
    SPARC64,
    SPARC32
} Arch;

typedef struct {
    Arch arch;
    OS os;
} Target;

typedef struct {
    Target target;
    bool help;
    bool verbose;
    bool keep_asm;
    char* input;
    char* output;
    bool warnings_as_errors;
    bool run_after_compile;
    bool compile_time;
    char** libs;
    int lib_count;
    bool unknown_flag;
} compiler_flags;

typedef void (*flag_setter)(compiler_flags*, void* value);

typedef struct {
    const char* name;
    const char** aliases;
    int aliasCount;
    flag_setter setter;
    void* value;
} FlagEntry;

compiler_flags* flags_init();
int parse_flags(int argc, char **argv, compiler_flags *flags);

#endif // FLAGS_H