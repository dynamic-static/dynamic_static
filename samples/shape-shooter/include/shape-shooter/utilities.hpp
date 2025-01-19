
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

#include "shape-shooter/defines.hpp"

namespace shape_shooter {

struct PlayField
{
    bool contains(const glm::vec3& point) const;
    glm::vec3 extent{ };
};

float get_orientation(const glm::vec3& v);
glm::vec3 from_polar(float angle, float magnitude);
glm::vec3 scale_to(const glm::vec3& v, float length);
float from_1920x1080(float base, float value);
glm::vec3 ray_plane_intersection(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const glm::vec3& planePoint, const glm::vec3& planeNormal);
glm::vec3 get_random_vector(float minLength, float maxLength);
glm::vec4 hsv_to_color(float hue, float saturation, float value, float alpha = 1.0f);
bool combo_gui(const char* pEnumName, size_t enumeratorCount, const char* const* pEnumeratorNames, size_t* pSelectionIndex);
void transform_gui(const char* pName, gvk::math::Transform* pTransform);
void reset_camera(gvk::math::Camera* pCamera, gvk::math::Camera::ProjectionMode projectionMode);
void reset_camera(gvk::math::Camera* pCamera);
void camera_gui(const char* pName, gvk::math::Camera* pCamera);

template <typename T>
inline uint64_t get_type_id()
{
    static const T* sPtr;
    return (uint64_t)&sPtr;
}

} // namespace shape_shooter
