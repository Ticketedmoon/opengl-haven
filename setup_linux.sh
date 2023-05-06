# Ubuntu (Change me if you have a different package manager)
sudo apt-get install libxrandr-dev -y
sudo apt-get install libxinerama-dev -y
sudo apt-get install libxcursor-dev -y
sudo apt-get install libxi-dev -y
sudo apt-get install pkg-config -y
sudo apt-get install zlib1g-dev -y

mkdir -p deps

cd deps
git clone https://github.com/glfw/glfw.git
git clone https://github.com/assimp/assimp.git
git clone https://github.com/ocornut/imgui.git

cd glfw
cmake -G "Unix Makefiles"
make

cd ../assimp
cmake CMakeLists.txt
cmake --build .

cd ../../
mkdir -p build

cmake .

cd ./build
# Use 4 cores / parallel processes for the build
make -j4 

cd ..
echo "Adding assimp shared library to /usr/lib [unix]"
sudo cp ./deps/assimp/bin/libassimp.so.5 /usr/lib
