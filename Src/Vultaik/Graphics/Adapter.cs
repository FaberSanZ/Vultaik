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

    public struct PresenQueueInfo
    {
        public uint? PresentFamily { get; set; }
        public bool IsComplete()
        {
            return PresentFamily.HasValue;
        }
    }

    public unsafe class Adapter
    {
        internal VkInstance instance;
        internal VkPhysicalDevice gpu;
        private bool enable_validation_layers;
        private VkDebugUtilsMessengerEXT debug_messenger = VkDebugUtilsMessengerEXT.Null;
        private string[] validation_layers = ["VK_LAYER_KHRONOS_validation"];
        internal VkVersion api_version;

        //private string name = string.Empty;
        private VkPhysicalDeviceType deviceType;


        public Adapter(bool debug = false)
        {
            enable_validation_layers = debug;
            CreateInstance();
            get_gpu();
        }

        public bool Vulka_1_0_Support = false;
        public bool Vulka_1_1_Support = false;
        public bool Vulka_1_2_Support = false;
        public bool Vulka_1_3_Support = false;
        public bool Vulka_1_4_Support = false;
        public bool SupportsPhysicalDeviceProperties2 = false;



        private void CreateInstance()
        {

            VkResult result = vkInitialize();
            if (result != VkResult.Success) // CheckIsSupported
                throw new Exception("Vulkan is not supported");


            if (enable_validation_layers && !CheckValidationLayerSupport())
                throw new Exception("validation layers requested, but not available!");


            var ins_api_version = vkEnumerateInstanceVersion();


            VkApplicationInfo appInfo = new VkApplicationInfo()
            {
                // sType = VkStructureType.ApplicationInfo,
                pNext = null,
                pApplicationName = (byte*)Marshal.StringToHGlobalAnsi("Hello Triangle"),
                applicationVersion = new VkVersion(0, 0, 1),
                pEngineName = (byte*)Marshal.StringToHGlobalAnsi("No Engine"),
                engineVersion = new VkVersion(1, 0, 0),
                apiVersion = ins_api_version,
            };



            IEnumerable<string> extensions = supports_extensions();
            VkStringArray instance_extensions = new VkStringArray(extensions.ToArray());


            VkInstanceCreateInfo createInfo = new VkInstanceCreateInfo()
            {
                // sType = VkStructureType.InstanceCreateInfo,
                pApplicationInfo = &appInfo,
                ppEnabledExtensionNames = (byte**)instance_extensions,
                enabledExtensionCount = (uint)extensions.Count(),
            };



            if (enable_validation_layers)
            {
                createInfo.enabledLayerCount = (uint)validation_layers.Length;
                createInfo.ppEnabledLayerNames = (byte**)new VkStringArray(validation_layers);

                VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = new();
                PopulateDebugMessengerCreateInfo(ref debugCreateInfo);
                createInfo.pNext = &debugCreateInfo;
            }
            else
            {
                createInfo.enabledLayerCount = 0;
                createInfo.pNext = null;
            }

            if (vkCreateInstance(&createInfo, null, out instance) != VkResult.Success)
                throw new Exception("failed to create instance!");


            vkLoadInstanceOnly(instance);
        }


        [UnmanagedCallersOnly]
        private static uint DebugCallback(VkDebugUtilsMessageSeverityFlagsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* userData)
        {

            //uint[] ignored_ids = new[]
            //{
            //    0xad0e15f6,
            //};

            //for (int i = 0; i < ignored_ids.Length; i++)
            //    if ((uint)pCallbackData->messageIdNumber == ignored_ids[i])
            //        return VK_FALSE;



            string message = new((char*)pCallbackData->pMessage);

            Console.WriteLine($"validation layer:" + message);

            return VK_FALSE;
        }

        private void SetupDebugMessenger()
        {
            if (!enable_validation_layers)
                return;

            VkDebugUtilsMessengerCreateInfoEXT createInfo = new();
            PopulateDebugMessengerCreateInfo(ref createInfo);

            if (vkCreateDebugUtilsMessengerEXT(instance, &createInfo, null, out debug_messenger) != VkResult.Success)
                throw new Exception("failed to set up debug messenger!");
        }

        private void get_gpu()
        {
            uint deviceCount = 0;
            vkEnumeratePhysicalDevices(instance, &deviceCount, null);

            if (deviceCount is 0)
                throw new Exception("failed to find GPUs with Vulkan support!");

            VkPhysicalDevice* gpus = stackalloc VkPhysicalDevice[(int)deviceCount];
            vkEnumeratePhysicalDevices(instance, &deviceCount, gpus);


            uint max_score = 0;
            // Prefer earlier entries in list.
            for (uint i = deviceCount; i > 0; i--)
            {
                uint score = device_score(gpus[i - 1], false);
                if (score >= max_score) /*&& physical_device_supports_surface_and_profile(gpus[i - 1], surface)*/
                {
                    max_score = score;
                    gpu = gpus[i - 1];
                }
            }
        }




        public uint device_score(VkPhysicalDevice gpu, bool force_integrated_gpu)
        {
            vkGetPhysicalDeviceProperties(gpu, out VkPhysicalDeviceProperties props);


            api_version = props.apiVersion;

            if (api_version >= VkVersion.Version_1_0)
                Vulka_1_0_Support = true;

            if (api_version >= VkVersion.Version_1_1)
                Vulka_1_1_Support = true;

            if (api_version >= VkVersion.Version_1_2)
                Vulka_1_2_Support = true;

            if (api_version >= VkVersion.Version_1_3)
                Vulka_1_3_Support = true;

            if (api_version >= VkVersion.Version_1_4)
                Vulka_1_4_Support = true;


            if (force_integrated_gpu && props.deviceType is VkPhysicalDeviceType.IntegratedGpu)
                return 4;

            switch (props.deviceType)
            {
                case VkPhysicalDeviceType.DiscreteGpu:
                    return 3;
                case VkPhysicalDeviceType.IntegratedGpu:
                    return 2;
                case VkPhysicalDeviceType.Cpu:
                    return 1;
                default:
                    return 0;
            }
        }


        private bool CheckValidationLayerSupport()
        {
            ReadOnlySpan<VkLayerProperties> availableLayers = vkEnumerateInstanceLayerProperties();

            foreach (var layer in availableLayers)

                if ("VK_LAYER_KHRONOS_validation" == VkStringInterop.ConvertToManaged(layer.layerName))
                    return true;

            return false;
        }


        private unsafe ReadOnlySpan<VkExtensionProperties> EnumerateInstanceExtension()
        {
            uint count = 0;
            vkEnumerateInstanceExtensionProperties(null, &count, null).CheckResult();

            ReadOnlySpan<VkExtensionProperties> properties = new VkExtensionProperties[count];
            fixed (VkExtensionProperties* ptr = properties)
            {
                vkEnumerateInstanceExtensionProperties(null, &count, ptr).CheckResult();
            }

            return properties;
        }


        private IEnumerable<string> supports_extensions()
        {
            IEnumerable<string> instance_extensions_names = EnumerateInstanceExtension()
                                                    .ToArray()
                                                    .Select(_ => VkStringInterop.ConvertToManaged(_.extensionName)!);
            
            List<string> InstanceExtensionsNames = new();


            if (InstanceExtensionsNames.Contains("VK_KHR_get_physical_device_properties2"))
            {
                InstanceExtensionsNames.Add("VK_KHR_get_physical_device_properties2");
                SupportsPhysicalDeviceProperties2 = true;
            }


            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
                if (instance_extensions_names.Contains("VK_KHR_win32_surface"))
                    InstanceExtensionsNames.Add("VK_KHR_win32_surface");


            if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
                if (instance_extensions_names.Contains("VK_MVK_macos_surface"))
                    InstanceExtensionsNames.Add("VK_MVK_macos_surface");

                if (instance_extensions_names.Contains("VK_MVK_ios_surface"))
                    InstanceExtensionsNames.Add("VK_MVK_ios_surface");
            }

            if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
            {
                if (instance_extensions_names.Contains("VK_KHR_android_surface"))
                    InstanceExtensionsNames.Add("VK_KHR_android_surface");


                if (instance_extensions_names.Contains("VK_KHR_xlib_surface"))
                    InstanceExtensionsNames.Add("VK_KHR_xlib_surface");


                if (instance_extensions_names.Contains("VK_KHR_wayland_surface"))
                    InstanceExtensionsNames.Add("VK_KHR_wayland_surface");

            }

            if (instance_extensions_names.Contains("VK_KHR_surface"))
                InstanceExtensionsNames.Add("VK_KHR_surface");


            if (enable_validation_layers && instance_extensions_names.Contains("VK_EXT_debug_utils")) ;
            {
                InstanceExtensionsNames.Add("VK_EXT_debug_utils");
            }



                //InstanceExtensionsNames.Add("VK_KHR_video_queue");

            return InstanceExtensionsNames;
        }




        private void PopulateDebugMessengerCreateInfo(ref VkDebugUtilsMessengerCreateInfoEXT createInfo)
        {
            //createInfo.SType = StructureType.DebugUtilsMessengerCreateInfoExt;
            createInfo.messageSeverity = VkDebugUtilsMessageSeverityFlagsEXT.Verbose |
                                         VkDebugUtilsMessageSeverityFlagsEXT.Warning |
                                         VkDebugUtilsMessageSeverityFlagsEXT.Error;
            createInfo.messageType = /*VkDebugUtilsMessageTypeFlagsEXT.General |*/
                                     VkDebugUtilsMessageTypeFlagsEXT.Validation |
                                     VkDebugUtilsMessageTypeFlagsEXT.Performance;

            createInfo.pfnUserCallback = &DebugCallback;
        }





        public PresenQueueInfo FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR? surface = null)
        {

            PresenQueueInfo indices = new PresenQueueInfo();
            uint queueFamilityCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilityCount, null);

            VkQueueFamilyProperties[] queueFamilies = new VkQueueFamilyProperties[queueFamilityCount];
            fixed (VkQueueFamilyProperties* queueFamiliesPtr = queueFamilies)
            {
                vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilityCount, queueFamiliesPtr);
            }



            uint i = 0;
            foreach (var queueFamily in queueFamilies)
            {
                //if (queueFamily.queueFlags.HasFlag(VkQueueFlags.Graphics))
                //{
                //    indices.GraphicsFamily = i;
                //}

                if (surface != null)
                {
                    VkBool32 presentSupport = false;
                    vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface!.Value, &presentSupport);

                    if (presentSupport)
                    {
                        indices.PresentFamily = i;
                    }
                }
   


                if (indices.IsComplete())
                {
                    break;
                }

                i++;
            }

            return indices;
        }



    }
}
