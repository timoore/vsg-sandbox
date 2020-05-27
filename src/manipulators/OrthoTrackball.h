#pragma once

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
