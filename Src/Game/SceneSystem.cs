using System;
using System.Collections.Generic;

namespace Vultaik;

public sealed class Scene
{
    public string Name { get; }
    public World World { get; } = new();

    public Scene(string name)
    {
        Name = name;
    }
}

public sealed class SceneSystem
{
    private readonly Dictionary<string, Scene> _scenes = new(StringComparer.Ordinal);
    private Scene? _activeScene;

    public Scene? ActiveScene => _activeScene;
    public IEnumerable<Scene> Scenes => _scenes.Values;
    public int Count => _scenes.Count;

    public Scene CreateScene(string name)
    {
        if (_scenes.TryGetValue(name, out Scene? scene)) return scene;

        scene = new Scene(name);
        _scenes.Add(name, scene);
        _activeScene ??= scene;
        return scene;
    }

    public bool DestroyScene(string name)
    {
        if (!_scenes.Remove(name, out Scene? scene)) return false;

        scene.World.Clear();

        if (!ReferenceEquals(_activeScene, scene)) return true;

        _activeScene = null;

        foreach (Scene remaining in _scenes.Values)
        {
            _activeScene = remaining;
            break;
        }

        return true;
    }

    public Scene? GetScene(string name)
    {
        _scenes.TryGetValue(name, out Scene? scene);
        return scene;
    }

    public bool SetActiveScene(string name)
    {
        if (!_scenes.TryGetValue(name, out Scene? scene)) return false;
        _activeScene = scene;
        return true;
    }

    public void Clear()
    {
        foreach (Scene scene in _scenes.Values) scene.World.Clear();
        _scenes.Clear();
        _activeScene = null;
    }
}
