#!/bin/bash

glfw="-DGLFW_BUILD_DOCS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF"
chipmunk="-DCMAKE_POSITION_INDEPENDENT_CODE=ON -DBUILD_SHARED=OFF -DBUILD_DEMOS=OFF"
freetype="-DCMAKE_POSITION_INDEPENDENT_CODE=ON"

if [[ "$RUNNER_OS" == Windows ]]
then
    extra=" -A $BASE"

    glfw+=${extra}
    chipmunk+=${extra}
    freetype+=${extra}

elif [[ "$RUNNER_OS" == macOS ]]
then
    extra=" -DCMAKE_OSX_ARCHITECTURES=$BASE"

    glfw+=${extra}
    chipmunk+=${extra}
    freetype+=${extra}

elif [[ "$RUNNER_OS" == Linux ]]
then
    if [[ "$BASE" == Musl ]]
    then
        apk update
        apk add libxcursor-dev libxi-dev libxinerama-dev libxrandr-dev libpng-dev
    else
        yum install libxcursor-devel libxi-devel libxinerama-devel libxrandr-devel libpng-devel
    fi
fi

git clone https://github.com/glfw/glfw.git
git clone https://github.com/slembcke/Chipmunk2D.git
git clone https://gitlab.freedesktop.org/freetype/freetype.git

cmake -S glfw -B glfw/build ${glfw}
cmake -S Chipmunk2D -B Chipmunk2D/build ${chipmunk}
cmake -S freetype -B freetype/build ${freetype}

cmake --build glfw/build --config Release
cmake --build Chipmunk2D/build --config Release
cmake --build freetype/build --config Release