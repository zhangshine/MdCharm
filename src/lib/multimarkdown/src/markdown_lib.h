#ifndef MARKDOWN_LIB_H
#define MARKDOWN_LIB_H

#include <stdlib.h>
#include <stdio.h>
#include "glib.h"

#ifdef _WIN32 || _EXPORTING
#define DECLSPEC __declspec(dllexport)
#else
#define DECLSPEC
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum markdown_extensions {
    EXT_SMART            = 1 << 0,
    EXT_NOTES            = 1 << 1,
    EXT_FILTER_HTML      = 1 << 2,
    EXT_FILTER_STYLES    = 1 << 3,
    EXT_COMPATIBILITY    = 1 << 4,
    EXT_PROCESS_HTML     = 1 << 5,
	EXT_NO_LABELS		 = 1 << 6,
};

enum markdown_formats {
    HTML_FORMAT,
    LATEX_FORMAT,
    MEMOIR_FORMAT,
    BEAMER_FORMAT,
    OPML_FORMAT,
    GROFF_MM_FORMAT,
    ODF_FORMAT,
    ODF_BODY_FORMAT,
	ORIGINAL_FORMAT
};

DECLSPEC GString * markdown_to_g_string(const char *text, int extensions, int output_format);
DECLSPEC char * markdown_to_string(const char *text, int extensions, int output_format);
DECLSPEC char * extract_metadata_value(char *text, int extensions, char *key);
DECLSPEC gboolean has_metadata(char *text, int extensions);
DECLSPEC char * mmd_version();

#ifdef __cplusplus
}
#endif

/* vim: set ts=4 sw=4 : */
#endif

