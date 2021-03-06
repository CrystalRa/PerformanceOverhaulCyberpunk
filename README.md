# Cyber Engine Tweaks

## What's this?

This plugin fixes some Cyberpunk 2077 issues and adds some features.

Works with 1.04 and 1.05.

### Current fixes

| Patches      | Description     |
| :------------- | :------------------------------ | 
| AMD SMT  | Improves performance on AMD CPUs by enabling all cores. |
| AVX | Fixes a crash when playing the game with a CPU that does not support AVX |
| Debug Menu   | Enables the debug menus in game so you can ~~cheat~~, investigate...  |
| Pools | Improves memory usage, can improve performance on some configurations. |
| Spectre | Removes spectre mitigation to improve performance on all configurations. |
| Steam Input | Enables the use of more gamepads (such as the Steam gamepad) |
| Skip start menu | Skips the menu asking you to press space bar to continue (Breaching...) |
| Remove pedestrians and traffic | Removes most of the pedestrians and traffic |
| Disable Async Compute | Disables async compute, this can give a boost on older GPUs (Nvidia 10xx series for example)|
| Disable Antialiasing TAA | Disables antialiasing, not recommended but you do what you want! |
| Console | Adds an overlay to draw a console so you can write any kind of console command |

### Upcoming

* Memory allocation performance
* Skip conditions that never fail
* Probably some pattern conversion to SIMD

## Usage and configuration

[Read the wiki](https://github.com/yamashi/PerformanceOverhaulCyberpunk/wiki)

[Usage with Proton](PROTON.md)

## Credits

* [UnhingedDoork](https://www.reddit.com/r/Amd/comments/kbp0np/cyberpunk_2077_seems_to_ignore_smt_and_mostly/gfjf1vo/?utm_source=reddit&utm_medium=web2x&context=3)
* [CookiePLMonster](https://www.reddit.com/r/pcgaming/comments/kbsywg/cyberpunk_2077_used_an_intel_c_compiler_which/gfknein/?utm_source=reddit&utm_medium=web2x&context=3)
* [SirLynix](https://github.com/DigitalPulseSoftware/BurgWar) for the CI file.
* [emoose](https://github.com/yamashi/PerformanceOverhaulCyberpunk/issues/75) for pedestrian removal and start menu skip research.
* [WopsS](https://github.com/WopsS/RED4ext) for being a good friend and for doing an excellent research on scripting.
