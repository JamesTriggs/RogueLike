# Choir of Ruin

## Brief

A turn-based survival roguelike in C++17. You are Serf 713, an ordinary bonded
voidsman aboard the Emperor's Children cruiser *Canticle of Excess*. As its
officers embrace forbidden perfection, the ship becomes a hunting ground.
Reach the launch crypt and escape. Saving other serfs is optional and costly.

This is an original fan scenario during the Legion's fall around the Horus
Heresy, in the history of the Warhammer 40,000 setting. The ship, characters
and dialogue are invented. It is not a depiction of a named canonical event.

## Playable scope and specification

- Three procedurally arranged, connected decks with fog of war and solid walls.
- Find a deck seal, then operate the lift. On deck three, launch the cutter.
- Four-direction movement, bump melee, ammunition-limited shooting, medicae,
  resolve-restoring rations, noise-making thrown scrap and a quiet movement mode.
- Enemies investigate noise, pursue visible targets and search last known
  positions. A corrupted Legionary is overwhelmingly stronger than a serf.
- Noise has a location and limited lifetime. Closed bulkheads block vision.
- Health, resolve and corruption carry between decks. Each deck has a rising
  choir clock. No real-time pressure, only successful actions advance time.
- One rescue and one tainted shrine per deck. Rescue provides hope and ending
  credit. The shrine offers recovery at a permanent corruption cost.
- Death and possession end a run. Restart generates a fresh seed.
- Graphical SDL2 frontend, keyboard controls, resizable letterboxed view,
  no flashing or continuous animation. Colour is reinforced by glyphs and text.
- No remote services, external art, runtime font downloads or telemetry.

## Resources considered

Read WORK_RESOURCES.md, ENGINEERING_RESOURCES.md, UI_DESIGN_RESOURCES.md and the
design reference playbook. Select deterministic seeds, an engine independent
of rendering, failure-path tests and a full three-deck journey test. Verify
generation over many seeds and exercise the actual frontend with SDL events.

Reference observations, 20 October 2026:

- [Brogue](https://sites.google.com/site/broguegame/): a compact symbolic map,
  explicit controls and finite objectives. Apply a persistent map legend and
  visible objective. No code or artwork reused.
- [Dungeon Crawl Stone Soup](https://crawl.develz.org/): map-led play and
  distinct threats. Apply a large tactical viewport and named visible-threat
  panel. No code or artwork reused.
- [Warhammer Community's Legion introduction](https://www.warhammer-community.com/en-gb/articles/n9NCTAu5/legions-of-the-horus-heresy-no-ones-perfect-except-the-emperors-children/):
  the Legion's perfectionism and Laer-linked fall anchor the premise.
  All playable incidents and prose are original.

## Presentation

Obsidian backgrounds, tarnished brass rules, parchment body text, restrained
violet for the Legion and rose for corruption. Original geometric pixel art
suggests ribbed bulkheads, grating, devotional machinery and ruptured shrines.
The user's visual direction is Sega Mega Drive era graphics. Render at a
320x224 logical resolution with integer scaling and nearest-neighbour pixels.
Use original 16x16 pixel sprites, a scrolling tactical camera, metal deck tiles,
ribbed bulkheads and an illustrated voidship title screen. Text uses a bundled
code-defined 5x7 bitmap alphabet. The map is never animated.
The UI reads: ship header, deck and objective, tactical viewport, serf status,
visible threats, chronological messages, persistent controls. Help, death and
escape use the same language.

## Build plan

1. Preserve the C# prototype under legacy and introduce CMake targets.
2. Write engine contract tests, then implement generation and turn resolution.
3. Build the SDL2 renderer, title/help/end states and input mapping.
4. Run tests, sanitiser checks, cppcheck and a graphical smoke capture.
5. Record results and local launch instructions in README.md.

## Acceptance

Generation is repeatable and all objectives reachable across 200 seeds per
deck. Walls and doors occlude vision. Invalid actions spend no turns. Enemy
turns, noise, consumables, deck seals, rescue, temptation, death and escape
have tests. A fresh run can traverse all three decks through public actions.
The SDL2 application builds and a real event-driven smoke run creates a local
screenshot. Manual play balance remains an ongoing design task.
