#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "simple_logger.h"

#include "simple_json.h"

#include "def.h"

typedef struct DefinitionManager_s {
    Definition *definitions;
    int defMax;
} DefinitionManager;

DefinitionManager def_manager = {0};

void def_close() {
    int i;
    if (def_manager.definitions) {
        for (i = 0; i < def_manager.defMax; i++) {
            if (def_manager.definitions[i]._refc) {
                def_free(&def_manager.definitions[i]);
            }
        }
        free(def_manager.definitions);
        def_manager.definitions = NULL;
        def_manager.defMax = 0;
    }
}

void def_init(unsigned int maxDefs) {
    def_manager.definitions = (Definition *)calloc(maxDefs, sizeof(Definition));
    if (!def_manager.definitions) {
        slog("Failed to allocate memory for definitions");
        return;
    }
    def_manager.defMax = maxDefs;
    atexit(def_close);
}

Definition *def_new() {
    int i;
    for (i = 0; i < def_manager.defMax; i++) {
        if (!def_manager.definitions[i]._refc) {
            def_manager.definitions[i]._refc += 1;
            return &def_manager.definitions[i];
        }
    }
    slog("No free definition slots available");
    return NULL;
}

DefinitionData *def_load(const char *filename) {
    Definition *def;
    DefinitionData *data;
    int i;
    if (!filename) return NULL;
    for (i = 0; i < def_manager.defMax; i++) {
        if (def_manager.definitions[i]._refc && strcmp(def_manager.definitions[i].name, filename) == 0) {
            def_manager.definitions[i]._refc++;
            return def_manager.definitions[i].data;
        }
    }

    def = def_new();
    if (!def) return NULL;

    data = sj_load(filename);
    if (!data) {
        slog("Failed to load definition from file: %s", filename);
        def_free(def);
        return NULL;
    }

    def->name = strdup(filename);
    def->data = data;

    return data;
}

void def_load_directory(const char *directory) {
	struct stat status;
	mode_t mode;
	int result;
	DIR* dirP;
    struct dirent* entryP;
	char fName[1000];

    result = stat(directory, &status);
    if (result != 0) {
        slog("Directory %s does not exist", directory);
        return;
    }

    mode = status.st_mode;
    if (!S_ISDIR(mode)) {
        slog("%s is not a directory", directory);
        return;
    }

    dirP = opendir(directory);
    if (!dirP) {
        slog("Failed to open directory: %s", directory);
        return;
    }

    while ((entryP = readdir(dirP)) != NULL) {
        if (entryP->d_type == DT_REG) {
            snprintf(fName, sizeof(fName), "%s/%s", directory, entryP->d_name);
            if (!def_load(fName)) {
                slog("Failed to load definition file: %s", fName);
            }
        }
    }

    closedir(dirP);
}

void def_free(Definition *def) {
    if (!def || !def->_refc) return;

    def->_refc--;
    if (def->_refc <= 0) return;

    if (def->name) {
        free(def->name);
        def->name = NULL;
    }
    if (def->data) {
        sj_free(def->data);
        def->data = NULL;
    }
    def->_refc = 0;
}

DefinitionData *def_data_get_obj(DefinitionData *def, const char *key) {
    return sj_object_get_value(def, key);
}

DefinitionData *def_data_get_array(DefinitionData *def, const char *key) {
    return sj_object_get_value(def, key);
}

const char *def_data_get_string(DefinitionData *def, const char *key) {
    return sj_object_get_value_as_string(def, key);
}

int def_data_get_int(DefinitionData *def, const char *key, int *output) {
    return sj_object_get_value_as_int(def, key, output);
}

int def_data_get_float(DefinitionData *def, const char *key, float *output) {
    return sj_object_get_value_as_float(def, key, output);
}

int def_data_get_double(DefinitionData *def, const char *key, double *output) {
    return sj_object_get_value_as_double(def, key, output);
}

DefinitionData *def_data_array_get_nth(DefinitionData *array, int n) {
    return sj_array_get_nth(array, n);
}

int def_data_array_get_count(DefinitionData *array, int *count) {
    int c;
    c = sj_array_get_count(array);
    if (count) {
        *count = c;
    }
    return c;
}