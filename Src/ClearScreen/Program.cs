// See https://aka.ms/new-console-template for more information

using Vultaik;

using var App = new Sample();
App.Run();


public class Sample : Application, IDisposable
{


    public override void Initialize()
    {


    }


    public override void Update(TimerTick time)
    {
        //Console.WriteLine(time.TotalTime);
    }


    public override void Draw(TimerTick time)
    {

    }

    public void Dispose()
    {

    }
}