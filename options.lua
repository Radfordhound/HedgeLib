-- Library Type
newoption(
{
    trigger = "type",
    value = "shared/static",
    description = "What type of library HedgeLib is/will be",
    default = "static",
    allowed =
    {
        { "static", "Static library (e.g. .lib/.a)" },
        { "shared", "Shared library (e.g. .dll/.so)" },
    }
})

LibType = _OPTIONS["type"]
