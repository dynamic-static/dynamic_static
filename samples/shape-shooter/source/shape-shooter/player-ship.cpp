
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

#include "shape-shooter/player-ship.hpp"
#include "shape-shooter/bullet.hpp"
#include "shape-shooter/context.hpp"
#include "shape-shooter/utilities.hpp"

namespace shape_shooter {

PlayerShip::PlayerShip()
    : Entity(Sprite::PlayerShip)
{
}

uint64_t PlayerShip::get_type_id() const
{
    return shape_shooter::get_type_id<PlayerShip>();
}

void PlayerShip::spawn()
{
    assert(mState == State::Inactive);
    if (Context::instance().scoreBoard.get_lives()) {
        mState = State::Spawning;
    }
}

void PlayerShip::kill()
{
    // TODO : Reset spawn timer
    Context::instance().scoreBoard.subtract_life();
    auto explosionColor = glm::vec4(0.8f, 0.8f, 0.4f, 1.0f);
    for (uint32_t i = 0; i < 1200; ++i) {
        float speed = 18.0f * (1.0f - 1.0f / Context::instance().rng.range(1.0f, 10.0f));
        Particle particle{ };
        particle.position = position;
        particle.velocity = get_random_vector(speed, speed) / OneOverSixty;
        particle.duration = 190.0f * OneOverSixty;
        particle.scale *= 1.5f;
        particle.type = Particle::Type::None;
        particle.color = glm::lerp(gvk::math::Color::White, explosionColor, Context::instance().rng.range(0.0f, 1.0f));
        Context::instance().particleManager.add(particle);
    }
    mState = State::Inactive;
    spawn();
}

void PlayerShip::update(Context& gameContext)
{
    switch (mState) {
    case State::Inactive: {

    } break;
    case State::Spawning: {
        // TODO : Apply force to grid
        // TODO : SFX
        // TODO : Spawn timer
        position = { };
        mState = State::Active;
    } break;
    case State::Active: {
        auto deltaTime = gameContext.gameClock.elapsed<gvk::system::Seconds<float>>();

        auto aimDirection = gameContext.inputManager.get_aim_direction();
        if (0 < glm::length2(aimDirection) && mCooldownTimer <= 0) {
            mCooldownTimer = mCooldownTime;
            // TODO : Sort out inconsistent rotations from get_orientation()
            // auto bulletOrientation = get_orientation(aimDirection);
            auto aimAngle = std::atan2(aimDirection.z, aimDirection.x);
            auto aimRotation = glm::angleAxis(aimAngle, glm::vec3{ 0, 1, 0 });
            (void)aimRotation;

            float bulletSpread = gameContext.rng.range(-0.04f, 0.04f) + gameContext.rng.range(-0.04f, 0.04f);
            auto bulletVelocity = from_polar(aimAngle + bulletSpread, 11.0f / OneOverSixty);

            gameContext.entityManager.create_entity<Bullet>(position + glm::vec3{ 35, 0, -8 } *aimRotation, bulletVelocity);
            gameContext.entityManager.create_entity<Bullet>(position + glm::vec3{ 35, 0,  8 } *aimRotation, bulletVelocity);
            gameContext.audio.play(SoundEffect::Shot);
        }
        mCooldownTimer -= deltaTime;

        velocity += gameContext.inputManager.get_movement_direction() * mSpeed * deltaTime;
        position += velocity;
        const auto& halfPlayFieldExtent = gameContext.playField.extent * 0.5f;
        position = glm::clamp(position, -halfPlayFieldExtent, halfPlayFieldExtent);
        if (0.0f < glm::length2(velocity)) {
            orientation = get_orientation(velocity);
        }
        make_exhaust_fire();
        velocity = { };
    } break;
    case State::Dying: {

    } break;
    default: {
        assert(false);
    } break;
    }
}

void PlayerShip::draw(dst::gfx::SpriteRenderer& spriteRenderer) const
{
    if (mState != State::Inactive) {
        Entity::draw(spriteRenderer);
    }
}

void PlayerShip::make_exhaust_fire()
{
    auto totalTime = Context::instance().gameClock.total<gvk::system::Seconds<float>>();
    (void)totalTime;
    if (0.1f < glm::length2(velocity)) {
        // TODO : Documentation
        orientation = get_orientation(velocity);
        auto particleRotation = glm::angleAxis(orientation, glm::vec3{ 0, 1, 0 });
        (void)particleRotation;

        // TODO : Update this comment to be world units/second
        // The primary velocity of the paritcles is 3 pixels/frame in the direction
        //  opposite to which the ship is travelling.
        auto baseVelocity = scale_to(velocity, -3.0f);

        // Calculate the sideways velocity for the two side streams.  The direction is
        //  perpendicular to the ship's velocity and the magnitude varies sinusoidally.
        auto perpendicularVelocity = glm::vec3(baseVelocity.z, baseVelocity.y, -baseVelocity.x) * (0.6f * std::sin(totalTime * 10.0f));

        // TODO : Documentation
        Particle particle{ };
        particle.type = Particle::Type::Enemy;
        particle.position = position; // +glm::vec3{ -25, 0, 0 } *particleRotation; // Position of the ship's exhaust pipe
        particle.duration = 60.0f * OneOverSixty;
        particle.scale = { 0.5f, 1.0f, 1.0f };
        auto& particleManager = Context::instance().particleManager;
        float alpha = 0.7f;

        // Middle particle stream
        auto middleColor = glm::vec4(1.0f, 187.0f / 255.0f, 30.0f / 255.0f, 1.0f); // Orange yellow
        particle.velocity = (baseVelocity + get_random_vector(0, 1)) / OneOverSixty;
        particle.sprite = Sprite::Laser;
        particle.color = gvk::math::Color::White * alpha;
        particleManager.add(particle);
        particle.sprite = Sprite::Glow;
        particle.color = middleColor * alpha;
        particleManager.add(particle);

        // Side particle streams
        auto perpendicularVelocity0 = baseVelocity + perpendicularVelocity + get_random_vector(0.0f, 0.3f);
        auto perpendicularVelocity1 = baseVelocity - perpendicularVelocity + get_random_vector(0.0f, 0.3f);
        particle.sprite = Sprite::Laser;
        particle.color = gvk::math::Color::White * alpha;
        particle.velocity = perpendicularVelocity0 / OneOverSixty;
        particleManager.add(particle);
        particle.velocity = perpendicularVelocity1 / OneOverSixty;
        particleManager.add(particle);

        auto sideColor = glm::vec4(200.0f / 255.0f, 38.0f / 255.0f, 9.0f / 255.0f, 1.0f); // Deep red
        particle.sprite = Sprite::Glow;
        particle.color = sideColor * alpha;
        particle.velocity = perpendicularVelocity0 / OneOverSixty;
        particleManager.add(particle);
        particle.velocity = perpendicularVelocity1 / OneOverSixty;
        particleManager.add(particle);
    }
}

} // namespace shape_shooter
