#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_ENABLE_DRACO
#define TINYGLTF_USE_RAPIDJSON
#define TINYGLTF_USE_RAPIDJSON_CRTALLOCATOR

#include "ReaderWriter_gltf.h"

#include "tinygltf/tinygltf.h"

#include <vsgsandbox/jpeg/ReaderWriter_jpeg.h>
#include <vsgsandbox/png/ReaderWriter_png.h>
#include <vsg/core/Object.h>
#include <vsg/core/Inherit.h>
#include <algorithm>
#include <vector>
#include <tuple>

using namespace vsgsandbox;

/*
  Notes
  glTF Buffer and BufferView have overlapping semantics with Vulkan
  buffers, vertex buffer bindings, and vertex input state:
  VkBuffer: usage flag
  vertex buffer binding: offset into buffer
  vertex input state: stride

  Accessors: VertexInputState + binding
  meshes: draw commands: BindVertexBuffers,
  BindIndexBuffer,DrawIndexed
  nodes: MatrixTransform
 */

// Group gltf BufferView objects into vk buffers / vsg::Data objects
// XXX Maybe later
struct BufferedZone : public vsg::Inherit<vsg::Object, BufferedZone>
{
    // Source of this BufferedZone's data
    int buffer;
    size_t bufferOffset;
    size_t length;
    int target;
    BufferedZone(int buffer, size_t bufferOffset, size_t length, int target)
        : buffer(buffer), bufferOffset(bufferOffset), length(length), target(target)
    {}
    // BufferViews coalesced into this zone
    std::vector<int> bufferViews;
    vsg::ref_ptr<vsg::Buffer> vkBuffer;
};

// Correspondence between a glTF buffer view and our BufferedZone
struct LoadedBufferView
{
    LoadedBufferView()
        : bufferView(-1), zone(nullptr), zoneOffset(0)
    {}
    LoadedBufferView(int bv, BufferedZone* z = nullptr, size_t zo = 0)
        : bufferView(bv), zone(z), zoneOffset(zo)
    {}
    int bufferView;
    vsg::ref_ptr<BufferedZone> zone;
    size_t zoneOffset;
};

// Class for processing one gltf file

struct AccessorBuffer
{
    bool interleaved;
    vsg::ref_ptr<vsg::Data> data;
};

class GLTFReader
{
public:
    void readAsciiModel(const std::string& filename);
    void readBinaryModel(const std::string& filename);
public:
    tinygltf::Model model;
    // The VSG objects that correspond, more or less, to glTF
    // artifacts.
    // textures
    std::vector<vsg::Data> images;
    std::vector<vsg::Sampler> samplers;
    std::vector<vsg::DescriptorImage> descriptorImages;
    std::vector<vsg::ref_ptr<BufferedZone>> bufferedZones;
    // "map" between gltf buffer views and buffered zones
    std::vector<LoadedBufferView> loadedBufferViews;

    std::vector<vsg::ref_ptr<vsg::ubyteArray>> buffers;
    std::vector<AccessorBuffer> accessorBuffers;
    vsg::ref_ptr<Data> makeAccessorData(const tinygltf::Accessor&)
    void makeBufferViews();
    std::tuple<vsg::ref_ptr<vsg::GraphicsPipeline>,
               vsg::ref_ptr<vsg::BindDescriptorSet>,
               vsg::ref_ptr<vsg::Commands>>
    getMeshComponents(int meshIndex);
    void initAccessors();
    
};

void GLTFReader::readAsciiModel(const std::string& filename)
{
    tinygltf::TinyGLTF reader;
    bool result = 
}

void GLTFReader::readBinaryModel(const std::string& filename)
{
    
}

void GLTFReader::initializeBuffers()
{
    for (const auto& tglfBuffer : model.buffers)
    {
        
    }
}
vsg::ref_ptr<Data> GLTFReader::makeAccessorData(const tinygltf::Accessor& accessor)
{
    
}

void GLTFReader::makeBufferViews()
{
    if (model.bufferViews.empty())
    {
        // wtf, throw?
        return;
    }
    std::vector<int> bvIndex(model.bufferViews.size());
    std::iota(bvIndex.begin(), bvIndex.end(), 0);
    // Arrange loaded buffer views by buffer, offset in
    // their source buffer
    std::sort(bvIndex.begin(), bvIndex.end(),
              [this](int lhs, int rhs)
              {
                  const tinygltf::BufferView& lbv = model.bufferViews[lhs];
                  const tinygltf::BufferView& rbv = model.bufferViews[rhs];
                  if (lbv.buffer < rbv.buffer)
                      return true;
                  else if (lbv.buffer > rbv.buffer)
                      return false;
                  return lbv.byteOffset < rbv.byteOffset;
              });
    // bvIndex are now ordered by zone membership
    loadedBufferViews.resize(model.bufferViews.size());
    for (int bv : bvIndex)
    {
        const tinygltf::BufferView& bv = model.bufferViews[bvIndex];
                
        if (bufferedZones.empty() || bufferedZones.back()->buffer != bv.buffer
            || bufferedZones.back()->target != bv.target)
        {
            bufferedZones.emplace_back(BufferedZone::create(bv.buffer, bv.byteOffset, 0, bv.target));
        }
        auto& zone = bufferedZones.back();
        auto& lbv = loadedBufferViews[bvIndex]
        lbv.zone = zone;
        lbv.zoneOffset = bv.byteOffset - zone->bufferOffset;
        zone->length = std::max(zone->length, lbv.zoneOffset + bv.byteLength);
        zone->bufferViews.push_back(lbv.bufferView); // needed?
    }
}

// Decoding a meshes' accessors to get Vulkan types

template<typename T1, typename T2>
struct ComparePair
{
    using PairType = std::pair<T1, T2>;
    bool operator(const PairType& lhs, const PairType& rhs) const
    {
        return lhs. first < rhs.first || (lhs.first == rhs.first && lhs.second < rhs.second);
    }
};

std::map<std::pair<int, int>, int, ComparePair<int, int>> formatMap = {
    {{TINYGLTF_COMPONENT_TYPE_BYTE, TINYGLTF_TYPE_SCALAR}, VK_FORMAT_R8_SINT},
    {{TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE, TINYGLTF_TYPE_SCALAR}, VK_FORMAT_R8_UINT},
    {{TINYGLTF_COMPONENT_TYPE_SHORT, TINYGLTF_TYPE_SCALAR}, VK_FORMAT_R8_SINT},
    {{TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT, TINYGLTF_TYPE_SCALAR}, VK_FORMAT_R16_UINT},
    {{TINYGLTF_COMPONENT_TYPE_INT, TINYGLTF_TYPE_SCALAR}, VK_FORMAT_R32_UINT},
    {{TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT, TINYGLTF_TYPE_SCALAR}, VK_FORMAT_R32_SINT},
    {{TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_SCALAR}, VK_FORMAT_R32_SFLOAT},
    {{TINYGLTF_COMPONENT_TYPE_DOUBLE, TINYGLTF_TYPE_SCALAR}, VK_FORMAT_R64_SFLOAT},

    {{TINYGLTF_COMPONENT_TYPE_BYTE, TINYGLTF_TYPE_VEC2}, VK_FORMAT_R8G8_SINT},
    {{TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE, TINYGLTF_TYPE_VEC2}, VK_FORMAT_R8G8_UINT},
    {{TINYGLTF_COMPONENT_TYPE_SHORT, TINYGLTF_TYPE_VEC2}, VK_FORMAT_R16G16_SINT},
    {{TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT, TINYGLTF_TYPE_VEC2}, VK_FORMAT_R16G16_UINT},
    {{TINYGLTF_COMPONENT_TYPE_INT, TINYGLTF_TYPE_VEC2}, VK_FORMAT_R32G32_UINT},
    {{TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT, TINYGLTF_TYPE_VEC2}, VK_FORMAT_R32G32_SINT},
    {{TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC2}, VK_FORMAT_R32G64_SFLOAT},
    {{TINYGLTF_COMPONENT_TYPE_DOUBLE, TINYGLTF_TYPE_VEC2}, VK_FORMAT_R64G64_SFLOAT},

    {{TINYGLTF_COMPONENT_TYPE_BYTE, TINYGLTF_TYPE_VEC3}, VK_FORMAT_R8G8B8_SINT},
    {{TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE, TINYGLTF_TYPE_VEC3}, VK_FORMAT_R8G8B8_UINT},
    {{TINYGLTF_COMPONENT_TYPE_SHORT, TINYGLTF_TYPE_VEC3}, VK_FORMAT_R16G16B16_SINT},
    {{TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT, TINYGLTF_TYPE_VEC3}, VK_FORMAT_R16G16B16_UINT},
    {{TINYGLTF_COMPONENT_TYPE_INT, TINYGLTF_TYPE_VEC3}, VK_FORMAT_R32G32B32_UINT},
    {{TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT, TINYGLTF_TYPE_VEC3}, VK_FORMAT_R32G32B32_SINT},
    {{TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC3}, VK_FORMAT_R32G32B32_SFLOAT},
    {{TINYGLTF_COMPONENT_TYPE_DOUBLE, TINYGLTF_TYPE_VEC3}, VK_FORMAT_R64G64B64_SFLOAT},

    {{TINYGLTF_COMPONENT_TYPE_BYTE, TINYGLTF_TYPE_VEC4}, VK_FORMAT_R8G8B8A8_SINT},
    {{TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE, TINYGLTF_TYPE_VEC4}, VK_FORMAT_R8G8B8A8_UINT},
    {{TINYGLTF_COMPONENT_TYPE_SHORT, TINYGLTF_TYPE_VEC4}, VK_FORMAT_R16G16B16A16_SINT},
    {{TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT, TINYGLTF_TYPE_VEC4}, VK_FORMAT_R16G16B16A16_UINT},
    {{TINYGLTF_COMPONENT_TYPE_INT, TINYGLTF_TYPE_VEC4}, VK_FORMAT_R32G32B32A32_UINT},
    {{TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT, TINYGLTF_TYPE_VEC4}, VK_FORMAT_R32G32B32A32_SINT},
    {{TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC4}, VK_FORMAT_R32G32B32A32_SFLOAT},
    {{TINYGLTF_COMPONENT_TYPE_DOUBLE, TINYGLTF_TYPE_VEC4}, VK_FORMAT_R64G64B64A64_SFLOAT}
};

// From the spec, with our location number
// 0 POSITION	"VEC3"	5126 (FLOAT)	XYZ vertex positions
// 1 NORMAL	"VEC3"	5126 (FLOAT)	Normalized XYZ vertex normals
// 2 TANGENT	"VEC4"	5126 (FLOAT)	XYZW vertex tangents where the w component is a sign value (-1 or +1) indicating handedness of the tangent basis
// 3 TEXCOORD_0	"VEC2"	5126 (FLOAT)
//			5121 (UNSIGNED_BYTE) normalized
//			5123 (UNSIGNED_SHORT) normalized	UV texture coordinates for the first set
// 4 TEXCOORD_1	"VEC2"	5126 (FLOAT)
//			5121 (UNSIGNED_BYTE) normalized
//			5123 (UNSIGNED_SHORT) normalized	UV texture coordinates for the second set
// 5 COLOR_0	"VEC3"
//		"VEC4"	5126 (FLOAT)
//			5121 (UNSIGNED_BYTE) normalized
//			5123 (UNSIGNED_SHORT) normalized	RGB or RGBA vertex color

std::map<std::string, int> attributeLocations = {
    {"POSITION", 0},
    {"NORMAL", 1},
    {"TANGENT", 2},
    {"TEXCOORD_0", 3},
    {"TEXCOORD_1", 4},
    {"COLOR_0", 5}
};
    

std::tuple<vsg::ref_ptr<vsg::GraphicsPipeline>,
               vsg::ref_ptr<vsg::BindDescriptorSet>,
               vsg::ref_ptr<vsg::Commands>>
GLTFReader::getMeshComponents(int meshIndex)
{
    auto commands = vsg::Commands::create();
    for (auto primitive : model.meshes[meshIndex].primitives)
    {
        
    }
}

ReaderWriter_gltf::ReaderWriter_gltf()
{
}

vsg::ref_ptr<vsg::Object> ReaderWriter_gltf::read(const vsg::Path& filename,
                                                  vsg::ref_ptr<const vsg::Options> options) const
{
    GLTFReader fileReader;
    vsg::Path filenameToUse = options ? findFile(filename, options) : filename;
    if (filenameToUse.empty())
        return {};
    auto ext = vsg::fileExtension(filename);
    if (ext == "gltf")
    {
        fileReader.readAsciiModel(filenameToUse);
    }
    else if (ext == "glb")
    {
        fileReader.readBinaryModel(filenameToUse);
    }
    else
    {
        return {};
    }
    
}

vsg::ref_ptr<vsg::Object> ReaderWriter_gltf::read(std::istream& fin,
                                                  vsg::ref_ptr<const vsg::Options>) const
{
}
