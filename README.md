# Sol

A sunny little programming language on a register-based virtual machine.


## VM design

Each scheduler has one run queue in which tasks are queued for execution

    VM
      Scheduler N
        Run queue
          Tasks
      (Task migration)

When more than one scheduler is running, tasks might migrate from one scheduler
to another.


## Building

Build Sol and run tests (when in the same directory as this README file):

    make

Build Sol in debug mode

    make DEBUG=1 sol

Run the debug version of Sol

    ./build/debug/bin/sol

Run tests and potentially build Sol and affected tests:

    make test

### Build flags

Special flags that can be passed to make (e.g. `make FLAG=VALUE ...`):

- `DEBUG=1|0` — When set to "1", build without optimizations, with debug symbols, with debug logging and with assertions. Defaults to "0", which causes building of "release" products (optimizations enabled, no debug logging and no assertions). Note that you need to pass this flag for `make clean` as the `clean` target will only clean the products for either "debug" or "release". To remove everything, simply `rm -rf ./build`.

- `TARGET_ARCH=NAME` — Set the architecture to build for. Valid values for `NAME` depends on the compiler. Defaults to the host architecture (as reported by `shell uname -m`). For instance, to build an IA32 product on a x64 system: `make TARGET_ARCH=i386`.

- `BUILD_PREFIX` — Base directory for products. Defaults to `<BUILD_PREFIX>/<DEBUG ? debug : release>`.

- `BASE_BUILD_PREFIX` — Base directory for tests and products. Defaults to `./build`.

- `TESTS_BUILD_PREFIX` — Base directory for generated tests. Defaults to `<BUILD_PREFIX>/test`.

# MIT License

Copyright (c) 2012 Rasmus Andersson <http://rsms.me/>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
