#include "ReaderWriter_image.h"
#include "jpeg/ReaderWriter_jpeg.h"
#include <png/ReaderWriter_png.h>

using namespace vsgSandbox;

ReaderWriter_image::ReaderWriter_image()
{
    add(ReaderWriter_jpeg::create());
    add(ReaderWriter_png::create());
}
