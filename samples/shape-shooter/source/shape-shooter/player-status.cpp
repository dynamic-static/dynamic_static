
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

#include "shape-shooter/player-status.hpp"
#include "shape-shooter/context.hpp"

namespace shape_shooter {

void PlayerStatus::update(Context& gameContext)
{
    assert(gameContext.pPlayerShip);
    switch (gameContext.gameState) {
    case GameState::Attract: {
    } break;
    case GameState::Playing: {
        switch (gameContext.pPlayerShip->get_state()) {
        case Entity::State::Inactive: {
            if (mLives) {

            }
        } break;
        case Entity::State::Spawning: {

        } break;
        case Entity::State::Active: {

        } break;
        case Entity::State::Dying: {

        } break;
        default: {
            assert(false);
        } break;
        }
    } break;
    case GameState::Paused: {
    } break;
    case GameState::GameOver: {
    } break;
    default: {
        assert(false);
    } break;
    }
}

} // namespace shape_shooter
