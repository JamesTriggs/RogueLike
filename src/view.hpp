#pragma once
#include "game.hpp"
#include <SDL.h>
#include <string>

namespace choir {
constexpr int screenWidth=320,screenHeight=224;
class View {
public:
    explicit View(SDL_Renderer* renderer): renderer(renderer) {}
    void draw(const Game& game,bool title,bool help,int mouseX=-1,int mouseY=-1,bool chart=false);
private:
    SDL_Renderer* renderer;
    void rect(int x,int y,int w,int h,SDL_Color colour);
    void line(int x,int y,int xx,int yy,SDL_Color colour);
    void text(int x,int y,const std::string& value,SDL_Color colour,int scale=1);
    void wrapped(int x,int y,const std::string& value,int columns,SDL_Color colour,int maxLines=3);
    void sprite(int x,int y,const char* const* pixels,const std::array<SDL_Color,10>& palette);
    void tile(int x,int y,Tile type,int hash,bool visible);
    void actor(int x,int y,int kind,bool alert);
    void titleScreen(const Game& game);
    void endScreen(const Game& game);
    void helpScreen();
    void mapScreen(const Game& game);
};
}
