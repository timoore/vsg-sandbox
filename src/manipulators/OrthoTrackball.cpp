/* <editor-fold desc="MIT License">

Copyright(c) 2020 Tim Moore

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

</editor-fold> */

#include "OrthoTrackball.h"

using namespace vsgsandbox;

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
