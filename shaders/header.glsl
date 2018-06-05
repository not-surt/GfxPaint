#define UINT_MAX 0xffffffff

struct SubImage {
    ivec2 pos;
    ivec2 size;
};

struct Colour {
    vec4 rgba;
    uint index;
};
