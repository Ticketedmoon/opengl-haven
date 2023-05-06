g++ \
    ./src/examples/instancing/advanced/asteroid_field/lib/*.cpp \
    ./deps/imgui/*.cpp \
    ./deps/imgui/backends/imgui_impl_opengl3.cpp \
    ./deps/imgui/backends/imgui_impl_glfw.cpp \
    $1 \
    ./src/*.c \
    -o application.exe \
    -I./include \
    -I./src/examples/models/headers \
    -I./src/examples/instancing/advanced/asteroid_field/headers \
    -I./deps/imgui \
    -I./deps/imgui/backends \
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
