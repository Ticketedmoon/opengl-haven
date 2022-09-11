mkdir -p external
cd external
git clone https://github.com/glfw/glfw.git
cd glfw
mkdir build
cd build

# Ubuntu (Change me if you have a different package manager)
sudo apt-get install libxrandr-dev -y
sudo apt-get install libxinerama-dev -y
sudo apt-get install libxcursor-dev -y
sudo apt-get install libxi-dev -y

cmake ..