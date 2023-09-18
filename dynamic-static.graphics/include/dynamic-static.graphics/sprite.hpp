
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
#include "dynamic-static.graphics/renderer.hpp"

#include <unordered_map>

namespace dst {
namespace gfx {

class Sprite final
{
public:
    class Pool final
    {
    public:
        struct CreateInfo
        {
            uint32_t filePathCount { };
            const char* pFilePaths { };
        };

        Pool() = default;
        static void create(const CreateInfo& createInfo, Pool* pPool);

    private:
        Pool(const Pool&) = delete;
        Pool& operator=(const Pool&) = delete;
    };

    Sprite() = default;

private:
    Sprite(const Sprite&) = delete;
    Sprite& operator=(const Sprite&) = delete;
};

template <>
class Renderer<Sprite> final
{
public:
    struct CreateInfo
    {
        uint32_t filePathCount { };
        const char* const* ppFilePaths { };
    };

    static VkResult create(const gvk::Context& gvkContext, const CreateInfo& createInfo, Renderer<Sprite>* pRenderer);
    ~Renderer();
    void reset();

private:
    std::unordered_map<std::string, gvk::ImageView> mImages;
};

} // namespace gfx
} // namespace dst
