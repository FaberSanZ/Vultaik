// Copyright (c) Faber Leonardo. All Rights Reserved. https://github.com/FaberSanZ
// This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)



using Vortice.Vulkan;


namespace Vultaik.Graphics
{
    public class QueueData
    {
        public QueueData()
        {
            FamilyIndex = 0;
            QueueIndex = 0;
            TimestampValidBits = 0;
            MinImageTransferGranularity = new VkExtent3D();
        }

        public QueueData(VkQueueFlags type, uint familyIndex, uint queueIndex, uint timestampValidBits, VkExtent3D minImageTransferGranularity)
        {
            Type = type;
            FamilyIndex = familyIndex;
            QueueIndex = queueIndex;
            TimestampValidBits = timestampValidBits;
            MinImageTransferGranularity = minImageTransferGranularity;
        }
        public uint FamilyIndex { get; set; }
        public VkQueueFlags Type { get; set; }
        public uint QueueIndex { get; set; }
        public uint TimestampValidBits { get; set; }
        public VkExtent3D MinImageTransferGranularity { get; set; }


        public override string ToString()
        {
            return $"FamilyIndex: {FamilyIndex}, QueueIndex: {QueueIndex}, TimestampValidBits: {TimestampValidBits}, MinImageTransferGranularity: {MinImageTransferGranularity}";
        }
    }
}
