#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <pwd.h>
#include <string.h>
#include <sys/types.h>

char *get_cwd() {
    long size = pathconf(".", _PC_PATH_MAX);
    if (size == -1) {
        size = PATH_MAX;
    }

    char *buf = malloc(size);
    if (!buf) {
        return NULL;
    }

    if (!getcwd(buf, size)) {
        free(buf);
        return NULL;
    }

    return buf;
}

char* get_current_user() {
    uid_t uid = getuid();
    struct passwd pwd;
    struct passwd *result = NULL;

    long bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufsize == -1) bufsize = 16384;

    char *buf = malloc(bufsize);
    if (!buf) return strdup("");

    if (getpwuid_r(uid, &pwd, buf, bufsize, &result) != 0 || !result) {
        free(buf);
        return strdup("");
    }

    char *username = result->pw_name ? strdup(result->pw_name) : strdup("");
    
    free(buf);

    if (!username) return strdup("");
    
    return username;
}

char *get_user_home() {
    struct passwd *pw = getpwuid(getuid());
    if (pw) {
        return strdup(pw->pw_dir);
    }
    return NULL;
}

char *get_env_variable(const char *name) {
    char *value = getenv(name);
    if (value) {
        return strdup(value);
    }
    return NULL;
}

void exec(const char *cmd) {
    system(cmd);
}