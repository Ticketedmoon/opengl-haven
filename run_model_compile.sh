g++ ./src/examples/models/lib/*.cpp \
    ./src/examples/models/survival_backpack.cpp \
    ./src/*.c \
    -o application.exe \
    -I./include \
    -I./src/examples/models/headers \
    -I./deps/glfw/include \
    -I./deps/assimp/include \
    -L./deps/glfw/src \
    -L./deps/assimp/bin \
    -lglfw3 \
    -lassimp \
    -lXrandr \
    -lXcursor \
    -lXi \
    -lXinerama
