#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_ENABLE_DRACO
#define TINYGLTF_USE_RAPIDJSON
#define TINYGLTF_USE_RAPIDJSON_CRTALLOCATOR

#include "tinygltf/tinygltf.h"

#include <vsgsandbox/jpeg/ReaderWriter_jpeg.h>
#include <vsgsandbox/png/ReaderWriter_png.h>
#include <vsg/core/Object.h>
#include <vsg/core/Inherit.h>
#include <algorithm>
#include <vector>

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

// Group gltf BufferView objects into vk buffers
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

class GLTFReader
{
public:
    tinygltf::Model model;
    // The VSG objects that correspond, more or less, to glTF
    // artifacts.
    // textures
    std::vector<vsg::Data> images;
    std::vector<vsg::Sampler> samplers;
    std::vector<vsg::DescriptorImage> descriptorImages;
    std::vector<vsg::ref_ptr<BufferedZone>> bufferedZones;
    std::vector<LoadedBufferView> loadedBufferViews;
    vsg::ref_ptr<Data> makeAccessorData(const tinygltf::Accessor&)
    void makeBufferViews();
    
};

vsg::ref_ptr<Data> GLTFReader::makeAccessorData(const tinygltf::Accessor& accessor)
{
    
}

void GLTFReader::makeBufferViews()
{
    for (int i = 0; i < model.bufferViews.size(); ++i)
    {
        loadedBufferViews.emplace_back(i);
    }
    // Arrange loaded buffer views by buffer, offset in
    // their source buffer
    std::sort(loadedBufferViews.begin(), loadedBufferViews.end(),
              [](const LoadedBufferView& lhs, const LoadedBufferView& rhs)
              {
                  const tinygltf::BufferView& lbv = model.bufferViews[lhs.bufferView];
                  const tinygltf::BufferView& rbv = model.bufferViews[rhs.bufferView];
                  if (lbv.buffer < rbv.buffer)
                      return true;
                  else if (lbv.buffer > rbv.buffer)
                      return false;
                  return lbv.byteOffset < rbv.byteOffset;
              });
    if (loadedBufferViews.empty())
    {
        // wtf, throw?
        return;
    }
    for (auto& lbv : loadedBufferViews)
    {
        const tinygltf::BufferView& bv = model.bufferViews[lbv.bufferView];
                
        if (bufferedZones.empty() || bufferedZones.back()->buffer != bv.buffer
            || bufferedZones.back()->target != bv.target)
        {
            bufferedZones.emplace_back(BufferedZone::create(bv.buffer, bv.byteOffset, 0, bv.target));
        }
        auto& zone = bufferedZones.back();
        lbv.zone = zone;
        lbv.zoneOffset = bv.byteOffset - zone->bufferOffset;
        zone->length = std::max(zone->length, lbv.zoneOffset + bv.byteLength);
        zone->bufferViews.push_back(lbv.bufferView);
    }
}
