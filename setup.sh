mkdir -p deps
cd deps
git clone https://github.com/glfw/glfw.git
cd glfw

# Ubuntu (Change me if you have a different package manager)
sudo apt-get install libxrandr-dev -y
sudo apt-get install libxinerama-dev -y
sudo apt-get install libxcursor-dev -y
sudo apt-get install libxi-dev -y

cmake -G "Unix Makefiles"
make

cd ../../
mkdir -p build

cmake .

cd ./build
make