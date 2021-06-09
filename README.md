# PSXFunkin
Friday Night Funkin' on the PSX lol

## Compilation
This port uses modern GCC and the PsyQ SDK, along with pcsx-redux's.. stuff

You can find all the information required for this here: https://github.com/ABelliqueux/nolibgs_hello_worlds

mkpsxiso is also required to create the final .iso and .cue files for the game: https://github.com/Lameguy64/mkpsxiso

You'll also need to either get a PSX license file and save it as licensea.dat in the same directory as funkin.xml, or remove the referencing line `<license file="licensea.dat"/>` from funkin.xml

Once you have everything set up, run `make` then `mkpsxiso funkin.xml`

## Disclaimer
This project is not endorsed by the original Friday Night Funkin' devs, this is just an unofficial fan project because I was bored.
