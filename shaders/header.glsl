#define FLOAT_INF (1.0 / 0.0)
#define UINT_MAX 0xffffffff

#define Index uint
const Index INDEX_INVALID = UINT_MAX;

#define Rgba vec4
const Rgba RGBA_INVALID = Rgba(FLOAT_INF, FLOAT_INF, FLOAT_INF, FLOAT_INF);

struct Colour {
    Rgba rgba;
    Index index;
};
const Colour COLOUR_INVALID = Colour(RGBA_INVALID, INDEX_INVALID);

struct RenderData {
    float opacity;
    bool indexed;
    Colour transparent;
};

struct SubImage {
    ivec2 pos;
    ivec2 size;
};
