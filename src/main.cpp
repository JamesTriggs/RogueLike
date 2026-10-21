#include "view.hpp"
#include <charconv>
#include <chrono>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

namespace {
using namespace choir;
std::optional<Action> actionFor(SDL_Keycode key) {
    switch(key) {
    case SDLK_UP:case SDLK_w:return Action::North;
    case SDLK_DOWN:case SDLK_s:return Action::South;
    case SDLK_LEFT:case SDLK_a:return Action::West;
    case SDLK_RIGHT:case SDLK_d:return Action::East;
    case SDLK_SPACE:case SDLK_PERIOD:return Action::Wait;
    case SDLK_e:return Action::Interact;
    case SDLK_m:return Action::Heal;
    case SDLK_r:return Action::Eat;
    case SDLK_f:return Action::Fire;
    case SDLK_t:return Action::Throw;
    case SDLK_c:return Action::Sneak;
    default:return std::nullopt;
    }
}
void saveFrame(SDL_Renderer* renderer,const std::string& path) {
    int w=0,h=0;
    if(SDL_GetRendererOutputSize(renderer,&w,&h)!=0) throw std::runtime_error(SDL_GetError());
    std::unique_ptr<SDL_Surface,decltype(&SDL_FreeSurface)> surface(
        SDL_CreateRGBSurfaceWithFormat(0,w,h,32,SDL_PIXELFORMAT_ARGB8888),SDL_FreeSurface);
    if(!surface || SDL_RenderReadPixels(renderer,nullptr,SDL_PIXELFORMAT_ARGB8888,surface->pixels,surface->pitch)!=0)
        throw std::runtime_error(SDL_GetError());
    if(SDL_SaveBMP(surface.get(),path.c_str())!=0) throw std::runtime_error(SDL_GetError());
}
}
int main(int argc,char** argv) {
    try {
        std::uint32_t seed=static_cast<std::uint32_t>(std::chrono::system_clock::now().time_since_epoch().count());
        bool smoke=false,captureTitle=false,captureHelp=false,small=false;
        std::string screenshot;
        for(int i=1;i<argc;++i) {
            std::string arg=argv[i];
            if(arg=="--seed" && i+1<argc) {
                std::string value=argv[++i];auto result=std::from_chars(value.data(),value.data()+value.size(),seed);
                if(result.ec!=std::errc() || result.ptr!=value.data()+value.size()) throw std::runtime_error("Seed must be an unsigned 32-bit integer.");
            } else if((arg=="--smoke" || arg=="--capture-title" || arg=="--capture-help") && i+1<argc) {
                smoke=true;captureTitle=arg=="--capture-title";captureHelp=arg=="--capture-help";screenshot=argv[++i];
            } else if(arg=="--small") {
                small=true;
            } else if(arg=="--help") {
                std::cout<<"Choir of Ruin\nUsage: choir [--seed N] [--small] [--smoke frame.bmp | --capture-title frame.bmp | --capture-help frame.bmp]\n";return 0;
            } else throw std::runtime_error("Unknown or incomplete option: "+arg);
        }
        SDL_SetMainReady();
        if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS)!=0) throw std::runtime_error(SDL_GetError());
        struct Cleanup {~Cleanup(){SDL_Quit();}} cleanup;
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,"0");
        std::unique_ptr<SDL_Window,decltype(&SDL_DestroyWindow)> window(SDL_CreateWindow(
            "Choir of Ruin",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,small?640:1280,small?448:896,
            SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI),SDL_DestroyWindow);
        if(!window) throw std::runtime_error(SDL_GetError());
        SDL_SetWindowMinimumSize(window.get(),640,448);
        std::unique_ptr<SDL_Renderer,decltype(&SDL_DestroyRenderer)> renderer(
            SDL_CreateRenderer(window.get(),-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC),SDL_DestroyRenderer);
        if(!renderer) renderer.reset(SDL_CreateRenderer(window.get(),-1,SDL_RENDERER_SOFTWARE));
        if(!renderer) throw std::runtime_error(SDL_GetError());
        if(SDL_RenderSetLogicalSize(renderer.get(),screenWidth,screenHeight)!=0) throw std::runtime_error(SDL_GetError());
        SDL_RenderSetIntegerScale(renderer.get(),SDL_TRUE);
        Game game(seed);View view(renderer.get());
        bool running=true,title=true,help=captureHelp,chart=false;
        int frames=0,mouseX=-1,mouseY=-1;
        constexpr SDL_Keycode sequence[]={SDLK_RETURN,SDLK_F1,SDLK_ESCAPE,
            SDLK_d,SDLK_d,SDLK_d,SDLK_d,SDLK_d,SDLK_d,SDLK_d,
            SDLK_d,SDLK_d,SDLK_d,SDLK_d,SDLK_d,SDLK_d,SDLK_d,
            SDLK_s,SDLK_SPACE,SDLK_TAB,SDLK_TAB};
        constexpr int sequenceLength=static_cast<int>(sizeof(sequence)/sizeof(sequence[0]));
        while(running) {
            if(smoke && !captureTitle && !captureHelp) {
                if(frames<sequenceLength) {
                    SDL_Event key{};key.type=SDL_KEYDOWN;key.key.keysym.sym=sequence[frames];
                    if(SDL_PushEvent(&key)<0) throw std::runtime_error(SDL_GetError());
                }
            }
            SDL_Event event;
            while(SDL_PollEvent(&event)) {
                if(event.type==SDL_QUIT) running=false;
                if(event.type==SDL_MOUSEMOTION) {
                    // SDL maps mouse coordinates to the logical rendering space.
                    mouseX=event.motion.x;mouseY=event.motion.y;
                }
                if(event.type!=SDL_KEYDOWN || event.key.repeat) continue;
                const auto key=event.key.keysym.sym;
                if(key==SDLK_q && (title || help || chart || game.outcome!=Outcome::Alive)) {running=false;continue;}
                if(key==SDLK_F1 || key==SDLK_ESCAPE) {help=!help;continue;}
                if(help) continue;
                if(key==SDLK_TAB && !title && game.outcome==Outcome::Alive) {chart=!chart;continue;}
                if(chart) continue;
                if(key==SDLK_RETURN && (title || game.outcome!=Outcome::Alive)) {
                    if(!title) game=Game(++seed);
                    title=false;continue;
                }
                if(!title) if(auto action=actionFor(key)) game.act(*action);
            }
            view.draw(game,title,help,mouseX,mouseY,chart);
            if(smoke && (captureTitle || captureHelp || frames>=sequenceLength)) {
                if(!captureTitle && !captureHelp && (title || help || chart || game.turn<4 || game.player.pos==game.entry))
                    throw std::runtime_error("SDL input smoke test failed to advance the game.");
                saveFrame(renderer.get(),screenshot);
                std::cout<<"SDL smoke PASS: "<<game.turn<<" turns. Screenshot: "<<screenshot<<'\n';running=false;
            }
            SDL_RenderPresent(renderer.get());++frames;
            SDL_Delay(16);
        }
        return 0;
    } catch(const std::exception& error) {std::cerr<<"Choir of Ruin: "<<error.what()<<'\n';return 1;}
}
