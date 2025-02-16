
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

#include "shape-shooter/bloom.hpp"
#include "shape-shooter/utilities.hpp"

#include "imgui.h"

namespace shape_shooter {

VkResult Bloom::create(const gvk::Context& gvkContext, const CreateInfo* pCreateInfo, Bloom* pBloom)
{
    (void)gvkContext;

    assert(pCreateInfo);
    assert(pBloom);
    // pGrid->mCreateInfo = *pCreateInfo;
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(VK_SUCCESS);
        // auto lineRendererCreateInfo = gvk::get_default<dst::gfx::LineRenderer::CreateInfo>();
        // gvk_result(dst::gfx::LineRenderer::create(gvkContext, renderPass, lineRendererCreateInfo, &pGrid->mLineRenderer));
        // pGrid->create_grid();
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult Bloom::begin_render_pass(const gvk::CommandBuffer& commandBuffer, const gvk::RenderTarget& outputRenderTarget)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(outputRenderTarget ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        mOutputRenderTarget = outputRenderTarget;
        auto inputFramebufferCreateInfo = mInputRenderTarget ? mInputRenderTarget.get<VkFramebufferCreateInfo>() : gvk::get_default<VkFramebufferCreateInfo>();
        auto outputFramebufferCreateInfo = mOutputRenderTarget ? mOutputRenderTarget.get<VkFramebufferCreateInfo>() : gvk::get_default<VkFramebufferCreateInfo>();
        if (inputFramebufferCreateInfo.width != outputFramebufferCreateInfo.width || inputFramebufferCreateInfo.height != outputFramebufferCreateInfo.height) {
            gvk::Auto<VkFramebufferCreateInfo> autoFramebufferCreateInfo = outputRenderTarget.get<VkFramebufferCreateInfo>();
            gvk::detail::enumerate_structure_handles(
                *autoFramebufferCreateInfo,
                [](VkObjectType, const uint64_t& handle)
                {
                    const_cast<uint64_t&>(handle) = 0;
                }
            );
            gvk_result(gvk::RenderTarget::create(outputRenderTarget.get<gvk::Device>(), &*autoFramebufferCreateInfo, nullptr, &mInputRenderTarget));
        }
        commandBuffer.CmdBeginRenderPass(&mInputRenderTarget.get<VkRenderPassBeginInfo>(), VK_SUBPASS_CONTENTS_INLINE);
    } gvk_result_scope_end;
    return gvkResult;
}

void Bloom::end_render_pass(const gvk::CommandBuffer& commandBuffer)
{
    commandBuffer.CmdEndRenderPass();

    // Extract
    commandBuffer.CmdBeginRenderPass(nullptr, VK_SUBPASS_CONTENTS_INLINE);
    commandBuffer.CmdEndRenderPass();

    // Horizontal blur
    commandBuffer.CmdBeginRenderPass(nullptr, VK_SUBPASS_CONTENTS_INLINE);
    commandBuffer.CmdEndRenderPass();

    // Vertical blur
    commandBuffer.CmdBeginRenderPass(nullptr, VK_SUBPASS_CONTENTS_INLINE);
    commandBuffer.CmdEndRenderPass();
}

void Bloom::draw_render_target(const gvk::CommandBuffer& commandBuffer)
{
    (void)commandBuffer;
}

void Bloom::on_gui()
{
    if (ImGui::CollapsingHeader("Bloom")) {
        ImGui::Indent();
        {
            ImGui::Checkbox("Enabled", &mEnabled);
            ImGui::BeginDisabled(!mEnabled);
            ImGui::DragFloat("Base Intensity", &mBaseIntensity);
            ImGui::DragFloat("Bloom Intensity", &mBloomIntensity);
            ImGui::DragFloat("Base Saturation", &mBaseSaturation);
            ImGui::DragFloat("Bloom Saturation", &mBloomSaturation);
            ImGui::EndDisabled();
        }
        ImGui::Unindent();
    }
}

} // namespace shape_shooter
