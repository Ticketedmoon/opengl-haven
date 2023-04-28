g++ ./src/examples/instancing/advanced/asteroid_field/lib/*.cpp \
    $1 \
    ./src/*.c \
    -o application.exe \
    -I./include \
    -I./src/examples/models/headers \
    -I./src/examples/instancing/advanced/asteroid_field/headers \
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
