# Compiling PSXFunkin

## Setting up the Development Environment
First off, you'll need a terminal one way or another.

### Windows
On Windows, you basically have two choices:
- MSYS2 (follow the instructions at https://www.msys2.org/ to set it up)
- WSL (Windows 10 Linux terminal from the Windows Store, maintained by Microsoft, follow Linux instructions from here on).

### MSYS2
Once you have MSYS2 set up, you'll need to copy the MIPS toolchain over. Download it here http://static.grumpycoder.net/pixel/mips/g++-mipsel-none-elf-11.2.0.zip.

Once you have it downloaded, make sure MSYS2 is closed, then open the zip up and extract the following folders into `C:/msys64/usr/local/`
- bin
- include
- lib
- libexec
- mipsel-none-elf

**IMPORTANT - DELETE THESE FILES IN `bin` OR YOUR TERMINAL WILL BE FUCKED**
- cat
- cp
- echo
- make
- mkdir
- touch
- rm
- touch

Next, open up `MSYS2 MinGW 64-bit` from the Start Menu, and you'll need to install some libraries, so run the following command and accept the prompts that follow `pacman -S mingw-w64-x86_64-cmake mingw-w64-x86_64-tinyxml2 mingw-w64-x86_64-ffmpeg `

### Linux
First, you'll need to install the GCC toolchain, run one of the following commands depending on your distro.

On Debian derivatives (Ubuntu, Mint...):

`sudo apt-get install gcc-mipsel-linux-gnu g++-mipsel-linux-gnu binutils-mipsel-linux-gnu`

On Arch derivatives (Manjaro), the mipsel environment can be installed from AUR : cross-mipsel-linux-gnu-binutils and cross-mipsel-linux-gnu-gcc using your AURhelper of choice:

`trizen -S cross-mipsel-linux-gnu-binutils cross-mipsel-linux-gnu-gcc`

You'll also need to install `ffmpeg` (you may also need to install `avformat`, `avfilter`, and `swscale` separately), and `cmake`, which of course, depends on your distro of choice.

## Compiling mkpsxiso
First, `cd` to somewhere safe where you can put source code (such as `Documents` or the PSXFunkin folder). Then run these few commands.

- `git clone https://github.com/CookiePLMonster/mkpsxiso`

- `cd mkpsxiso`

- `git submodule update --init --recursive --remote`

- `cmake -B build -DCMAKE_BUILD_TYPE=Release` (add `-G "MinGW Makefiles" --install-prefix /usr/local/` to the end of this if you're using MSYS2)

- `cmake --build build --config Release`

- `cmake --install build`

## Copying PsyQ files
First, go to the [mips](/mips/) folder of the repo, and create a new folder named `psyq`.

Then, download the converted PsyQ library from http://psx.arthus.net/sdk/Psy-Q/psyq-4_7-converted-light.zip. Just extract the contents of this into the new `psyq` folder.

## Compiling PSXFunkin
First, make sure to `cd` to the repo directory where all the makefile is. You're gonna want to run a few commands from here.

Then you must run a few makes to convert assets and compile the code.
- `make -f Makefile.tools`
- `make -f Makefile.assets`
- `make`

TIP: For `make`, try appending `-jX` to the end of it, where X is the number of CPU cores you have times two. This will try to put as much of your CPU as it can to doing whatever it needs to do and makes it go way quicker.

You'll need to either get a PSX license file and save it as licensea.dat in the same directory as funkin.xml (you can get them at http://www.psxdev.net/downloads.html's `PsyQ SDK`), or remove the referencing line `<license file="licensea.dat"/>` from funkin.xml. Without the license file, the game may fail on a bunch of emulators due to bios checks (unless you use fast boot, I believe?)

Finally, you can run `mkpsxiso -y funkin.xml`, which will create the `.bin` and `.cue` files using the ps-exe and assets in `iso/`.

## Modifying the game
You can read more about the file formats used by the game and the conversion process in [FORMATS.md](/FORMATS.md)
