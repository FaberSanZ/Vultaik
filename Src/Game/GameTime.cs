using System;
using System.Diagnostics;

namespace Vultaik;

public sealed class GameTime
{
    private readonly Stopwatch _timer = new();
    private double _previousTime;

    public float DeltaTime { get; private set; }
    public double TotalTime => _timer.Elapsed.TotalSeconds;

    public void Reset()
    {
        _timer.Restart();
        _previousTime = 0.0;
        DeltaTime = 0.0f;
    }

    public void Update()
    {
        double currentTime = _timer.Elapsed.TotalSeconds;
        float elapsed = (float)(currentTime - _previousTime);

        _previousTime = currentTime;
        DeltaTime = Math.Min(elapsed, 0.25f);
    }
}
