#define INF (1.0 / 0.0)
#define UINT_MAX 0xffffffff

#define Index uint
#define INDEX_INVALID UINT_MAX
#define RGBA vec4
#define RGBA_INVALID RGBA(INF)
#define COLOUR_INVALID Colour(RGBA_INVALID, INDEX_INVALID)

struct SubImage {
    ivec2 pos;
    ivec2 size;
};

struct Colour {
    vec4 rgba;
    Index index;
};

struct RenderData {
    float opacity;
    bool indexed;
    Colour transparent;
};
