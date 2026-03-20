#ifndef EMBEDDED_LIBS_H
#define EMBEDDED_LIBS_H

typedef struct
{
	const char* import_path;
	const char* adn_source;
	const char* c_source;
	const char* h_filename;
	const char* h_source;
} EmbeddedLib;

const EmbeddedLib* embedded_lib_get(const char* import_path);

const char* embedded_lib_get_adn_source(const char* import_path);

const char* embedded_lib_get_c_source(const char* import_path);

const char* embedded_lib_get_h_filename(const char* import_path);

const char* embedded_lib_get_h_source(const char* import_path);

const char* embedded_lib_get_all_import_paths();

#endif