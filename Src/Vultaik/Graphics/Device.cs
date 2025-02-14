﻿// Copyright (c) Faber Leonardo. All Rights Reserved. https://github.com/FaberSanZ
// This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)



using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Vultaik;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using System.Reflection.Metadata;

namespace Vultaik.Graphics
{
    public unsafe  class Device
    {

        internal VkDevice device;
        internal VkQueue graphics_queue;
        internal VkQueue present_queue;
        internal VkSemaphore image_semaphore;
        internal VkSemaphore render_semaphore;
        internal VkFence in_flight_fence;

        internal VkPhysicalDeviceVulkan11Features vk_1_1;
        internal VkPhysicalDeviceVulkan12Features vk_1_2;
        internal VkPhysicalDeviceVulkan13Features vk_1_3;
        internal VkPhysicalDeviceVulkan14Features vk_1_4;
        internal VkPhysicalDeviceDynamicRenderingFeatures physical_device_dynamic_rendering_features;


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


        public uint? QueueGraphicsFamily { get; set; }
        public uint? QueueComputeFamily { get; set; }
        public uint? QueueTransferFamily { get; set; }
        public uint? QueuePresentFamily { get; set; }


        private void createLogicalDevice()
        {


            if (Adapter.instance_version >= VkVersion.Version_1_3 && Adapter.api_version >= VkVersion.Version_1_3)
                Console.WriteLine("Vulkan 1.3 is supported");
            else
                throw new Exception("Vulkan 1.3 is not supported");



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

            QueuePresentFamily = Adapter.FindQueueFamilies(Adapter.gpu, Surface.surface).PresentFamily;

            QueueGraphicsFamily = queue.GetFamilyIndex(VkQueueFlags.Graphics);




            uint[] uniqueQueueFamilies = new[] { QueueGraphicsFamily!.Value, QueuePresentFamily!.Value };
            uniqueQueueFamilies = uniqueQueueFamilies.Distinct().ToArray();


            float* pri = stackalloc float[] { 1, 1 };


            bool new_queue = QueueGraphicsFamily!.Value != QueuePresentFamily!.Value;
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
                queueFamilyIndex = uniqueQueueFamilies[0],
                queueCount = 1,
                //flags = VkDeviceQueueCreateFlags.Protected,
                pQueuePriorities = pri,
            };


            VkDeviceQueueCreateInfo* queue_create_infos = stackalloc VkDeviceQueueCreateInfo[]
            {
                queue_create_info,
                //new_queue ? present_queue_create_info : 0
            };


            List<string> device_extensions_list = new();

            foreach (VkExtensionProperties item in vkEnumerateDeviceExtensionProperties(Adapter.gpu))
            {
                string name = VkStringInterop.ConvertToManaged(item.extensionName)!;

                if (name == "VK_KHR_swapchain")
                {
                    device_extensions_list.Add("VK_KHR_swapchain");
                    //Swapchain = true;
                }
            }
            VkStringArray device_extensions = new(device_extensions_list);




            vk_1_1 = new VkPhysicalDeviceVulkan11Features()
            {
                sType = VkStructureType.PhysicalDeviceVulkan11Features,
            };
            vk_1_2 = new VkPhysicalDeviceVulkan12Features()
            {
                sType = VkStructureType.PhysicalDeviceVulkan12Features,
            };
            vk_1_3 = new VkPhysicalDeviceVulkan13Features()
            {
                sType = VkStructureType.PhysicalDeviceVulkan13Features,
            };
            vk_1_4 = new VkPhysicalDeviceVulkan14Features()
            {
                sType = VkStructureType.PhysicalDeviceVulkan14Features,
            };

            VkPhysicalDeviceFeatures2 features = new()
            {
                sType = VkStructureType.PhysicalDeviceFeatures2,
            };


            void** ppNext = &features.pNext;


            if (Adapter.Vulka_1_1_Support)
            {
                fixed (VkPhysicalDeviceVulkan11Features* feature = &vk_1_1)
                {
                    *ppNext = feature;
                    ppNext = &feature->pNext;
                }
            }


            if (Adapter.Vulka_1_2_Support)
            {
                fixed (VkPhysicalDeviceVulkan12Features* feature = &vk_1_2)
                {
                    *ppNext = feature;
                    ppNext = &feature->pNext;
                }
            }


            if (Adapter.Vulka_1_3_Support)
            {
                fixed (VkPhysicalDeviceVulkan13Features* feature = &vk_1_3)
                {
                    *ppNext = feature;
                    ppNext = &feature->pNext;
                }
            }


            if (Adapter.Vulka_1_1_Support)
                vkGetPhysicalDeviceFeatures2(Adapter.gpu, &features);

            else if (Adapter.SupportsPhysicalDeviceProperties2)
                vkGetPhysicalDeviceFeatures2KHR(Adapter.gpu, &features);
            else
                vkGetPhysicalDeviceFeatures(Adapter.gpu, out features.features);


            if (!vk_1_2.descriptorIndexing)
                throw new Exception("DescriptorIndexing is not Supported - Update Vulkan Driver");


            if (!vk_1_3.dynamicRendering)
                throw new Exception("DynamicRendering is not Supported - Update Vulkan Driver");


            VkDeviceCreateInfo device_create_info = new()
            {
                pNext = &features,
                pQueueCreateInfos = queue_create_infos,
                queueCreateInfoCount = 1,
                pEnabledFeatures = null,
                ppEnabledExtensionNames = device_extensions,
                enabledExtensionCount = device_extensions.Length, // TODO: swapchain
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

            vkGetDeviceQueue(device, QueueGraphicsFamily!.Value, queue.GetGraphicsQueue.QueueIndex, out graphics_queue);
            vkGetDeviceQueue(device, QueuePresentFamily!.Value, 0, out present_queue);


            VkSemaphoreCreateInfo semaphoreInfo = new VkSemaphoreCreateInfo() { };
            //semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            VkFenceCreateInfo fenceInfo = new VkFenceCreateInfo() { };
            fenceInfo.sType = VkStructureType.FenceCreateInfo;
            fenceInfo.flags = VkFenceCreateFlags.Signaled;

            vkCreateSemaphore(device, &semaphoreInfo, null, out image_semaphore);
            vkCreateSemaphore(device, &semaphoreInfo, null, out render_semaphore);
            vkCreateFence(device, &fenceInfo, null, out in_flight_fence);


        }



        public void ResetFences()
        {
            vkWaitForFences(device, in_flight_fence, true, uint.MaxValue);
            vkResetFences(device, in_flight_fence);
        }

        public void Submit(CommandBuffer commandList)
        {

            VkSemaphore* waitSemaphores = stackalloc[] { image_semaphore };
            VkSemaphore* signalSemaphores = stackalloc[] { render_semaphore };
            VkPipelineStageFlags* waitStages = stackalloc[] { VkPipelineStageFlags.ColorAttachmentOutput };
            VkCommandBuffer* cmd = stackalloc[] { commandList.cmd };

            VkSubmitInfo submitInfo = new VkSubmitInfo()
            {
                signalSemaphoreCount = 1,
                pSignalSemaphores = signalSemaphores,

                waitSemaphoreCount = 1,
                pWaitSemaphores = waitSemaphores,

                pWaitDstStageMask = waitStages,


                commandBufferCount = 1,
                pCommandBuffers = cmd,
            };

            vkQueueSubmit(graphics_queue, 1, &submitInfo, in_flight_fence).CheckResult();
        }
    }
}
