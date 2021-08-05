# utf_convert

Other languages:

[简体中文](./README.md)

## Introduction

First, I'd have to say that my English is poor but I'll try my best to introduce this project.

This is a C++ library for UTF convertion, except for code associated with UTF-16 until I figure it out.

The whole project contails a single header file and a cpp file in total, ~~but code written in the cpp file is ugly~~.

## Compile

Eventhough you can compile this project with C++98 standard, C++11 is suggested.

### Add to your project

Simple copy the hpp file and cpp file to your project then you can use it. CMake is simply used for test. Of course you can also build static library using this CMakeLists.txt:

```shell
mkdir build
cd build
cmake ..
make
```

Then you can find the static library in directory lib.

### test

Follow the following commands to test:

```shell
mkdir build
cd build
cmake ..
make
make test
```
