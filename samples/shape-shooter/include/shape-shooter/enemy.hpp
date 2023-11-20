
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
    Enemy(Sprite sprite);

    uint64_t get_type_id() const override final;
    uint32_t get_point_value() const;
    bool is_active() const;
    void handle_collision(const Enemy& other);
    void update() override final;
    void draw() const override final;

private:
    class Behavior
    {
    public:
        Behavior() = default;
        virtual void update(Enemy& enemy) = 0;
    private:
        Behavior(const Behavior&) = delete;
        Behavior& operator=(const Behavior&) = delete;
    };

    class FollowPlayer final
        : public Behavior
    {
    public:
        FollowPlayer() = default;
        void update(Enemy& enemy) override final;
    private:
        float mAcceleration{ 0.9f };
    };

    class MoveRandomly final
        : public Behavior
    {
    public:
        MoveRandomly();
        void update(Enemy& enemy) override final;
    private:
        float mDirection{ };
        uint32_t mUpdateCounter{ };
    };

    std::vector<std::unique_ptr<Behavior>> mBehaviors;
    float mTimeUntilStart{ 1.0f };
    uint32_t mPointValue{ 1 };
};

} // namespace shape_shooter
