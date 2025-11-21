#ifndef __DEF_H__
#define __DEF_H__

#include <gfc_types.h>

typedef struct SJson_S DefinitionData;

typedef struct Definition_s {
    Uint8 _inuse;
    char *name;
    DefinitionData *data;
} Definition;

void def_init(unsigned int maxDefs);

/**
 * @brief load a definition file
 * @param filename the file to load
 * @return NULL on error or the loaded definition
 */
DefinitionData *def_load(const char *filename);

/**
 * @brief load all definition files in a directory
 * @param directory the directory to load from
 */
void def_load_directory(const char *directory);

/**
 * @brief free a previously loaded definition
 * @param def the definition to free
 */
void def_free(Definition *def);

/**
 * @brief Get an object value from a definition data
 * @param def the definition data to search
 * @param key the key to search by
 * @return NULL on error or the definition data value
 */
DefinitionData *def_data_get_obj(DefinitionData *def, const char *key);

/**
 * @brief Get an array value from a definition data
 * @param def the definition data to search
 * @param key the key to search by
 * @return NULL on error or the definition data value
 */
DefinitionData *def_data_get_array(DefinitionData *def, const char *key);

/**
 * @brief Get an string value from a definition data
 * @param def the definition data to search
 * @param key the key to search by
 * @return NULL on error or the definition data value
 */
const char *def_data_get_string(DefinitionData *def, const char *key);

/**
 * @brief Get an int value from a definition data
 * @param def the definition data to search
 * @param key the key to search by
 * @param output where the value is written to
 * @return 0 on error, 1 if the value was retrieved successfully
 */
int def_data_get_int(DefinitionData *def, const char *key, int *output);

/**
 * @brief Get a float value from a definition data
 * @param def the definition data to search
 * @param key the key to search by
 * @param output where the value is written to
 * @return 0 on error, 1 if the value was retrieved successfully
 */
int def_data_get_float(DefinitionData *def, const char *key, float *output);

/**
 * @brief Get a double value from a definition data
 * @param def the definition data to search
 * @param key the key to search by
 * @param output where the value is written to
 * @return 0 on error, 1 if the value was retrieved successfully
 */
int def_data_get_double(DefinitionData *def, const char *key, double *output);

/**
 * @brief retrieve the nth element in the definition data array
 * @param array the definition data array
 * @param n the index of the element to get
 * @return NULL on error or the DefinitionData value otherwise
 */
DefinitionData *def_data_array_get_nth(DefinitionData *array, int n);

/**
 * @brief get the count of elements in a definition data array
 * @param array the definition data array
 * @param count where the count is written to
 * @return 0 on error, 1 if the count was retrieved successfully
 */
int def_data_array_get_count(DefinitionData *array, int *count);


#endif /* __DEF_H__ */