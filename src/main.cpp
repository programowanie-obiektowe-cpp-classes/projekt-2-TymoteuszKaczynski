#include <SDL.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>

class Food
{
public:
    Food(int x, int y) : position({x, y}) {}

    SDL_Rect getPosition() const { return {position.x, position.y, 20, 20}; }

    void render(SDL_Renderer* renderer) const
    {
        SDL_Rect rect = {position.x, position.y, 20, 20};
        SDL_SetRenderDrawColor(renderer, 255, 60, 0, 255);
        SDL_RenderFillRect(renderer, &rect);
    }

    Food createFood(int width, int height)
    {
        int x = (std::rand() % (width / 20)) * 20;
        int y = (std::rand() % (height / 20)) * 20;
        return Food(x, y);
    }

private:
    SDL_Point position;
};

class Snake
{
public:
    Snake()
    {
        body.push_back({100, 100, 20, 20});
        direction = {20, 0};
    }

    void move()
    {
        for (int i = body.size() - 1; i > 0; --i)
        {
            body[i] = body[i - 1];
        }
        body[0].x += direction.x;
        body[0].y += direction.y;
    }

    void grow()
    {
        SDL_Rect lastSegment       = body.back();
        SDL_Rect secondLastSegment = body[body.size() - 2];
        // check for proper direction for new segment
        int growDirectionX = lastSegment.x - secondLastSegment.x;
        int growDirectionY = lastSegment.y - secondLastSegment.y;

        SDL_Rect newSegment = lastSegment;
        newSegment.x += growDirectionX;
        newSegment.y += growDirectionY;

        body.push_back(newSegment);
    }

    void render(SDL_Renderer* renderer)
    {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        for (int i = 0; i < body.size(); ++i)
        {
            SDL_RenderFillRect(renderer, &body[i]);
        }
    }

    SDL_Rect getHeadPosition() const { return body.front(); }

    SDL_Point getDirection() const { return direction; }

    void setDirection(SDL_Point dir) { direction = dir; }

    bool checkSelfCollision() const
    {
        for (int i = 2; i < body.size(); ++i) // check from 3rd segment because second is always safe therefore i=2
        {
            if (body[0].x == body[i].x && body[0].y == body[i].y)
            {
                return true;
            }
        }
        return false;
    }

private:
    std::vector< SDL_Rect > body;
    SDL_Point               direction;
};

class Game
{
public:
    Game(int w, int h) : width(w), height(h), running(true), foodSpawnCounterTimeBetween(0), foodSpawnFrequency(40)
    {
        SDL_Init(SDL_INIT_VIDEO);
        window   = SDL_CreateWindow("Game", 400, 150, width, height, 0);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
        std::srand(std::time(nullptr));
        spawnFood();
    }

    ~Game()
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void run()
    {
        while (running)
        {
            handleEvents();
            update();
            render();
            SDL_Delay(100);
        }
    }

private:
    void handleEvents()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN)
            {
                SDL_Point newDirection = {0, 0};
                switch (event.key.keysym.sym)
                {
                case SDLK_w:
                    newDirection = {0, -20};
                    break;
                case SDLK_s:
                    newDirection = {0, 20};
                    break;
                case SDLK_a:
                    newDirection = {-20, 0};
                    break;
                case SDLK_d:
                    newDirection = {20, 0};
                    break;
                }

                if ((newDirection.x != 0 && snake.getDirection().x == 0) ||
                    (newDirection.y != 0 && snake.getDirection().y == 0))
                {
                    snake.setDirection(newDirection);
                }
            }
        }
    }

    void update()
    {
        snake.move();
        //iterator has to be used for easier erase call
        for (auto it = foods.begin(); it != foods.end();)
        {
            SDL_Rect headPosition = snake.getHeadPosition();
            SDL_Rect foodPosition = it->getPosition();

            if (headPosition.x == foodPosition.x && headPosition.y == foodPosition.y)
            {
                snake.grow();
                it = foods.erase(it);
            }
            else
            {
                ++it;
            }
        }
        //border collision check
        if (snake.getHeadPosition().x < 0 || snake.getHeadPosition().y < 0 ||
            snake.getHeadPosition().x + snake.getHeadPosition().w > width ||
            snake.getHeadPosition().y + snake.getHeadPosition().h > height)
        {
            running = false;
        }
        
        if (snake.checkSelfCollision() == true)
        {
            running = false;
        }

        if (++foodSpawnCounterTimeBetween >= foodSpawnFrequency)
        {
            spawnFood();
            foodSpawnCounterTimeBetween = 0;
        }
    }

    void render()
    {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        snake.render(renderer);

        for (int i = 0; i < foods.size(); ++i)
        {
            foods[i].render(renderer);
        }

        SDL_RenderPresent(renderer);
    }

    void spawnFood()
    {

        Food foodTempInstance(0, 0);
        Food newFood = foodTempInstance.createFood(width, height);
        foods.push_back(newFood);
    }

    SDL_Window*         window;
    SDL_Renderer*       renderer;
    Snake               snake;
    std::vector< Food > foods;
    int                 foodSpawnCounterTimeBetween;
    const int           foodSpawnFrequency;
    int                 width, height;
    bool                running;
};

int main(int argc, char* argv[]) // neccessary for SDL
{
    Game game(800, 600);
    game.run();
    return 0;
}
