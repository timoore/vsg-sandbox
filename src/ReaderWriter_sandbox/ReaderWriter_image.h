#pragma once

#include <vsg/io/ReaderWriter.h>
#include <vsgsandbox/Export.h>

namespace vsgSandbox
{
    class VSGSANDBOX_DECLSPEC ReaderWriter_image : public vsg::Inherit<vsg::CompositeReaderWriter, ReaderWriter_image>
    {
    public:
        ReaderWriter_image();
    };
}
