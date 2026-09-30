using System;

namespace Vultaik;

[AttributeUsage(AttributeTargets.Struct, Inherited = false)]
public sealed class ComponentAttribute : Attribute
{
    public string? Name { get; }
    public string? Category { get; }

    public ComponentAttribute(string? name = null, string? category = null)
    {
        Name = name;
        Category = category;
    }
}

[AttributeUsage(AttributeTargets.Field, Inherited = false)]
public sealed class DataMemberAttribute : Attribute
{
    public string? Name { get; }
    public int Order { get; }

    public DataMemberAttribute(string? name = null, int order = 0)
    {
        Name = name;
        Order = order;
    }
}

[AttributeUsage(AttributeTargets.Field, Inherited = false)]
public sealed class DisplayAttribute : Attribute
{
    public string Name { get; }
    public string? Category { get; }
    public int Order { get; }

    public DisplayAttribute(string name, string? category = null, int order = 0)
    {
        Name = name;
        Category = category;
        Order = order;
    }
}
