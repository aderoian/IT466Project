#ifndef __GF3D_NOISE__
#define __GF3D_NOISE__

#include "gfc_vector.h"

#define SourceLength 256

#define RandomSize 256
#define Sqrt3 .7320508075688772935
#define Sqrt5 2.2360679774997896964

#define F2 (0.5*(Sqrt3 - 1.0))
#define G2 ((3.0 - Sqrt3)/6.0)
#define G22 (G2*2.0 - 1)
#define F3 (1.0/3.0)
#define G3 (1.0/6.0)
#define F4 ((Sqrt5 - 1.0)/4.0)
#define G4 ((5.0 - Sqrt5)/20.0)
#define G42 (G4*2.0)
#define G43 (G4*3.0)
#define G44 (G4*4.0 - 1.0)

#define fast_floor(n) ((n) >= 0 ? (int)(n) : (int)(n) - 1)
#define dot_xyzt(g, x, y, z, t) (g[0] * (x) + g[1] * (y) + g[2] * (z) + g[3] * (t))
#define dot_xyz(g, x, y, z) (g[0] * (x) + g[1] * (y) + g[2] * (z))
#define dot_xy(g, x, y) (g[0] * (x) + g[1] * (y))

#define unpack_uint32(v, buffer) buffer[0] = (char)(v & 0x00ff); \
        buffer[1] = (char)((v & 0xff00) >> 8); \
        buffer[2] = (char)((v & 0x00ff0000) >> 16); \
        buffer[3] = (char)((v & 0xff000000) >> 24)

typedef struct Noise_S {
    int _random[RandomSize * 2];
} Noise;

Noise* noise_new();
Noise* noise_new_seed(int seed);

float noise_evaluate(const Noise* noise, GFC_Vector3D point);

#endif // __GF3D_NOISE__