******************************* ENCODE ********************************
* A simple "AN encoder" based on the LLVM compiler framework and the  *
* CLANG compiler front-end.                                           *
*                                                                     *
* Created: 2015-03-03                                                 *
* Author: Norman Rink                                                 *
*         TU Dresden                                                  *
*         Center for Advancing Electronics Dresden (cfaed)            *
*         Chair for Compiler Construction                             *
*         Georg-Schumann-Str. 7A                                      *
*         01062 Dresden, GERMANY                                      *
***********************************************************************



*** Licensing and acknowledgements: ***

LLVM and CLANG are subject to the University of Illinois Open Source
License. Please refer to the LLVM project's main web page at

  http://llvm.org

and also to the LICENSE.TXT files in your LLVM and CLANG source trees
respectively.

This work was inspired by discussions and code reviews with Dmitry
Kuvayskiy (TU Dresden), and the encoded operations implemented in
'anlib.c' were taken directly from D. Kuvayskiy's "AN-transfomer".



*** Description: ***

TODO: TO BE WRITTEN



*** Requirements: ***

The AN encoder must be built against a modified versions of LLVM/CLANG
3.5.1. These versions of LLVM/CLANG are provided by the 'intrinsics'
branches of the following repositories:

  https://nrink@bitbucket.org/nrink/llvm-351.git (for LLVM)

  https://nrink@bitbucket.org/nrink/clang-351.git (for CLANG)

Make sure you run the command

  git fetch && git checkout intrinsics

in both repositories after cloning them.

You must have built both LLVM and CLANG 3.5.1 before you can proceed to
building the AN encoder. Please refer to the LLVM documentation at

  http://llvm.org/docs

If you are new to LLVM, you may find the following pages useful:

  http://llvm.org/docs/GettingStarted.html
  http://llvm.org/docs/CMake.html

Note that the AN encoder is configured to be built with cmake. You
should therefore build LLVM and CLANG with cmake too. Building the AN
encoder against an LLVM/CLANG build that has not been obtained by using
cmake has not been tested and is unlikely to work.



*** Building the encoder: ***

The AN encoder project comes with a build script in

  <TOP LEVEL SOURCE DIR>/encode/bin/build.sh

To build the AN encoder, execute the build.sh script from within the
directory where you want the AN encoder to be created - subsequently
refered to as the 'build directory'. This will generate a directory
structure below the build directory, which includes the following:

  a) the AN encoder's main executable, named "encode"
  b) runtime resources for the AN encoder
  c) the script "apply-an.sh" (see below for details)
  d) C-source files for testing the AN encoder

Before the build.sh script can be run, two environment variables must
be set:

  1) ENCODE_DIR
        This must be set to the top level source directory of the
        AN encoder code, i.e. the directory into which you have cloned
        the AN encoder repository.

  2) LLVM_BUILD_DIR
        This must be set to the top level directory of your LLVM 3.5.1
        build that you have generated as part fo the requirements for
        building the AN encoder (please refer to the previous section).

Note that if these environment variables are not set, the build.sh
script will assume default values for these variables, which are
unlikely to be correct in your environment.



*** Running the encoder: ***

After build.sh has successfully completed, the following executables
will exist:

  1) <'build directory'>/encode/src/encode
        This is the binary file that applies the AN encoder to its
        input. It takes as input an LLVM bitcode file, and outputs
        a modified bitcode file which operates on coded data.
  2) <'build directory'>/encode/apply-an.sh
        This script takes as input a C-source file and produces two
        executables from it: The first executable has been instrumented
        with a cycle counter. This serves as the reference point for
        performance measurements. The second executable is the
        AN-encoded version of the first one. It has also been
        instrumented with the same cycle counter.

For running the apply-an.sh script the following environment variables must be set:

  1) ENCODE_BINARY_DIR
        This should be set to the directory in which the "encode"
        binary has been created, which should be
        <'build directory'>/encode/src.
  2) ENCODE_RUNTIME_DIR
        This should be set to <'build directory'>/encode/runtime.
  3) LLVM_BUILD_DIR
        This should be set to the top level directory under which the
        LLVM and CLANG 3.5.1 builds have been created.
