# Seabsy (C++ Compiler)

Seabsy is a hobby programming language written in C++. I am working on it solely for educational purposes and nothing more is (yet) expected to come out of it. It currently only compiles to ARM64 assembly.
The inspiration (and kickstart) for this project I owe to the one and only [Pixeled](https://www.pixeled.site)

## Building

```bash
git clone https://github.com/lseab/seabsy
cd seabsy
mkdir build
cmake -S . -B build
cmake --build build
```

Executable will be `seabsy` in the `build/` directory.

## Testing

Tests are written using Catch2.

```bash
cmake --build build
./build/tests
```