echo -n "Building project: "
echo source/*.c
gcc -Wall \
 -Os -fdata-sections -ffunction-sections -fno-asynchronous-unwind-tables \
 -I include/ source/*.c \
 -mwindows -nostdlib -e start -Wl,--gc-sections -s \
 -L lib/ -l opengl32 -l glu32 -l glfw3dll \
 -l msvcrt -l kernel32 -l shell32 -l user32 -l gdi32 -l gdiplus \
 -o build/build-new.exe
gcc \
 -Os -fdata-sections -ffunction-sections -fno-asynchronous-unwind-tables \
 -I include/ source/*.c \
 -mwindows -nostartfiles -e start -Wl,--gc-sections -s \
 -L lib/ -l opengl32 -l glu32 -l glfw3 -l gdiplus \
 -o build/build-static.exe
echo
if test -a build/build-new.exe; then
 mv -f build/build-new.exe build/build.exe
 if test -a build/build-static.exe; then
  upx -9 build/build-static.exe
  echo
 fi
 tput setaf 2
 echo "Build complete. Running..."
 tput setaf 7
 pushd build/ >/dev/null
 ./build.exe
 result=$?
 popd >/dev/null
 echo
 if test $result -ne 0; then
  tput setaf 3
 fi
 echo "Exited with ${result}."
else
 rm -f build/build-static.exe
 tput setaf 3
 echo "Build failed."
fi
read -s -n 1
