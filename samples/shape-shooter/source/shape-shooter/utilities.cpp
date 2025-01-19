
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

#include "shape-shooter/utilities.hpp"
#include "shape-shooter/context.hpp"

#include "glm/gtx/fast_trigonometry.hpp"

namespace shape_shooter {

bool PlayField::contains(const glm::vec3& point) const
{
    auto halfExtent = extent * 0.5f;
    return
        -halfExtent.x < point.x && point.x < halfExtent.x &&
        -halfExtent.y < point.y && point.y < halfExtent.y &&
        -halfExtent.z < point.z && point.z < halfExtent.z;
}

float get_orientation(const glm::vec3& v)
{
    return std::atan2(-v.z, v.x);
}

glm::vec3 from_polar(float angle, float magnitude)
{
    return glm::vec3{ glm::cos(angle), 0, glm::sin(angle) } * magnitude;
}

glm::vec3 scale_to(const glm::vec3& v, float length)
{
    return v * (length / glm::length(v));
}

float from_1920x1080(float base, float value)
{
    return value / 1920 * base;
}

glm::vec3 ray_plane_intersection(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const glm::vec3& planePoint, const glm::vec3& planeNormal)
{
    // FROM : https://stackoverflow.com/questions/69257700/finding-the-intersection-of-a-ray-and-a-plane-programmatically
    auto difference = planePoint - rayOrigin;
    auto dot0 = glm::dot(difference, planeNormal);
    auto dot1 = glm::dot(rayDirection, planeNormal);
    auto distance = dot0 / dot1;
    return rayOrigin + rayDirection * distance;
}

glm::vec3 get_random_vector(float minLength, float maxLength)
{
    auto& rng = Context::instance().rng;
    double theta = rng.range<double>(0, 1) * 2 * M_PI;
    auto length = rng.range(minLength, maxLength);
    return { length * (float)std::cos(theta), 0, length * (float)std::sin(theta) };
}

glm::vec4 hsv_to_color(float hue, float saturation, float value, float alpha)
{
    if (!hue && !saturation) {
        return { value, value, value, alpha };
    } else {
        float c = saturation * value;
        float x = c * (1.0f - std::abs(glm::mod(hue, 2.0f) - 1));
        float m = value - c;
        if        (hue < 1.0f) { return { c + m, x + m, m, alpha };
        } else if (hue < 2.0f) { return { x + m, c + m, m, alpha };
        } else if (hue < 3.0f) { return { m, c + m, x + m, alpha };
        } else if (hue < 4.0f) { return { m, x + m, c + m, alpha };
        } else if (hue < 5.0f) { return { x + m, m, c + m, alpha };
        } else                 { return { c + m, m, x + m, alpha };
        }
    }
}

bool combo_gui(const char* pLabel, size_t optionCount, const char* const* ppOptions, size_t* pSelectionIndex)
{
    assert(pLabel);
    assert(optionCount);
    assert(ppOptions);
    assert(pSelectionIndex);
    assert(*pSelectionIndex < optionCount);
    bool selectionMade = false;
    if (ImGui::BeginCombo(pLabel, ppOptions[*pSelectionIndex])) {
        for (size_t i = 0; i < optionCount; ++i) {
            ImGui::PushID((int)i);
            auto selected = (size_t)*pSelectionIndex == i;
            if (ImGui::Selectable(ppOptions[(size_t)i], selected)) {
                *pSelectionIndex = i;
                selectionMade = true;
            }
            if (selected) {
                ImGui::SetItemDefaultFocus();
            }
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    return selectionMade;
}

void transform_gui(const char* pName, gvk::math::Transform* pTransform)
{
    assert(pTransform);
    ImGui::PushID(pTransform);
    if (ImGui::CollapsingHeader(pName)) {
        ImGui::Indent();
        {
            ImGui::DragFloat3("translation", &pTransform->translation[0]);
            ImGui::DragFloat4("rotation", &pTransform->rotation[0]);
            ImGui::Indent();
            {
                auto eulerRad = glm::eulerAngles(pTransform->rotation);
                if (ImGui::DragFloat3("Euler(rad)", &eulerRad[0], 1.0f / 90.0f, -FLT_MAX, FLT_MAX)) {
                    pTransform->rotation = eulerRad;
                }
                auto eulerDeg = glm::degrees(eulerRad);
                if (ImGui::DragFloat3("Euler(deg)", &eulerDeg[0], 1.0f, -FLT_MAX, FLT_MAX)) {
                    pTransform->rotation = glm::wrapAngle(glm::radians(eulerDeg));
                }
            }
            ImGui::Unindent();
            ImGui::DragFloat3("scale", &pTransform->scale[0]);
        }
        ImGui::Unindent();
    }
    ImGui::PopID();
}

void reset_camera(gvk::math::Camera* pCamera, gvk::math::Camera::ProjectionMode projectionMode)
{
    assert(pCamera);
    pCamera->transform.translation = { 0, 965, 0 };
    pCamera->transform.rotation = glm::radians(glm::vec3(90, 0, 0));
    pCamera->transform.scale = { 1, 1, 1 };
    pCamera->aspectRatio = 16.0f / 9.0f;
    switch (projectionMode) {
    case gvk::math::Camera::ProjectionMode::Perspective: {
        pCamera->fieldOfView = 60.0f;
    } break;
    case gvk::math::Camera::ProjectionMode::Orthographic: {
        pCamera->fieldOfView = 2048.0f;
    } break;
    default: {
        assert(false);
    } break;
    }
    pCamera->nearPlane = 0.001f;
    pCamera->farPlane = 10000.0f;
    pCamera->projectionMode = projectionMode;
}

void reset_camera(gvk::math::Camera* pCamera)
{
    assert(pCamera);
    reset_camera(pCamera, pCamera->projectionMode);
}

void camera_gui(const char* pName, gvk::math::Camera* pCamera)
{
    assert(pCamera);
    ImGui::PushID(pCamera);
    if (ImGui::CollapsingHeader(pName)) {
        ImGui::Indent();
        {
            if (ImGui::Button("Reset")) {
                reset_camera(pCamera);
            }
            transform_gui("transform", &pCamera->transform);
            ImGui::DragFloat("aspectRatio", &pCamera->aspectRatio);
            ImGui::DragFloat("fieldOfView", &pCamera->fieldOfView);
            ImGui::DragFloat("nearPlane", &pCamera->nearPlane);
            ImGui::DragFloat("farPlane", &pCamera->farPlane);
            auto projectionModeIndex = (size_t)pCamera->projectionMode;
            static const std::array<const char*, 2> scProjectionModes{ "Perspective", "Orthographic", };
            if (combo_gui("projectionMode", scProjectionModes.size(), scProjectionModes.data(), &projectionModeIndex)) {
                reset_camera(pCamera, (gvk::math::Camera::ProjectionMode)projectionModeIndex);
            }
        }
        ImGui::Unindent();
    }
    ImGui::PopID();
}

} // namespace shape_shooter
