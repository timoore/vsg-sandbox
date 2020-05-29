#pragma once

#include <vsgsandbox/Export.h>
#include <vsg/io/ReaderWriter.h>

namespace vsgSandbox
{
    class VSGSANDBOX_DECLSPEC EXIF : public vsg::Inherit<vsg::Object, EXIF>
    {
    public:
        // Positions of the "0th row and 0th column"" when the
        // picture was taken. TopLeft is the normal orientation.
        // Note that the jpeg reader actually writes out the bottom row first.
        // Another view of the image's orientation from
        // http://sylvana.net/jpegcrop/exif_orientation.html:
        /*
          Here is another description given by Adam M. Costello:

          For convenience, here is what the letter F would look like if it were tagged correctly
          and displayed by a program that ignores the orientation tag (thus showing the stored image): 
          1        2       3      4         5            6           7          8

          888888  888888      88  88      8888888888  88                  88  8888888888
          88          88      88  88      88  88      88  88          88  88      88  88
          8888      8888    8888  8888    88          8888888888  8888888888          88
          88          88      88  88
          88          88  888888  888888
        */
        enum Orientation
        {
            TopLeft = 1,
            TopRight,
            BottomRight,
            BottomLeft,
            LeftTop,
            RightTop,
            RightBottom,
            LeftBottom
        };
        EXIF(Orientation orient = TopLeft)
            : orientation(orient)
        {
        }
        Orientation orientation;
        // Getter / setter for use as VSG auxilliary data
        static vsg::ref_ptr<EXIF> get(vsg::Object* obj);
        static void set(vsg::Object* obj, EXIF* exif);
    };

    class VSGSANDBOX_DECLSPEC ReaderWriter_jpeg : public vsg::Inherit<vsg::ReaderWriter, ReaderWriter_jpeg>
    {
    public:
        ReaderWriter_jpeg();
        // Returns a vsg::Data object. EXIF data is stored in the
        // auxilliary object; retrieve with EXIF::get().
        // contains EXIF object.
        vsg::ref_ptr<vsg::Object> read(const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> options = {}) const override;
        vsg::ref_ptr<vsg::Object> read(std::istream& fin, vsg::ref_ptr<const vsg::Options> = {}) const override;
    };
}
