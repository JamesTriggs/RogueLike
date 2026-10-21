# Local verification

20 October 2026. macOS, AppleClang 21.0.0, C++17, locally installed SDL2.
All builds, test runs and captures were performed on this device.

## Results

| Check | Result |
| --- | --- |
| Release CMake build, app and test targets | Passed |
| CTest `engine_contracts` | Passed |
| CTest `pixel_renderer` | Passed |
| AddressSanitizer and UndefinedBehaviorSanitizer, both test targets | Passed |
| Sanitised SDL application, real keyboard-event smoke | Passed, 16 turns |
| Release SDL application, real keyboard-event smoke | Passed, 16 turns |
| Small-window title and full-size help captures | Passed |
| cppcheck warning, performance and portability checks | Passed without findings |
| Shell launcher syntax and `git diff --check` | Passed |
| Invalid seed and unknown command-line option | Rejected with explanatory errors and status 1 |

Engine coverage includes 200 seeds on each of three decks, repeatability,
objective reachability, valid spawn locations, item consumption, invalid-action
turn costs, bulkheads, machinery, vision, stance detection, noise investigation
and expiry, attacks, supplies, rescue, temptation, hope depletion, choir
escalation, environmental death, possession and escape. Thirty additional
seeded action simulations check resource bounds and actor placement.

The three-deck journey starts a fresh game, walks to each real seal and lift,
opens the necessary doors and escapes through public actions. It removes
enemies to isolate progression. It does **not** prove a balanced combat run.

Renderer tests draw seven distinct screen states without a display server:
title, help, gameplay, remembered map, death, possession and escape. They check
that changing unseen objects and adding an unseen enemy does not change the
gameplay image, and that rendering does not advance turns. Capture fixtures
for endings exercise presentation independently of progression tests.

The native SDL smoke runner sends Enter, F1, Escape, movement, wait and Tab
events through the real input handler. It verifies that the player moves and
turns advance, that overlays close, and that the frame can be captured.

## Visual review

Inspected native gameplay, title and help screenshots, plus software-rendered
map and escape screens. Checked glyph legibility, resource numbers, distinct
enemy and player sprites, fog memory, walls, item contrast and consistent
screen styling. Added green foot markers to distinguish the serf, and
shortened the help prose to fit its panel. All screens are static, with no
flashing or motion dependency. Colour is supplemented by shapes and text.

Local screenshots are under `docs/screenshots/`. The 320x224 base resolution
is scaled to whole pixels, with a 640x448 minimum desktop window. The game
does not target a mobile browser or offer alternate colour themes.

## Limits

Human survival balance and a complete, combat-enabled escape run still need
playtesting. This is a first playable slice, without sound, save/load,
controller support or a larger campaign. Only macOS was built and exercised.
Windows and Linux compatibility is intended by the SDL2/CMake architecture
but remains unverified.

The previous C# solution and its already tracked generated files were moved
to `legacy/` intact. They are excluded from the new CMake build. Nothing was
published or deployed as a hosted application.
