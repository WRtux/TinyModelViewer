echo -n "Building project: "
echo source/*.c
gcc -Wall \
 -Os -fdata-sections -ffunction-sections -fno-asynchronous-unwind-tables \
 -mwindows -nostdlib -e start -Wl,--gc-sections -s \
 -I include/ source/*.c -L lib/ -l msvcrt -l kernel32 -l user32 -l gdiplus -l opengl32 -l glu32 -l glfw3dll \
 -o build/build-new.exe
echo
if test -x build/build-new.exe; then
 mv -f build/build-new.exe build/build.exe
 tput setaf 2
 echo "Build complete. Running..."
 tput setaf 7
 ./build/build.exe
 result=$?
 echo
 if test $result -ne 0; then
  tput setaf 3
 fi
 echo "Exited with ${result}."
else
 tput setaf 3
 echo "Build failed."
fi
read -s -n 1
