#pragma once

#include <vsg/core/Array2D.h>
// Some useful functions

namespace vsgSandbox
{
    template<typename EType>
    vsg::ref_ptr<vsg::Data> createArray(std::uint32_t width, std::uint32_t height, void* data,
                                        VkFormat format = VK_FORMAT_UNDEFINED)
    {
        auto result = vsg::Array2D<EType>::create(width, height, static_cast<EType*>(data));
        result->setFormat(format);
        return result;
    }
}
