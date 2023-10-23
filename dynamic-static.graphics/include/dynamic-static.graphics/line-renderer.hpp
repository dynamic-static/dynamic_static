
/*******************************************************************************

MIT License

Copyright (c) dynamic-static

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*******************************************************************************/

#pragma once

#include "dynamic-static.graphics/defines.hpp"

#include "gvk-math.hpp"

namespace dst {
namespace gfx {

struct Point
{
    glm::vec4 position{ 0.0f, 0.0f, 0.0f, 1.0f };
    glm::vec4 color{ gvk::math::Color::White };
    glm::vec4 width{ 4.0f, 1.0f, 0.0f, 0.0f };
};

class LineRenderer final
{
public:
    class CreateInfo final
    {
    public:
        uint32_t capVertexCount{ 0 };
        VkPrimitiveTopology primitiveTopology{ VK_PRIMITIVE_TOPOLOGY_LINE_STRIP };
    };

    LineRenderer() = default;
    static VkResult create(const gvk::Context& gvkContext, const gvk::Pipeline& pipeline, const CreateInfo& createInfo, LineRenderer* pRenderer);
    static VkResult create(const gvk::Context& gvkContext, const gvk::RenderPass& renderPass, const CreateInfo& createInfo, LineRenderer* pRenderer);
    ~LineRenderer();
    void reset();

    const gvk::Pipeline get_pipeline() const;

    void submit(uint32_t pointCount, const Point* pPoints);
    void record_draw_cmds(const gvk::CommandBuffer& commandBuffer, const gvk::math::Camera& camera, const glm::vec2& resolution) const;

private:
    VkResult create_pipeline(const gvk::Context& gvkContext, const gvk::RenderPass& renderPass);
    VkResult create_vertex_buffer(const gvk::Context& gvkContext, uint32_t capVertexCount);
    VkResult allocate_descriptor_set(const gvk::Context& gvkContext);

    gvk::Pipeline mPipeline;
    gvk::Buffer mVertexBuffer;
    gvk::Buffer mStorageBuffer;
    gvk::DescriptorSet mDescriptorSet;
    uint32_t mVertexCount{ };
    uint32_t mPointCount{ };

    LineRenderer(const LineRenderer&) = delete;
    LineRenderer& operator=(const LineRenderer&) = delete;
};

} // namespace gfx
} // namespace dst
