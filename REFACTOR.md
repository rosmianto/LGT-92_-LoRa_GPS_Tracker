For context this is the firmware codebase for Dragino LGT92. The code quality is awful, and I'm trying to refactor to improve readability, modularity, and maintainability.

Since I don't have the hardware, I need to make a deliberate effort to decouple the application layer (aka business logic) with the hardware-dependent codebase so I can test everything on host machine (aka my macbook laptop) and not breaking any business logic.

My technical judgments so far are like these:

* CMake build system instead of raw Makefile
* C++ instead of C (but this looks like a daunting task)
* C++ Interface and Dependency Injection to separate hardware vs application layer
* Ability to recompile the firmware codebase via Emscripten to WASM so it can be run inside a browser (cool portfolio to show off)
* Use lwgps (https://github.com/MaJerle/lwgps) instead of ugly GPS NMEA sentence parser
* Use Fusion (https://github.com/xioTechnologies/Fusion) instead of ugly (and broken) AHRS algorithm
* Treat the libraries as CMake libraries

I've done the preliminary effort to understand the codebase, and it looks like the codebase is actually
a hacked-together version of Semtech and ST examples projects. There are lots of dead code, dead modules, dead functions. As well as dead files.

I've done a refactor manually, but the codebase is hell and extremely difficult to tame.