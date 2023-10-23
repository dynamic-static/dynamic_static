
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

#include "dynamic-static.graphics/coordinate-renderer.hpp"

namespace dst {
namespace gfx {

VkResult CoordinateRenderer::create(const gvk::Context& gvkContext, const CreateInfo& createInfo, CoordinateRenderer* pCoordinateRenderer)
{
    assert(createInfo.renderPass);
    assert(createInfo.pTtfFilePath);
    assert(createInfo.fontSize);
    assert(pCoordinateRenderer);
    pCoordinateRenderer->reset();
    gvk_result_scope_begin(VK_ERROR_INITIALIZATION_FAILED) {
        LineRenderer::CreateInfo lineRendererCreateInfo{ };
        gvk_result(LineRenderer::create(gvkContext, createInfo.renderPass, lineRendererCreateInfo, &pCoordinateRenderer->mLineRenderer));
        text::Font::create(createInfo.pTtfFilePath, nullptr, createInfo.fontSize, &pCoordinateRenderer->mspFont);
        gvk_result(Renderer<text::Font>::create(gvkContext, createInfo.renderPass, *pCoordinateRenderer->mspFont, &pCoordinateRenderer->mFontRenderer));
        pCoordinateRenderer->set_min(createInfo.min);
        pCoordinateRenderer->set_max(createInfo.max);
        pCoordinateRenderer->set_extent(createInfo.extent);
        pCoordinateRenderer->set_origin(createInfo.origin);
        pCoordinateRenderer->set_major_tick(createInfo.majorTick);
        pCoordinateRenderer->set_minor_tick(createInfo.minorTick);
    } gvk_result_scope_end;
    return gvkResult;
}

CoordinateRenderer::~CoordinateRenderer()
{
    reset();
}

void CoordinateRenderer::reset()
{
    mLineRenderer.reset();
    mspFont.reset();
    mFontRenderer = { };
    mTextMeshes.clear();
}

void CoordinateRenderer::update()
{
    if (mUpdate) {
        mPoints.clear();
        std::vector<dst::text::Mesh> textMeshes;
        auto getTextMesh = [&](auto& textMesh)
        {
            if (!mTextMeshes.empty()) {
                textMesh = std::move(mTextMeshes.back());
                textMesh.set_text(std::string{ });
                mTextMeshes.pop_back();
            } else {
                textMesh.create_renderer<Renderer<dst::text::Mesh>>(
                    [&](const auto& /* textMesh */, auto& renderer)
                    {
                        return Renderer<text::Mesh>::create(mFontRenderer.get_pipeline().get<gvk::Device>(), textMesh, mFontRenderer, &renderer);
                    }
                );
            }
        };
        ///////////////////////////////////////////////////////////////////////////////

        Point point{ };
        point.position.x = mMin.x;
        mPoints.push_back(point);
        point.position.x = mMax.x;
        mPoints.push_back(point);
        mPoints.push_back(mPoints.back());
        mPoints.back().width.g = 0;

        point = { };
        point.position.y = mMin.y;
        mPoints.push_back(point);
        point.position.y = mMax.y;
        mPoints.push_back(point);
        mPoints.push_back(mPoints.back());
        mPoints.back().width.g = 0;

        point = { };
        point.position.z = mMin.z;
        mPoints.push_back(point);
        point.position.z = mMax.z;
        mPoints.push_back(point);
        mPoints.push_back(mPoints.back());
        mPoints.back().width.g = 0;

        ///////////////////////////////////////////////////////////////////////////////
        mLineRenderer.submit((uint32_t)mPoints.size(), mPoints.data());
        std::swap(textMeshes, mTextMeshes);
        for (auto& textMesh : mTextMeshes) {
            textMesh.update(0);
        }
    }
}

void CoordinateRenderer::record_draw_cmds(const gvk::CommandBuffer& commandBuffer, const gvk::math::Camera& camera, const glm::vec2& resolution) const
{
    mLineRenderer.record_draw_cmds(commandBuffer, camera, resolution);
    for (const auto& textMesh : mTextMeshes) {
        // TODO : This is kludgy...
        assert(textMesh.get_renderers().size() == 1);
        auto pTextMeshRenderer = reinterpret_cast<dst::gfx::Renderer<dst::text::Mesh>*>(textMesh.get_renderers()[0].get());
        pTextMeshRenderer->record_draw_cmds(commandBuffer, mFontRenderer);
    }
}

const glm::vec3& CoordinateRenderer::get_min() const
{
    return mMin;
}

void CoordinateRenderer::set_min(const glm::vec3& min)
{
    set_member_value(min, mMin);
}

const glm::vec3& CoordinateRenderer::get_max() const
{
    return mMax;
}

void CoordinateRenderer::set_max(const glm::vec3& max)
{
    set_member_value(max, mMax);
}

const glm::vec3& CoordinateRenderer::get_extent() const
{
    return mExtent;
}

void CoordinateRenderer::set_extent(const glm::vec3& extent)
{
    set_member_value(extent, mExtent);
}

const glm::vec3& CoordinateRenderer::get_origin() const
{
    return mOrigin;
}

void CoordinateRenderer::set_origin(const glm::vec3& origin)
{
    set_member_value(origin, mOrigin);
}

const glm::vec3& CoordinateRenderer::get_major_tick() const
{
    return mMajorTick;
}

void CoordinateRenderer::set_major_tick(const glm::vec3& majorTick)
{
    set_member_value(majorTick, mMajorTick);
}

const glm::vec3& CoordinateRenderer::get_minor_tick() const
{
    return mMinorTick;
}

void CoordinateRenderer::set_minor_tick(const glm::vec3& minorTick)
{
    set_member_value(minorTick, mMinorTick);
}

template <typename T>
void CoordinateRenderer::set_member_value(const T& value, T& member)
{
    if (member != value) {
        member = value;
        mUpdate = true;
    }
}

} // namespace gfx
} // namespace dst
