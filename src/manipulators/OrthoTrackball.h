#pragma once

/* <editor-fold desc="MIT License">

Copyright(c) 2020 Tim Moore

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include <vsgsandbox/Export.h>
#include <vsg/viewer/Trackball.h>

namespace vsgSandbox
{
    class VSGSANDBOX_DECLSPEC OrthoTrackball : public vsg::Inherit<vsg::Trackball, OrthoTrackball>
    {
    public:
        OrthoTrackball(vsg::ref_ptr<vsg::Camera> camera);

        void apply(vsg::KeyPressEvent& keyPress) override;
        void apply(vsg::MoveEvent& moveEvent) override;
        void apply(vsg::ButtonPressEvent& buttonPress) override;

        void orthozoom(double ratio);
    protected:
        vsg::ref_ptr<vsg::Orthographic> _ortho;
        vsg::ref_ptr<vsg::Orthographic> _homeOrtho;
    };
    
}
