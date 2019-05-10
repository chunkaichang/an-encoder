An LLVM-based AN encoder (originally developed by [Norman Rink](https://github.com/normanrink/an-encoder)).

Encoding an application with AN code requires the following steps:
1. Encode and decode the input and output of the application
2. Generate LLVM bitcode for the application
3. Applies AN code to instructions in bitcode
4. Generate binary 

Step 1 requires the user to annotate the source code with encoding/decoding macros.
Step 2 and 4 is done using `clang`.
Step 3 is fullfiled with a series of LLVM passess wrapped in a single binary called `encode`.


# Build

## Test Platform
`OpenSUSE 42.3` with `gcc 4.8.5` and `cmake 3.12`


## Build LLVM 3.5
```
    mkdir llvm-3.5-build && cd llvm-3.5-build
    cmake ../llvm-3.5-src
    cmake --build . -j 4
```

These commands should build LLVM 3.5 and Clang 3.5 in `llvm-3.5-build`.

## Build AN encoder
```
    mkdir encoder-build && cd encoder-build
    cmake .. -DLLVM_DIR=<$root_dir/llvm-3.5-build/cmake/modules/CMakeFiles> -DLLVM_MORE_INCLUDE_DIRS=<$root_dir/llvm-3.5-src/include>
    cmake --build .
```
where `$root_dir` is the root directory of this repository.
 
These commands should build the `encode` binary in the `encoder-build/bin` directory.


# Run 

## Quick start

1. Create a directory for the application to encode under the `encode/tests` directory.
2. Annotate the source code to encode/decode input and ouput, respectively.
3. Add a `CMakeLists.txt` for the application in the created directory.
4. Open `encode/tests/CMakeLists.txt` and add the created directory with `add_subdirectory`.
5. Modify the encoding profile (see [below](#encoding-profile)).
6. Rebuild

## Encoding profile

An encoding profile is used to configure the AN encoder.
It consists of a list of directives delimitted by semicolons. 
There are two types of directives: *profile directives* and *operation-position directives*.

Here is the default encoding profile at `encode/tests/test.ep`:

```
Store: Before;
CheckAfterDecode;
```
The first line is an operation-position directive,
which specifies error checking should be performed before each store.

The second line is a profile directives,
which specifies error checking should happen after each decode operation.

Decode operations are inserted in the following scenarios:
- Library function calls
- Store operations (since address needs decoding)
- Specific operations (e.g., bitwise and)
 
### Profile directives

### Operation-position directives


## Detailed steps

### How to use the `encode` binray

This binary takes in an LLVM bitcode, applies AN-code, and outputs transformed bitcode.
The basic usage is:

```
    ./encode -p <encoding_profile> -o <output_bitcode> <input_bitcode>
```

Run `./encode --help` for other flags. 

