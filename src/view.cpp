#include "view.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <sstream>
#include <string_view>

namespace choir {
namespace {
constexpr SDL_Color ink{10,10,22,255},panel{21,20,36,255},gold{190,145,72,255},
    paper{222,215,183,255},muted{124,123,143,255},purple{113,65,139,255},
    rose{221,96,130,255},green{125,192,151,255},darkGold{77,57,42,255};
// Original compact bitmap alphabet, five columns by seven rows.
constexpr std::string_view alphabet="ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,:;!?+-/()><=&@%_'#";
constexpr const char* glyphs[]={
 "01110100011000111111100011000110001", // A
 "11110100011000111110100011000111110",
 "01111100001000010000100001000001111",
 "11110100011000110001100011000111110",
 "11111100001000011110100001000011111",
 "11111100001000011110100001000010000",
 "01111100001000010111100011000101110",
 "10001100011000111111100011000110001",
 "11111001000010000100001000010011111",
 "00111000100001000010100101001001100",
 "10001100101010011000101001001010001",
 "10000100001000010000100001000011111",
 "10001110111010110101100011000110001",
 "10001110011010110011100011000110001",
 "01110100011000110001100011000101110",
 "11110100011000111110100001000010000",
 "01110100011000110001101011001001101",
 "11110100011000111110101001001010001",
 "01111100001000001110000010000111110",
 "11111001000010000100001000010000100",
 "10001100011000110001100011000101110",
 "10001100011000110001100010101000100",
 "10001100011000110101101011101110001",
 "10001100010101000100010101000110001",
 "10001100010101000100001000010000100",
 "11111000010001000100010001000011111",
 "01110100011001110101110011000101110",
 "00100011000010000100001000010001110",
 "01110100010000100010001000100011111",
 "11110000010000101110000010000111110",
 "00010001100101010010111110001000010",
 "11111100001000011110000010000111110",
 "01110100001000011110100011000101110",
 "11111000010001000100010000100001000",
 "01110100011000101110100011000101110",
 "01110100011000101111000010000101110",
 "00000000000000000000000000011000110", // .
 "00000000000000000000001100011000100",
 "00000001100011000000001100011000000",
 "00000001100011000000001100011000100",
 "00100001000010000100001000000000100",
 "01110100010000100010001000000000100",
 "00000001000010011111001000010000000",
 "00000000000000011111000000000000000",
 "00001000100001000100010000100010000",
 "00010001000100001000010000010000010",
 "01000001000001000010000100010001000",
 "10000010000010000010001000100010000",
 "00001000100010001000001000001000001",
 "00000000001111100000111110000000000",
 "01100100101010001000101011001001101",
 "01110100011011110101101111000001110",
 "11001110100001000100010001011110011",
 "00000000000000000000000000000011111",
 "00100001000000000000000000000000000",
 "01010010101111101010111110101001010"
};
constexpr bool validFont() {
    if(sizeof(glyphs)/sizeof(glyphs[0])!=alphabet.size()) return false;
    for(const char* glyph:glyphs) if(std::string_view(glyph).size()!=35) return false;
    return true;
}
static_assert(validFont(),"Each bitmap glyph must contain exactly 5x7 pixels.");
// All sprite pixels are authored here. Digits index each sprite's palette.
const char* const serf[]={
 "................","......111.......",".....12221......",".....12321......",
 "......22........",".....3443.......","....344443......","...33455433.....",
 "...23455432.....","...2.4444.2.....",".....4554.......",".....3443.......",
 ".....3..3.......","....33..33......","....66..66......","................"};
const char* const thrall[]={
 "................",".....111........","....12221.......","....12231.......",
 ".....22.........","....4444........","...444444.......","..24454442......",
 "..2.44544.2.....","..2.44444.2.....","....4444........","....4444........",
 "....4..4........","...44..44.......","...33..33.......","................"};
const char* const disciple[]={
 "................","......11........",".....1221.......","....122221......",
 "....123321......","....123321......",".....1441.......","....222222......",
 "...22255222.....","...42555224.....","...4.2552.4.....","....222222......",
 "....225522......","...22222222.....","...11111111.....","................"};
const char* const marine[]={
 "......111.......",".....12221......",".....12321......",".....13331......",
 ".4444122214444..","145554111455541.","145554555455541.",".4444555554444..",
 "..66455455466...","..66455555466...","..66.45554.66...",".....44444......",
 "....4554554.....","....4554554.....","...1111.1111....","...6666.6666...."};
std::string number(int n) { return (n<10?"0":"")+std::to_string(n); }
SDL_Color dim(SDL_Color c) { return {static_cast<Uint8>(c.r/3),static_cast<Uint8>(c.g/3),static_cast<Uint8>(c.b/3),255}; }
}
void View::rect(int x,int y,int w,int h,SDL_Color c) {
    SDL_SetRenderDrawColor(renderer,c.r,c.g,c.b,c.a); SDL_Rect r{x,y,w,h}; SDL_RenderFillRect(renderer,&r);
}
void View::line(int x,int y,int xx,int yy,SDL_Color c) {
    SDL_SetRenderDrawColor(renderer,c.r,c.g,c.b,c.a); SDL_RenderDrawLine(renderer,x,y,xx,yy);
}
void View::text(int x,int y,const std::string& value,SDL_Color colour,int scale) {
    for(unsigned char raw:value) {
        char c=static_cast<char>(std::toupper(raw));
        auto i=alphabet.find(c);
        if(i!=std::string_view::npos) {
            std::string_view bitmap(glyphs[i]);
            for(int row=0;row<7;++row) for(int col=0;col<5;++col)
                if(bitmap[static_cast<std::size_t>(row*5+col)]=='1') rect(x+col*scale,y+row*scale,scale,scale,colour);
        }
        x+=6*scale;
    }
}
void View::wrapped(int x,int y,const std::string& value,int columns,SDL_Color colour,int maxLines) {
    std::istringstream stream(value); std::string word,row; int lines=0;
    while(stream>>word) {
        if(!row.empty() && row.size()+1+word.size()>static_cast<std::size_t>(columns)) {
            text(x,y,row,colour); y+=10; row.clear(); if(++lines>=maxLines) return;
        }
        if(!row.empty()) row+=' ';
        row+=word;
    }
    if(!row.empty() && lines<maxLines) text(x,y,row,colour);
}
void View::sprite(int x,int y,const char* const* pixels,const std::array<SDL_Color,10>& palette) {
    for(int row=0;row<16;++row) for(int col=0;col<16;++col) {
        char c=pixels[row][col];
        if(c>='0' && c<='9') rect(x+col,y+row,1,1,palette[static_cast<std::size_t>(c-'0')]);
    }
}
void View::tile(int x,int y,Tile type,int hash,bool visible) {
    auto colour=[&](SDL_Color c){return visible?c:dim(c);};
    rect(x,y,16,16,colour({29,30,42,255}));
    if(type==Tile::Wall) {
        rect(x,y,16,16,colour({44,35,60,255}));
        rect(x+1,y+1,14,2,colour({91,68,99,255}));
        rect(x+2,y+4,12,10,colour({52,43,67,255}));
        rect(x+4,y+4,2,9,colour({68,53,80,255}));
        rect(x+10,y+4,2,9,colour({68,53,80,255}));
        line(x,y+15,x+15,y+15,ink);
        rect(x+1,y+2,1,1,colour(gold)); rect(x+14,y+2,1,1,colour(gold));
        if(hash%5==0) {
            rect(x+5,y+3,6,11,colour({105,53,116,255}));
            rect(x+7,y+5,2,6,colour(gold));rect(x+6,y+7,4,1,colour(gold));
        } else if(hash%3==0) {
            rect(x+3,y+6,3,6,colour({29,29,40,255}));
            rect(x+4,y+7,1,4,colour({140,116,77,255}));
            rect(x+10,y+6,3,6,colour({29,29,40,255}));
            rect(x+11,y+7,1,4,colour({140,116,77,255}));
        }
        return;
    }
    line(x,y,x+15,y,colour({50,46,57,255}));
    line(x,y,x,y+15,colour({45,42,53,255}));
    rect(x+2,y+2,1,1,colour({70,64,70,255}));
    rect(x+13,y+13,1,1,colour({70,64,70,255}));
    if(hash%4==0) {
        for(int i=3;i<14;i+=3) line(x+3,y+i,x+12,y+i,colour({20,21,31,255}));
    } else if(hash%7==0) {
        line(x+4,y+6,x+11,y+6,colour({54,46,59,255}));
        line(x+4,y+9,x+8,y+9,colour({54,46,59,255}));
    }
    if(type==Tile::Floor && hash%23==0) {
        rect(x+4,y+8,6,3,colour({83,36,53,255}));
        rect(x+7,y+6,4,3,colour({99,40,57,255}));
        rect(x+3,y+5,1,1,colour({112,45,64,255}));
        rect(x+11,y+12,2,1,colour({83,36,53,255}));
    }
    const SDL_Color brass=colour(gold),white=colour(paper),red=colour(rose),mint=colour(green);
    switch(type) {
    case Tile::Machinery:
        rect(x+1,y+4,14,11,colour({23,25,35,255}));
        rect(x+2,y+3,12,10,colour({71,72,82,255}));
        rect(x+3,y+4,10,1,colour({111,105,109,255}));
        rect(x+4,y+6,5,4,colour({32,47,48,255}));
        line(x+5,y+8,x+7,y+8,mint);rect(x+6,y+7,1,1,mint);
        rect(x+11,y+6,1,2,red);rect(x+11,y+9,1,1,brass);
        line(x+3,y+12,x+12,y+12,ink);break;
    case Tile::Door:
        rect(x+1,y+1,14,14,colour({109,84,60,255}));
        rect(x+3,y+2,10,12,colour({57,56,70,255}));
        line(x+7,y+2,x+7,y+13,ink); line(x+8,y+2,x+8,y+13,brass);
        rect(x+11,y+6,2,3,red); break;
    case Tile::Lift:
        rect(x+1,y+1,14,14,brass); rect(x+2,y+2,12,12,ink);
        for(int i=4;i<14;i+=3) line(x+3,y+i,x+12,y+i,colour({85,89,99,255}));
        text(x+6,y+4,">",mint); break;
    case Tile::Seal:
        rect(x+5,y+3,6,9,brass); rect(x+3,y+5,10,5,brass);
        rect(x+6,y+5,4,5,ink); rect(x+7,y+6,2,3,white); break;
    case Tile::Med:
        rect(x+4,y+4,8,8,white); rect(x+7,y+5,2,6,red); rect(x+5,y+7,6,2,red); break;
    case Tile::Ammo:
        rect(x+3,y+5,10,7,colour({74,61,47,255}));
        for(int i=4;i<13;i+=3) {rect(x+i,y+6,2,5,brass);rect(x+i,y+5,1,1,white);} break;
    case Tile::Ration:
        rect(x+4,y+5,8,7,colour({86,110,91,255})); line(x+4,y+5,x+11,y+5,mint);
        rect(x+6,y+7,4,3,white); break;
    case Tile::Scrap:
        line(x+4,y+4,x+12,y+11,white); line(x+3,y+9,x+10,y+5,brass);
        rect(x+7,y+10,3,3,colour({117,108,119,255})); break;
    case Tile::Shrine:
        rect(x+3,y+11,10,3,colour(purple)); rect(x+5,y+5,6,7,colour(purple));
        rect(x+7,y+3,2,7,red); rect(x+5,y+5,6,2,red);
        rect(x+2,y+6,1,5,brass);rect(x+13,y+6,1,5,brass);
        rect(x+2,y+5,1,1,white);rect(x+13,y+5,1,1,white); break;
    case Tile::Survivor:
        rect(x+6,y+4,4,4,white); rect(x+5,y+8,6,4,colour({91,133,123,255}));
        rect(x+3,y+12,10,2,colour({70,91,94,255})); break;
    case Tile::Warp:
        line(x+1,y+12,x+5,y+7,red);line(x+5,y+7,x+10,y+9,red);line(x+10,y+9,x+14,y+3,red);
        rect(x+4,y+11,2,2,colour(purple));rect(x+10,y+5,2,2,colour(purple));break;
    default:break;
    }
}
void View::actor(int x,int y,int kind,bool alert) {
    std::array<SDL_Color,10> p{};
    if(kind==0) {
        p={ink,{46,35,41,255},{211,177,129,255},{54,65,81,255},{124,157,158,255},
           {54,85,97,255},{31,29,39,255},paper,gold,rose};
        rect(x+3,y+14,10,2,{16,17,25,255});sprite(x,y,serf,p);
        rect(x+7,y+1,2,1,gold);
        rect(x+2,y+13,1,2,green);rect(x+2,y+15,3,1,green);
        rect(x+12,y+13,1,2,green);rect(x+10,y+15,3,1,green);
    } else if(kind==1) {
        p={ink,{60,34,46,255},{178,121,111,255},{87,35,54,255},{106,76,86,255},
           {145,98,101,255},ink,ink,ink,ink};sprite(x,y,thrall,p);
    } else if(kind==2) {
        p={ink,{50,28,66,255},{122,65,146,255},{219,118,155,255},{189,148,101,255},
           {173,103,150,255},ink,ink,ink,ink};sprite(x,y,disciple,p);
    } else {
        p={ink,{75,51,62,255},{198,147,70,255},{211,67,103,255},{102,55,125,255},
           {153,88,157,255},{51,42,63,255},ink,ink,ink};sprite(x,y,marine,p);
    }
    if(alert) { rect(x+13,y+1,2,4,rose);rect(x+13,y+6,2,1,rose); }
}
void View::titleScreen(const Game& game) {
    for(int i=0;i<110;++i) {
        int x=(i*97+31)%320,y=(i*i*13+41)%137;
        rect(x,y,1,1,i%3==0?muted:dim(muted));
    }
    // A side-on cathedral cruiser. Original code-native pixel illustration.
    rect(119,96,175,20,{32,27,49,255});
    for(int i=0;i<7;++i) {
        rect(148+i*18,84,12,30,{47,34,64,255});
        rect(151+i*18,78-i%2*6,6,28,{67,44,83,255});
        rect(153+i*18,73-i%2*6,2,10,gold);
        rect(150+i*18,97,8,2,darkGold);
        rect(150+i*18,109,8,3,gold);
    }
    rect(132,115,138,11,{42,33,57,255});
    rect(159,126,95,4,{32,27,49,255});
    for(int i=0;i<14;++i) rect(145+i*9,119,3,2,{194,105,98,255});
    for(int i=0;i<5;++i) rect(288+i*3,99+i*2,3,14-i*3,{86,64,81,255});
    rect(112,99,18,17,{82,59,85,255});rect(105,102,11,11,{160,93,143,255});
    rect(98,105,10,5,{213,130,166,255});
    rect(203,64,18,26,{63,43,79,255});rect(208,53,8,30,{85,58,95,255});
    rect(211,46,2,11,gold);rect(205,59,14,1,gold);
    text(10,10,"CHOIR",paper,4);
    text(12,44,"OF RUIN",gold,2);
    text(12,67,"A SERF'S SURVIVAL",muted);
    text(12,79,"III LEGION / M31",muted);
    line(8,137,311,137,darkGold);
    wrapped(12,145,"The Canticle of Excess is singing. Your masters call it perfection. The bonded decks call it the end.",49,paper,3);
    text(12,179,"THREE DECKS. ONE CUTTER. STAY HUMAN.",gold);
    rect(10,193,170,16,purple);text(18,198,"ENTER  BEGIN YOUR WATCH",paper);
    text(191,198,"F1  FIELD MANUAL",muted);
    text(12,215,"SEED "+std::to_string(game.seed),muted);
    text(221,215,"Q QUIT / V0.1",muted);
}
void View::helpScreen() {
    rect(6,6,308,212,ink);rect(8,8,304,1,gold);rect(8,215,304,1,gold);
    text(16,18,"SERF'S FIELD MANUAL",gold,2);
    text(16,41,"WASD / ARROWS  MOVE. BUMP TO USE YOUR KNIFE.",paper);
    text(16,54,"E  USE LIFT / RESCUE / ACCEPT SHRINE GIFT",paper);
    text(16,67,"F  SHOOT NEAREST VISIBLE TARGET (8 PACES)",paper);
    text(16,80,"T  THROW SCRAP IN LAST MOVEMENT DIRECTION",paper);
    text(16,93,"C  TOGGLE QUIET STEPS     SPACE  WAIT",paper);
    text(16,106,"M MEDICAE  R RATION  TAB REMEMBERED DECK MAP",paper);
    line(16,120,303,120,darkGold);
    wrapped(16,129,"Find each deck's brass seal, then use its lift. On the third deck, launch the cutter. Supplies are collected by walking over them.",47,muted,3);
    wrapped(16,162,"Quiet steps shorten enemy sight. Draw patrols away with scrap. A Legionary can kill you in two blows. Break sight and flee.",47,rose,3);
    text(16,200,"ESC / F1  RETURN                  Q  QUIT",gold);
}
void View::endScreen(const Game& game) {
    rect(12,30,296,163,ink);rect(14,32,292,1,gold);rect(14,190,292,1,gold);
    const bool escaped=game.outcome==Outcome::Escaped;
    text(25,47,escaped?"VOID BETWEEN US":game.outcome==Outcome::Dead?"SERVICE ENDED":"ONE MORE VOICE",escaped?gold:rose,2);
    const std::string ending=escaped ?
        (game.player.corruption<40?"The cutter clears the hull. For the first time in your life, no bell tells you what to do.":
         "The cutter clears the hull. You stop the engines. In the silence, something inside you is still singing."):
        (game.outcome==Outcome::Dead?"Your number is struck from the duty slate. Somewhere above, the officers applaud.":
         "There is no need to run. There never was. You understand the song now. You open your mouth.");
    wrapped(25,73,ending,44,paper,4);
    text(25,119,"TURNS "+std::to_string(game.turn)+"   SOULS HELPED "+std::to_string(game.player.rescued),gold);
    text(25,132,"TAINT "+std::to_string(game.player.corruption)+"%   SEED "+std::to_string(game.seed),muted);
    text(25,153,"ENTER  ANOTHER SERF TAKES THE WATCH",paper);
    text(25,173,"Q  QUIT             F1  FIELD MANUAL",muted);
}
void View::mapScreen(const Game& game) {
    rect(6,6,308,212,ink);line(8,8,311,8,gold);line(8,215,311,215,gold);
    text(16,18,"DUTY SLATE",gold,2);text(16,38,Game::deckName(game.deck),muted);
    for(int y=0;y<height;++y) for(int x=0;x<width;++x) {
        const auto& c=game.at({x,y});
        if(!c.seen) continue;
        SDL_Color colour=c.tile==Tile::Wall?dim(purple):muted;
        if(c.tile==Tile::Machinery) colour=dim(muted);
        if(c.tile==Tile::Door || c.tile==Tile::Seal || c.tile==Tile::Lift) colour=gold;
        if(c.tile==Tile::Shrine || c.tile==Tile::Warp) colour=rose;
        if(c.tile==Tile::Survivor || c.tile==Tile::Med) colour=green;
        rect(16+x*3,51+y*3,2,2,colour);
    }
    for(const auto& e:game.enemies) if(game.at(e.pos).visible) rect(16+e.pos.x*3,51+e.pos.y*3,3,3,rose);
    rect(16+game.player.pos.x*3,51+game.player.pos.y*3,3,3,green);
    text(225,55,"YOU",green);text(225,70,"TRANSIT",gold);text(225,85,"DANGER",rose);
    text(225,110,"TURN",muted);text(225,120,std::to_string(game.turn),paper);
    text(225,137,"HELPED",muted);text(225,147,std::to_string(game.player.rescued),paper);
    line(16,171,303,171,darkGold);
    wrapped(16,179,game.objective(),47,paper,2);
    text(16,204,"TAB RETURN / ONLY EXPLORED AREAS ARE SHOWN",muted);
}
void View::draw(const Game& game,bool title,bool help,int mouseX,int mouseY,bool chart) {
    rect(0,0,screenWidth,screenHeight,ink);
    if(title) titleScreen(game);
    else {
        text(8,7,"CHOIR OF RUIN",paper,2);
        text(224,8,"III LEGION",gold);
        text(224,19,"DECK 0"+std::to_string(game.deck+1)+"/03",muted);
        const char* shortNames[]={"BONDED DECKS","PROCESSIONAL GALLERIES","LAUNCH CRYPT"};
        text(8,29,std::string(shortNames[game.deck])+" / "+(game.player.seal?"TRANSIT UNLOCKED":"FIND BRASS SEAL"),gold);
        const int cx=std::clamp(game.player.pos.x-7,0,width-14);
        const int cy=std::clamp(game.player.pos.y-4,0,height-9);
        SDL_Rect clip{8,40,224,144};SDL_RenderSetClipRect(renderer,&clip);
        for(int row=0;row<9;++row) for(int col=0;col<14;++col) {
            Pos p{cx+col,cy+row}; const auto& c=game.at(p);
            if(!c.seen) {
                rect(8+col*16,40+row*16,16,16,{14,14,25,255});
                rect(15+col*16,47+row*16,1,1,{25,24,36,255}); continue;
            }
            tile(8+col*16,40+row*16,c.tile,p.x*31+p.y*17,c.visible);
        }
        for(const auto& e:game.enemies) if(game.at(e.pos).visible)
            actor(8+(e.pos.x-cx)*16,40+(e.pos.y-cy)*16,1+static_cast<int>(e.kind),e.alert>0);
        actor(8+(game.player.pos.x-cx)*16,40+(game.player.pos.y-cy)*16,0,false);
        SDL_RenderSetClipRect(renderer,nullptr);
        line(6,38,233,38,darkGold);line(6,185,233,185,darkGold);
        rect(238,38,76,148,panel);
        auto bar=[&](int y,const char* label,int value,int max,SDL_Color c){
            text(242,y,label,muted);text(296,y,number(value),c);
            rect(242,y+9,68,3,ink);rect(242,y+9,68*std::clamp(value,0,max)/max,3,c);
        };
        bar(42,"VITAL",game.player.hp,24,green);
        bar(59,"HOPE",game.player.resolve,100,gold);
        bar(76,"TAINT",game.player.corruption,100,rose);
        text(242,96,game.player.sneaking?"C  QUIET":"C  LOUD",game.player.sneaking?green:rose);
        text(242,111,"F SHOT "+number(game.player.ammo),paper);
        text(242,122,"M MED  "+number(game.player.meds),paper);
        text(242,133,"R FOOD "+number(game.player.rations),paper);
        text(242,144,"T LURE "+number(game.player.scrap),paper);
        line(242,157,309,157,darkGold);
        text(242,162,game.player.seal?(game.deck==2?"E ESCAPE":"E LIFT"):"FIND SEAL",gold);
        text(242,175,"HYMN "+std::to_string(game.choirStage())+"/3",rose);
        // Inspection never reveals an unseen cell or an enemy outside visibility.
        std::string inspect;
        if(mouseX>=8 && mouseX<232 && mouseY>=40 && mouseY<184) {
            Pos p{cx+(mouseX-8)/16,cy+(mouseY-40)/16};
            if(game.at(p).visible) {
                SDL_Rect focus{8+(p.x-cx)*16,40+(p.y-cy)*16,16,16};
                SDL_SetRenderDrawColor(renderer,gold.r,gold.g,gold.b,255);SDL_RenderDrawRect(renderer,&focus);
                for(const auto& e:game.enemies) if(e.pos==p) inspect=Game::enemyName(e.kind)+(e.kind==Kind::Legionary?" / DO NOT ENGAGE":" / F TO SHOOT");
                if(p==game.player.pos) inspect="SERF 713 / WORK KNIFE, STUB PISTOL, NO ARMOUR";
                if(inspect.empty()) {
                    switch(game.at(p).tile) {
                    case Tile::Seal:inspect="BRASS SEAL / WALK OVER TO UNLOCK TRANSIT";break;
                    case Tile::Shrine:inspect="E: FULL HEALTH FOR +22 TAINT. IT IS A CHOICE.";break;
                    case Tile::Survivor:inspect="E: MARK A SAFE ROUTE FOR A FELLOW SERF";break;
                    case Tile::Lift:inspect=game.objective();break;
                    case Tile::Warp:inspect="WARP RUPTURE / CROSSING COSTS HEALTH AND TAINT";break;
                    case Tile::Door:inspect="SEALED BULKHEAD / MOVE INTO IT TO OPEN";break;
                    case Tile::Machinery:inspect="DEAD MACHINERY / BLOCKS MOVEMENT AND SIGHT";break;
                    default:break;
                    }
                }
            }
        }
        line(8,188,311,188,darkGold);
        if(!inspect.empty()) wrapped(8,193,inspect,50,gold,2);
        else if(!game.messages.empty()) {
            bool warning=false;
            for(const auto& e:game.enemies) if(game.at(e.pos).visible && e.kind==Kind::Legionary) warning=true;
            const auto& m=game.messages.back();
            if(warning) { text(8,193,"LEGIONARY IN SIGHT. BREAK SIGHT. DO NOT FIGHT.",rose);wrapped(8,204,m.text,50,muted,1); }
            else wrapped(8,193,m.text,50,m.danger?rose:paper,2);
        }
        text(8,216,"WASD MOVE  E USE  TAB MAP  F1 HELP",muted);
        if(game.outcome!=Outcome::Alive) endScreen(game);
    }
    if(chart && !title && game.outcome==Outcome::Alive) mapScreen(game);
    if(help) helpScreen();
}
} // namespace choir
