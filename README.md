# LLGI

How to build
----------

### Windows

```
$ git clone https://github.com/altseed/LLGI.git
$ cd LLGI
$ git submodule update --init
$ cmake -S . -B build -DBUILD_TEST=ON
$ cmake --build build
```

### macOS

```
$ git clone https://github.com/altseed/LLGI.git
$ cd LLGI
$ git submodule update --init
$ cmake -S . -B build -G "Xcode" -DBUILD_TEST=ON
$ cmake --build build
```

### Vulkan(Window, Linux)

```
$ git clone https://github.com/altseed/LLGI.git
$ cd LLGI
$ git submodule update --init
$ cmake -S . -B build -DBUILD_VULKAN=ON -DBUILD_TEST=ON
$ cmake --build build
```

### WebGPU

Install depod-tools. Please read dawn documents

```
$ git clone https://github.com/altseed/LLGI.git
$ cd LLGI
$ git submodule update --init
$ cd thirdparty
$ git clone https://dawn.googlesource.com/dawn dawn && cd dawn
$ cp scripts/standalone.gclient .gclient
$ gclient sync
```

Test
----------

Run with Vulkan
```
./LLGI_Test --vulkan
```

Run with single test

```
./LLGI_Test --filter=<TestName*>
```
