#include "platform.h"

Target* get_self_as_target(void)
{
    static Target target;

#ifdef __x86_64__
    target.arch = ARCH_AMD64;
#elif defined __i386__
    target.arch = ARCH_AMD32;
#elif defined __aarch64__
    target.arch = ARCH_ARM64;
#elif defined __arm__
    target.arch = ARCH_ARM32;
#else
    target.arch = ARCH_UNKNOWN;
#endif

#ifdef _WIN32
    target.kernel = KERNEL_WINDOWS_NT;
#elif defined __linux__
    target.kernel = KERNEL_LINUX;
#elif defined __APPLE__ && defined __MACH__
    target.kernel = KERNEL_UNX;
#elif defined __FreeBSD__
    target.kernel = KERNEL_FREE_BSD;
#elif defined __OpenBSD__
    target.kernel = KERNEL_OPEN_BSD;
#else
    target.kernel = KERNEL_UNKNOWN;
#endif

    return &target;
}
