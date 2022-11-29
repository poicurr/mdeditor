mkdir build
cd build

cmake ..
cmake --build . --config Release

cd ..
copy .\build\src\Release\main.bin.exe .\editor\bin\

cd .\editor
start node .\app.js
start http://127.0.0.1:8000
