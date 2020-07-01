/* <editor-fold desc="MIT License">

Copyright(c) 2020 Tim Moore

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include "ImageTranslator.h"

#include <vsg/core/Array2D.h>

using namespace vsgsandbox;

ImageTranslator::ImageTranslator(vsg::Device* device)
    : _device(device)
{
}

vsg::ref_ptr<vsg::Data>
ImageTranslator::translateToSupported(vsg::Data* texData)
{
    vsg::ref_ptr<vsg::Data> result;
    auto format = texData->getFormat();
    if (_device)
    {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(*_device->getPhysicalDevice(), texData->getFormat(),
                                            &formatProperties);
        if ((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) != 0)
        {
            result = texData;
            return result;
        }
    }

    if (format == VK_FORMAT_R8G8B8A8_SRGB)
    {
        result = texData;
        return result;
    }
    vsg::ref_ptr<vsg::ubvec4Array2D> array4 = vsg::ubvec4Array2D::create(texData->width(), texData->height());
    if (format == VK_FORMAT_R8G8B8_SRGB)
    {
        vsg::ubvec3Array2D* array3 = dynamic_cast<vsg::ubvec3Array2D*>(texData);
        array4->setFormat(VK_FORMAT_R8G8B8A8_SRGB);
        for (uint32_t j = 0; j < texData->height(); ++j)
        {
            for (uint32_t i = 0; i < texData->width(); ++i)
            {
                vsg::ubvec3 v3 = array3->at(i, j);
                array4->set(i, j, vsg::ubvec4{v3.r, v3.g, v3.b, 255});
            }
        }
    }
    else if (format == VK_FORMAT_R8_SRGB)
    {
        vsg::ubyteArray2D* array1 = dynamic_cast<vsg::ubyteArray2D*>(texData);
        array4->setFormat(VK_FORMAT_R8G8B8A8_SRGB);
        for (uint32_t j = 0; j < texData->height(); ++j)
        {
            for (uint32_t i = 0; i < texData->width(); ++i)
            {
                uint8_t val = array1->at(i, j);
                array4->set(i, j, vsg::ubvec4(val, val, val, 255));
            }
        }
    }
    return array4;
}
