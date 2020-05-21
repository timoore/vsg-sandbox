#pragma once

#include <vsgsandbox/Export.h>
#include <vsg/io/ReaderWriter.h>

namespace vsgSandbox
{
    class VSGSANDBOX_DECLSPEC ReaderWriter_jpeg : public vsg::Inherit<vsg::ReaderWriter, ReaderWriter_jpeg>
    {
    public:
        ReaderWriter_jpeg();
        vsg::ref_ptr<vsg::Object> read(const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> options = {}) const override;
        vsg::ref_ptr<vsg::Object> read(std::istream& fin, vsg::ref_ptr<const vsg::Options> = {}) const override;
    };
}
