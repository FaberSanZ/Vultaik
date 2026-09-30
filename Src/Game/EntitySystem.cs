using System;
using System.Buffers.Binary;
using System.Collections.Generic;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Security.Cryptography;

namespace Vultaik;

public readonly struct Entity : IEquatable<Entity>
{
    public readonly int Id;
    public readonly uint Version;

    public Entity(int id, uint version)
    {
        Id = id;
        Version = version;
    }

    public static Entity Null => new(-1, 0);

    public bool Equals(Entity other) => Id == other.Id && Version == other.Version;
    public override bool Equals(object? obj) => obj is Entity other && Equals(other);
    public override int GetHashCode() => HashCode.Combine(Id, Version);
    public static bool operator ==(Entity left, Entity right) => left.Equals(right);
    public static bool operator !=(Entity left, Entity right) => !left.Equals(right);
    public override string ToString() => $"Entity({Id}:{Version})";
}

internal interface IComponentPool
{
    int Count { get; }
    bool Has(int entityId);
    int EntityAt(int denseIndex);
    void Remove(int entityId);
    void Clear();
}

internal sealed class ComponentPool<T> : IComponentPool where T : struct
{
    private T[] _components;
    private int[] _entities;
    private int[] _sparse;
    private int _count;

    public int Count => _count;

    public ComponentPool(int capacity = 128)
    {
        capacity = Math.Max(capacity, 1);
        _components = new T[capacity];
        _entities = new int[capacity];
        _sparse = new int[capacity];
        Array.Fill(_sparse, -1);
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public bool Has(int entityId)
    {
        if ((uint)entityId >= (uint)_sparse.Length) return false;
        int denseIndex = _sparse[entityId];
        return (uint)denseIndex < (uint)_count && _entities[denseIndex] == entityId;
    }

    public ref T Set(int entityId)
    {
        EnsureSparseCapacity(entityId);
        int denseIndex = _sparse[entityId];

        if ((uint)denseIndex < (uint)_count && _entities[denseIndex] == entityId) return ref _components[denseIndex];

        EnsureDenseCapacity(_count + 1);
        denseIndex = _count++;
        _components[denseIndex] = default;
        _entities[denseIndex] = entityId;
        _sparse[entityId] = denseIndex;
        return ref _components[denseIndex];
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public ref T Get(int entityId)
    {
        if (!Has(entityId)) throw new InvalidOperationException($"Entity {entityId} does not have component {typeof(T).Name}.");
        return ref _components[_sparse[entityId]];
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public int EntityAt(int denseIndex)
    {
        if ((uint)denseIndex >= (uint)_count) throw new ArgumentOutOfRangeException(nameof(denseIndex));
        return _entities[denseIndex];
    }

    public void Remove(int entityId)
    {
        if (!Has(entityId)) return;

        int denseIndex = _sparse[entityId];
        int lastIndex = _count - 1;

        if (denseIndex != lastIndex)
        {
            _components[denseIndex] = _components[lastIndex];
            int movedEntity = _entities[lastIndex];
            _entities[denseIndex] = movedEntity;
            _sparse[movedEntity] = denseIndex;
        }

        if (RuntimeHelpers.IsReferenceOrContainsReferences<T>()) _components[lastIndex] = default;
        _entities[lastIndex] = 0;
        _sparse[entityId] = -1;
        _count--;
    }

    public void Clear()
    {
        if (RuntimeHelpers.IsReferenceOrContainsReferences<T>()) Array.Clear(_components, 0, _count);
        for (int i = 0; i < _count; i++) _sparse[_entities[i]] = -1;
        Array.Clear(_entities, 0, _count);
        _count = 0;
    }

    private void EnsureDenseCapacity(int required)
    {
        if (required <= _components.Length) return;
        int capacity = _components.Length;
        while (capacity < required) capacity *= 2;
        Array.Resize(ref _components, capacity);
        Array.Resize(ref _entities, capacity);
    }

    private void EnsureSparseCapacity(int entityId)
    {
        if (entityId < _sparse.Length) return;
        int oldLength = _sparse.Length;
        int capacity = oldLength;
        while (entityId >= capacity) capacity *= 2;
        Array.Resize(ref _sparse, capacity);
        Array.Fill(_sparse, -1, oldLength, capacity - oldLength);
    }
}

public sealed class World
{
    private uint[] _versions;
    private bool[] _alive;
    private int[] _freeIds;
    private int _freeCount;
    private int _nextId;
    private int _count;
    private readonly Dictionary<Type, IComponentPool> _pools = new();

    public int Count => _count;
    public int Capacity => _versions.Length;
    public EntityEnumerable Entities => new(this);

    public World(int capacity = 128)
    {
        capacity = Math.Max(capacity, 1);
        _versions = new uint[capacity];
        _alive = new bool[capacity];
        _freeIds = new int[capacity];
    }

    public Entity Create()
    {
        int id;

        if (_freeCount > 0) id = _freeIds[--_freeCount];
        else
        {
            id = _nextId++;
            EnsureEntityCapacity(id + 1);
        }

        if (_versions[id] == 0) _versions[id] = 1;
        _alive[id] = true;
        _count++;
        return new Entity(id, _versions[id]);
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public bool IsAlive(Entity entity) => entity.Id >= 0 && entity.Id < _nextId && _alive[entity.Id] && _versions[entity.Id] == entity.Version;

    public bool Destroy(Entity entity)
    {
        if (!IsAlive(entity)) return false;

        foreach (IComponentPool pool in _pools.Values) pool.Remove(entity.Id);

        _alive[entity.Id] = false;
        uint version = _versions[entity.Id] + 1;
        _versions[entity.Id] = version == 0 ? 1u : version;

        EnsureFreeCapacity(_freeCount + 1);
        _freeIds[_freeCount++] = entity.Id;
        _count--;
        return true;
    }

    public ref T Set<T>(Entity entity) where T : struct
    {
        ValidateAlive(entity);
        return ref GetOrCreatePool<T>().Set(entity.Id);
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public bool Has<T>(Entity entity) where T : struct => IsAlive(entity) && TryGetPool<T>(out ComponentPool<T>? pool) && pool.Has(entity.Id);

    public ref T Get<T>(Entity entity) where T : struct
    {
        ValidateAlive(entity);
        if (!TryGetPool<T>(out ComponentPool<T>? pool)) 
            throw new InvalidOperationException($"No component pool exists for {typeof(T).Name}.");
        return ref pool!.Get(entity.Id);
    }

    public bool Remove<T>(Entity entity) where T : struct
    {
        if (!IsAlive(entity)) return false;
        if (!TryGetPool<T>(out ComponentPool<T>? pool) || !pool.Has(entity.Id)) return false;
        pool.Remove(entity.Id);
        return true;
    }

    public EntityQuery<T> Query<T>() where T : struct
    {
        TryGetPool<T>(out ComponentPool<T>? pool);
        return new EntityQuery<T>(this, pool);
    }

    public EntityQuery<T1, T2> Query<T1, T2>() where T1 : struct where T2 : struct
    {
        TryGetPool<T1>(out ComponentPool<T1>? first);
        TryGetPool<T2>(out ComponentPool<T2>? second);
        return new EntityQuery<T1, T2>(this, first, second);
    }

    public EntityQuery<T1, T2, T3> Query<T1, T2, T3>() where T1 : struct where T2 : struct where T3 : struct
    {
        TryGetPool<T1>(out ComponentPool<T1>? first);
        TryGetPool<T2>(out ComponentPool<T2>? second);
        TryGetPool<T3>(out ComponentPool<T3>? third);
        return new EntityQuery<T1, T2, T3>(this, first, second, third);
    }

    public void Clear()
    {
        foreach (IComponentPool pool in _pools.Values) pool.Clear();

        Array.Clear(_alive, 0, _alive.Length);
        Array.Clear(_versions, 0, _versions.Length);
        Array.Clear(_freeIds, 0, _freeIds.Length);

        _freeCount = 0;
        _nextId = 0;
        _count = 0;
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    internal Entity GetEntityUnchecked(int entityId) => new(entityId, _versions[entityId]);

    private ComponentPool<T> GetOrCreatePool<T>() where T : struct
    {
        Type type = typeof(T);
        if (_pools.TryGetValue(type, out IComponentPool? pool)) return (ComponentPool<T>)pool;
        ComponentPool<T> newPool = new();
        _pools.Add(type, newPool);
        return newPool;
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    private bool TryGetPool<T>(out ComponentPool<T>? pool) where T : struct
    {
        if (_pools.TryGetValue(typeof(T), out IComponentPool? componentPool))
        {
            pool = (ComponentPool<T>)componentPool;
            return true;
        }

        pool = null;
        return false;
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    private void ValidateAlive(Entity entity)
    {
        if (!IsAlive(entity)) throw new InvalidOperationException($"Entity {entity} is not alive in this World.");
    }

    private void EnsureEntityCapacity(int required)
    {
        if (required <= _versions.Length) return;
        int capacity = _versions.Length;
        while (capacity < required) capacity *= 2;
        Array.Resize(ref _versions, capacity);
        Array.Resize(ref _alive, capacity);
    }

    private void EnsureFreeCapacity(int required)
    {
        if (required <= _freeIds.Length) return;
        int capacity = _freeIds.Length;
        while (capacity < required) capacity *= 2;
        Array.Resize(ref _freeIds, capacity);
    }

    public readonly struct EntityEnumerable
    {
        private readonly World _world;

        internal EntityEnumerable(World world)
        {
            _world = world;
        }

        public Enumerator GetEnumerator() => new(_world);

        public struct Enumerator
        {
            private readonly World _world;
            private int _id;
            private Entity _current;

            internal Enumerator(World world)
            {
                _world = world;
                _id = -1;
                _current = Entity.Null;
            }

            public Entity Current => _current;

            public bool MoveNext()
            {
                while (++_id < _world._nextId)
                {
                    if (!_world._alive[_id]) continue;
                    _current = new Entity(_id, _world._versions[_id]);
                    return true;
                }

                return false;
            }
        }
    }
}

public readonly struct EntityQuery<T> where T : struct
{
    private readonly World _world;
    private readonly ComponentPool<T>? _pool;

    internal EntityQuery(World world, ComponentPool<T>? pool)
    {
        _world = world;
        _pool = pool;
    }

    public Enumerator GetEnumerator() => new(_world, _pool);

    public struct Enumerator
    {
        private readonly World _world;
        private readonly ComponentPool<T>? _pool;
        private int _index;
        private Entity _current;

        internal Enumerator(World world, ComponentPool<T>? pool)
        {
            _world = world;
            _pool = pool;
            _index = -1;
            _current = Entity.Null;
        }

        public Entity Current => _current;

        public bool MoveNext()
        {
            if (_pool is null) return false;

            while (++_index < _pool.Count)
            {
                int entityId = _pool.EntityAt(_index);
                Entity entity = _world.GetEntityUnchecked(entityId);
                if (!_world.IsAlive(entity)) continue;
                _current = entity;
                return true;
            }

            return false;
        }
    }
}

public readonly struct EntityQuery<T1, T2> where T1 : struct where T2 : struct
{
    private readonly World _world;
    private readonly IComponentPool? _driver;
    private readonly ComponentPool<T1>? _first;
    private readonly ComponentPool<T2>? _second;

    internal EntityQuery(World world, ComponentPool<T1>? first, ComponentPool<T2>? second)
    {
        _world = world;
        _first = first;
        _second = second;
        _driver = first is null || second is null ? null : first.Count <= second.Count ? first : second;
    }

    public Enumerator GetEnumerator() => new(_world, _driver, _first, _second);

    public struct Enumerator
    {
        private readonly World _world;
        private readonly IComponentPool? _driver;
        private readonly ComponentPool<T1>? _first;
        private readonly ComponentPool<T2>? _second;
        private int _index;
        private Entity _current;

        internal Enumerator(World world, IComponentPool? driver, ComponentPool<T1>? first, ComponentPool<T2>? second)
        {
            _world = world;
            _driver = driver;
            _first = first;
            _second = second;
            _index = -1;
            _current = Entity.Null;
        }

        public Entity Current => _current;

        public bool MoveNext()
        {
            if (_driver is null || _first is null || _second is null) return false;

            while (++_index < _driver.Count)
            {
                int entityId = _driver.EntityAt(_index);
                if (!_first.Has(entityId) || !_second.Has(entityId)) continue;

                Entity entity = _world.GetEntityUnchecked(entityId);
                if (!_world.IsAlive(entity)) continue;

                _current = entity;
                return true;
            }

            return false;
        }
    }
}

public readonly struct EntityQuery<T1, T2, T3> where T1 : struct where T2 : struct where T3 : struct
{
    private readonly World _world;
    private readonly IComponentPool? _driver;
    private readonly ComponentPool<T1>? _first;
    private readonly ComponentPool<T2>? _second;
    private readonly ComponentPool<T3>? _third;

    internal EntityQuery(World world, ComponentPool<T1>? first, ComponentPool<T2>? second, ComponentPool<T3>? third)
    {
        _world = world;
        _first = first;
        _second = second;
        _third = third;

        if (first is null || second is null || third is null)
        {
            _driver = null;
            return;
        }

        _driver = first;
        if (second.Count < _driver.Count) _driver = second;
        if (third.Count < _driver.Count) _driver = third;
    }

    public Enumerator GetEnumerator() => new(_world, _driver, _first, _second, _third);

    public struct Enumerator
    {
        private readonly World _world;
        private readonly IComponentPool? _driver;
        private readonly ComponentPool<T1>? _first;
        private readonly ComponentPool<T2>? _second;
        private readonly ComponentPool<T3>? _third;
        private int _index;
        private Entity _current;

        internal Enumerator(World world, IComponentPool? driver, ComponentPool<T1>? first, ComponentPool<T2>? second, ComponentPool<T3>? third)
        {
            _world = world;
            _driver = driver;
            _first = first;
            _second = second;
            _third = third;
            _index = -1;
            _current = Entity.Null;
        }

        public Entity Current => _current;

        public bool MoveNext()
        {
            if (_driver is null || _first is null || _second is null || _third is null) return false;

            while (++_index < _driver.Count)
            {
                int entityId = _driver.EntityAt(_index);
                if (!_first.Has(entityId) || !_second.Has(entityId) || !_third.Has(entityId)) continue;

                Entity entity = _world.GetEntityUnchecked(entityId);
                if (!_world.IsAlive(entity)) continue;

                _current = entity;
                return true;
            }

            return false;
        }
    }
}


public static class EntityFactory
{


    private static ulong GenerateID()
    {
        Span<byte> bytes = stackalloc byte[8];
        ulong id;

        do
        {
            RandomNumberGenerator.Fill(bytes);
            id = BinaryPrimitives.ReadUInt64LittleEndian(bytes);
        }
        while (id == 0);

        return id;
    }
}