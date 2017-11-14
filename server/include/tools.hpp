constexpr bool equal(const char *a, const char *b) {
    return *a == *b && (*a == '\0' || equal(a + 1, b + 1));
}
