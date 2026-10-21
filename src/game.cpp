#include "game.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <queue>

namespace choir {
namespace {
constexpr Pos directions[] = {{0,-1},{0,1},{-1,0},{1,0}};
Pos add(Pos a, Pos b) { return {a.x+b.x,a.y+b.y}; }
}
int distance(Pos a, Pos b) { return std::abs(a.x-b.x)+std::abs(a.y-b.y); }
Game::Game(std::uint32_t value): seed(value), rng(value) {
    generateDeck();
    log("You are Serf 713. The officers have stopped answering.");
    log("Find the brass seal. Reach the lift. Do not join the singing.");
}
int Game::random(int low,int high) { return std::uniform_int_distribution<int>(low,high)(rng); }
bool Game::inside(Pos p) const { return p.x>=0 && p.x<width && p.y>=0 && p.y<height; }
Cell& Game::at(Pos p) { return cells.at(static_cast<std::size_t>(p.y*width+p.x)); }
const Cell& Game::at(Pos p) const { return cells.at(static_cast<std::size_t>(p.y*width+p.x)); }
bool Game::walkable(Pos p) const {
    return inside(p) && at(p).tile!=Tile::Wall && at(p).tile!=Tile::Machinery;
}
void Game::log(const std::string& text,bool danger) {
    messages.push_back({text,danger});
    if(messages.size()>60) messages.pop_front();
}
std::string Game::deckName(int d) {
    static const char* names[]={"THE BONDED DECKS","PROCESSIONAL GALLERIES","THE LAUNCH CRYPT"};
    return names[std::clamp(d,0,2)];
}
std::string Game::enemyName(Kind kind) {
    switch(kind) {
    case Kind::Thrall:return "Maddened thrall";
    case Kind::Chorister:return "Choir disciple";
    case Kind::Legionary:return "Fallen Legionary";
    }
    return "Unknown";
}
std::string Game::objective() const {
    if(!player.seal) return "Recover the brass deck seal (&).";
    return deck==2 ? "Reach the cutter (>) and press E to escape." : "Reach the lift (>) and press E to descend.";
}
int Game::choirStage() const { return std::min(3,deckTurns/60); }
void Game::generateDeck() {
    cells.fill({}); enemies.clear(); deckTurns=0; noiseLife=0; noiseRadius=0;
    player.seal=false;
    struct Room { int l=0,t=0,r=0,b=0; Pos centre; };
    std::array<Room,12> rooms{};
    for(int gy=0;gy<3;++gy) for(int gx=0;gx<4;++gx) {
        Pos c{gx*16+8,gy*12+6};
        Room room{c.x-random(4,6),c.y-random(3,4),c.x+random(4,6),c.y+random(3,4),c};
        rooms[gy*4+gx]=room;
        for(int y=room.t;y<=room.b;++y) for(int x=room.l;x<=room.r;++x) at({x,y}).tile=Tile::Floor;
    }
    for(int gy=0;gy<3;++gy) for(int gx=0;gx<3;++gx) {
        const auto& a=rooms[gy*4+gx]; const auto& b=rooms[gy*4+gx+1];
        for(int x=a.centre.x;x<=b.centre.x;++x) at({x,a.centre.y}).tile=Tile::Floor;
        at({a.r+1,a.centre.y}).tile=Tile::Door;
        at({b.l-1,b.centre.y}).tile=Tile::Door;
    }
    for(int gy=0;gy<2;++gy) for(int gx=0;gx<4;++gx) {
        if(gx!=0 && random(0,3)==0) continue;
        const auto& a=rooms[gy*4+gx]; const auto& b=rooms[(gy+1)*4+gx];
        for(int y=a.centre.y;y<=b.centre.y;++y) at({a.centre.x,y}).tile=Tile::Floor;
        at({a.centre.x,a.b+1}).tile=Tile::Door;
        at({b.centre.x,b.t-1}).tile=Tile::Door;
    }
    entry=rooms[0].centre; exit=rooms[11].centre; sealPos=rooms[8].centre;
    player.pos=entry;
    at(exit).tile=Tile::Lift; at(sealPos).tile=Tile::Seal;
    at(rooms[5].centre).tile=Tile::Survivor;
    at(rooms[6].centre).tile=Tile::Shrine;
    at({entry.x+2,entry.y}).tile=Tile::Med;
    at({entry.x,entry.y+2}).tile=Tile::Ammo;
    for(const auto& room:rooms) {
        at({room.l+1,room.b-1}).tile=Tile::Machinery;
        at({room.l+2,room.b-1}).tile=Tile::Machinery;
        at({room.r-1,room.t+2}).tile=Tile::Machinery;
    }
    for(int i=1;i<12;++i) {
        const auto& room=rooms[i];
        Pos p{room.l+1,room.t+1};
        at(p).tile=i%3==0 ? Tile::Med : (i%3==1 ? Tile::Ration : Tile::Ammo);
        at({room.r-1,room.b-1}).tile=Tile::Scrap;
        if(deck>0 || i%3==0) at({room.r-1,room.t+1}).tile=Tile::Warp;
        if(i==8 || i==11 || i==4) continue;
        Pos ep{room.centre.x+2,room.centre.y+1};
        Kind kind=(i==7 || (deck==2 && i==10)) ? Kind::Legionary :
                  ((i+deck)%3==0 ? Kind::Chorister : Kind::Thrall);
        enemies.push_back({ep,kind,kind==Kind::Legionary?70:8,ep,0});
    }
    refreshVision();
}
bool Game::lineOfSight(Pos a,Pos b) const {
    if(!inside(a)||!inside(b)) return false;
    int dx=std::abs(b.x-a.x),sx=a.x<b.x?1:-1;
    int dy=-std::abs(b.y-a.y),sy=a.y<b.y?1:-1,err=dx+dy;
    while(a!=b) {
        const int twice=2*err;
        if(twice>=dy) { err+=dy; a.x+=sx; }
        if(twice<=dx) { err+=dx; a.y+=sy; }
        if(a==b) return true; // A blocking destination itself remains visible.
        if(at(a).tile==Tile::Wall || at(a).tile==Tile::Door || at(a).tile==Tile::Machinery) return false;
    }
    return true;
}
void Game::refreshVision() {
    for(auto& c:cells) c.visible=false;
    for(int y=std::max(0,player.pos.y-10);y<=std::min(height-1,player.pos.y+10);++y)
        for(int x=std::max(0,player.pos.x-10);x<=std::min(width-1,player.pos.x+10);++x) {
            Pos p{x,y};
            const int dx=x-player.pos.x,dy=y-player.pos.y;
            if(dx*dx+dy*dy<=100 && lineOfSight(player.pos,p)) {
                at(p).visible=true; at(p).seen=true;
            }
        }
}
int Game::enemyAt(Pos p) const {
    for(std::size_t i=0;i<enemies.size();++i) if(enemies[i].pos==p) return static_cast<int>(i);
    return -1;
}
void Game::emitNoise(Pos p,int radius) { noise=p; noiseLife=4; noiseRadius=radius; }
void Game::pickup() {
    auto& tile=at(player.pos).tile;
    switch(tile) {
    case Tile::Seal: player.seal=true; log("A brass seal, warm as skin. The transit lock accepts you."); break;
    case Tile::Med: ++player.meds; log("Recovered one sealed medicae ampoule."); break;
    case Tile::Ammo: player.ammo+=3; log("Three stub rounds. Count them carefully."); break;
    case Tile::Ration: ++player.rations; log("A ration tin bearing the old Imperial eagle."); break;
    case Tile::Scrap: player.scrap+=2; log("Loose scrap. Throw it to draw a patrol away."); break;
    case Tile::Warp: player.corruption+=3; --player.hp; log("The deck gives beneath your feet. Something notices.",true); return;
    case Tile::Survivor: log("A bonded serf whispers: please. E to rescue."); return;
    case Tile::Shrine: log("The shrine promises relief. E accepts: full health, +22 corruption.",true); return;
    default:return;
    }
    tile=Tile::Floor;
}
bool Game::interact() {
    std::array<Pos,5> points{player.pos,add(player.pos,{0,-1}),add(player.pos,{0,1}),add(player.pos,{-1,0}),add(player.pos,{1,0})};
    for(Pos p:points) {
        if(!inside(p)) continue;
        auto& tile=at(p).tile;
        if(tile==Tile::Lift) {
            if(!player.seal) { log("The transit lock needs this deck's brass seal (&)."); return false; }
            if(deck==2) {
                outcome=Outcome::Escaped;
                log("The cutter tears free. The ship sings into the void behind you.");
            } else {
                ++deck; generateDeck();
                log("Transit complete. "+deckName(deck)+".");
                log(deck==1?"Portraits turn towards you. The choir has learned your name.":"Launch rites are still running. Find the last seal.");
            }
            return true;
        }
        if(tile==Tile::Survivor) {
            tile=Tile::Floor; ++player.rescued;
            player.resolve=std::min(100,player.resolve+20);
            log("You mark a safe route to the cutter. One more serf has a chance."); return true;
        }
        if(tile==Tile::Shrine) {
            tile=Tile::Floor; player.hp=24; player.corruption+=22;
            log("Pain becomes distant music. You have accepted its gift.",true); return true;
        }
    }
    log("Nothing here answers your hand."); return false;
}
bool Game::act(Action action) {
    if(outcome!=Outcome::Alive) return false;
    if(action==Action::Sneak) {
        player.sneaking=!player.sneaking;
        log(player.sneaking?"You soften your steps. Keep to the shadows.":"You stand upright. Your boots ring on the deck."); return true;
    }
    const int oldDeck=deck;
    if(action==Action::North || action==Action::South || action==Action::West || action==Action::East) {
        const int d=static_cast<int>(action);
        player.facing=directions[d];
        Pos to=add(player.pos,player.facing);
        if(!walkable(to)) return false;
        int e=enemyAt(to);
        if(e>=0) {
            auto& enemy=enemies[static_cast<std::size_t>(e)];
            const int damage=enemy.kind==Kind::Legionary?1:random(3,5);
            enemy.hp-=damage; emitNoise(player.pos,10);
            log("Your work knife strikes the "+enemyName(enemy.kind)+" for "+std::to_string(damage)+".",true);
            if(enemy.hp<=0) { log("It falls. It was human once."); enemies.erase(enemies.begin()+e); }
        } else if(at(to).tile==Tile::Door) {
            at(to).tile=Tile::Floor;
            emitNoise(to,player.sneaking?4:8); log("You ease the bulkhead open.");
        } else {
            player.pos=to;
            if(!player.sneaking) emitNoise(to,6);
            pickup();
        }
    } else if(action==Action::Interact) {
        if(!interact()) return false;
    } else if(action==Action::Heal) {
        if(player.meds==0 || player.hp>=24) { log("No medicae needed, or none left."); return false; }
        --player.meds; player.hp=std::min(24,player.hp+12); log("The ampoule steadies your hands. +12 health.");
    } else if(action==Action::Eat) {
        if(player.rations==0 || player.resolve>=100) { log("No ration needed, or none left."); return false; }
        --player.rations; player.resolve=std::min(100,player.resolve+35); log("You remember an ordinary meal. +35 resolve.");
    } else if(action==Action::Fire) {
        if(player.ammo<=0) { log("The stub pistol clicks empty."); return false; }
        int target=-1,nearest=9;
        for(std::size_t i=0;i<enemies.size();++i) {
            const auto& e=enemies[i]; const int dist=distance(e.pos,player.pos);
            if(dist<nearest && lineOfSight(player.pos,e.pos)) { nearest=dist; target=static_cast<int>(i); }
        }
        if(target<0) { log("No clear target within eight paces."); return false; }
        --player.ammo;
        auto& e=enemies[static_cast<std::size_t>(target)];
        const int damage=e.kind==Kind::Legionary?4:9;
        e.hp-=damage; emitNoise(player.pos,22);
        log("Stub fire hits "+enemyName(e.kind)+" for "+std::to_string(damage)+". The shot echoes.",true);
        if(e.hp<=0) { enemies.erase(enemies.begin()+target); log("The target collapses."); }
    } else if(action==Action::Throw) {
        if(player.scrap<=0) { log("No loose scrap left."); return false; }
        Pos landing=player.pos;
        for(int i=0;i<7;++i) {
            Pos next=add(landing,player.facing);
            if(!walkable(next) || at(next).tile==Tile::Door) break;
            landing=next;
        }
        if(landing==player.pos) { log("No space to throw in that direction. Face another way."); return false; }
        --player.scrap; emitNoise(landing,14); log("Scrap clatters down the passage. Something listens.");
    }
    if(oldDeck!=deck) { ++turn; refreshVision(); return true; }
    if(outcome==Outcome::Alive) advance(); else ++turn;
    refreshVision(); return true;
}
void Game::enemyTurns() {
    for(auto& e:enemies) {
        const int sight=player.sneaking?4:8;
        const int dist=distance(e.pos,player.pos);
        bool sees=dist<=sight && lineOfSight(e.pos,player.pos);
        if(sees) { e.target=player.pos; e.alert=7; }
        else if(noiseLife>0 && distance(e.pos,noise)<=noiseRadius) { e.target=noise; e.alert=6; }
        if(dist==1 && (sees || e.alert>0)) {
            const int damage=e.kind==Kind::Legionary?random(9,13):random(2,4);
            player.hp-=damage;
            player.resolve=std::max(0,player.resolve-(e.kind==Kind::Chorister?8:3));
            log(enemyName(e.kind)+" strikes. -"+std::to_string(damage)+" health.",true);
            if(player.hp<=0) return;
            continue;
        }
        if(e.kind==Kind::Chorister && sees && dist<=5) {
            player.resolve=std::max(0,player.resolve-2);
            if(deckTurns%4==0) log("The disciple's song presses against your thoughts.",true);
        }
        if(e.alert>0) {
            // A distance field avoids getting stuck on the corners of ship corridors.
            std::array<int,width*height> cost; cost.fill(-1);
            std::queue<Pos> q; q.push(e.target); cost[static_cast<std::size_t>(e.target.y*width+e.target.x)]=0;
            while(!q.empty()) {
                Pos p=q.front(); q.pop();
                for(Pos d:directions) {
                    Pos n=add(p,d);
                    if(!walkable(n)) continue;
                    auto index=static_cast<std::size_t>(n.y*width+n.x);
                    if(cost[index]<0) { cost[index]=cost[static_cast<std::size_t>(p.y*width+p.x)]+1; q.push(n); }
                }
            }
            Pos best=e.pos;
            int score=cost[static_cast<std::size_t>(e.pos.y*width+e.pos.x)];
            if(score<0) score=std::numeric_limits<int>::max();
            for(Pos d:directions) {
                Pos n=add(e.pos,d);
                if(!walkable(n) || n==player.pos || enemyAt(n)>=0) continue;
                const int c=cost[static_cast<std::size_t>(n.y*width+n.x)];
                if(c>=0 && c<score) { best=n; score=c; }
            }
            if(at(best).tile==Tile::Door) at(best).tile=Tile::Floor; else e.pos=best;
            --e.alert;
        } else if(deckTurns%3==0) {
            Pos p=add(e.pos,directions[random(0,3)]);
            if(walkable(p) && at(p).tile!=Tile::Door && p!=player.pos && enemyAt(p)<0) e.pos=p;
        }
    }
}
void Game::advance() {
    ++turn; ++deckTurns;
    if(player.hp>0 && player.corruption<100) enemyTurns();
    if(noiseLife>0) --noiseLife;
    if(deckTurns%6==0) player.resolve=std::max(0,player.resolve-1);
    if(deckTurns%60==0) {
        player.corruption+=3+deck;
        log(choirStage()==1?"The first vox hymn rises. Something in the walls keeps time.":
            choirStage()==2?"Second movement. The crew answer with voices that are not theirs.":
            "The final movement begins. The ship is becoming a mouth.",true);
    }
    if(player.resolve==0) {
        ++player.corruption;
        if(deckTurns%6==0) log("Without hope, the song finds a place inside you.",true);
    }
    player.hp=std::max(0,player.hp); player.corruption=std::min(100,player.corruption);
    if(player.hp<=0) { outcome=Outcome::Dead; log("Your service ends here. The ship does not notice.",true); }
    else if(player.corruption>=100) { outcome=Outcome::Possessed; log("You no longer wish to leave. Your voice joins the choir.",true); }
}
} // namespace choir
