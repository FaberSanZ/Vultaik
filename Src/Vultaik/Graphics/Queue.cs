// Copyright (c) Faber Leonardo. All Rights Reserved. https://github.com/FaberSanZ
// This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)



using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Vultaik;
using Vortice.Vulkan;
using static Vortice.Vulkan.Vulkan;
using System.Security.AccessControl;


namespace Vultaik.Graphics
{

    public unsafe class Queue
    {


        public Queue(Adapter adapter)
        {

            Adapter = adapter;
            Queues = new List<QueueData>();
        }
        public Adapter Adapter { get; }
        public List<QueueData> Queues { get; set; }

        public QueueData GetGraphicsQueue
        {
            get
            {
                foreach (QueueData queue in Queues)
                {
                    if (queue.Type == VkQueueFlags.Graphics)
                        return queue;
                }

                return new QueueData();
            }
        }


        public QueueData GetComputeQueue
        {
            get
            {
                foreach (QueueData queue in Queues)
                {
                    if (queue.Type == VkQueueFlags.Compute)
                        return queue;
                }

                return new QueueData();
            }
        }


        public QueueData GetTransferQueue
        {
            get
            {
                foreach (QueueData queue in Queues)
                {
                    if (queue.Type == VkQueueFlags.Transfer)
                        return queue;
                }

                return new QueueData();
            }
        }

        public uint GetFamilyIndex(VkQueueFlags type)
        {
            foreach (QueueData queue in Queues)
            {
                if(queue.Type == type)
                    return queue.FamilyIndex;
            }

            return uint.MaxValue;
        }


        public QueueData GetQueue(VkQueueFlags type)
        {
            foreach (QueueData queue in Queues)
            {
                if (queue.Type == type)
                    return queue;
            }

            return new QueueData();
        }



        public uint GetScore(VkQueueFlags type)
        {
            return scores[(int)type];
        }


        byte[] scores = new byte[16 * 16];



        public static uint FindActiveFlagIndex(VkQueueFlags allFlags, VkQueueFlags searchFlag)
        {
            var Flags = GetActiveFlags(allFlags);

            int index = Flags.IndexOf(searchFlag);


            return (uint)index;
        }

        public static List<VkQueueFlags> GetActiveFlags(VkQueueFlags allFlags)
        {
            List<VkQueueFlags> activeFlags = new List<VkQueueFlags>();

            foreach (VkQueueFlags flag in Enum.GetValues<VkQueueFlags>())
            {
                if (flag != VkQueueFlags.None && allFlags.HasFlag(flag))
                {
                    activeFlags.Add(flag);
                }
            }

            return activeFlags;
        }


        public unsafe void FillFamilyIndices(uint[]? enabledFamilyIndices, uint familyIndexNum, Surface? surface = null)
        {
            VkPhysicalDevice gpu = Adapter.gpu;
            uint family_count = 0;

            vkGetPhysicalDeviceQueueFamilyProperties(gpu, &family_count, null);

            VkQueueFamilyProperties[] familyProps = new VkQueueFamilyProperties[family_count];
            fixed (VkQueueFamilyProperties* queueFamiliesPtr = familyProps)
            {
                vkGetPhysicalDeviceQueueFamilyProperties(gpu, &family_count, queueFamiliesPtr);
            }




            for (uint i = 0; i < family_count; i++)
            {
                if (enabledFamilyIndices != null)
                {
                    bool isFamilyEnabled = false;
                    for (uint j = 0; j < familyIndexNum && !isFamilyEnabled; j++)
                    {
                        isFamilyEnabled = enabledFamilyIndices[j] == i;
                    }

                    if (!isFamilyEnabled)
                    {
                        continue;
                    }
                }

                var flags = familyProps[i].queueFlags;
                byte score;
                bool taken = false;

                bool graphics = (flags & VkQueueFlags.Graphics) != 0;
                bool compute = (flags & VkQueueFlags.Compute) != 0;
                bool copy = (flags & VkQueueFlags.Transfer) != 0;
                bool sparse = (flags & VkQueueFlags.SparseBinding) != 0;
                bool protect = (flags & VkQueueFlags.Protected) != 0;
                bool video_encode = (flags & VkQueueFlags.VideoEncodeKHR) != 0;
                bool video_decode = (flags & VkQueueFlags.VideoDecodeKHR) != 0;
                bool opticalFlow = (flags & VkQueueFlags.OpticalFlowNV) != 0;
                VkBool32 presentSupport = false;
                
                if(surface != null)
                {
                    vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface!._surface, &presentSupport);
                }
 


                score = (byte)((graphics ? 100 : 0) + (compute ? 10 : 0) + (copy ? 10 : 0) + (sparse ? 5 : 0) + (opticalFlow ? 2 : 0) + (video_decode ? 1 : 0) + (video_encode ? 1 : 0) + (protect ? 1 : 0));

                if (!taken && graphics && score > scores[(int)VkQueueFlags.Graphics])
                {
                    Queues.Add(new QueueData(VkQueueFlags.Graphics, i, FindActiveFlagIndex(flags, VkQueueFlags.Graphics), familyProps[i].timestampValidBits, familyProps[i].minImageTransferGranularity));
                    scores[(int)VkQueueFlags.Graphics] = score;
                    taken = true;

                }

                score = (byte)((!graphics ? 10 : 0) + (compute ? 100 : 0) + (!copy ? 10 : 0) + (sparse ? 5 : 0) + (opticalFlow ? 2 : 0) + (video_decode ? 1 : 0) + (video_encode ? 1 : 0) + (protect ? 1 : 0));
                if (!taken && compute && score > scores[(int)VkQueueFlags.Compute])
                {
                    Queues.Add(new QueueData(VkQueueFlags.Compute, i, FindActiveFlagIndex(flags, VkQueueFlags.Compute), familyProps[i].timestampValidBits, familyProps[i].minImageTransferGranularity));
                    scores[(int)VkQueueFlags.Compute] = score;
                    taken = true;
                }

                score = (byte)((!graphics ? 10 : 0) + (!compute ? 10 : 0) + (copy ? 100 : 0) + (sparse ? 5 : 0) + (opticalFlow ? 2 : 0) + (video_decode ? 1 : 0) + (video_encode ? 1 : 0) + (protect ? 1 : 0));
                if (!taken && copy && score > scores[(int)VkQueueFlags.Transfer])
                {
                    Queues.Add(new QueueData(VkQueueFlags.Transfer, i, FindActiveFlagIndex(flags, VkQueueFlags.Transfer), familyProps[i].timestampValidBits, familyProps[i].minImageTransferGranularity));
                    scores[(int)VkQueueFlags.Transfer] = score;
                    taken = true;
                }

                score = (byte)((sparse ? 100 : 0) + (opticalFlow ? 2 : 0) + (video_decode ? 1 : 0) + (video_encode ? 1 : 0) + (protect ? 1 : 0));
                if (!taken && sparse && score > scores[(int)VkQueueFlags.SparseBinding])
                {
                    Queues.Add(new QueueData(VkQueueFlags.SparseBinding, i, FindActiveFlagIndex(flags, VkQueueFlags.SparseBinding), familyProps[i].timestampValidBits, familyProps[i].minImageTransferGranularity));
                    scores[(int)VkQueueFlags.SparseBinding] = score;
                    taken = true;
                }




                // TODO: 
                if (video_decode)
                {
                    Queues.Add(new QueueData(VkQueueFlags.VideoDecodeKHR, i, FindActiveFlagIndex(flags, VkQueueFlags.VideoDecodeKHR), familyProps[i].timestampValidBits, familyProps[i].minImageTransferGranularity));
                    scores[(int)VkQueueFlags.VideoDecodeKHR] = score;
                    taken = true;
                }



                // TODO: 
                if (video_encode)
                {
                    Queues.Add(new QueueData(VkQueueFlags.VideoEncodeKHR, i, FindActiveFlagIndex(flags, VkQueueFlags.VideoEncodeKHR), familyProps[i].timestampValidBits, familyProps[i].minImageTransferGranularity));
                    scores[(int)VkQueueFlags.VideoEncodeKHR] = score;
                    taken = true;
                }



                //score = (byte)((protect ? 100 : 0));
                //if (protect)
                //{
                //    Queues.Add(new QueueData(QueueType.VideoEncodeKHR, i, FindActiveFlagIndex(flags, VkQueueFlags.VideoEncodeKHR), familyProps[i].timestampValidBits, familyProps[i].minImageTransferGranularity));

                //    scores[(int)QueueType.VideoEncodeKHR] = score;
                //    taken = true;
                //}
            }
        }

    }
}
