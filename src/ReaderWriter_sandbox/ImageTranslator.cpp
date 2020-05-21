#include "ImageTranslator.h"

#include <vsg/core/Array2D.h>

using namespace vsgSandbox;

vsg::ref_ptr<vsg::Data>
ImageTranslator::translateToSupported(vsg::Data* texData)
{
    vsg::ref_ptr<vsg::Data> result;
    auto format = texData->getFormat();
    if (format == VK_FORMAT_R8G8B8A8_SRGB)
    {
        result = texData;
        return result;
    }
    if (format != VK_FORMAT_R8G8B8_SRGB)
    {
        return result;
    }
    vsg::ubvec3Array2D* array3 = dynamic_cast<vsg::ubvec3Array2D*>(texData);
    vsg::ref_ptr<vsg::ubvec4Array2D> array4 = vsg::ubvec4Array2D::create(texData->width(), texData->height());
    array4->setFormat(VK_FORMAT_R8G8B8A8_SRGB);
    for (uint32_t j = 0; j < texData->height(); ++j)
    {
        for (uint32_t i = 0; i < texData->width(); ++i)
        {
            vsg::ubvec3 v3 = array3->at(i, j);
            array4->set(i, j, vsg::ubvec4{v3.r, v3.g, v3.b, 255});
        }
    }
    return array4;
}
