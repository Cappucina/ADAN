#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

#ifndef BUILDING_COMPILER_MAIN
const char* read_file(const char* file_path) {
	FILE* fp = fopen(file_path, "rb");
	if (!fp) {
		printf("error: failed to open file %s\n", file_path);
		return NULL;
	}

	char* out = NULL;
	size_t len = 0;
	size_t cap = 1024;

	out = malloc(cap);
	if (!out) {
		fclose(fp);
		return NULL;
	}

	char buf[256];
	while (fgets(buf, sizeof(buf), fp)) {
		size_t blen = strlen(buf);
		if (len + blen + 1 > cap) {
			cap *= 2;
			char* tmp = realloc(out, cap);
			if (!tmp) {
				free(out);
				fclose(fp);
				return NULL;
			}
			out = tmp;
		}

		memcpy(out + len, buf, blen);
		len += blen;
	}

	out[len] = '\0';
	fclose(fp);

	return out;
}

bool write_file(const char* file_path, const char* content) {
	FILE* fp = fopen(file_path, "wb");
	if (!fp) {
		char cmd[1024];
		snprintf(cmd, sizeof(cmd), "mkdir -p $(dirname \"%s\")", file_path);
		system(cmd);
		fp = fopen(file_path, "wb");
		if (!fp) {
			printf("error: failed to open file %s for writing\n", file_path);
			return false;
		}
	}

	size_t written = fwrite(content, 1, strlen(content), fp);
	fclose(fp);

	return written == strlen(content);
}

bool append_file(const char* file_path, const char* content) {
	FILE* fp = fopen(file_path, "ab");
	if (!fp) {
		char cmd[1024];
		snprintf(cmd, sizeof(cmd), "mkdir -p $(dirname \"%s\")", file_path);
		system(cmd);
		fp = fopen(file_path, "ab");
		if (!fp) {
			printf("error: failed to open file %s for appending\n", file_path);
			return false;
		}
	}

	size_t written = fwrite(content, 1, strlen(content), fp);
	fclose(fp);

	return written == strlen(content);
}

bool file_exists(const char* file_path) {
	FILE* fp = fopen(file_path, "rb");
	if (!fp) return false;
	fclose(fp);
	return true;
}

bool is_directory(const char* path) {
    struct stat st;
    int res = stat(path, &st);
    if (res != 0) {
        return false;
    }
    return S_ISDIR(st.st_mode);
}

bool delete_file(const char* file_path) {
	if (remove(file_path) == 0) {
		return true;
	} else {
		return false;
	}
}

bool create_directory(const char* dir_path) {
	if (is_directory(dir_path)) return true;

	// reciursively create parent directories
	const char* last_slash = strrchr(dir_path, '/');
	if (last_slash) {
		const char* parent_dir = strndup(dir_path, last_slash - dir_path);
		if (!create_directory(parent_dir)) {
			free((void*)parent_dir);
			return false;
		}
		free((void*)parent_dir);
	}

	mkdir(dir_path, 0777);
	return mkdir(dir_path, 0777) == 0;
}

bool delete_directory(const char* dir_path) {
	DIR* dir = opendir(dir_path);
    if (!dir) {
        perror("opendir failed");
        return false;
    }

    struct dirent* entry;
    char path[4096];

    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(path, sizeof(path), "%s/%s", dir_path, entry->d_name);

        struct stat st;
        if (stat(path, &st) != 0) {
            perror("stat failed");
            closedir(dir);
            return false;
        }

        if (S_ISDIR(st.st_mode)) {
            // Recursively delete subdirectory
            if (!delete_directory(path)) {
                closedir(dir);
                return false;
            }
        } else {
            // Delete file
            if (unlink(path) != 0) {
                perror("unlink failed");
                closedir(dir);
                return false;
            }
        }
    }

    closedir(dir);

    // Delete the now-empty directory
    if (rmdir(dir_path) != 0) {
        perror("rmdir failed");
        return false;
    }

    return true;
}

bool is_file(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) return false;
	return S_ISREG(st.st_mode);
}

bool copy_file(const char* src_path, const char* dest_path) {
    int src_fd = open(src_path, O_RDONLY);
    if (src_fd < 0) return false;

    int dest_fd = open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd < 0) {
        close(src_fd);
        return false;
    }

    char buffer[8192];
    ssize_t n;
    while ((n = read(src_fd, buffer, sizeof(buffer))) > 0) {
        if (write(dest_fd, buffer, n) != n) {
            close(src_fd);
            close(dest_fd);
            return false;
        }
    }

    close(src_fd);
    close(dest_fd);
    return n == 0;  // ensure read completed without error
}

bool move_file(const char* src_path, const char* dest_path) {
    if (rename(src_path, dest_path) == 0) return true; 
	
    if (!copy_file(src_path, dest_path)) return false;
    if (unlink(src_path) != 0) return false;
    return true;
}

void sleep_ns(int64_t nanoseconds) {
	struct timespec req, rem;
	req.tv_sec = nanoseconds / 1000000000;
	req.tv_nsec = nanoseconds % 1000000000;
	nanosleep(&req, &rem);
}
#endif