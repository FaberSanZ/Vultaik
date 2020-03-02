using System;

namespace _01__CreatingWindow
{
    class Program
    {
        static void Main(string[] args)
        {
            bool e = Equals("1" + 1, 1 + "1");
            Console.WriteLine("1" + 1 == 1 + "1" || e);
        }
    }
}
