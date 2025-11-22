#ifndef __RESOURCE_H__
#define __RESOURCE_H__

struct SJson_S;

typedef enum ResourceType_e {
    RESOURCE_TYPE_UNKNOWN
} ResourceType;

typedef struct Resource_s {
    const char* name;
    ResourceType type;
} Resource;

typedef struct ResourceAmount_s {
    const Resource *resource;
    int amount;
} ResourceAmount;

void resource_init();

const Resource* resource_get_by_name(const char *name);

void resource_amount_from_config(struct SJson_S *cfg, ResourceAmount *out);

#endif /* __RESOURCE_H__ */