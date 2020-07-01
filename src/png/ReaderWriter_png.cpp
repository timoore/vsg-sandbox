/* <editor-fold desc="MIT License">

Copyright(c) 2020 Tim Moore

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Based on the OSG PNG plugin, with  copyright:(c) Robert Osfield

</editor-fold> */

#include "ReaderWriter_png.h"
#include <vsgsandbox/Debug.h>
#include <vsgsandbox/Endian.h>
#include <vsgsandbox/Utils.h>

#include <sstream>
#include <fstream>

extern "C"
{
    #include <zlib.h>
    #include <png.h>
}

using namespace vsgSandbox;

/* Transparency parameters */
#define PNG_ALPHA     -2         /* Use alpha channel in PNG file, if there is one */
#define PNG_SOLID     -1         /* No transparency                                */
#define PNG_STENCIL    0         /* Sets alpha to 0 for r=g=b=0, 1 otherwise       */

typedef struct
{
    unsigned int Width;
    unsigned int Height;
    unsigned int Depth;
    unsigned int Alpha;
} pngInfo;

class PNGError
{
public:
    PNGError(const char* message)
    {
        _message = "PNG lib error : ";
        _message += message;
    }
    friend std::ostream& operator<<(std::ostream& stream, const PNGError& err)
    {
        stream << err._message;
        return stream;
    }
private:
    std::string _message;
};

void user_error_fn(png_structp /*png_ptr*/, png_const_charp error_msg)
{
    throw PNGError(error_msg);
}

void user_warning_fn(png_structp /*png_ptr*/, png_const_charp warning_msg)
{
    VSGSB_DEBUG << "PNG lib warning : " << warning_msg << std::endl;
}

void png_read_istream(png_structp png_ptr, png_bytep data, png_size_t length)
{
    std::istream *stream = (std::istream*)png_get_io_ptr(png_ptr); //Get pointer to istream
    stream->read((char*)data,length); //Read requested amount of data
}

void png_write_ostream(png_structp png_ptr, png_bytep data, png_size_t length)
{
    std::ostream *stream = (std::ostream*)png_get_io_ptr(png_ptr); //Get pointer to ostream
    stream->write((char*)data,length); //Write requested amount of data
}

void png_flush_ostream(png_structp png_ptr)
{
    std::ostream *stream = (std::ostream*)png_get_io_ptr(png_ptr); //Get pointer to ostream
    stream->flush();
}

#if 0
WriteResult::WriteStatus writePngStream(std::ostream& fout, const osg::Image& img, int compression_level) const
{
    png_structp png = NULL;
    png_infop   info = NULL;
    int color;
    int bitDepth;
    png_bytep *rows = NULL;

    //Create write structure
    png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png) return WriteResult::ERROR_IN_WRITING_FILE;

    //Create infr structure
    info = png_create_info_struct(png);
    if(!info) return WriteResult::ERROR_IN_WRITING_FILE;

    //Set custom write function so it will write to ostream
    png_set_write_fn(png,&fout,png_write_ostream,png_flush_ostream);

    //Set compression level
    png_set_compression_level(png, compression_level);

    switch(img.getPixelFormat()) {
    case(GL_DEPTH_COMPONENT): color = PNG_COLOR_TYPE_GRAY; break;
    case(GL_LUMINANCE): color = PNG_COLOR_TYPE_GRAY; break;
    case(GL_ALPHA): color = PNG_COLOR_TYPE_GRAY; break; //Couldn't find a color type for pure alpha, using gray instead
    case(GL_LUMINANCE_ALPHA): color = PNG_COLOR_TYPE_GRAY_ALPHA ; break;
    case(GL_RGB): color = PNG_COLOR_TYPE_RGB; break;
    case(GL_RGBA): color = PNG_COLOR_TYPE_RGB_ALPHA; break;
    case(GL_BGR): color = PNG_COLOR_TYPE_RGB; png_set_bgr(png); break;
    case(GL_BGRA): color = PNG_COLOR_TYPE_RGB_ALPHA; png_set_bgr(png); break;
    default: return WriteResult::ERROR_IN_WRITING_FILE; break;
    }

    //wish there was a Image::computeComponentSizeInBits()
    unsigned int numComponents = Image::computeNumComponents(img.getPixelFormat());
    bitDepth = (numComponents>0) ? (Image::computePixelSizeInBits(img.getPixelFormat(),img.getDataType())/numComponents) : 0;
    if(bitDepth!=8 && bitDepth!=16) return WriteResult::ERROR_IN_WRITING_FILE;

    //Create row data
    rows = new png_bytep[img.t()];
    for(int i = 0; i < img.t(); ++i) {
        rows[i] = (png_bytep)img.data(0,img.t() - i - 1);
    }

    //Write header info
    png_set_IHDR(png, info, img.s(), img.t(),
                 bitDepth, color, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);


    png_write_info(png, info);

    //must take place after png_write_info: png_set_swap verifies 16bit depth before setting transformation flag.
    if(bitDepth > 8 && getCpuByteOrder()==LittleEndian) png_set_swap(png);

    //Write data
    png_write_image(png, rows);

    //End write
    png_write_end(png, NULL);

    //Cleanup
    png_destroy_write_struct(&png,&info);
    delete [] rows;

    return WriteResult::FILE_SAVED;
}
#endif

vsg::ref_ptr<vsg::Object> readPNGStream(std::istream& fin)
{
    int trans = PNG_ALPHA;
    pngInfo pInfo;
    pngInfo *pinfo = &pInfo;

    unsigned char header[8];
    png_structp png;
    png_infop   info;
    png_infop   endinfo;
    png_bytep   data;    //, data2;
    png_bytep  *row_p;
    double  fileGamma;

    png_uint_32 width, height;
    int depth, color;

    png_uint_32 i;
    png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    // Set custom error handlers
    png_set_error_fn(png, png_get_error_ptr(png), user_error_fn, user_warning_fn);

    try
    {
        info = png_create_info_struct(png);
        endinfo = png_create_info_struct(png);

        fin.read((char*)header,8);
        if (fin.gcount() == 8 && png_sig_cmp(header, 0, 8) == 0)
            png_set_read_fn(png,&fin,png_read_istream); //Use custom read function that will get data from istream
        else
        {
            png_destroy_read_struct(&png, &info, &endinfo);
            return {};
        }
        png_set_sig_bytes(png, 8);

        png_read_info(png, info);
        png_get_IHDR(png, info, &width, &height, &depth, &color, NULL, NULL, NULL);

        if (pinfo != NULL)
        {
            pinfo->Width  = width;
            pinfo->Height = height;
            pinfo->Depth  = depth;
        }

        VSGSB_DEBUG<<"width="<<width<<" height="<<height<<" depth="<<depth<<std::endl;
        if ( color == PNG_COLOR_TYPE_RGB) { VSGSB_DEBUG << "color == PNG_COLOR_TYPE_RGB "<<std::endl; }
        if ( color == PNG_COLOR_TYPE_GRAY) { VSGSB_DEBUG << "color == PNG_COLOR_TYPE_GRAY "<<std::endl; }
        if ( color == PNG_COLOR_TYPE_GRAY_ALPHA) { VSGSB_DEBUG << "color ==  PNG_COLOR_TYPE_GRAY_ALPHA"<<std::endl; }

        // png default to big endian, so we'll need to swap bytes if on a little endian machine.
        if (depth>8 && !vsgSandbox::isHostBigEndian())
            png_set_swap(png);


        if (color == PNG_COLOR_TYPE_GRAY || color == PNG_COLOR_TYPE_GRAY_ALPHA)
        {
            //png_set_gray_to_rgb(png);
        }

        if (color&PNG_COLOR_MASK_ALPHA && trans != PNG_ALPHA)
        {
            png_set_strip_alpha(png);
            color &= ~PNG_COLOR_MASK_ALPHA;
        }



        //    if (!(PalettedTextures && mipmap >= 0 && trans == PNG_SOLID))
        //if (color == PNG_COLOR_TYPE_PALETTE)
        //    png_set_expand(png);

        // In addition to expanding the palette, we also need to check
        // to expand greyscale and alpha images.  See libpng man page.
        if (color == PNG_COLOR_TYPE_PALETTE)
            png_set_palette_to_rgb(png);
        if (color == PNG_COLOR_TYPE_GRAY && depth < 8)
        {
#if PNG_LIBPNG_VER >= 10209
            png_set_expand_gray_1_2_4_to_8(png);
#else
            // use older now deprecated but identical call
            png_set_gray_1_2_4_to_8(png);
#endif
        }
        if (png_get_valid(png, info, PNG_INFO_tRNS))
            png_set_tRNS_to_alpha(png);

        // Make sure that files of small depth are packed properly.
        if (depth < 8)
            png_set_packing(png);


        /*--GAMMA--*/
        //    checkForGammaEnv();
        // XXX Use this to decide whether or not to return an SRGB format
        double screenGamma = 2.2 / 1.0;
        if (png_get_gAMA(png, info, &fileGamma))
            png_set_gamma(png, screenGamma, fileGamma);
        else
            png_set_gamma(png, screenGamma, 1.0/2.2);

        png_read_update_info(png, info);

        data = (png_bytep) new unsigned char [png_get_rowbytes(png, info)*height];
        row_p = new png_bytep [height];

        bool StandardOrientation = true;
        for (i = 0; i < height; i++)
        {
            if (StandardOrientation)
                row_p[height - 1 - i] = &data[png_get_rowbytes(png, info)*i];
            else
                row_p[i] = &data[png_get_rowbytes(png, info)*i];
        }

        png_read_image(png, row_p);
        delete [] row_p;
        png_read_end(png, endinfo);

       
         // Some paletted images contain alpha information.  To be
        // able to give that back to the calling program, we need to
        // check the number of channels in the image.  However, the
        // call might not return correct information unless
        // png_read_end is called first.  See libpng man page.
        if ((color == PNG_COLOR_TYPE_RGB || color == PNG_COLOR_TYPE_PALETTE)
            && png_get_channels(png, info) == 4)
        {
            color = PNG_COLOR_TYPE_RGB_ALPHA;
        }

        vsg::ref_ptr<vsg::Data> result;
        if (depth <= 8)
        {
            switch(color)
            {
                // XXX Will PNG_SOLID and PNG_ALPHA be returned by libpng?
            case(PNG_SOLID):
                result = createArray<std::uint8_t>(width, height, data,
                                                   VK_FORMAT_R8_SRGB);
                break;
            case(PNG_ALPHA):
                // XXX tag as alpha
                result = createArray<std::uint8_t>(width, height, data,
                                                   VK_FORMAT_R8_UNORM);
                break; 
            case(PNG_COLOR_TYPE_GRAY):
                result = createArray<std::uint8_t>(width, height, data,
                                                   VK_FORMAT_R8_SRGB);
                break;
            case(PNG_COLOR_TYPE_GRAY_ALPHA):
                result = createArray<vsg::ubvec2>(width, height, data,
                                                  VK_FORMAT_R8G8_SRGB);
                break;
            case(PNG_COLOR_TYPE_RGB):
            case(PNG_COLOR_TYPE_PALETTE):
                result = createArray<vsg::ubvec3>(width, height, data,
                                                  VK_FORMAT_R8G8B8_SRGB);
                break;
            case(PNG_COLOR_TYPE_RGB_ALPHA):
                result = createArray<vsg::ubvec4>(width, height, data,
                                                  VK_FORMAT_R8G8B8A8_SRGB);
                break;
            default:
                break;
            }            
        }
        else
        {
            // 16 bit pixels
            // Not sure what to do with SRGB. Should we convert to
            // linear color before returning the image? Can we use
            // png_set_gamma() to do that?
            switch(color)
            {
            case(PNG_SOLID):
                result = createArray<std::uint16_t>(width, height, data,
                                                    VK_FORMAT_R16_UNORM);
                break;
            case(PNG_ALPHA):
                // XXX tag as alpha
                result = createArray<std::uint16_t>(width, height, data,
                                                    VK_FORMAT_R16_UNORM);
                break; 
            case(PNG_COLOR_TYPE_GRAY):
                result = createArray<std::uint16_t>(width, height, data,
                                                    VK_FORMAT_R16_UNORM);
                break;
            case(PNG_COLOR_TYPE_GRAY_ALPHA):
                result = createArray<vsg::ubvec2>(width, height, data,
                                                  VK_FORMAT_R16G16_UNORM);
                break;
            case(PNG_COLOR_TYPE_PALETTE):
            case(PNG_COLOR_TYPE_RGB):
                result = createArray<vsg::ubvec3>(width, height, data,
                                                  VK_FORMAT_R16G16B16_UNORM);
                break;
            case(PNG_COLOR_TYPE_RGB_ALPHA):
                result = createArray<vsg::ubvec4>(width, height, data,
                                                  VK_FORMAT_R16G16B16A16_UNORM);
                break;
            default:
                break;
            }
        }

        png_destroy_read_struct(&png, &info, &endinfo);

        //    delete [] data;

        if (result->getFormat() == VK_FORMAT_UNDEFINED)
            return {};
        return result;
    }
    catch (PNGError& err)
    {
        VSGSB_DEBUG << err << std::endl;
        png_destroy_read_struct(&png, &info, &endinfo);
        return {};
    }
}

#if 0
int getCompressionLevel(const osgDB::ReaderWriter::Options *options) const
{
    if(options) {
        std::istringstream iss(options->getOptionString());
        std::string opt;
        while (iss >> opt) {
            if(opt=="PNG_COMPRESSION") {
                int level;
                iss >> level;
                return level;
            }
        }
    }

    return Z_DEFAULT_COMPRESSION;
}
#endif

ReaderWriter_png::ReaderWriter_png()
{}

vsg::ref_ptr<vsg::Object> ReaderWriter_png::read(std::istream& fin,
                                                 const vsg::ref_ptr<const vsg::Options>) const
{
    return readPNGStream(fin);
}

vsg::ref_ptr<vsg::Object> ReaderWriter_png::read(const vsg::Path& filename,
                                                 const vsg::ref_ptr<const vsg::Options> options) const
{
    auto ext = vsg::fileExtension(filename);
    if (ext == "png")
    {
        vsg::Path filenameToUse = options ? findFile(filename, options) : filename;
        if (filenameToUse.empty()) return {};

        std::ifstream fin(filenameToUse, std::ios::in | std::ios::binary);
        if (!fin) return {};
        return readPNGStream(fin);
    }
    return {};
}

#if 0
virtual WriteResult writeImage(const osg::Image& img,std::ostream& fout,const osgDB::ReaderWriter::Options *options) const
{
    WriteResult::WriteStatus ws = writePngStream(fout,img,getCompressionLevel(options));
    return ws;
}

virtual WriteResult writeImage(const osg::Image &img,const std::string& fileName, const osgDB::ReaderWriter::Options *options) const
{
    std::string ext = osgDB::getFileExtension(fileName);
    if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

    osgDB::ofstream fout(fileName.c_str(), std::ios::out | std::ios::binary);
    if(!fout) return WriteResult::ERROR_IN_WRITING_FILE;

    return writeImage(img,fout,options);
}
#endif
