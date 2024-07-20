# vv

Simple image viewer written in C using raylib.

## Dependencies

* [raylib](https://github.com/raysan5/raylib) - what an awesome project!!!
* make

Since I needed jpg support, I needed to clone, modify config and build raylib myself.
Here are the steps I did:

```sh
git clone --depth 1 https://github.com/raysan5/raylib.git raylib
cd raylib/src/
sed -i 's/\/\/#define SUPPORT_FILEFORMAT_JPG/#define SUPPORT_FILEFORMAT_JPG/g' config.h
make PLATFORM=PLATFORM_DESKTOP 
```

## Compiling

```sh
cc -Wall -Wpedantic -o vv main.c ./raylib/src/*.o -I./raylib/src -L./raylib/src -lm -lpthread -DSUPPORT_FILEFORMAT_JPG
```

## Author

Meelis Utt
