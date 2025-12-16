#include "simple_logger.h"

#include "simple_json.h"

#include "def.h"
#include "resource.h"

ResourceList g_resourceList = {0};

void resource_init() {
    DefinitionData *def, *rListDef, *rDef;
    int i;
    def = def_load("defs/resources.def");
    if (!def) {
        slog("Failed to load resources.def\n");
        return;
    }

    rListDef = def_data_get_array(def, "resources");
    if (!rListDef) {
        slog("No resources found in resources.def\n");
        return;
    }

    def_data_array_get_count(rListDef, &g_resourceList.count);
    g_resourceList.resources = malloc(sizeof(Resource) * g_resourceList.count);
    for (i = 0; i < g_resourceList.count; i++) {
        rDef = def_data_array_get_nth(rListDef, i);
        g_resourceList.resources[i].name = strdup(def_data_get_string(rDef, "name"));
        g_resourceList.resources[i].type = RESOURCE_TYPE_UNKNOWN;
        g_resourceList.resources[i].icon = strdup(def_data_get_string(rDef, "icon"));
        def_data_get_float(rDef, "asteroidChance", &g_resourceList.resources[i].asteroidChance);
    }
}

const Resource* resource_get_by_name(const char *name) {
    int i;
    for (i = 0; i < g_resourceList.count; i++) {
        if (strcmp(g_resourceList.resources[i].name, name) == 0)
            return &g_resourceList.resources[i];
    }
    return NULL;
}

void resource_amount_from_config(SJson *cfg, ResourceAmount *out) {
    if (!cfg || !out) return;
    out->resource = resource_get_by_name(sj_object_get_value_as_string(cfg, "resource"));
    if (!out->resource) slog("Failed to load resource: %s", sj_object_get_value_as_string(cfg, "resource"));
    sj_object_get_value_as_int(cfg, "amount", &out->amount);
}