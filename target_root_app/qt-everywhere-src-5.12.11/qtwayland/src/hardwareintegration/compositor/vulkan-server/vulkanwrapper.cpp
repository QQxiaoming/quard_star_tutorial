/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

// NOTE: Some of the code below is adapted from the public domain code at https://vulkan-tutorial.com/

#define  GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <QtVulkanSupport/private/qvkconvenience_p.h>

#include "vulkanwrapper.h"

#include <QImage>
#include <QOpenGLContext>

#include <set>

#include <unistd.h>

#include <QDebug>

QT_BEGIN_NAMESPACE

static constexpr bool extraDebug = false;

#define DECL_VK_FUNCTION(name) \
    PFN_ ## name name = nullptr;

#define IMPL_VK_FUNCTION(name) \
    name = reinterpret_cast<PFN_ ## name>(f_glGetVkProcAddrNV(#name)); \
    if (!name) { \
        qCritical() << "ERROR in Vulkan proc lookup. Could not find " #name; \
    }

struct QueueFamilyIndices {
    int graphicsFamily = -1;
    int presentFamily = -1;

    bool isComplete() {
        return graphicsFamily >= 0 && presentFamily >= 0;
    }
};

class VulkanWrapperPrivate
{
public:
    explicit VulkanWrapperPrivate(QOpenGLContext *glContext);

    VulkanImageWrapper *createTextureImage(const QImage &img);
    VulkanImageWrapper *createTextureImageFromData(const uchar *pixels, uint bufferSize, const QSize &size, VkFormat vkFormat);

    void freeTextureImage(VulkanImageWrapper *imageWrapper);

private:
    DECL_VK_FUNCTION(vkAllocateCommandBuffers);
    DECL_VK_FUNCTION(vkAllocateMemory);
    DECL_VK_FUNCTION(vkBeginCommandBuffer);
    DECL_VK_FUNCTION(vkBindImageMemory);
    DECL_VK_FUNCTION(vkCmdCopyBufferToImage);
    DECL_VK_FUNCTION(vkCmdPipelineBarrier);
    DECL_VK_FUNCTION(vkCreateImage);
    DECL_VK_FUNCTION(vkDestroyImage);
    DECL_VK_FUNCTION(vkDestroyBuffer);
    DECL_VK_FUNCTION(vkEndCommandBuffer);
    DECL_VK_FUNCTION(vkFreeCommandBuffers);
    DECL_VK_FUNCTION(vkFreeMemory);
    DECL_VK_FUNCTION(vkGetImageMemoryRequirements);
    DECL_VK_FUNCTION(vkGetPhysicalDeviceMemoryProperties);
    DECL_VK_FUNCTION(vkMapMemory);
    DECL_VK_FUNCTION(vkQueueSubmit);
    DECL_VK_FUNCTION(vkQueueWaitIdle);
    DECL_VK_FUNCTION(vkUnmapMemory);
    DECL_VK_FUNCTION(vkCreateBuffer);
    DECL_VK_FUNCTION(vkGetBufferMemoryRequirements);
    DECL_VK_FUNCTION(vkBindBufferMemory);

    DECL_VK_FUNCTION(vkCreateInstance);
    DECL_VK_FUNCTION(vkEnumeratePhysicalDevices);
    DECL_VK_FUNCTION(vkGetPhysicalDeviceProperties);
    DECL_VK_FUNCTION(vkCreateDevice);
    DECL_VK_FUNCTION(vkGetPhysicalDeviceFormatProperties);

    DECL_VK_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties);
    DECL_VK_FUNCTION(vkCreateCommandPool);

    DECL_VK_FUNCTION(vkGetDeviceQueue);
    DECL_VK_FUNCTION(vkGetImageMemoryRequirements2KHR);
    DECL_VK_FUNCTION(vkGetMemoryFdKHR);

    //DECL_VK_FUNCTION(vkGetPhysicalDeviceSurfaceSupportKHR);

    void initFunctions(PFNGLGETVKPROCADDRNVPROC f_glGetVkProcAddrNV) {
        IMPL_VK_FUNCTION(vkAllocateCommandBuffers);
        IMPL_VK_FUNCTION(vkAllocateMemory);
        IMPL_VK_FUNCTION(vkBeginCommandBuffer);
        IMPL_VK_FUNCTION(vkBindImageMemory);
        IMPL_VK_FUNCTION(vkCmdCopyBufferToImage);
        IMPL_VK_FUNCTION(vkCmdPipelineBarrier);
        IMPL_VK_FUNCTION(vkCreateImage);
        IMPL_VK_FUNCTION(vkDestroyImage);
        IMPL_VK_FUNCTION(vkDestroyBuffer);
        IMPL_VK_FUNCTION(vkEndCommandBuffer);
        IMPL_VK_FUNCTION(vkFreeCommandBuffers);
        IMPL_VK_FUNCTION(vkFreeMemory);
        IMPL_VK_FUNCTION(vkGetImageMemoryRequirements);
        IMPL_VK_FUNCTION(vkGetPhysicalDeviceMemoryProperties);
        IMPL_VK_FUNCTION(vkMapMemory);
        IMPL_VK_FUNCTION(vkQueueSubmit);
        IMPL_VK_FUNCTION(vkQueueWaitIdle);
        IMPL_VK_FUNCTION(vkUnmapMemory);
        IMPL_VK_FUNCTION(vkCreateBuffer);
        IMPL_VK_FUNCTION(vkGetBufferMemoryRequirements);
        IMPL_VK_FUNCTION(vkBindBufferMemory);

        IMPL_VK_FUNCTION(vkCreateInstance);
        IMPL_VK_FUNCTION(vkEnumeratePhysicalDevices);
        IMPL_VK_FUNCTION(vkGetPhysicalDeviceProperties);
        IMPL_VK_FUNCTION(vkCreateDevice);
        IMPL_VK_FUNCTION(vkGetPhysicalDeviceFormatProperties);

        IMPL_VK_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties);
        IMPL_VK_FUNCTION(vkCreateCommandPool);

        IMPL_VK_FUNCTION(vkGetDeviceQueue);
        IMPL_VK_FUNCTION(vkGetImageMemoryRequirements2KHR);
        IMPL_VK_FUNCTION(vkGetMemoryFdKHR);

        //IMPL_VK_FUNCTION(vkGetPhysicalDeviceSurfaceSupportKHR);
    }

    int findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    VulkanImageWrapper *createImage(VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, const QSize &size, int memSize);
    bool transitionImageLayout(VkImage image, VkFormat /*format*/, VkImageLayout oldLayout, VkImageLayout newLayout);
    bool createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void createCommandPool();
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    bool createLogicalDevice();

private:
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkCommandPool m_commandPool = VK_NULL_HANDLE;

    VkQueue m_graphicsQueue = VK_NULL_HANDLE;

    bool m_initFailed = false;
};

struct VulkanImageWrapper
{
    VkImage textureImage = VK_NULL_HANDLE;
    int imgMemSize = -1;
    QSize imgSize;
    int imgFd = -1;
    VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
};

int VulkanWrapperPrivate::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    qCritical("VulkanWrapper: failed to find suitable memory type!");
    return -1;
}


VulkanImageWrapper *VulkanWrapperPrivate::createImage(VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, const QSize &size, int memSize)
{
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = size.width();
    imageInfo.extent.height = size.height();
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkImage image = VK_NULL_HANDLE;

    if (vkCreateImage(m_device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        qCritical("VulkanWrapper: failed to create image!");
        return nullptr;
    }

    QScopedPointer<VulkanImageWrapper> imageWrapper(new VulkanImageWrapper);
    imageWrapper->textureImage = image;
    imageWrapper->imgMemSize = memSize;
    imageWrapper->imgSize = size;

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_device, image, &memRequirements);

    VkExportMemoryAllocateInfoKHR exportAllocInfo = {};
    exportAllocInfo.sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_KHR;
    exportAllocInfo.handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    int memoryType = findMemoryType(memRequirements.memoryTypeBits, properties);
    if (memoryType < 0)
        return nullptr;
    allocInfo.memoryTypeIndex = memoryType;
    allocInfo.pNext = &exportAllocInfo;

    if (vkAllocateMemory(m_device, &allocInfo, nullptr, &imageWrapper->textureImageMemory) != VK_SUCCESS) {
        qCritical("VulkanWrapper: failed to allocate image memory!");
        return nullptr;
    }

    int res = vkBindImageMemory(m_device, image, imageWrapper->textureImageMemory, 0);
    Q_UNUSED(res);
    if (extraDebug) qDebug() << "vkBindImageMemory res" << res;

    VkMemoryGetFdInfoKHR memoryFdInfo = {};
    memoryFdInfo.sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;
    memoryFdInfo.memory = imageWrapper->textureImageMemory;
    memoryFdInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT_KHR;

    res = vkGetMemoryFdKHR(m_device, &memoryFdInfo, &imageWrapper->imgFd);
    if (extraDebug) qDebug() << "vkGetMemoryFdKHR res" << res << "fd" << imageWrapper->imgFd;

    return imageWrapper.take();
}


bool VulkanWrapperPrivate::transitionImageLayout(VkImage image, VkFormat /*format*/, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        qCritical("VulkanWrapper: unsupported layout transition!");
        return false;
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
        );

    endSingleTimeCommands(commandBuffer);
    return true;
}

bool VulkanWrapperPrivate::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        qCritical("VulkanWrapper: failed to create buffer!");
        return false;
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        qCritical("VulkanWrapper: failed to allocate buffer memory!");
        return false;
    }

    vkBindBufferMemory(m_device, buffer, bufferMemory, 0);
    return true;
}


VkCommandBuffer VulkanWrapperPrivate::beginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_commandPool;
    allocInfo.commandBufferCount = 1;

    if (extraDebug) qDebug() << "allocating...";

    VkCommandBuffer commandBuffer;
    int res = vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);
    Q_UNUSED(res);
    if (extraDebug) qDebug() << "vkAllocateCommandBuffers res" << res;

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    res = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    if (extraDebug) qDebug() << "BEGIN res" << res;

    return commandBuffer;
}

void VulkanWrapperPrivate::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    int res = vkEndCommandBuffer(commandBuffer);
    Q_UNUSED(res);
    if (extraDebug) qDebug() << "END res" << res;

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_graphicsQueue);

    vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
}

void VulkanWrapperPrivate::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    endSingleTimeCommands(commandBuffer);
}

void VulkanWrapperPrivate::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_physicalDevice);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

    if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
        m_initFailed = true;
        qCritical("VulkanWrapperPrivate: could not create command pool");
    }
}

QueueFamilyIndices VulkanWrapperPrivate::findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    if (extraDebug) qDebug() << "queueFamilyCount" << queueFamilyCount;


    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

#ifdef VULKAN_SERVER_BUFFER_EXTRA_DEBUG
    for (const auto& queueFamily : queueFamilies) {
        qDebug() << "....q" << "count" << queueFamily.queueCount << queueFamily.timestampValidBits  << hex << queueFamily.queueFlags;
    }
#endif

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
            break;
        }
        i++;
    }

    return indices;
}

bool VulkanWrapperPrivate::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<int> uniqueQueueFamilies = {indices.graphicsFamily}; //////, indices.presentFamily};

    float queuePriority = 1.0f;
    for (int queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;

    if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
        qCritical("VulkanWrapper: failed to create logical device!");
        return false;
    }

    vkGetDeviceQueue(m_device, indices.graphicsFamily, 0, &m_graphicsQueue);
    return true;
}

VulkanImageWrapper *VulkanWrapperPrivate::createTextureImage(const QImage &img)
{
    return createTextureImageFromData(img.constBits(), img.sizeInBytes(), img.size(), VK_FORMAT_R8G8B8A8_UNORM);
}

VulkanImageWrapper *VulkanWrapperPrivate::createTextureImageFromData(const uchar *pixels, uint bufferSize, const QSize &size, VkFormat vkFormat)
{
    if (m_initFailed)
        return nullptr;

    int texWidth = size.width();
    int texHeight = size.height();
    bool ok;
    if (extraDebug) qDebug("image load %p %dx%d", pixels, texWidth, texHeight);
    if (!pixels) {
        qCritical("VulkanWrapper: failed to load texture image!");
        return nullptr;
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    ok = createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    if (!ok)
        return nullptr;

    void* data;
    vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
    if (extraDebug) qDebug() << "mapped" << data << bufferSize;
    memcpy(data, pixels, static_cast<size_t>(bufferSize));
    vkUnmapMemory(m_device, stagingBufferMemory);

    if (extraDebug) qDebug() << "creating image...";

    QScopedPointer<VulkanImageWrapper> imageWrapper(createImage(vkFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, size, bufferSize));
    if (imageWrapper.isNull())
        return nullptr;

    if (extraDebug) qDebug() << "transition...";

    const VkImage textureImage = imageWrapper->textureImage;

    ok = transitionImageLayout(textureImage, vkFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    if (!ok)
        return nullptr;

    if (extraDebug) qDebug() << "copyBufferToImage...";
    copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    transitionImageLayout(textureImage, vkFormat, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(m_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device, stagingBufferMemory, nullptr);

    return imageWrapper.take();
}

void VulkanWrapperPrivate::freeTextureImage(VulkanImageWrapper *imageWrapper)
{
    if (!imageWrapper)
        return;

    //"To avoid leaking resources, the application must release ownership of the file descriptor using the close system call"
    ::close(imageWrapper->imgFd);

    // clean up the image memory
    vkDestroyImage(m_device, imageWrapper->textureImage, nullptr);
    vkFreeMemory(m_device, imageWrapper->textureImageMemory, nullptr);
}

VulkanWrapperPrivate::VulkanWrapperPrivate(QOpenGLContext *glContext)
{
    if (extraDebug) qDebug("Creating Vulkan instance");
    VkApplicationInfo applicationInfo = {};
    applicationInfo.sType               = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pNext               = nullptr;
    applicationInfo.pApplicationName    = nullptr;
    applicationInfo.applicationVersion  = 0;
    applicationInfo.pEngineName         = nullptr;
    applicationInfo.engineVersion       = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.apiVersion          = VK_MAKE_VERSION(1, 0, 5);

    VkInstanceCreateInfo instanceCreateInfo = {};
    instanceCreateInfo.sType                    = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pNext                    = nullptr;
    instanceCreateInfo.flags                    = 0;
    instanceCreateInfo.pApplicationInfo         = &applicationInfo;
    instanceCreateInfo.enabledLayerCount        = 0;
    instanceCreateInfo.ppEnabledLayerNames      = nullptr;
    instanceCreateInfo.enabledExtensionCount    = 0;
    instanceCreateInfo.ppEnabledExtensionNames  = nullptr;

    auto f_glGetVkProcAddrNV = reinterpret_cast<PFNGLGETVKPROCADDRNVPROC>(glContext->getProcAddress("glGetVkProcAddrNV"));

    if (!f_glGetVkProcAddrNV) {
        qCritical("VulkanWrapper: Could not find Vulkan/GL interop function glGetVkProcAddrNV");
        m_initFailed = true;
        return;
    }

    initFunctions(f_glGetVkProcAddrNV);

    VkResult   instanceCreationResult = vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance);

    if (extraDebug) qDebug() << "result" << instanceCreationResult;

    if (instanceCreationResult != VK_SUCCESS) {
        qCritical() << "VulkanWrapper: Failed to create Vulkan instance: Error "
                 << instanceCreationResult;
        m_initFailed = true;
        return;
    }

    uint32_t devCount;

    auto res = vkEnumeratePhysicalDevices(m_instance, &devCount, nullptr);
    if (extraDebug) qDebug() << "vkEnumeratePhysicalDevices res =" << res << "count =" << devCount;

    QVarLengthArray<VkPhysicalDevice, 5> dev(devCount);

    res = vkEnumeratePhysicalDevices(m_instance, &devCount, dev.data());
    if (extraDebug) qDebug() << "...devs res =" << res << "count =" << devCount;

#ifdef VULKAN_SERVER_BUFFER_EXTRA_DEBUG
    VkPhysicalDeviceProperties props;

    vkGetPhysicalDeviceProperties(dev[0], &props);

    qDebug() << "Properties " << hex
             << "apiVersion" << props.apiVersion
             << "driverVersion" << props.driverVersion
             << "vendorID" << props.vendorID
             << "deviceID" << props.deviceID
             << "deviceType" << props.deviceType
             << "deviceName" << props.deviceName;
#endif

    m_physicalDevice = dev[0]; //TODO handle the case of multiple GPUs where only some support Vulkan

    bool ok = createLogicalDevice();
    if (!ok) {
        qCritical("VulkanWrapperPrivate: could not create logical device");
        m_initFailed = true;
        return;
    }

    VkPhysicalDeviceMemoryProperties memProps;


    vkGetPhysicalDeviceMemoryProperties(dev[0], &memProps);

#ifdef VULKAN_SERVER_BUFFER_EXTRA_DEBUG
    qDebug() << "Physical memory properties:\n" << "types:" << memProps.memoryTypeCount << "heaps:" << memProps.memoryHeapCount;
    for (uint i = 0; i < memProps.memoryTypeCount; ++i)
        qDebug() << "   " << i << "heap" << memProps.memoryTypes[i].heapIndex << "flags" << hex << memProps.memoryTypes[i].propertyFlags;

    for (uint i = 0; i < memProps.memoryHeapCount; ++i)
        qDebug() << "   " << i << "size" << memProps.memoryHeaps[i].size << "flags" << hex << memProps.memoryHeaps[i].flags;
#endif

    int gpuMemoryType = -1;

    for (uint i = 0; i < memProps.memoryTypeCount; ++i) {
        if (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
            gpuMemoryType = i;
            break;
        }
    }

    if (gpuMemoryType < 0) {
        qCritical("VulkanWrapper: Could not find GPU memory!");
        m_initFailed = true;
        return;
    }

#ifdef VULKAN_SERVER_BUFFER_EXTRA_DEBUG
    qDebug() << "GPU memory type:" << gpuMemoryType << "heap:" << memProps.memoryTypes[gpuMemoryType].heapIndex;

    for (int f = 0; f <= VK_FORMAT_ASTC_12x12_SRGB_BLOCK; f++)
    {
        VkFormatProperties formatProps;
        vkGetPhysicalDeviceFormatProperties(dev[0], VkFormat(f), &formatProps);
        qDebug() << "format" << f << "features" << hex << formatProps.linearTilingFeatures << formatProps.optimalTilingFeatures << formatProps.bufferFeatures;
    }
#endif
    createCommandPool();
}


VulkanWrapper::VulkanWrapper(QOpenGLContext *glContext)
    : d_ptr(new VulkanWrapperPrivate(glContext))
{
}

VulkanImageWrapper *VulkanWrapper::createTextureImage(const QImage &img)
{
    return d_ptr->createTextureImage(img);
}

VulkanImageWrapper *VulkanWrapper::createTextureImageFromData(const uchar *pixels, uint bufferSize, const QSize &size, uint glInternalFormat)
{
    VkFormat vkFormat = VkFormat(QVkConvenience::vkFormatFromGlFormat(glInternalFormat));

    if (vkFormat == VK_FORMAT_UNDEFINED)
        return nullptr;

    return d_ptr->createTextureImageFromData(pixels, bufferSize, size, vkFormat);
}

int VulkanWrapper::getImageInfo(const VulkanImageWrapper *imgWrapper, int *memSize, int *w, int *h)
{
    if (memSize)
        *memSize = imgWrapper->imgMemSize;
    if (w)
        *w = imgWrapper->imgSize.width();
    if (h)
        *h = imgWrapper->imgSize.height();
    return imgWrapper->imgFd;
}

void VulkanWrapper::freeTextureImage(VulkanImageWrapper *imageWrapper)
{
    d_ptr->freeTextureImage(imageWrapper);
}

QT_END_NAMESPACE
