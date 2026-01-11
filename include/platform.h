#ifndef PLATFORM_H
#define PLATFORM_H

typedef enum
{
    KERNEL_WINDOWS_NT,
    KERNEL_LINUX,
    KERNEL_VIOS,
    KERNEL_UNX,
    KERNEL_FREE_BSD,
    KERNEL_OPEN_BSD,
    KERNELLESS,
    KERNEL_UNKNOWN
} Kernel;

typedef enum
{
    ARCH_AMD64,
    ARCH_AMD32,
    ARCH_ARM32,
    ARCH_ARM64,
    ARCH_HPU,
    ARCH_UNKNOWN
} Arch;

typedef struct
{
    Kernel kernel;
    Arch arch;
} Target;

Target* get_self_as_target(void);

#endif