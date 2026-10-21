#include "game.hpp"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <queue>
#include <stdexcept>

using namespace choir;
namespace {
int checks = 0;
void check(bool condition, const char* message) {
    ++checks;
    if (!condition) throw std::runtime_error(message);
}
constexpr Pos dirs[] = {{0,-1},{0,1},{-1,0},{1,0}};
constexpr Action moves[] = {Action::North,Action::South,Action::West,Action::East};
std::vector<Action> route(const Game& g, Pos destination) {
    std::array<int, width*height> prev; prev.fill(-1);
    std::array<int, width*height> step{};
    std::queue<Pos> q; q.push(g.player.pos);
    const auto idx=[](Pos p){return p.y*width+p.x;};
    prev[idx(g.player.pos)] = idx(g.player.pos);
    while(!q.empty()) {
        Pos p=q.front(); q.pop();
        for(int d=0;d<4;++d) {
            Pos n{p.x+dirs[d].x,p.y+dirs[d].y};
            if(g.walkable(n) && prev[idx(n)]==-1) {
                prev[idx(n)]=idx(p); step[idx(n)]=d; q.push(n);
            }
        }
    }
    check(prev[idx(destination)]!=-1,"objective must be reachable");
    std::vector<Action> result;
    for(int p=idx(destination);p!=idx(g.player.pos);p=prev[p]) result.push_back(moves[step[p]]);
    std::reverse(result.begin(),result.end()); return result;
}
void arena(Game& g) {
    for(auto& c:g.cells) c={Tile::Wall,false,false};
    for(int y=1;y<15;++y) for(int x=1;x<25;++x) g.at({x,y}).tile=Tile::Floor;
    g.player.pos={5,5}; g.enemies.clear(); g.refreshVision();
}
void generation() {
    for(std::uint32_t seed=0;seed<200;++seed) {
        Game g(seed), same(seed);
        for(int deck=0;deck<3;++deck) {
            g.deck=deck; g.generateDeck(); same.deck=deck; same.generateDeck();
            check(g.walkable(g.entry),"spawn is traversable");
            route(g,g.sealPos); route(g,g.exit);
            for(std::size_t i=0;i<g.cells.size();++i)
                check(g.cells[i].tile==same.cells[i].tile,"same seed reproduces layout");
            for(std::size_t i=0;i<g.enemies.size();++i) {
                check(g.walkable(g.enemies[i].pos),"enemy is on floor");
                for(std::size_t j=0;j<i;++j) check(g.enemies[i].pos!=g.enemies[j].pos,"enemies cannot overlap");
            }
        }
    }
}
void contracts() {
    Game g(42); arena(g);
    g.at({6,5}).tile=Tile::Wall;
    check(!g.act(Action::East) && g.turn==0,"wall bumps are free");
    check(!g.act(Action::Fire) && g.player.ammo==5 && g.turn==0,"no-target shot is free");
    check(!g.act(Action::Heal) && g.player.meds==2,"full health conserves medicae");
    check(!g.act(Action::Eat) && g.player.rations==2,"full resolve conserves rations");
    check(!g.act(Action::Interact) && g.turn==0,"empty interaction is free");
    g.act(Action::Sneak); check(g.turn==0 && !g.player.sneaking,"stance toggle is free");
    g.player.hp=10; check(g.act(Action::Heal) && g.player.hp==22 && g.player.meds==1,"medicae heals and consumes");
    g.player.resolve=20; check(g.act(Action::Eat) && g.player.resolve>=54 && g.player.rations==1,"ration restores resolve");
    g.at({6,5}).tile=Tile::Door;
    g.refreshVision(); check(!g.at({7,5}).visible,"closed bulkhead occludes sight");
    Pos before=g.player.pos;
    g.act(Action::East); check(g.player.pos==before && g.at({6,5}).tile==Tile::Floor,"opening bulkhead takes a turn");
    g.act(Action::East); check(g.player.pos==Pos{6,5},"open bulkhead permits movement");
    g.at({7,5}).tile=Tile::Seal; g.act(Action::East);
    check(g.player.seal && g.at({7,5}).tile==Tile::Floor,"seal pickup changes objective");
    g.at({8,5}).tile=Tile::Survivor; g.player.resolve=40; g.act(Action::Interact);
    check(g.player.rescued==1 && g.player.resolve>40,"rescue rewards hope");
    g.player.hp=3; g.at({8,5}).tile=Tile::Shrine; g.act(Action::Interact);
    check(g.player.hp==24 && g.player.corruption>=22,"temptation has a cost");
    g.act(Action::Interact); check(g.player.corruption<44,"shrine cannot be farmed");
    arena(g); g.player.seal=false; g.at({5,5}).tile=Tile::Lift;
    int old=g.turn; check(!g.act(Action::Interact) && g.deck==0 && g.turn==old,"lift requires seal");
    g.player.corruption=100; g.act(Action::Wait);
    check(g.outcome==Outcome::Possessed,"full corruption ends run");
    old=g.turn; check(!g.act(Action::Wait) && g.turn==old,"terminal runs reject actions");
}
void threats() {
    Game g(9); arena(g);
    g.enemies.push_back({{7,5},Kind::Thrall,8,{7,5},0});
    g.act(Action::Wait); check(g.enemies[0].pos==Pos{6,5},"visible enemy pursues");
    const int hp=g.player.hp; g.act(Action::Wait);
    check(g.player.hp<hp,"adjacent enemy attacks");
    g.act(Action::Fire); check(g.enemies.empty() && g.player.ammo==4,"shot kills a thrall");
    arena(g); g.enemies.push_back({{12,5},Kind::Legionary,70,{12,5},0});
    g.act(Action::Fire); check(g.enemies[0].hp>=65,"legionary armour resists pistol");
    arena(g); g.player.hp=1; g.enemies.push_back({{6,5},Kind::Legionary,70,{6,5},8});
    g.act(Action::Wait); check(g.outcome==Outcome::Dead,"lethal attack ends run");
    Game n(12); arena(n); n.at({7,5}).tile=Tile::Wall;
    n.enemies.push_back({{10,5},Kind::Thrall,8,{10,5},0});
    check(!n.lineOfSight(n.player.pos,{10,5}),"wall blocks enemy sight");
    n.act(Action::Throw); check(n.player.scrap==2 && n.noiseLife>0,"throw spends scrap and creates noise");
    check(n.enemies[0].alert>0 && n.enemies[0].target==n.noise,"enemy investigates thrown noise");
    for(int i=0;i<6 && n.outcome==Outcome::Alive;++i) n.act(Action::Wait);
    check(n.noiseLife==0,"noise expires");
}
void survival() {
    Game g(44); arena(g);
    g.player.meds=0;g.player.hp=2;g.player.ammo=0;g.player.scrap=0;g.player.rations=0;
    check(!g.act(Action::Heal) && !g.act(Action::Eat) && !g.act(Action::Fire) && !g.act(Action::Throw),"empty supplies do not grant free resources");
    check(g.turn==0,"failed supply use spends no turns");
    g.player.resolve=0;g.act(Action::Wait);
    check(g.player.corruption==1,"despair adds corruption each turn");
    g.player.resolve=100;g.player.corruption=0;
    for(int i=1;i<60;++i) g.act(Action::Wait);
    check(g.choirStage()==1 && g.player.corruption==3,"choir escalates at sixty turns");
    g.at({6,5}).tile=Tile::Warp;g.act(Action::East);
    check(g.player.hp==1 && g.player.corruption==6,"warp rupture has immediate costs");
    g.at({7,5}).tile=Tile::Warp;g.act(Action::East);
    check(g.outcome==Outcome::Dead,"environmental damage can end a run");
    Game h(45);arena(h);h.player.corruption=90;h.at({6,5}).tile=Tile::Shrine;
    h.act(Action::Interact);check(h.outcome==Outcome::Possessed,"temptation can cause possession");
    Game b(46);arena(b);b.at({6,5}).tile=Tile::Machinery;b.refreshVision();
    check(!b.act(Action::East) && !b.at({7,5}).visible,"machinery blocks walking and sight");
    Game quiet(47),loud(47);arena(quiet);arena(loud);
    quiet.enemies.push_back({{11,5},Kind::Thrall,8,{11,5},0});loud.enemies=quiet.enemies;
    loud.act(Action::Sneak);quiet.act(Action::Wait);loud.act(Action::Wait);
    check(quiet.enemies[0].alert==0 && loud.enemies[0].alert>0,"quiet stance reduces detection distance");
}
void simulation() {
    std::mt19937 random(713);
    for(std::uint32_t seed=0;seed<30;++seed) {
        Game g(seed);
        for(int turn=0;turn<350 && g.outcome==Outcome::Alive;++turn) {
            g.act(static_cast<Action>(random()%12));
            check(g.walkable(g.player.pos),"simulated player remains on traversable deck");
            check(g.player.hp>=0 && g.player.hp<=24,"health stays bounded");
            check(g.player.resolve>=0 && g.player.resolve<=100,"resolve stays bounded");
            check(g.player.corruption>=0 && g.player.corruption<=100,"corruption stays bounded");
            check(g.player.ammo>=0 && g.player.meds>=0 && g.player.rations>=0 && g.player.scrap>=0,"supplies cannot become negative");
            for(std::size_t i=0;i<g.enemies.size();++i) {
                check(g.walkable(g.enemies[i].pos) && g.enemies[i].pos!=g.player.pos,"enemy state remains valid");
                for(std::size_t j=0;j<i;++j) check(g.enemies[i].pos!=g.enemies[j].pos,"enemy turns cannot overlap actors");
            }
        }
    }
}
void journey() {
    Game g(713);
    // Isolate navigation and progression from combat. No fixture auto-completes objectives.
    for(int deck=0;deck<3;++deck) {
        g.enemies.clear();
        auto follow=[&](Pos destination){
            for(Action a:route(g,destination)) {
                Pos p=g.player.pos;
                check(g.act(a),"route action is accepted");
                if(g.player.pos==p) check(g.act(a),"walk through opened door");
                check(g.outcome==Outcome::Alive,"navigation journey remains alive");
            }
        };
        follow(g.sealPos); check(g.player.seal,"journey collects deck seal");
        follow(g.exit); check(g.act(Action::Interact),"journey operates lift or cutter");
        if(deck<2) check(g.deck==deck+1 && !g.player.seal,"descent resets only deck seal");
    }
    check(g.outcome==Outcome::Escaped,"fresh run reaches escape through actions");
}
}
int main() {
    try { generation(); contracts(); threats(); survival(); simulation(); journey();
        std::cout<<"PASS: "<<checks<<" checks, generation over 600 decks, 30 simulated runs, contracts and escape journey\n";
        return EXIT_SUCCESS;
    } catch(const std::exception& e) { std::cerr<<"FAIL: "<<e.what()<<'\n'; return EXIT_FAILURE; }
}
