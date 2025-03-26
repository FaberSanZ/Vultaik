// Copyright (c) Faber Leonardo. All Rights Reserved. https://github.com/FaberSanZ
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
    public unsafe  class Device : IDisposable
    {

        internal VkDevice _device;
        internal VkQueue _graphicsQueue;
        internal VkQueue _presentQueue;
        internal VkSemaphore _imageSemaphore;
        internal VkSemaphore _renderSemaphore;
        internal VkFence _inFlightFence;

        internal VkPhysicalDeviceVulkan11Features vk_1_1;
        internal VkPhysicalDeviceVulkan12Features vk_1_2;
        internal VkPhysicalDeviceVulkan13Features vk_1_3;
        internal VkPhysicalDeviceVulkan14Features vk_1_4;
        internal VkPhysicalDeviceDynamicRenderingFeatures _physicalDeviceDynamicRenderingFeatures;


        public Device(Adapter adapter)
        {
            Adapter = adapter;

            createLogicalDevice();
        }



        public Device(Surface surface)
        {
            Adapter = surface._adapter;
            Surface = surface;

            createLogicalDevice();
        }

        public Adapter Adapter { get; }
        public Surface? Surface { get; }


        public uint QueueGraphicsFamily { get; set; }
        public uint QueueComputeFamily { get; set; }
        public uint QueueTransferFamily { get; set; }
        public uint QueuePresentFamily { get; set; }


        private void createLogicalDevice()
        {



            if (Adapter.instance_version >= VkVersion.Version_1_3 && Adapter.api_version >= VkVersion.Version_1_3)
                Console.WriteLine("--Vulkan 1.3 is supported \n \n");
            else
                throw new Exception("Vulkan 1.3 is not supported");


            bool present = Surface != null;
            Queue queue = new Queue(Adapter);
            queue.FillFamilyIndices(null, 0);

            


            QueueGraphicsFamily = queue.GetFamilyIndex(VkQueueFlags.Graphics);
            QueueComputeFamily = queue.GetFamilyIndex(VkQueueFlags.Compute);
            QueueTransferFamily = queue.GetFamilyIndex(VkQueueFlags.Transfer);
            QueuePresentFamily = Adapter.FindQueueFamilies(Adapter.gpu, Surface!._surface).PresentFamily!.Value; // swapchain

            //TODO: Queue count
            VkDeviceQueueCreateInfo* queue_create_infos = stackalloc VkDeviceQueueCreateInfo[3];
            float default_queue_priority = 1.0f;
            float graphics_queue_prio = 0.5f;
            float transfer_queue_prio = 1.0f;
            float compute_queue_prio = 1.0f;
            uint queue_count = 0;

            if (QueueGraphicsFamily != uint.MinValue)
            {

                VkDeviceQueueCreateInfo queue_info = new()
                {
                    sType = VkStructureType.DeviceQueueCreateInfo,
                    queueFamilyIndex = QueueGraphicsFamily,
                    queueCount = 1,
                    pQueuePriorities = &default_queue_priority
                };

                queue_create_infos[0] = queue_info;
                queue_count++;
            }
            else
            {
                QueueGraphicsFamily = uint.MinValue;
            }

            // Dedicated compute queue
            if (QueueComputeFamily != QueueGraphicsFamily)
            {
                // If compute family index differs, we need an additional queue create info for the compute queue
                VkDeviceQueueCreateInfo queue_info = new()
                {
                    sType = VkStructureType.DeviceQueueCreateInfo,
                    queueFamilyIndex = QueueComputeFamily,
                    queueCount = 1,
                    pQueuePriorities = &default_queue_priority
                };

                queue_create_infos[1] = queue_info;
                queue_count++;
            }
            else
            {
                // Else we use the same queue
                QueueComputeFamily = QueueGraphicsFamily;
            }



            // Dedicated transfer queue
            if (QueueTransferFamily != QueueGraphicsFamily && QueueTransferFamily != QueueComputeFamily)
            {
                // If compute family index differs, we need an additional queue create info for the compute queue
                VkDeviceQueueCreateInfo queue_info = new()
                {
                    sType = VkStructureType.DeviceQueueCreateInfo,
                    queueFamilyIndex = QueueComputeFamily,
                    queueCount = 1,
                    pQueuePriorities = &default_queue_priority
                };

                queue_create_infos[2] = queue_info;
                queue_count++;
            }
            else
            {
                // Else we use the same queue
                QueueComputeFamily = QueueGraphicsFamily;
            }




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


            vkGetPhysicalDeviceFeatures2(Adapter.gpu, &features);



            if (!vk_1_2.descriptorIndexing)
                throw new Exception("DescriptorIndexing is not Supported - Update Vulkan Driver");


            if (!vk_1_3.dynamicRendering)
                throw new Exception("DynamicRendering is not Supported - Update Vulkan Driver");


            VkDeviceCreateInfo device_create_info = new()
            {
                pNext = &features,
                pQueueCreateInfos = queue_create_infos,
                queueCreateInfoCount = queue_count,
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




            if (vkCreateDevice(Adapter.gpu, &device_create_info, null, out _device) != VK_SUCCESS)
                throw new Exception("failed to create logical device!");


            vkLoadDevice(_device);

            vkGetDeviceQueue(_device, QueueGraphicsFamily, queue.GetGraphicsQueue.QueueIndex, out _graphicsQueue);
            vkGetDeviceQueue(_device, QueuePresentFamily, 0, out _presentQueue);


            VkSemaphoreCreateInfo semaphoreInfo = new VkSemaphoreCreateInfo()
            {
                sType = VkStructureType.SemaphoreCreateInfo,
                flags = VkSemaphoreCreateFlags.None,
            };
            vkCreateSemaphore(_device, &semaphoreInfo, null, out _imageSemaphore);
            vkCreateSemaphore(_device, &semaphoreInfo, null, out _renderSemaphore);


            VkFenceCreateInfo fenceInfo = new VkFenceCreateInfo()
            {
                sType = VkStructureType.FenceCreateInfo,
                flags = VkFenceCreateFlags.Signaled,
            };
            vkCreateFence(_device, &fenceInfo, null, out _inFlightFence);


        }



        public void ResetFences()
        {
            vkWaitForFences(_device, _inFlightFence, true, uint.MaxValue);
            vkResetFences(_device, _inFlightFence);
        }

        public void Submit(CommandBuffer commandList)
        {

            VkSemaphore* waitSemaphores = stackalloc[] { _imageSemaphore };
            VkSemaphore* signalSemaphores = stackalloc[] { _renderSemaphore };
            VkPipelineStageFlags* waitStages = stackalloc[] { VkPipelineStageFlags.ColorAttachmentOutput };
            VkCommandBuffer* cmd = stackalloc[] { commandList._commandBuffer };

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

            vkQueueSubmit(_graphicsQueue, 1, &submitInfo, _inFlightFence).CheckResult();
        }

        public void Dispose()
        {
            //TODO: Sync
            vkDestroySemaphore(_device, _imageSemaphore, null);
            vkDestroySemaphore(_device, _imageSemaphore, null);
            vkDestroyFence(_device, _inFlightFence, null);



            if (_device != VkDevice.Null)
                vkDestroyDevice(_device, null);
        }
    }
}
