
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

static VkResult create_bloom_pipeline(
    const gvk::RenderPass& renderPass,
    VkCullModeFlagBits cullMode,
    VkPolygonMode polygonMode,
    gvk::spirv::ShaderInfo& vertexShaderInfo,
    gvk::spirv::ShaderInfo& fragmentShaderInfo,
    gvk::Pipeline* pPipeline
)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        // Create a gvk::spirv::Compiler, compile GLSL to SPIR-V, then validate both
        //  shaders.
        gvk::spirv::Context spirvContext;
        gvk_result(gvk::spirv::Context::create(&gvk::get_default<gvk::spirv::Context::CreateInfo>(), &spirvContext));
        spirvContext.compile(&vertexShaderInfo);
        gvk_result(dst_sample_validate_shader_info(vertexShaderInfo));
        spirvContext.compile(&fragmentShaderInfo);
        gvk_result(dst_sample_validate_shader_info(fragmentShaderInfo));
        auto vsVkResult = dst_sample_validate_shader_info(vertexShaderInfo);
        auto fsVkResult = dst_sample_validate_shader_info(fragmentShaderInfo);
        gvk_result(vsVkResult);
        gvk_result(fsVkResult);

        // Create a gvk::ShaderModule for the vertex shader.
        auto vertexShaderModuleCreateInfo = gvk::get_default<VkShaderModuleCreateInfo>();
        vertexShaderModuleCreateInfo.codeSize = vertexShaderInfo.bytecode.size() * sizeof(uint32_t);
        vertexShaderModuleCreateInfo.pCode = vertexShaderInfo.bytecode.data();
        gvk::ShaderModule vertexShaderModule;
        gvk_result(gvk::ShaderModule::create(renderPass.get<gvk::Device>(), &vertexShaderModuleCreateInfo, nullptr, &vertexShaderModule));
        auto vertexPipelineShaderStageCreateInfo = gvk::get_default<VkPipelineShaderStageCreateInfo>();
        vertexPipelineShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexPipelineShaderStageCreateInfo.module = vertexShaderModule;

        // Create a gvk::ShaderModule for the fragment shader.
        auto fragmentShaderModuleCreateInfo = gvk::get_default<VkShaderModuleCreateInfo>();
        fragmentShaderModuleCreateInfo.codeSize = fragmentShaderInfo.bytecode.size() * sizeof(uint32_t);
        fragmentShaderModuleCreateInfo.pCode = fragmentShaderInfo.bytecode.data();
        gvk::ShaderModule fragmentShaderModule;
        gvk_result(gvk::ShaderModule::create(renderPass.get<gvk::Device>(), &fragmentShaderModuleCreateInfo, nullptr, &fragmentShaderModule));
        auto fragmentPipelineShaderStageCreateInfo = gvk::get_default<VkPipelineShaderStageCreateInfo>();
        fragmentPipelineShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentPipelineShaderStageCreateInfo.module = fragmentShaderModule;

        // Create an array of VkPipelineShaderStageCreateInfos for the shaders used in
        //  this gvk::Pipeline.
        std::array<VkPipelineShaderStageCreateInfo, 2> pipelineShaderStageCreateInfos {
            vertexPipelineShaderStageCreateInfo,
            fragmentPipelineShaderStageCreateInfo,
        };

#if 0
        // VkVertexInputBindingDescription describes how the gvk::Pipeline should
        //  traverse vertex buffer data when draw calls are issued.
        // NOTE : gvk::get_vertex_description<VertexType>(0) is used to get an array of
        //  VkVertexInputAttributeDescriptions at binding 0 which indicates that the
        //  array is associated with the 0th element of pVertexBindingDescriptions.
        VkVertexInputBindingDescription vertexInputBindingDescription { 0, sizeof(VertexType), VK_VERTEX_INPUT_RATE_VERTEX };
        auto vertexInputAttributeDescriptions = gvk::get_vertex_description<VertexType>(0);
        auto pipelineVertexInputStateCreateInfo = gvk::get_default<VkPipelineVertexInputStateCreateInfo>();
        pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
        pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
        pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = (uint32_t)vertexInputAttributeDescriptions.size();
        pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data();
#endif

        // VkPipelineRasterizationStateCreateInfo describes how rasterization should
        //  occur...this includes parameters for polygon mode, winding order, face
        //  culling, etc.
        auto pipelineRasterizationStateCreateInfo = gvk::get_default<VkPipelineRasterizationStateCreateInfo>();
        pipelineRasterizationStateCreateInfo.cullMode = cullMode;
        pipelineRasterizationStateCreateInfo.polygonMode = polygonMode;

        // VkPipelineMultisampleStateCreateInfo describes how multi sampling should
        //  occur.  rasterizationSamples should match the sample count of the
        //  gvk::RenderPass objects that will be used with this gvk::Pipeline.
        auto pipelineMultisampleStateCreateInfo = gvk::get_default<VkPipelineMultisampleStateCreateInfo>();
        pipelineMultisampleStateCreateInfo.rasterizationSamples = dst_sample_get_render_pass_sample_count(renderPass);

        // VkPipelineDepthStencilStateCreateInfo describes how depth should be handled
        //  during fragment shading.
        auto depthTestEnable = dst_sample_get_render_pass_depth_format(renderPass) != VK_FORMAT_UNDEFINED;
        auto pipelineDepthStencilStateCreateInfo = gvk::get_default<VkPipelineDepthStencilStateCreateInfo>();
        pipelineDepthStencilStateCreateInfo.depthTestEnable = depthTestEnable;
        pipelineDepthStencilStateCreateInfo.depthWriteEnable = depthTestEnable;
        pipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;

        // Populate a gvk::spirv::BindingInfo with our gvk::spirv::ShaderInfo objects.
        //  This will run the shader byte code through SPIRV-Cross to reflect the
        //  resource bindings used in the shaders.  When the gvk::spirv::BindingInfo is
        //  passed to create_pipeline_layout() it will be used to create the necessary
        //  gvk::DescriptorSetLayout objects for the gvk::PipelineLayout.
        gvk::spirv::BindingInfo spirvBindingInfo;
        spirvBindingInfo.add_shader(vertexShaderInfo);
        spirvBindingInfo.add_shader(fragmentShaderInfo);
        gvk::PipelineLayout pipelineLayout;
        gvk_result(gvk::spirv::create_pipeline_layout(renderPass.get<gvk::Device>(), spirvBindingInfo, nullptr, &pipelineLayout));

        // Finally we populate a VkGraphicsPipelineCreateInfo with the components
        //  necessary for this gvk::Pipeline.
        // NOTE : gvk::get_default<VkGraphicsPipelineCreateInfo>() is used to get the
        //  VkGraphicsPipelineCreateInfo that is prepared for this gvk::Pipeline.
        //  gvk::get_default<>() automatically sets sTypes and sensible default values
        //  where appropriate, for gvk::get_default<VkGraphicsPipelineCreateInfo>() the
        //  default values are generally configured to noop so that only portions of
        //  gvk::Pipeline that are purposefully configured will have an effect.
        auto graphicsPipelineCreateInfo = gvk::get_default<VkGraphicsPipelineCreateInfo>();
        graphicsPipelineCreateInfo.stageCount = (uint32_t)pipelineShaderStageCreateInfos.size();
        graphicsPipelineCreateInfo.pStages = pipelineShaderStageCreateInfos.data();
#if 0
        if (pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount) {
            graphicsPipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;
        }
#endif
        graphicsPipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
        graphicsPipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;
        graphicsPipelineCreateInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;
        graphicsPipelineCreateInfo.layout = pipelineLayout;
        graphicsPipelineCreateInfo.renderPass = renderPass;
        gvk_result(gvk::Pipeline::create(renderPass.get<gvk::Device>(), VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, pPipeline));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult BloomRenderer::create(const gvk::Context& gvkContext, const CreateInfo* pCreateInfo, BloomRenderer* pBloom)
{
    (void)gvkContext;
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(pCreateInfo ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pCreateInfo->renderPass ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        gvk_result(pBloom ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);

        // TODO : Documentation
        gvk::spirv::ShaderInfo vertexShaderInfo {
            .language = gvk::spirv::ShadingLanguage::Glsl,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .lineOffset = __LINE__,
            .source = R"(
                #version 450

                layout(location = 0) out vec2 fsTexcoord;
                out gl_PerVertex { vec4 gl_Position; };

                void main()
                {
                    fsTexcoord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
                    gl_Position = vec4(fsTexcoord * 2 - 1, 0, 1);
                }
            )"
        };

        // TODO : Documentation
        gvk::spirv::ShaderInfo extractFragmentShaderInfo {
            .language = gvk::spirv::ShadingLanguage::Glsl,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .lineOffset = __LINE__,
            .source = R"(
                #version 450

                layout(set = 0, binding = 0) uniform sampler2D image;
                layout(location = 0) in vec2 fsTexcoord;
                layout(location = 0) out vec4 fragColor;

                void main()
                {
                    fragColor = texture(image, fsTexcoord);
                }
            )"
        };

        // TODO : Documentation
        gvk::spirv::ShaderInfo blurFragmentShaderInfo{
            .language = gvk::spirv::ShadingLanguage::Glsl,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .lineOffset = __LINE__,
            .source = R"(
                #version 450

                layout(set = 0, binding = 0) uniform sampler2D image;
                layout(location = 0) in vec2 fsTexcoord;
                layout(location = 0) out vec4 fragColor;

                void main()
                {
                    fragColor = texture(image, fsTexcoord);
                }
            )"
        };

        // TODO : Documentation
        gvk::spirv::ShaderInfo combineFragmentShaderInfo{
            .language = gvk::spirv::ShadingLanguage::Glsl,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .lineOffset = __LINE__,
            .source = R"(
                #version 450

                layout(set = 0, binding = 0) uniform sampler2D image;
                layout(location = 0) in vec2 fsTexcoord;
                layout(location = 0) out vec4 fragColor;

                void main()
                {
                    fragColor = texture(image, fsTexcoord);
                }
            )"
        };

        gvk_result(create_bloom_pipeline(pCreateInfo->renderPass, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL, vertexShaderInfo, extractFragmentShaderInfo, &pBloom->mExtractPipeline));
        gvk_result(create_bloom_pipeline(pCreateInfo->renderPass, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL, vertexShaderInfo, blurFragmentShaderInfo, &pBloom->mBlurPipeline));
        gvk_result(create_bloom_pipeline(pCreateInfo->renderPass, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL, vertexShaderInfo, combineFragmentShaderInfo, &pBloom->mCombinePipeline));
    } gvk_result_scope_end;
    return gvkResult;
}

VkResult BloomRenderer::begin_render_pass(const gvk::CommandBuffer& commandBuffer, const gvk::RenderTarget& outputRenderTarget)
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

void BloomRenderer::end_render_pass(const gvk::CommandBuffer& commandBuffer)
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

void BloomRenderer::draw_render_target(const gvk::CommandBuffer& commandBuffer)
{
    (void)commandBuffer;
}

VkResult BloomRenderer::record_cmds(const gvk::Context& gvkContext, const gvk::CommandBuffer& commandBuffer, VkFormat outputColorFormat, const gvk::RenderTarget& inputRenderTarget)
{
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        gvk_result(inputRenderTarget ? VK_SUCCESS : VK_ERROR_INITIALIZATION_FAILED);
        auto framebufferCreateInfo = inputRenderTarget.get<VkFramebufferCreateInfo>();
        if (!mRenderTargets[0] ||
            mRenderTargets[0].get<VkFramebufferCreateInfo>().width != framebufferCreateInfo.width ||
            mRenderTargets[0].get<VkFramebufferCreateInfo>().height != framebufferCreateInfo.height) {
            auto dstSampleRenderTargetCreateInfo = gvk::get_default<DstSampleRenderTargetCreateInfo>();
            dstSampleRenderTargetCreateInfo.extent = { framebufferCreateInfo.width, framebufferCreateInfo.height };
            dstSampleRenderTargetCreateInfo.colorFormat = outputColorFormat;
            for (auto& renderTarget : mRenderTargets) {
                gvk_result(dst_sample_create_render_target(gvkContext, dstSampleRenderTargetCreateInfo, &renderTarget));
            }
        }

        // Extract
        commandBuffer.CmdBeginRenderPass(nullptr, VK_SUBPASS_CONTENTS_INLINE);
        {

        }
        commandBuffer.CmdEndRenderPass();

        // Blur horizontal
        commandBuffer.CmdBeginRenderPass(nullptr, VK_SUBPASS_CONTENTS_INLINE);
        {

        }
        commandBuffer.CmdEndRenderPass();

        // Blur vertical
        commandBuffer.CmdBeginRenderPass(nullptr, VK_SUBPASS_CONTENTS_INLINE);
        {

        }
        commandBuffer.CmdEndRenderPass();

        // Combine
        commandBuffer.CmdBeginRenderPass(nullptr, VK_SUBPASS_CONTENTS_INLINE);
        {

        }
        commandBuffer.CmdEndRenderPass();
    } gvk_result_scope_end;
    return gvkResult;
}

void BloomRenderer::on_gui()
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
