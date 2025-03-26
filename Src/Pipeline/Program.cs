using System.Numerics;
using System.Runtime.CompilerServices;
using Vultaik;
using Vultaik.Graphics;

using var App = new Sample(false);
App.Run();


public unsafe class Sample(bool debug) : Application, IDisposable
{

    private Adapter? _adapter;
    private Surface? _surface;
    private Device? _device;
    private SwapChain? _swapChain;
    private CommandBuffer? _command;


    public override void Initialize()
    {
        _adapter = new Adapter(debug);
        _surface = new Surface(_adapter, Window);
        _device = new Device(_surface); // new Device(adapter); // compute
        _swapChain = new SwapChain(_device);
        _command = new CommandBuffer(_device, CommandBufferType.Graphics); // default
    }



    public override void Update(TimerTick time)
    {

    }


    public override void Draw(TimerTick time)
    {
        _device!.ResetFences();

        _command!.ResetCommandBuffer();
        _command.BeginCommandBuffer();


        _command.BeginRendering(_swapChain!.ColorImage);

        // draw

        _command.EndRendering();
        _command.EndCommandBuffer();


        _device.Submit(_command);


        _swapChain!.Present();

    }

    public void Dispose()
    {
        //TODO: Sync
        //vkDeviceWaitIdle(device);

        _swapChain!.Dispose();
        _command!.Dispose();
        _device!.Dispose();
        _surface!.Dispose();
        _adapter!.Dispose();

        Window.Dispose();

    }
}
