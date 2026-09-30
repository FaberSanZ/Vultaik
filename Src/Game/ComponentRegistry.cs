using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;

namespace Vultaik;

public sealed class ComponentMemberDescriptor
{
    public string Name { get; }
    public string DisplayName { get; }
    public string? Category { get; }
    public int Order { get; }
    public Type ValueType => Field.FieldType;
    public FieldInfo Field { get; }

    internal ComponentMemberDescriptor(FieldInfo field)
    {
        Field = field;

        DataMemberAttribute data = field.GetCustomAttribute<DataMemberAttribute>()!;
        DisplayAttribute? display = field.GetCustomAttribute<DisplayAttribute>();

        Name = data.Name ?? field.Name;
        DisplayName = display?.Name ?? field.Name;
        Category = display?.Category;
        Order = display?.Order ?? data.Order;
    }

    public object? GetValue(object component) => Field.GetValue(component);
    public void SetValue(object component, object? value) => Field.SetValue(component, value);
}

public sealed class ComponentDescriptor
{
    private readonly Func<World, Entity, bool> _has;
    private readonly Action<World, Entity> _add;
    private readonly Action<World, Entity> _remove;
    private readonly Func<World, Entity, object> _get;
    private readonly Action<World, Entity, object> _set;

    public string Name { get; }
    public string Category { get; }
    public Type Type { get; }
    public IReadOnlyList<ComponentMemberDescriptor> Members { get; }

    internal ComponentDescriptor(Type type, string name, string category, IReadOnlyList<ComponentMemberDescriptor> members, Func<World, Entity, bool> has, Action<World, Entity> add, Action<World, Entity> remove, Func<World, Entity, object> get, Action<World, Entity, object> set)
    {
        Type = type;
        Name = name;
        Category = category;
        Members = members;
        _has = has;
        _add = add;
        _remove = remove;
        _get = get;
        _set = set;
    }

    public bool Has(World world, Entity entity) => _has(world, entity);
    public void Add(World world, Entity entity) => _add(world, entity);
    public void Remove(World world, Entity entity) => _remove(world, entity);
    public object GetBoxed(World world, Entity entity) => _get(world, entity);
    public void SetBoxed(World world, Entity entity, object component) => _set(world, entity, component);
}

public sealed class ComponentRegistry
{
    private readonly Dictionary<Type, ComponentDescriptor> _byType = new();
    private readonly Dictionary<string, ComponentDescriptor> _byName = new(StringComparer.Ordinal);
    private ComponentDescriptor[] _components = [];

    public IReadOnlyList<ComponentDescriptor> Components => _components;
    public int Count => _components.Length;

    public void Discover(params Assembly[] assemblies)
    {
        _byType.Clear();
        _byName.Clear();

        HashSet<Assembly> uniqueAssemblies = new(assemblies);

        foreach (Assembly assembly in uniqueAssemblies)
        {
            foreach (Type type in assembly.GetTypes())
            {
                ComponentAttribute? component = type.GetCustomAttribute<ComponentAttribute>();

                if (component is null) continue;
                if (!type.IsValueType || type.IsEnum) throw new InvalidOperationException($"{type.FullName} uses [Component] but is not a struct.");

                string name = component.Name ?? type.Name;
                string category = component.Category ?? "Components";

                if (_byName.ContainsKey(name)) throw new InvalidOperationException($"Duplicate component name '{name}'.");

                ComponentMemberDescriptor[] members = type.GetFields(BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic)
                    .Where(field => field.GetCustomAttribute<DataMemberAttribute>() is not null)
                    .Select(field => new ComponentMemberDescriptor(field))
                    .OrderBy(member => member.Order)
                    .ThenBy(member => member.Name, StringComparer.Ordinal)
                    .ToArray();

                ComponentDescriptor descriptor = CreateDescriptor(type, name, category, members);
                _byType.Add(type, descriptor);
                _byName.Add(name, descriptor);
            }
        }

        _components = _byType.Values.OrderBy(component => component.Category, StringComparer.Ordinal).ThenBy(component => component.Name, StringComparer.Ordinal).ToArray();
    }

    public bool TryGet(Type type, out ComponentDescriptor descriptor) => _byType.TryGetValue(type, out descriptor!);
    public bool TryGet(string name, out ComponentDescriptor descriptor) => _byName.TryGetValue(name, out descriptor!);
    public ComponentDescriptor Get(Type type) => _byType.TryGetValue(type, out ComponentDescriptor? descriptor) ? descriptor : throw new KeyNotFoundException($"Component '{type.FullName}' is not registered.");
    public ComponentDescriptor Get(string name) => _byName.TryGetValue(name, out ComponentDescriptor? descriptor) ? descriptor : throw new KeyNotFoundException($"Component '{name}' is not registered.");

    private static ComponentDescriptor CreateDescriptor(Type type, string name, string category, IReadOnlyList<ComponentMemberDescriptor> members)
    {
        Type bridgeType = typeof(ComponentBridge<>).MakeGenericType(type);

        Func<World, Entity, bool> has = (Func<World, Entity, bool>)bridgeType.GetMethod(nameof(ComponentBridge<int>.Has), BindingFlags.Public | BindingFlags.Static)!.CreateDelegate(typeof(Func<World, Entity, bool>));
        Action<World, Entity> add = (Action<World, Entity>)bridgeType.GetMethod(nameof(ComponentBridge<int>.Add), BindingFlags.Public | BindingFlags.Static)!.CreateDelegate(typeof(Action<World, Entity>));
        Action<World, Entity> remove = (Action<World, Entity>)bridgeType.GetMethod(nameof(ComponentBridge<int>.Remove), BindingFlags.Public | BindingFlags.Static)!.CreateDelegate(typeof(Action<World, Entity>));
        Func<World, Entity, object> get = (Func<World, Entity, object>)bridgeType.GetMethod(nameof(ComponentBridge<int>.GetBoxed), BindingFlags.Public | BindingFlags.Static)!.CreateDelegate(typeof(Func<World, Entity, object>));
        Action<World, Entity, object> set = (Action<World, Entity, object>)bridgeType.GetMethod(nameof(ComponentBridge<int>.SetBoxed), BindingFlags.Public | BindingFlags.Static)!.CreateDelegate(typeof(Action<World, Entity, object>));

        return new ComponentDescriptor(type, name, category, members, has, add, remove, get, set);
    }

    private static class ComponentBridge<T> where T : struct
    {
        public static bool Has(World world, Entity entity) => world.Has<T>(entity);
        public static void Add(World world, Entity entity) => world.Set<T>(entity);
        public static void Remove(World world, Entity entity) => world.Remove<T>(entity);
        public static object GetBoxed(World world, Entity entity) => world.Get<T>(entity);

        public static void SetBoxed(World world, Entity entity, object component)
        {
            ref T destination = ref world.Set<T>(entity);
            destination = (T)component;
        }
    }
}
