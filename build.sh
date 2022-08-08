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

elif [[ "$RUNNER_OS" == Linux ]]
then
    if [[ "$BASE" == Manylinux ]]
    then
        apt-get update
        apt-get install -y xorg-dev
    else
        apk update
        apk add libxcursor-dev libxi-dev libxinerama-dev libxrandr-dev libpng-dev
    fi
fi

curl https://github.com/glfw/glfw/archive/refs/heads/master.zip -O -J -L
curl https://github.com/slembcke/Chipmunk2D/archive/refs/heads/master.zip -O -J -L
curl https://gitlab.freedesktop.org/freetype/freetype/-/archive/master/freetype-master.zip -O -J -L

unzip glfw-master.zip
unzip Chipmunk2D-master.zip
unzip freetype-master.zip

cmake -S glfw-master -B glfw-master/build ${glfw}
cmake -S Chipmunk2D-master -B Chipmunk2D-master/build ${chipmunk}
cmake -S freetype-master -B freetype-master/build ${freetype}

cmake --build glfw-master/build --config Release
cmake --build Chipmunk2D-master/build --config Release
cmake --build freetype-master/build --config Release