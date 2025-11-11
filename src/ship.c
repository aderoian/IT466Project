#include "ship.h"

#include "gfc_types.h"
#include "weapon.h"

ShipReactor g_reactors[] = {
    {
        1000,   // Capacity
        20      // Recharge Speed
    },
    {
        1250,   // Capacity
        40      // Recharge Speed
    },
    {
        5000,   // Capacity
        100     // Recharge Speed
    },
    {
        6500,   // Capacity
        500     // Recharge Speed
    },
    {
        10000,  // Capacity
        750     // Recharge Speed
    }
};

ShipHull    g_hulls[] = {
    {
        500,     // Durability
        1,       // Max Reactors
        1        // Max Engines
    },
    {
        1000,    // Durability
        1,       // Max Reactors
        1        // Max Engines
    },
    {
        3000,    // Durability
        1,       // Max Reactors
        1        // Max Engines
    },
    {
        5000,    // Durability
        1,       // Max Reactors
        1        // Max Engines
    },
    {
        75000,   // Durability
        1,       // Max Reactors
        1       // Max Engines
    }
};

ShipEngine  g_engines[] = {
    {
        110,    // Thrust
        15,     // Usage
        500,    // Boost Thrust
        100     // Boost Usage
    },
    {
        135,    // Thrust
        25,     // Usage
        750,    // Boost Thrust
        200     // Boost Usage
    },
    {
        150,    // Thrust
        45,     // Usage
        1250,   // Boost Thrust
        350     // Boost Usage
    },
    {
        200,    // Thrust
        55,     // Usage
        2000,   // Boost Thrust
        500     // Boost Usage
    },
    {
        300,    // Thrust
        75,     // Usage
        3000,   // Boost Thrust
        750     // Boost Usage
    }
};

void ship_init_basic(Ship* out) {
    if(!out) return;

    out->hull = &g_hulls[0];
    out->reactors = (ShipReactor**) gfc_allocate_array(sizeof(ShipReactor*), 1);
    out->storedEnergy = calloc(sizeof(float), 1);
    out->engines = (ShipEngine**) gfc_allocate_array(sizeof(ShipEngine*), 1);
    *out->reactors = &g_reactors[0];
    *out->engines = &g_engines[0];
    out->weapons[0].weapon = &g_weapons[0];
    out->durability = out->hull->durability;
}
