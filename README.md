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
