// ============================================================
// Definitions for fixed vec3 type as a patch for the std140 vec3 alignment bug
// ============================================================

#define FIXED_VEC3 vec4

#define FIXED_VEC3_INIT(VARNAME) vec3 VARNAME = vec3(VARNAME.x, VARNAME.y, VARNAME.z)

// ===========================================================
// Commonly used constants/macros
// ===========================================================

#define INSTANCE_LAYOUT 12