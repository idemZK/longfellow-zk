# README_ZAC



## Building Emscripten

* You can either build it from the source: https://github.com/emscripten-core/emscripten
doc: https://emscripten.org/docs/building_from_source/index.html#installing-from-source


* Or you can build it with the sdk: https://github.com/emscripten-core/emsdk
doc: https://emscripten.org/docs/getting_started/downloads.html


## Get set up with the wasm dependancies for the static mdoc libray

* Download openssl-wasm, it has pre-compiled binaries: https://github.com/jedisct1/openssl-wasm 
  
## Try to recompile the static mdoc library into a webassembly shared library

* The goal is to run 
    ```
        clang++ mdoc.cc -o mdoc ../install/lib/libmdoc_static.a -I../lib -L/opt/homebrew/opt/zstd/lib -L/opt/homebrew/opt/openssl@3/lib -lzstd -lcrypto
    ```
with em++ instead of clang++. but we need the -I pathes to include webassembly formatted libaries. Use the precompiled openssl-wasm libraries for openssl, (there probably is something like that for zstd as well).


The issue is getting libmdoc_static.a to be compiled using an emscripten tool. 

```
    ./em++ -msimd128 -mfpu=neon -std=c++17 ../lib/circuits/mdoc/mdoc_zk.cc -I../lib/ -I/opt/homebrew/include -L/opt/homebrew/opt/zstd/lib -L/opt/homebrew/opt/openssl@3/lib -lzstd -lcrypto -o libmdoc_static.a -c
```


Documentation for this is in https://emscripten.org/docs/porting/simd.html . You will get ```tools/simde_update.py``` only if you build from source which you need llvm for, but the option necessary for arm_neon.h is really ``` -mfpu=neon```.


* Also if you want into the archiver follow the ./emar command.

Good luck!






