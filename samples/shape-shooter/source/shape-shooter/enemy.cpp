
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

#include "shape-shooter/enemy.hpp"
#include "shape-shooter/context.hpp"
#include "shape-shooter/utilities.hpp"

#include "glm/gtx/fast_trigonometry.hpp"

namespace shape_shooter {

Enemy::Behavior::~Behavior()
{
}

uint32_t Enemy::FollowPlayer::get_point_value() const
{
    return 2;
}

void Enemy::FollowPlayer::update(Context& gameContext, Enemy& enemy)
{
    assert(gameContext.pPlayerShip);
    const auto& playerShip = *gameContext.pPlayerShip;
    if (playerShip.get_state() != State::Inactive) {
        auto direction = playerShip.position - enemy.position;
        if (direction.x || direction.y || direction.z) {
            direction = glm::normalize(direction);
        }
        enemy.velocity += direction * (mAcceleration / OneOverSixty);
    }
    if (enemy.velocity.x || enemy.velocity.y) {
        enemy.orientation = std::atan2(-enemy.velocity.z, enemy.velocity.x);
    }
}

uint32_t Enemy::MoveRandomly::get_point_value() const
{
    return 1;
}

void Enemy::MoveRandomly::update(Context& gameContext, Enemy& enemy)
{
    // TODO : Hardcoded values...
    directionTimer -= gameContext.programClock.elapsed<gvk::system::Seconds<float>>();
    if (0 <= directionTimer) {
        direction += gameContext.rng.range(-0.1f, 0.1f);
        direction = glm::wrapAngle(direction);
        directionTimer = 0.1f;
    }
    enemy.velocity += from_polar(direction, 0.4f / OneOverSixty);
    enemy.orientation -= 0.05f;
    if (!gameContext.playField.contains(enemy.position)) {
        direction = std::atan2(-enemy.position.z, -enemy.position.x) + gameContext.rng.range(-glm::half_pi<float>(), glm::half_pi<float>());
    }
#if 0
    if (enemy.position.x <= -gameContext.playField.x * 0.5f || gameContext.playField.x * 0.5f <= enemy.position.x ||
        enemy.position.z <= -gameContext.playField.y * 0.5f || gameContext.playField.y * 0.5f <= enemy.position.z) {
        mDirection = std::atan2(-enemy.position.z, -enemy.position.x) + rng.range(-glm::half_pi<float>(), glm::half_pi<float>());
    }
#endif
}

Enemy::Enemy(Context& gameContext, Sprite sprite, std::unique_ptr<Behavior>&& upBehavior)
    : Entity(sprite)
{
    assert(upBehavior);
    mBehaviors.push_back(std::move(upBehavior));
    assert(!gameContext.spriteRenderers.empty());
    radius = get_sprite_extent(gameContext.spriteRenderers[0]).x * 0.5f;
    color = gvk::math::Color::Transparent;
}

uint64_t Enemy::get_type_id() const
{
    return shape_shooter::get_type_id<Enemy>();
}

uint32_t Enemy::get_point_value() const
{
    uint32_t pointValue = 0;
    for (const auto& upBehavior : mBehaviors) {
        pointValue += upBehavior->get_point_value();
    }
    return pointValue;
}

bool Enemy::is_active() const
{
    return mTimeUntilStart <= 0;
}

void Enemy::handle_collision(const Enemy& other)
{
    auto direction = position - other.position;
    velocity += 10.0f * direction / (glm::length2(direction) + 1);
}

void Enemy::update(Context& gameContext)
{
    auto deltaTime = gameContext.gameClock.elapsed<gvk::system::Seconds<float>>();
    if (is_active()) {
        for (const auto& upBehavior : mBehaviors) {
            upBehavior->update(gameContext, *this);
        }
    } else {
        mTimeUntilStart = glm::max(0.0f, mTimeUntilStart - deltaTime);
        color = gvk::math::Color::White * (1.0f - mTimeUntilStart);
    }
    // TODO : Hardcoded values...
    position += velocity * deltaTime;
    const auto& halfPlayFieldExtent = gameContext.playField.extent * 0.5f;
    position = glm::clamp(position, -halfPlayFieldExtent, halfPlayFieldExtent);
    velocity *= 0.8f;
}

void Enemy::draw(dst::gfx::SpriteRenderer& spriteRenderer) const
{
    Entity::draw(spriteRenderer);
}

} // namespace shape_shooter
