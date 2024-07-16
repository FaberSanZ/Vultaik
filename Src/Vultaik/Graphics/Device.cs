// Copyright (c) Faber Leonardo. All Rights Reserved. https://github.com/FaberSanZ
// This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)



using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Vultaik;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;

namespace Vultaik.Graphics
{
    public unsafe  class Device
    {

        public VkDevice device;
        public VkQueue graphics_queue;
        public VkQueue present_queue;


        public Device(Adapter adapter)
        {
            Adapter = adapter;

            createLogicalDevice();
        }



        public Device(Surface surface)
        {
            Adapter = surface.adapter;
            Surface = surface;

            createLogicalDevice();
        }

        public Adapter Adapter { get; }
        public Surface? Surface { get; }



        public QueueFamilyIndices indices; 





        // checkDeviceExtensionSupport
        private void createLogicalDevice()
        {
            bool present = Surface != null;
            Queue queue = new Queue(Adapter);

            queue.FillFamilyIndices(null, 0);


            Console.WriteLine("Graphics: " + queue.GetQueue(VkQueueFlags.Graphics).ToString());
            Console.WriteLine();

            Console.WriteLine("Compute: " + queue.GetQueue(VkQueueFlags.Compute).ToString());
            Console.WriteLine();

            Console.WriteLine("Transfer: " + queue.GetQueue(VkQueueFlags.Transfer).ToString());
            Console.WriteLine();

            Console.WriteLine("SparseBinding: " + queue.GetQueue(VkQueueFlags.SparseBinding).ToString());
            Console.WriteLine();

            Console.WriteLine("VideoDecode: " + queue.GetQueue(VkQueueFlags.VideoDecodeKHR).ToString());
            Console.WriteLine();

            Console.WriteLine("VideoEncode: " + queue.GetQueue(VkQueueFlags.VideoEncodeKHR).ToString());
            Console.WriteLine();



            indices = new QueueFamilyIndices();
            indices.GraphicsFamily = queue.GetFamilyIndex(VkQueueFlags.Graphics);

            indices.PresentFamily = Adapter.FindQueueFamilies(Adapter.gpu, Surface.surface).PresentFamily;

            //Console.WriteLine(indices.GraphicsFamily);
            //Console.WriteLine(indices.PresentFamily);


            var uniqueQueueFamilies = new[] { indices.GraphicsFamily!.Value, indices.PresentFamily!.Value };
            uniqueQueueFamilies = uniqueQueueFamilies.Distinct().ToArray();


            float* pri = stackalloc float[] { 1, 1 };


            bool new_queue = indices.GraphicsFamily!.Value != indices.PresentFamily!.Value;
            uint queue_count = 0;

            if (new_queue)
            {

            }

            //Console.WriteLine("queue_count: " + queue_count);

            VkDeviceQueueCreateInfo queue_create_info = new()
            {
                pNext = null,
                flags = VkDeviceQueueCreateFlags.None,
                queueFamilyIndex = uniqueQueueFamilies[0],
                queueCount = 1, // TODO: graphics, present ?
                pQueuePriorities = pri,
            };



            VkDeviceQueueCreateInfo present_queue_create_info = new()
            {
                pNext = null,
                queueFamilyIndex = uniqueQueueFamilies[0] + 2,
                queueCount = 1,
                //flags = VkDeviceQueueCreateFlags.Protected,
                pQueuePriorities = pri,
            };


            VkDeviceQueueCreateInfo* queue_create_infos = stackalloc VkDeviceQueueCreateInfo[]
            {
                queue_create_info,
                //new_queue ? present_queue_create_info : 0
            };



            VkPhysicalDeviceFeatures device_features = new()
            {
                
            };


            string[] layers =
            {
                "VK_KHR_swapchain",
                "VK_KHR_video_queue"
            };

            VkStringArray device_extensions = new(layers);
            VkStringArray validation_layers = new(layers);


            VkDeviceCreateInfo device_create_info = new()
            {
                //pNext = null,
                pQueueCreateInfos = queue_create_infos,
                queueCreateInfoCount = 1,
                pEnabledFeatures = &device_features,
                ppEnabledExtensionNames = device_extensions,
                enabledExtensionCount = 2, // TODO: swapchain
            };



            if (false)
            {
                //device_create_info.enabledLayerCount = (uint)validationLayers.Length;
                //device_create_info.ppEnabledLayerNames = (sbyte**)validation_layers.Pointer;
            }
            else
            {
                device_create_info.enabledLayerCount = 0;
            }




            if (vkCreateDevice(Adapter.gpu, &device_create_info, null, out device) != VK_SUCCESS)
                throw new Exception("failed to create logical device!");


            vkLoadDevice(device);

            //Console.WriteLine(device);
            vkGetDeviceQueue(device, indices.GraphicsFamily!.Value, 0, out graphics_queue);
            vkGetDeviceQueue(device, indices.PresentFamily!.Value, 0, out present_queue);

        }
    }
}
