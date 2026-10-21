#include "view.hpp"
#include <cstdint>
#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
#include <vector>

using namespace choir;
int main(int argc,char** argv) {
    try {
        if(SDL_Init(0)!=0) throw std::runtime_error(SDL_GetError());
        struct Cleanup {~Cleanup(){SDL_Quit();}} cleanup;
        std::unique_ptr<SDL_Surface,decltype(&SDL_FreeSurface)> surface(
            SDL_CreateRGBSurfaceWithFormat(0,screenWidth,screenHeight,32,SDL_PIXELFORMAT_ARGB8888),SDL_FreeSurface);
        if(!surface) throw std::runtime_error(SDL_GetError());
        std::unique_ptr<SDL_Renderer,decltype(&SDL_DestroyRenderer)> renderer(SDL_CreateSoftwareRenderer(surface.get()),SDL_DestroyRenderer);
        if(!renderer) throw std::runtime_error(SDL_GetError());
        View view(renderer.get());Game g(713);std::set<std::uint64_t> frames;
        auto capture=[&](const std::string& name,bool title,bool help,bool chart) {
            view.draw(g,title,help,-1,-1,chart);
            std::vector<std::uint32_t> pixels(screenWidth*screenHeight);
            if(SDL_RenderReadPixels(renderer.get(),nullptr,SDL_PIXELFORMAT_ARGB8888,pixels.data(),screenWidth*4)!=0)
                throw std::runtime_error(SDL_GetError());
            std::uint64_t hash=1469598103934665603ULL;
            for(auto p:pixels) {hash^=p;hash*=1099511628211ULL;}
            if(argc==2 && SDL_SaveBMP(surface.get(),(std::string(argv[1])+"/"+name+".bmp").c_str())!=0)
                throw std::runtime_error(SDL_GetError());
            return hash;
        };
        frames.insert(capture("title",true,false,false));
        frames.insert(capture("help",true,true,false));
        const auto gameFrame=capture("initial-deck",false,false,false);frames.insert(gameFrame);
        frames.insert(capture("map",false,false,true));
        const auto originalCells=g.cells;
        for(auto& c:g.cells) if(!c.seen) c.tile=Tile::Shrine;
        g.enemies.push_back({{0,0},Kind::Legionary,70,{0,0},0});
        if(capture("fog-check",false,false,false)!=gameFrame) throw std::runtime_error("Fog revealed a hidden object or enemy.");
        g.cells=originalCells;
        for(Outcome outcome:{Outcome::Dead,Outcome::Possessed,Outcome::Escaped}) {
            g.outcome=outcome;
            frames.insert(capture(outcome==Outcome::Dead?"death":outcome==Outcome::Possessed?"possession":"escape",false,false,false));
        }
        if(frames.size()!=7 || g.turn!=0) throw std::runtime_error("Screen states must be distinct and rendering must not advance time.");
        std::cout<<"PASS: seven distinct screens, hidden-state masking, rendering leaves turn state unchanged\n";
        return 0;
    } catch(const std::exception& e) {std::cerr<<"Renderer FAIL: "<<e.what()<<'\n';return 1;}
}
