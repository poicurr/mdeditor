mkdir build
cd build

cmake ..
cmake --build . --config Release

cd ..

start .\build\src\Release\main.bin.exe
start http://127.0.0.1:8000
