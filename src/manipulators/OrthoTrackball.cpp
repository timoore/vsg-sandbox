#include "OrthoTrackball.h"

using namespace vsgSandbox;

OrthoTrackball::OrthoTrackball(vsg::ref_ptr<vsg::Camera> camera)
    : Inherit(camera)
{
    _ortho = dynamic_cast<vsg::Orthographic*>(_camera->getProjectionMatrix());
    if (_ortho)
    {
        _homeOrtho = vsg::Orthographic::create(*_ortho);
    }
}

void OrthoTrackball::apply(vsg::KeyPressEvent& keyPress)
{
    if (keyPress.keyBase == _homeKey)
    {
        *_ortho = *_homeOrtho;
    }
    Trackball::apply(keyPress);
}

void OrthoTrackball::apply(vsg::MoveEvent& moveEvent)
{
    if (moveEvent.mask & vsg::BUTTON_MASK_3)
    {
        vsg::dvec2 new_ndc = ndc(moveEvent);
        vsg::dvec3 new_tbc = tbc(moveEvent);
        vsg::dvec2 delta = new_ndc - prev_ndc;
        orthozoom(delta.y);
        prev_ndc = new_ndc;
        prev_tbc = new_tbc;
    }
    else
    {
        Trackball::apply(moveEvent);
    }
}

void OrthoTrackball::apply(vsg::ButtonPressEvent& buttonPress)
{
    prev_ndc = ndc(buttonPress);
    prev_tbc = tbc(buttonPress);

    if (buttonPress.button == 4)
    {
        orthozoom(-0.1);
    }
    else if (buttonPress.button == 5)
    {
        orthozoom(0.1);
    }
}

void OrthoTrackball::orthozoom(double ratio)
{
    if (_ortho)
    {
        double factor = 1.0 + ratio;
        _ortho->left *= factor;
        _ortho->right *= factor;
        _ortho->bottom *= factor;
        _ortho->top *= factor;
    }
}
