# LLGI

How to build
----------

### macOS

```
$ git clone https://github.com/altseed/LLGI.git
$ cd LLGI
$ git submodule update --init
$ cmake -S . -B build -G "Xcode" -DBUILD_TEST=ON
$ cmake --build build
```

