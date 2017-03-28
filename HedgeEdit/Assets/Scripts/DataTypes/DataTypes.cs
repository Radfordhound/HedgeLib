public static class DataTypes
{
    //Methods
    public static IGameDataType GetDataType(string dataType)
    {
        switch (dataType.ToLower())
        {
            case "gens":
                return new Gens();

            case "lw":
                return new LW();

            case "colors":
                return new Colors();
        }

        return null;
    }
}