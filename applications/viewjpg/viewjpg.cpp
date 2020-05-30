#include <vsg/all.h>
#include <iostream>
#include <algorithm>

#include "jpeg/ReaderWriter_jpeg.h"
#include "manipulators/OrthoTrackball.h"
#include "ReaderWriter_sandbox/ImageTranslator.h"

// For the graphics pipeline. The descriptor bindings and descriptor
// set layout are needed both for pipeline creation and setting up the
// descriptor set for each textured quad

vsg::DescriptorSetLayoutBindings descriptorBindings
{
    {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr} // { binding, descriptorTpe, descriptorCount, stageFlags, pImmutableSamplers}
};

vsg::ref_ptr<vsg::MatrixTransform> createTextureGraph(vsg::ref_ptr<vsg::Data> textureData,
                                                      vsg::ref_ptr<vsgSandbox::EXIF> exif,
                                                      vsg::ref_ptr<vsg::PipelineLayout> pipelineLayout,
                                                      vsg::ref_ptr<vsg::DescriptorSetLayout> descriptorSetLayout)
{
    // create texture image and associated DescriptorSets and binding
    auto imageWidth = textureData->width();
    auto imageHeight = textureData->height();
    float ratio = static_cast<float>(imageWidth) / static_cast<float>(imageHeight);
    auto texture = vsg::DescriptorImage::create(vsg::Sampler::create(), textureData, 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

    auto descriptorSet = vsg::DescriptorSet::create(descriptorSetLayout, vsg::Descriptors{texture});
    auto bindDescriptorSet = vsg::BindDescriptorSet::create(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0,
                                                            descriptorSet);

    // set up model transformation node
    vsg::ref_ptr<vsg::MatrixTransform> transform = vsg::MatrixTransform::create(); // VK_SHADER_STAGE_VERTEX_BIT

        // create a StateGroup for binding of the texture
    // descriptor. Should it go lower in the graph?
    auto scenegraph = vsg::StateGroup::create();
    scenegraph->add(bindDescriptorSet);

    // add transform to root of the scene graph
    transform->addChild(scenegraph);

    // set up vertex and index arrays
    // Starting point, to be modified by image aspect ratio and EXIF data
    auto vertices = vsg::vec3Array::create(
    {
        {-0.5f, -0.5f, 0.0f},
        {0.5f,  -0.5f, 0.0f},
        {0.5f , 0.5f, 0.0f},
        {-0.5f, 0.5f, 0.0f}
    }); // VK_FORMAT_R32G32B32_SFLOAT, VK_VERTEX_INPUT_RATE_INSTANCE, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE
    // XXX Not using the colors, better to change the input rate or
    // not allocate them at all.
    auto colors = vsg::vec3Array::create(
    {
        {1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 1.0f}
    }); // VK_FORMAT_R32G32B32_SFLOAT, VK_VERTEX_INPUT_RATE_VERTEX, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE
    auto texcoords = vsg::vec2Array::create(
    {
        {0.0f, 0.0f},
        {1.0f, 0.0f},
        {1.0f, 1.0f},
        {0.0f, 1.0f}
    }); // VK_FORMAT_R32G32_SFLOAT, VK_VERTEX_INPUT_RATE_VERTEX, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE
    {
        using namespace vsgSandbox;
        EXIF::Orientation orient = exif->orientation;
        if (orient == EXIF::LeftTop
            || orient == EXIF::RightTop
            || orient == EXIF::RightBottom
            || orient == EXIF::LeftBottom)
        {
            // Portrait mode
            for (auto& vert : *vertices)
            {
                vert.y *= ratio;
            }
        }
        else
        {
            // Landscape
            for (auto& vert : *vertices)
            {
                vert.y /= ratio;
            }
        }
        // Rotate the image to match
        switch (orient)
        {
        case EXIF::TopLeft:
            // already fine
            break;
        case EXIF::TopRight:
            // flip left-to-right
            std::swap((*texcoords)[0], (*texcoords)[1]);
            std::swap((*texcoords)[2], (*texcoords)[3]);
            break;
        case EXIF::BottomRight:
            // rotate 180
            std::rotate(texcoords->begin(), texcoords->begin() + 2, texcoords->end());
            break;
        case EXIF::BottomLeft:
            // flip top-to-bottom
            std::swap((*texcoords)[0], (*texcoords)[3]);
            std::swap((*texcoords)[1], (*texcoords)[2]);
            break;
        case EXIF::LeftTop:
            // swap diagonally
            std::swap((*texcoords)[0], (*texcoords)[2]);
            break;
        case EXIF::RightTop:
            // rotate clockwise 90
            std::rotate(texcoords->begin(), texcoords->begin() + 1, texcoords->end());
            break;
        case EXIF::RightBottom:
            // swap diagonally
            std::swap((*texcoords)[1], (*texcoords)[3]);
            break;
        case EXIF::LeftBottom:
            // rotate counter-clockwise 90
            std::rotate(texcoords->begin(), texcoords->begin() + 3, texcoords->end());
            break;
        default:
            // Shouldn't happen
            break;
        }
    }

    auto indices = vsg::ushortArray::create(
    {
        0, 1, 2,
        2, 3, 0
    }); // VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE

    // setup geometry
    auto drawCommands = vsg::Commands::create();
    drawCommands->addChild(vsg::BindVertexBuffers::create(0, vsg::DataList{vertices, colors, texcoords}));
    drawCommands->addChild(vsg::BindIndexBuffer::create(indices));
    drawCommands->addChild(vsg::DrawIndexed::create(6, 1, 0, 0, 0));

    // add drawCommands to transform
    scenegraph->addChild(drawCommands);
    return transform;
}

int main(int argc, char** argv)
{
    // set up defaults and read command line arguments to override them
    vsg::CommandLine arguments(&argc, argv);

    auto windowTraits = vsg::WindowTraits::create();
    windowTraits->swapchainPreferences.surfaceFormat = VkSurfaceFormatKHR{VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    windowTraits->debugLayer = arguments.read({"--debug","-d"});
    windowTraits->apiDumpLayer = arguments.read({"--api","-a"});
    arguments.read({"--window", "-w"}, windowTraits->width, windowTraits->height);

    if (arguments.errors()) return arguments.writeErrorMessages(std::cerr);

    if (argc < 1)
    {
        return 1;
    }

    // create the viewer and assign window(s) to it
    auto viewer = vsg::Viewer::create();

    auto window = vsg::Window::create(windowTraits);
    if (!window)
    {
        std::cout<<"Could not create windows."<<std::endl;
        return 1;
    }

    vsgSandbox::ReaderWriter_jpeg jpegReader;
    vsgSandbox::ImageTranslator ImageTranslator(window->getOrCreateDevice());

    // set up search paths to SPIRV shaders and textures
    vsg::Paths searchPaths = vsg::getEnvPaths("VSG_FILE_PATH");

    // load shaders
    vsg::ref_ptr<vsg::ShaderStage> vertexShader = vsg::ShaderStage::read(VK_SHADER_STAGE_VERTEX_BIT, "main", vsg::findFile("shaders/vert_PushConstants.spv", searchPaths));
    vsg::ref_ptr<vsg::ShaderStage> fragmentShader = vsg::ShaderStage::read(VK_SHADER_STAGE_FRAGMENT_BIT, "main", vsg::findFile("shaders/frag_PushConstants.spv", searchPaths));
    if (!vertexShader || !fragmentShader)
    {
        std::cout<<"Could not create shaders."<<std::endl;
        return 1;
    }

    // set up graphics pipeline

    auto descriptorSetLayout = vsg::DescriptorSetLayout::create(descriptorBindings);

    vsg::PushConstantRanges pushConstantRanges
    {
        {VK_SHADER_STAGE_VERTEX_BIT, 0, 128} // projection view, and model matrices, actual push constant calls automatically provided by the VSG's DispatchTraversal
    };

    vsg::VertexInputState::Bindings vertexBindingsDescriptions
    {
        VkVertexInputBindingDescription{0, sizeof(vsg::vec3), VK_VERTEX_INPUT_RATE_VERTEX}, // vertex data
        VkVertexInputBindingDescription{1, sizeof(vsg::vec3), VK_VERTEX_INPUT_RATE_VERTEX}, // colour data
        VkVertexInputBindingDescription{2, sizeof(vsg::vec2), VK_VERTEX_INPUT_RATE_VERTEX}  // tex coord data
    };

    vsg::VertexInputState::Attributes vertexAttributeDescriptions
    {
        VkVertexInputAttributeDescription{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0}, // vertex data
        VkVertexInputAttributeDescription{1, 1, VK_FORMAT_R32G32B32_SFLOAT, 0}, // colour data
        VkVertexInputAttributeDescription{2, 2, VK_FORMAT_R32G32_SFLOAT, 0},    // tex coord data
    };

    vsg::GraphicsPipelineStates pipelineStates
    {
        vsg::VertexInputState::create( vertexBindingsDescriptions, vertexAttributeDescriptions ),
        vsg::InputAssemblyState::create(),
        vsg::RasterizationState::create(),
        vsg::MultisampleState::create(),
        vsg::ColorBlendState::create(),
        vsg::DepthStencilState::create()
    };

    auto pipelineLayout = vsg::PipelineLayout::create(vsg::DescriptorSetLayouts{descriptorSetLayout}, pushConstantRanges);
    auto graphicsPipeline = vsg::GraphicsPipeline::create(pipelineLayout, vsg::ShaderStages{vertexShader, fragmentShader}, pipelineStates);
    auto bindGraphicsPipeline = vsg::BindGraphicsPipeline::create(graphicsPipeline);

    auto scenegraph = vsg::StateGroup::create();
    scenegraph->add(bindGraphicsPipeline);

    // read texture images
    double imageOffset = 0.0;
    for (int i = 1; i < argc; ++i, imageOffset += 1.1)
    {
        vsg::Path jpegFilename = arguments[i];
        vsg::ref_ptr<vsg::Data> textureData(dynamic_cast<vsg::Data*>(jpegReader.read(jpegFilename).get()));
        if (!textureData)
        {
            std::cout<<"Could not read texture file : "<<jpegFilename<<std::endl;
            return 1;
        }
        auto exif = vsgSandbox::EXIF::get(textureData);
        textureData = ImageTranslator.translateToSupported(textureData);
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(*window->getOrCreateDevice()->getPhysicalDevice(), textureData->getFormat(),
                                            &formatProperties);
        if ((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) == 0)
        {
            std::cerr << "no no no\n";
            break;
        }

        auto transform = createTextureGraph(textureData, exif, pipelineLayout, descriptorSetLayout);
        vsg::dmat4 transformMat = transform->getMatrix();
        vsg::dmat4 translate = vsg::translate(imageOffset, 0.0, 0.0);
        transformMat = translate * transformMat;
        transform->setMatrix(transformMat);

        // add transform to root of the scene graph
        scenegraph->addChild(transform);

    }

    viewer->addWindow(window);

    // camera related details
    auto viewport = vsg::ViewportState::create(window->extent2D());
    double l, r, b = -1.0, t = 1.0;
    const VkViewport& vp = viewport->getViewport();
    double aspectRatio = vp.width / vp.height;
    l = -(t - b) * aspectRatio / 2;
    r = -l;
    auto ortho = vsg::Orthographic::create(l, r, b, t, 1.0, 10000.0);
    auto lookAt = vsg::LookAt::create(vsg::dvec3(0.0, 0.0, 1.0), vsg::dvec3(0.0, 0.0, 0.0), vsg::dvec3(0.0, 1.0, 0.0));
    auto camera = vsg::Camera::create(ortho, lookAt, viewport);

    auto commandGraph = vsg::createCommandGraphForView(window, camera, scenegraph);
    viewer->assignRecordAndSubmitTaskAndPresentation({commandGraph});

    // compile the Vulkan objects
    viewer->compile();

    // assign a CloseHandler to the Viewer to respond to pressing Escape or press the window close button
    viewer->addEventHandlers({vsgSandbox::OrthoTrackball::create(camera),
                              vsg::CloseHandler::create(viewer)});

    // main frame loop
    while (viewer->advanceToNextFrame())
    {
        // pass any events into EventHandlers assigned to the Viewer
        viewer->handleEvents();

        viewer->update();

        viewer->recordAndSubmit();

        viewer->present();
    }

    // clean up done automatically thanks to ref_ptr<>
    return 0;
}
