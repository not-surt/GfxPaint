#if !defined(ATTRIBUTELESS_GLSL)
#define ATTRIBUTELESS_GLSL

#define ATTRIBUTELESS_SINGLE_VERTEX\
const vec2 vertices[1] = vec2[](\
    vec2(0.0, 0.0)\
);

#define ATTRIBUTELESS_CLIP_QUAD\
const vec2 vertices[4] = vec2[](\
    vec2(-1.0, 1.0),\
    vec2(-1.0, -1.0),\
    vec2(1.0, 1.0),\
    vec2(1.0, -1.0)\
);

#define ATTRIBUTELESS_UNIT_QUAD\
const vec2 vertices[4] = vec2[](\
    vec2(0.0, 1.0),\
    vec2(0.0, 0.0),\
    vec2(1.0, 1.0),\
    vec2(1.0, 0.0)\
);

#endif // ATTRIBUTELESS_GLSL
