# utf_convert

### 其他语言

[English](./README_en.md)

## 简介

这是一个完全由C++实现的UTF转换的代码，但是UTF-16相关的代码还没有写，~~因为作者还没有弄明白UTF-16~~。

代码总共只有一个头文件和一个cpp文件，~~但cpp文件里写得很丑~~只需要将它们包含到你的工程中就可以使用了。

## 编译

虽然C++98可以编译通过，但还是建议启用C++11。~~不会吧不会吧，不会2021年了还有人在用C++98吧~~

### 放到你的项目中

把hpp文件放入你的include path中，把cpp文件随你的项目一起编辑即可，CMake只是用来辅助测试的。测试请直接make test，~~因为test代码我一点注释也没写~~。当然，你也可以选择使用CMake生成静态库然后链接静态库。生成方法如下：

```shell
mkdir build
cd build
cmake ..
make
```
然后在lib文件夹下会生成静态库。

### 测试

测试方法如下：

```shell
mkdir build
cd build
cmake ..
make
make test
```
