#if !defined(DISTANCE_GLSL)
#define DISTANCE_GLSL

float distanceEuclidean(const vec2 pos) {
//    return sqrt(pow(pos.x, 2) + pow(pos.y, 2));
    return length(pos);
}

float distanceManhattan(const vec2 pos) {
    return abs(pos.x) + abs(pos.y);
}

float distanceChebyshev(const vec2 pos) {
    return max(abs(pos.x), abs(pos.y));
}

float distanceMinimum(const vec2 pos) {
    return min(abs(pos.x), abs(pos.y));
}

float distanceOctagonal(const vec2 pos) {
    return (1007.0/1024.0) * max(abs(pos.x), abs(pos.y)) + (441.0/1024.0) * min(abs(pos.x), abs(pos.y));
}

#endif // DISTANCE_GLSL
