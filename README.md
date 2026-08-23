# Dragino LGT92 Firmware Codebase Rewrite/Redesign

How to compile:

mkdir build
cd build
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_TOOLCHAIN_FILE=../cmake/gcc-arm-none-eabi.cmake ..
cp ./compile_commands.json ../.vscode/compile_commands.json