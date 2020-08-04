/* Test hlFileLoad. */
TEST(FileLoadTest,
{
    char* data;
    HlResult result;

    result = hlFileLoad(HL_NTEXT("data/file_load_test_data.bin"), ((void**)&data), NULL);
    if (HL_FAILED(result)) return result;

    result = (!strcmp(data, "Test string")) ? HL_RESULT_SUCCESS : HL_ERROR_UNKNOWN;
    hlFree(data);

    return result;
})
