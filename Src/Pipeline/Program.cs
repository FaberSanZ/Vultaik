using System.Numerics;
using System.Runtime.CompilerServices;
using Vultaik;
using Vultaik.Graphics;

using var App = new Sample(false);
App.Run();



public class Sample(bool debug) : Application, IDisposable
{


    public Adapter adapter;
    public Surface surface;
    public Device device;
    public SwapChain swapChain;
    public CommandBuffer command;





    public override void Initialize()
    {
        adapter = new Adapter(debug);
        surface = new Surface(adapter, Window);
        device = new Device(surface); // new Device(adapter); // compute
        swapChain = new SwapChain(device);
        command = new CommandBuffer(device, CommandBufferType.Graphics); // default
    }




    public override void Update(TimerTick time)
    {

    }


    public override void Draw(TimerTick time)
    {
        device.ResetFences();

        command.ResetCommandBuffer();
        command.BeginCommandBuffer();


        command.BeginRendering(swapChain.ColorImage);

        // draw

        command.EndRendering();
        command.EndCommandBuffer();


        device.Submit(command);


        swapChain.Present();

    }

    public void Dispose()
    {
        //vkDeviceWaitIdle(device);
        Window.Dispose();

    }
}





