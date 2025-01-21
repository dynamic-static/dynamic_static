
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
#include "shape-shooter/entity.hpp"

namespace shape_shooter {

class Enemy final
    : public Entity
{
public:
    class Behavior
    {
    public:
        Behavior() = default;
        virtual ~Behavior() = 0;
        virtual uint32_t get_point_value() const = 0;
        virtual void update(Context& gameContext, Enemy& enemy) = 0;
    private:
        Behavior(const Behavior&) = delete;
        Behavior& operator=(const Behavior&) = delete;
    };

    class FollowPlayer final
        : public Behavior
    {
    public:
        uint32_t get_point_value() const override final;
        void update(Context& gameContext, Enemy& enemy) override final;
    private:
        float mAcceleration{ 0.9f };
    };

    class MoveRandomly final
        : public Behavior
    {
    public:
        uint32_t get_point_value() const override final;
        void update(Context& gameContext, Enemy& enemy) override final;
        float directionTimer{ };
        float direction{ };
    };

    Enemy(Context& gameContext, Sprite sprite, std::unique_ptr<Behavior>&& upBehavior);

    uint64_t get_type_id() const override final;
    uint32_t get_point_value() const;
    bool is_active() const;
    void handle_collision(const Enemy& other);
    void update(Context& gameContext) override final;
    void draw(dst::gfx::SpriteRenderer& spriteRenderer) const override final;

private:
    std::vector<std::unique_ptr<Behavior>> mBehaviors;
    float mTimeUntilStart{ 1.0f };
};

} // namespace shape_shooter
