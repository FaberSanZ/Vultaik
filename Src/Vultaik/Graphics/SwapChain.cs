// Copyright (c) Faber Leonardo. All Rights Reserved. https://github.com/FaberSanZ
// This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)



using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Vultaik;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using System.Net.Mail;
using System.Reflection.Metadata;
using Silk.NET.Core.Native;

namespace Vultaik.Graphics
{
    public ref struct SwapChainSupportDetails
    {
        public VkSurfaceCapabilitiesKHR capabilities;
        public VkSurfaceFormatKHR[] formats;
        public VkPresentModeKHR[] presentModes;

        public bool IsComplete => formats.Length > 0 && presentModes.Length > 0;

        public unsafe SwapChainSupportDetails(VkPhysicalDevice device, VkSurfaceKHR surface)
        {
            formats = default;
            presentModes = default;
            capabilities = default;

            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, out capabilities);

            uint formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, null); // Count
            formats = new VkSurfaceFormatKHR[formatCount];
            fixed (VkSurfaceFormatKHR* formatsPtr = formats)
            {
                vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, formatsPtr);
            }

            uint presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, null); //Count 
            presentModes = new VkPresentModeKHR[presentModeCount];
            fixed (VkPresentModeKHR* presentsPtr = presentModes)
            {
                vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, presentsPtr);
            }



        }
    }


    public unsafe class SwapChain
    {

        public VkSwapchainKHR swapChain;
        public VkImage[]? images;
        public VkFormat swapChainImageFormat;
        public VkExtent2D swapChainExtent;


        public SwapChain(Device device)
        {
            Device = device;
            CreateSwapChain();
            CreateImageViews();

        }

        public Device Device { get; }
        public Image[] SwapChainImages { get; set; }
        public List<FramebufferAttachment> ColorAttachments { get; set; }


        private void CreateSwapChain()
        {
            var device = Device;
            SwapChainSupportDetails swapChainSupport = new SwapChainSupportDetails(Device.Adapter.gpu, Device.Surface!.surface);

            VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
            VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
            VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

            uint imageCount = swapChainSupport.capabilities.minImageCount + 1;
            if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
            {
                imageCount = Math.Min(imageCount, swapChainSupport.capabilities.maxImageCount);
            }
            imageCount = swapChainSupport.capabilities.maxImageCount - 1;


            VkSwapchainCreateInfoKHR createInfo = new VkSwapchainCreateInfoKHR()
            {
                //sType = VkStructureType.SwapchainCreateInfoKHR,
                surface = Device.Surface.surface,
                minImageCount = imageCount,
                imageFormat = surfaceFormat.format,
                imageColorSpace = surfaceFormat.colorSpace,
                imageExtent = extent,
                imageArrayLayers = 1,
                imageUsage = VkImageUsageFlags.ColorAttachment,
                preTransform = swapChainSupport.capabilities.currentTransform,
                compositeAlpha = VkCompositeAlphaFlagsKHR.Opaque,
                presentMode = presentMode,
                clipped = true,
            };

            //QueueFamilyIndices indices = new QueueFamilyIndices(physicalDevice, surface);

            uint* QueueFamilyIndicesPtr = stackalloc uint[]
            {
                (uint)device.indices.GraphicsFamily,
                (uint)device.indices.PresentFamily,
            };

            if (device.indices.GraphicsFamily != device.indices.PresentFamily) // diferent queue family
            {
                createInfo.imageSharingMode = VkSharingMode.Concurrent;
                createInfo.pQueueFamilyIndices = QueueFamilyIndicesPtr;
            }
            else
            {
                createInfo.imageSharingMode = VkSharingMode.Exclusive;
            }

            vkCreateSwapchainKHR(device.device, &createInfo, null, out swapChain);


            vkGetSwapchainImagesKHR(device.device, swapChain, &imageCount, null);
            images = new VkImage[imageCount];

            fixed (VkImage* img = images)
                vkGetSwapchainImagesKHR(device.device, swapChain, &imageCount, img);

            swapChainImageFormat = surfaceFormat.format;
            swapChainExtent = extent;
        }


        internal void CreateImageViews()
        {
            SwapChainImages = new Image[images.Length];

            for (int i = 0; i < SwapChainImages.Length; i++)
            {
                SwapChainImages[i] = new Image()
                {
                    image = images[i],
                };

                VkImageViewCreateInfo createInfo = new VkImageViewCreateInfo()
                {
                    //sType = VkStructureType.ImageViewCreateInfo,
                    image = images[i],
                    viewType = VkImageViewType.Image2D,
                    format = swapChainImageFormat,
                    components = new VkComponentMapping()
                    {
                        r = VkComponentSwizzle.Identity,
                        g = VkComponentSwizzle.Identity,
                        b = VkComponentSwizzle.Identity,
                        a = VkComponentSwizzle.Identity,
                    },
                    subresourceRange = new VkImageSubresourceRange()
                    {
                        aspectMask = VkImageAspectFlags.Color,
                        baseMipLevel = 0,
                        levelCount = 1,
                        baseArrayLayer = 0,
                        layerCount = 1
                    }
                };

                vkCreateImageView(Device.device, &createInfo, null, out SwapChainImages[i].view).CheckResult();
            }
        }





        private VkSurfaceFormatKHR ChooseSwapSurfaceFormat(VkSurfaceFormatKHR[] formats)
        {
            if (formats.Length == 1 && formats[0].format == VkFormat.Undefined)
            {
                return new VkSurfaceFormatKHR()
                {
                    format = VkFormat.B8G8R8A8Unorm,// 32 BITS BGRA
                    colorSpace = VkColorSpaceKHR.SrgbNonLinear
                };
            }

            foreach (VkSurfaceFormatKHR availableFormat in formats)
            {
                if (availableFormat.format == VkFormat.B8G8R8A8Unorm && availableFormat.colorSpace == VkColorSpaceKHR.SrgbNonLinear)
                {
                    return availableFormat;
                }
            }

            return formats[0];
        }

        private VkPresentModeKHR ChooseSwapPresentMode(VkPresentModeKHR[] presentModes)
        {
            //VkPresentModeKHR bestMode = VkPresentModeKHR.FifoKHR;

            foreach (VkPresentModeKHR availablePresentMode in presentModes)
            {
                if (availablePresentMode == VkPresentModeKHR.Mailbox)
                {
                    return availablePresentMode; // MailboxKHR
                }
                else if (availablePresentMode == VkPresentModeKHR.Immediate)
                {
                    return availablePresentMode; // ImmediateKHR;
                }
            }

            return VkPresentModeKHR.Immediate;
        }

        private VkExtent2D ChooseSwapExtent(VkSurfaceCapabilitiesKHR capabilities)
        {
            if (capabilities.currentExtent.width != int.MaxValue)
            {
                return capabilities.currentExtent;
            }

            return new VkExtent2D()
            {
                width = (uint)Math.Max(capabilities.minImageExtent.width, Math.Min(capabilities.maxImageExtent.width, 800)),
                height = (uint)Math.Max(capabilities.minImageExtent.height, Math.Min(capabilities.maxImageExtent.height, 600)),
            };
        }

        public uint AcquireNextImage()
        {
            // By setting timeout to UINT64_MAX we will always wait until the next image has been acquired or an actual error is thrown
            // With that we don't have to handle VK_NOT_READY
            vkAcquireNextImageKHR(Device.device, swapChain, ulong.MaxValue, Device.imageAvailableSemaphore, new VkFence(), out uint i);
            return i;
        }


        public void Present(uint imageIndex)
        {
            VkSemaphore semaphore = Device.renderFinishedSemaphore;
            VkSwapchainKHR _swapchain = swapChain;


            VkPresentInfoKHR present_info = new()
            {
                //sType = VkStructureType.PresentInfoKHR,
                //pNext = null,
                //pResults = null,

                waitSemaphoreCount = 1,
                pWaitSemaphores = &semaphore,
                swapchainCount = 1,
                pSwapchains = &_swapchain,
                pImageIndices = &imageIndex,
            };



            vkQueuePresentKHR(Device.present_queue, &present_info);
        }
    }
}
