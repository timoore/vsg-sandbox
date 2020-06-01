#pragma once

#include <vsgsandbox/Export.h>
#include <vsg/io/ReaderWriter.h>

namespace vsgSandbox
{
    class VSGSANDBOX_DECLSPEC ReaderWriter_png : public vsg::Inherit<vsg::ReaderWriter, ReaderWriter_png>
    {
    public:
        ReaderWriter_png();
        // Returns a vsg::Data object.
        vsg::ref_ptr<vsg::Object> read(const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> options = {}) const override;
        vsg::ref_ptr<vsg::Object> read(std::istream& fin, vsg::ref_ptr<const vsg::Options> = {}) const override;
    };
}
