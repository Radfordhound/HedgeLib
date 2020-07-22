/* Test generic usage of memory allocation functions. */
TEST(GenericAllocTest,
{
    struct Foo
    {
        int a;
        int b;
    } *f;

    int* arr;

    /* Test initial allocation. */
    arr = HL_ALLOC_ARR(int, 2);
    if (!arr) return HL_ERROR_OUT_OF_MEMORY;

    arr[0] = 1;
    arr[1] = 7;

    if (arr[0] != 1 || arr[1] != 7) return HL_ERROR_UNKNOWN;

    /* Test initial reallocation. */
    arr = HL_RESIZE_ARR(int, 4, arr);
    if (!arr) return HL_ERROR_OUT_OF_MEMORY;

    arr[2] = 2;
    arr[3] = 9;

    if (arr[2] != 2 || arr[3] != 9) return HL_ERROR_UNKNOWN;

    /* Test initial free. */
    hlFree(arr);

    /* Test secondary allocation. */
    f = HL_ALLOC_OBJ(struct Foo);
    if (!f) return HL_ERROR_OUT_OF_MEMORY;

    f->a = 4;
    f->b = 2;

    if (f->a != 4 || f->b != 2) return HL_ERROR_UNKNOWN;

    /* Test secondary reallocation. */
    f = hlRealloc(f, sizeof(struct Foo) * 2);
    if (!f) return HL_ERROR_OUT_OF_MEMORY;

    f[1].a = (f[0].a * 2);
    f[1].b = (f[0].b * 2);

    if (f[1].a != 8 || f[1].b != 4) return HL_ERROR_UNKNOWN;

    /* Test secondary free. */
    hlFree(f);
    return HL_RESULT_SUCCESS;
})

/* Test realloc and free on null pointer. */
TEST(NullAllocTest,
{
    /* NULL hlRealloc test. */
    int* data = (int*)hlRealloc(NULL, sizeof(int));
    if (!data) return HL_ERROR_OUT_OF_MEMORY;

    *data = 7;

    /* hlRealloc size 0 test. */
    data = hlRealloc(data, 0);
    if (!data) return HL_ERROR_OUT_OF_MEMORY;

    /* null hlFree test. */
    hlFree(0);
    return HL_RESULT_SUCCESS;
})
