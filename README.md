# vv

`vv` is a simple image viewer written in C using raylib.

## Dependencies

* [raylib](https://github.com/raysan5/raylib) - what an awesome project!!!
* `make`

### With JPG support

Since I wanted also jpg support, I needed to clone, modify config and build raylib myself.
Here are the steps I did:

```sh
git clone --depth 1 https://github.com/raysan5/raylib.git raylib
cd raylib/src/
sed -i 's/\/\/#define SUPPORT_FILEFORMAT_JPG/#define SUPPORT_FILEFORMAT_JPG/g' config.h
make PLATFORM=PLATFORM_DESKTOP 
```

Compiling `vv` with jpg support

```sh
cc -Wall -Wpedantic -o vv main.c ./raylib/src/*.o -I./raylib/src -L./raylib/src -lm -lpthread -DSUPPORT_FILEFORMAT_JPG
```

### Without JPG support

```sh
wget https://github.com/raysan5/raylib/releases/download/5.0/raylib-5.0_linux_amd64.tar.gz
tar -xvzf ./raylib-5.0_linux_amd64.tar.gz
rm ./raylib-5.0_linux_amd64.tar.gz
```

Compiling `vv` without jpg support

```sh
cc -Wall -Wpedantic -o vv main.c -I./raylib-5.0_linux_amd64/include -L./raylib-5.0_linux_amd64/lib -l:libraylib.a -lm -lpthread
```

## Synopsis

```sh
vv [OPTIONS] <dir|file> ...
```

## Options

```verbatim
* -h, -help, --help, help
	print this help
```

## Usage

```verbatim
* q
	close the application
* r/shift+r
	rotate image clockwise/counter-clockwise
* =
	reset image
* LEFT_MOUSE_BUTTON
	go to previous image
* MIDDLE_MOUSE_BUTTON
	drag current image
* scroll MIDDLE_MOUSE_BUTTON
	zoom in/out current image
* RIGHT_MOUSE_BUTTON
	go to next image
```

## Author

Meelis Utt
