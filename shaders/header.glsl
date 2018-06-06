#define INF (1.0 / 0.0)
#define UINT_MAX 0xffffffff

#define Index uint
#define INDEX_INVALID 0xffffffff
#define RGBA_INVALID vec4(INF)

struct SubImage {
    ivec2 pos;
    ivec2 size;
};

struct Colour {
    vec4 rgba;
    Index index;
};
