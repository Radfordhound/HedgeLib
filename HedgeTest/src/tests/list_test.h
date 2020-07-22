/* Test creation and basic use of an HL_LIST. */
TEST(ListTest,
{
    HL_LIST(int) list;
    int i = 0;
    HlResult result;

    /* Initialize list. */
    HL_LIST_INIT(list);
    
    /* Push items into list. */
    while (i++ < 4)
    {
        result = HL_LIST_PUSH(list, i);
        if (HL_FAILED(result)) goto end;
    }

    /* Verify list integrity. */
    result = (list.count == 4) ? HL_RESULT_SUCCESS : HL_ERROR_UNKNOWN;
    if (HL_FAILED(result)) goto end;

    for (i = 0; i < 4; ++i)
    {
        result = (list.data[i] == (i + 1)) ? HL_RESULT_SUCCESS : HL_ERROR_UNKNOWN;
        if (HL_FAILED(result)) goto end;
    }

end:
    HL_LIST_FREE(list);
    return result;
})
