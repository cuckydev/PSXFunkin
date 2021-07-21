# PSXFunkin
Friday Night Funkin' on the PSX LOL

## Compilation
This port uses the Nugget + PsyQ SDK.

First, you'll need to install MIPS GCC stuff in order to compile the source.

Then, you'll need to get the converted PsyQ SDK's include and lib folders, and extract them to mips/psyq/.

You can find more information regarding these here: https://github.com/ABelliqueux/nolibgs_hello_worlds

Once you have the SDK setup and have compiled the game into funkin.ps-exe, you'll need mkpsxiso to create a .iso for the game.

You can find mkpsxiso here, you will have to compile it yourself since the latest release is broken: https://github.com/Lameguy64/mkpsxiso

You'll also need to either get a PSX license file and save it as licensea.dat in the same directory as funkin.xml, or remove the referencing line `<license file="licensea.dat"/>` from funkin.xml.

## Disclaimer
This project is not endorsed by the original Friday Night Funkin' devs, this is just an unofficial fan project because I was bored.

Assets such as Week 7, VS Kapi, VS Tricky are used with permission from their respective owners.
