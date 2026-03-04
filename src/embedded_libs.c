#include "embedded_libs.h"
#include <string.h>

#include "embedded_libs_data.h"

static const EmbeddedLib REGISTRY[] = {
    {"adan/io", LIB_IO_ADN, LIB_IO_C, "print.h", LIB_IO_H},
};

const EmbeddedLib* embedded_lib_get(const char* import_path)
{
	if (!import_path)
		return NULL;
	for (size_t i = 0; i < sizeof(REGISTRY) / sizeof(REGISTRY[0]); i++)
	{
		if (strcmp(REGISTRY[i].import_path, import_path) == 0)
			return &REGISTRY[i];
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
