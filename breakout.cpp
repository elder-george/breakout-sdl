#include <windows.h>
#include <stdexcept>
#include <functional>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <map>
#include <vector>
#include <tuple>
#include <cstdio>
#include <cmath>
#include "sdlpp.h"
#include "events.h"

class Sprite{
    Sdl::SdlTexture& t;
public:
    SDL_Rect region;
    SDL_Point origin;
    SDL_Point pos;
    Sprite(Sdl::SdlTexture& t, SDL_Rect region, SDL_Point origin, SDL_Point pos):
        t(t), region(region), origin(origin), pos(pos)
    {}

    void Render(Sdl::SdlRenderer &renderer){
        auto dst = SDL_Rect{ pos.x - origin.x, pos.y - origin.y, region.w, region.h };
        renderer.Copy(t, &region, &dst);
    }

    SDL_Point GetSize() const{
        return SDL_Point{region.w, region.h};
    }
};

enum Origin{
    LEFT_TOP,
    CENTER,
    BOTTOM,
};

class SpriteAtlas{
    Sdl::SdlTexture& t;
    SDL_Rect region;
    std::vector<Sprite> sprites;
    static SDL_Point GetOriginCoords(Origin o, const SDL_Point& size){
        switch(o){
            case Origin::LEFT_TOP: return SDL_Point{0,0};
            case Origin::CENTER: return SDL_Point{size.x / 2, size.y/2};
            case Origin::BOTTOM:    return SDL_Point{size.x/2, size.y};
            default: throw std::runtime_error("Unexpected origin");
        }
    }


public:
    SpriteAtlas(Sdl::SdlTexture &t, SDL_Rect region, SDL_Point itemSize, Origin o):
        t(t), region(region)
    {
        sprites.reserve(region.w/itemSize.x * region.h/itemSize.y);
        SDL_Point origin = GetOriginCoords(o, itemSize);
        for(auto i = 0; i < region.w/itemSize.x; i++)
        for(auto j = 0; j < region.h/itemSize.y; j++){
            sprites.push_back(Sprite(t, 
                                    SDL_Rect{ region.x+i*itemSize.x, region.y+j * itemSize.y, itemSize.x, itemSize.y },
                                    origin,
                                    SDL_Point{ 0, 0 }));
        }
    }

    const Sprite & MakeSprite(size_t idx) const{
        return sprites[idx];
    }
};


enum Command{
    NONE,
    LEFT,
    RIGHT
};

struct BreakoutState{
    long score = 0;
    int lives = 3;
    bool lost = false;
    bool won = false;
};

class BreakoutLevel{
    const SDL_Rect gameArea;
    const SpriteAtlas &brickAtlas;
    const SpriteAtlas &ballAtlas;
    const SpriteAtlas &paddleAtlas;
public:    

    Sprite paddle;
    Sprite ball;
    std::vector<Sprite> bricks;
    std::vector<int>    hitpoints;
    std::vector<Sprite*> sprites;
    SDL_Point velocity;

    BreakoutLevel(const SDL_Rect gameArea,
                const SpriteAtlas &brickAtlas, 
                const SpriteAtlas &ballAtlas,
                const SpriteAtlas &paddleAtlas,
                const std::vector<std::tuple<int, int, int>> &brickCoords):
            gameArea(gameArea),
            brickAtlas(brickAtlas), ballAtlas(ballAtlas), paddleAtlas(paddleAtlas),
            paddle(paddleAtlas.MakeSprite(0)),
            ball(ballAtlas.MakeSprite(0)),
            velocity({3,-6})
    {
        bricks.reserve(brickCoords.size());
        hitpoints.reserve(brickCoords.size());
        sprites.reserve(brickCoords.size() + 2);
        for(auto &coord : brickCoords){
            auto brick = brickAtlas.MakeSprite(std::get<2>(coord));
            brick.pos.x = std::get<0>(coord) * brick.GetSize().x;
            brick.pos.y = std::get<1>(coord) * brick.GetSize().y;
            bricks.push_back(brick);
            hitpoints.push_back(std::get<2>(coord));
            sprites.push_back(&(bricks[bricks.size()-1]));
        }
        paddle.pos.x = gameArea.w/2;
        paddle.pos.y = gameArea.h - paddle.GetSize().y/2;
        sprites.push_back(&paddle);

        ball.pos.x = gameArea.w/2;
        ball.pos.y = gameArea.h - paddle.GetSize().y - ball.GetSize().y - 5;
        sprites.push_back(&ball);
    }

    void Update(Command &cmd, BreakoutState& gameState){
        // move paddle
        auto paddleW = paddle.GetSize().x;
        const auto PADDLE_SPEED = 10;
        if (cmd == LEFT && paddle.pos.x - paddleW/2 >= PADDLE_SPEED){
            paddle.pos.x -= PADDLE_SPEED;
        }
        if (cmd == RIGHT && paddle.pos.x + paddleW/2 < gameArea.w - PADDLE_SPEED){
            paddle.pos.x += PADDLE_SPEED;
        }

        auto radius = ball.region.w/2;
        // check if hits walls and bounce
        auto newBallPos = SDL_Point{ ball.pos.x + velocity.x, ball.pos.y + velocity.y };
        if (newBallPos.x <= radius || newBallPos.x >= gameArea.w - radius){
            velocity.x = -velocity.x;
        }
        if (newBallPos.y <= radius){
            velocity.y = -velocity.y;
        }

        if (newBallPos.y >= gameArea.h - radius){
            gameState.lives--;
            ball.pos = SDL_Point { gameArea.w/2, gameArea.h - paddle.GetSize().y - ball.GetSize().y - 5};
            velocity = SDL_Point{3,-6};
            return;
        }

        // check if hits paddle and bounce.
        SDL_Rect ballBounds{newBallPos.x - radius, newBallPos.y - radius, ball.region.w, ball.region.h };
        SDL_Rect paddleBounds{paddle.pos.x - paddle.region.w/2, paddle.pos.y - paddle.region.h/2, 
                                paddle.region.w, paddle.region.h};
        if (SDL_HasIntersection(&ballBounds, &paddleBounds)){
            //TODO: change velocity.x basing on the hit position on the paddle.
            velocity.y = -velocity.y;
        }
        // find if there's intersection with bricks
        for(size_t i = 0; i < bricks.size(); i++){
            auto &brick = bricks[i];
            auto brickBounds = SDL_Rect{ brick.pos.x - brick.origin.x, brick.pos.y - brick.origin.y, 
                                        brick.region.w, brick.region.h };
            if (SDL_HasIntersection(&ballBounds, &brickBounds) && hitpoints[i] >= 0){
                // hitting from the right
                if (ballBounds.x >= brickBounds.w + brickBounds.x && velocity.x < 0){
                    velocity.x = -velocity.x;
                }
                // hitting from the left
                if (ballBounds.x < brickBounds.x && velocity.x > 0){
                    velocity.x = -velocity.x;
                }
                // hitting from the bottom
                if (ballBounds.y >= brickBounds.h + brickBounds.y - 5 && velocity.y < 0){
                    velocity.y = -velocity.y;
                }
                // hitting from the top
                if (ballBounds.y < brickBounds.y && velocity.y > 0){
                    velocity.y = -velocity.y;
                }
                hitpoints[i]--;
                gameState.score+=5;
                if (hitpoints[i] < 0){
                    sprites[i] = nullptr;
                }
                break;
            }
        }
        // update ball pos
        ball.pos = newBallPos;
    }
};

class BreakoutGame{
public:    

    BreakoutGame()
    {}

    void Play(
              BreakoutLevel &level,
              Sdl::SdlRenderer &renderer, Sdl::SdlTtfFont &font){
        BreakoutState gameState;
        Sdl::GameLoop loop;
        auto cmd = Command::NONE;
        loop.On<SDL_QUIT>([&](SDL_QuitEvent& e){ loop.Stop(); });
        loop.On<SDL_KEYDOWN>([&](SDL_KeyboardEvent& e){
            switch(e.keysym.sym){
                case SDLK_LEFT:
                    cmd = Command::LEFT; break;
                case SDLK_RIGHT:
                    cmd = Command::RIGHT; break;
                case SDLK_ESCAPE:
                    loop.Stop();
            }
        });
        loop.On<SDL_KEYUP>([&](SDL_KeyboardEvent& e){
            cmd = Command::NONE;
        });

        loop.On<SDL_CONTROLLERAXISMOTION>([&](SDL_ControllerAxisEvent& e){
            if (e.axis %2 == 0){
                if (e.value < 0){
                    cmd = Command::LEFT;
                }
                if (e.value > 0){
                    cmd = Command::RIGHT;
                }
                if (e.value == 0){
                    cmd = Command::NONE;
                }
            }
        });
        auto lastRun = ::GetTickCount();
        loop.Run([&](){
            auto currentRun = ::GetTickCount();
            if (currentRun - lastRun > 20){
                renderer.Clear();
        	    for(auto sprite : level.sprites){
                    if (sprite){
                        sprite->Render(renderer);
                    }
                }
                char score[100];
                std::sprintf(score, "%ld", gameState.score);
                auto renderedScore = font.RenderTextSolid(score, SDL_Color{255,255,255, 0} )
                                        .SurfaceToTexture(renderer);
                auto scoreSize = renderedScore.GetSize();
                auto scoreRect = SDL_Rect{0,0, scoreSize.x, scoreSize.y};
                renderer.Copy(renderedScore, &scoreRect, &scoreRect);

                
            	renderer.Present();
                level.Update(cmd, gameState);
                if (gameState.lives <= 0){
                    MessageBox(NULL, "GameOver", "GameOver", MB_OK);
                    loop.Stop();
                }
                lastRun = currentRun;
            }
        });
    }
};


int CALLBACK WinMain(HINSTANCE hinst, HINSTANCE hprev, LPSTR args, int cmdshow){
    try{
        Sdl::Sdl sdl;
        Sdl::SdlGameController controller(0);   // ignore errors if it's not connected.
        Sdl::SdlTtf ttf;
        Sdl::SdlWindow win("SDL", 100, 100, 640, 480);
        Sdl::SdlTtfFont font("./assets/SourceSansPro-Regular.ttf", 18);
        Sdl::SdlRenderer renderer(win);
        Sdl::SdlSurface surface = Sdl::SdlSurface::Load("./assets/breakout_pieces_1.png");
        Sdl::SdlTexture texture = surface.SurfaceToTexture(renderer);

        SpriteAtlas brickAtlas(texture, SDL_Rect{8, 6, 32, 144}, SDL_Point{32, 20}, Origin::LEFT_TOP);
        SpriteAtlas ballAtlas(texture, SDL_Rect{ 48, 136, 52, 9}, SDL_Point{9,9}, Origin::CENTER);
        SpriteAtlas paddleAtlas(texture, SDL_Rect{ 48, 72, 64, 40}, SDL_Point{64, 20}, Origin::CENTER);

        std::vector<std::tuple<int, int, int>> bricks;
        for(auto i = 0; i < 5; i++)
        for(auto j = 0; j < (640 / 32); j++)
        {
            bricks.push_back(std::make_tuple(j, i + 2, 3-(i % 3)));
        }

        BreakoutLevel level(SDL_Rect{0,0,640, 480}, brickAtlas, ballAtlas, paddleAtlas, bricks);
        
        BreakoutGame game;
        game.Play(level, renderer, font);

        return 0;
    }catch(std::exception &e){
        MessageBox(NULL, e.what(), "Error", MB_ICONERROR|MB_OK);
        return -1;
    }
}