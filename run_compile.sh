# Note: You must specify the cpp filename to compile.
#       e.g., ./run_compile.sh triangle.cpp
g++ ./src/$1 ./src/*.c -o application.exe -I./include -I./deps/glfw/include -L./deps/glfw/src -lglfw3 -lXrandr -lXcursor -lXi -lXinerama