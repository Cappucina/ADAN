#include "embedded_libs.h"
#include <string.h>
#include <stdlib.h>

#include "embedded_libs_data.h"

static const EmbeddedLib REGISTRY[] = {
    {"adan/io", LIB_IO_ADN, LIB_IO_STDOUT_C, "io.h", LIB_IO_H},
	{"adan/collections/object", LIB_COLLECTIONS_OBJECT_ADN, LIB_COLLECTIONS_OBJECT_C,
	 "object.h", LIB_COLLECTIONS_OBJECT_H},
	{"adan/libcrypto", LIB_LIBCRYPTO_ADN, LIB_LIBCRYPTO_C, NULL, NULL},
	{"adan/libsodium", LIB_LIBSODIUM_ADN, LIB_LIBSODIUM_C, NULL, NULL},
	{"adan/process", LIB_PROCESS_ADN, LIB_PROCESS_C, "process.h", LIB_PROCESS_H},
	{"adan/regex", LIB_REGEX_ADN, LIB_REGEX_C, "regex.h", LIB_REGEX_H},
    {"adan/runtime", LIB_RUNTIME_ADN, LIB_RUNTIME_C, "runtime.h", LIB_RUNTIME_H},
};

const EmbeddedLib* embedded_lib_get(const char* import_path)
{
	if (!import_path)
	{
		return NULL;
	}
	for (size_t i = 0; i < sizeof(REGISTRY) / sizeof(REGISTRY[0]); i++)
	{
		if (strcmp(REGISTRY[i].import_path, import_path) == 0)
		{
			return &REGISTRY[i];
		}
	}
	return NULL;
}

const char* embedded_lib_get_adn_source(const char* import_path)
{
	const EmbeddedLib* lib = embedded_lib_get(import_path);
	return lib ? lib->adn_source : NULL;
}

const char* embedded_lib_get_c_source(const char* import_path)
{
	const EmbeddedLib* lib = embedded_lib_get(import_path);
	return lib ? lib->c_source : NULL;
}

const char* embedded_lib_get_h_filename(const char* import_path)
{
	const EmbeddedLib* lib = embedded_lib_get(import_path);
	return lib ? lib->h_filename : NULL;
}

const char* embedded_lib_get_h_source(const char* import_path)
{
	const EmbeddedLib* lib = embedded_lib_get(import_path);
	return lib ? lib->h_source : NULL;
}

const char* embedded_lib_get_all_import_paths()
{
	size_t count = sizeof(REGISTRY) / sizeof(REGISTRY[0]);
	size_t total = 0;
	for (size_t i = 0; i < count; i++)
	{
		if (REGISTRY[i].import_path)
		{
			total += strlen(REGISTRY[i].import_path) + 1;
		}
	}

	char* csv = malloc(total + 1);
	if (!csv)
	{
		return NULL;
	}

	csv[0] = '\0';
	for (size_t i = 0; i < count; i++)
	{
		if (!REGISTRY[i].import_path)
		{
			continue;
		}
		if (csv[0] != '\0')
		{
			strcat(csv, ",");
		}
		strcat(csv, REGISTRY[i].import_path);
	}

	return csv;
}
