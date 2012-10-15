# Sol

A sunny little programming language on a register-based virtual machine.


## VM design

Each scheduler has one run queue in which tasks are queued for execution

    VM
      Scheduler N
        RunQueue
          Task
            next -> Task...
            ActivationRecord
              next -> ActivationRecord...
              Function
                Constants
                Instructions
              ProgramCounter
              Registry

      (Task migration)

When more than one scheduler is running, tasks might migrate from one scheduler
to another.

## Examples

The examples below are expressed in a simplified assembly language that is almost 1:1 with the C API code for defining these programs programatically and thus the assembly language itself should be considered irrelevant beyond explaining the instructions executed.

- In the output, lines like these: `[vm] ______________ ...` denote whent he scheduler regains control after running a task and the task either returned or yielded. This is one "execution iteration". When running multiple tasks, you will usually see tasks interleved in round-robin order between these "execution iteration" marker lines.

- In the output, lines starting with "..." are comments and/or simplifications and not part of the actual output.

- In assembly comments ("; ..."), `R(x)` means "Register x", `RK(x)` means "Register x if x is less than 256 else Constant (x-255)", `K(x)` means "Constant x".

- In assembly comments ("; ..."), `PC` signifies the "program counter" which is sort of a cursor to the instructions of a program. It is incremented by one for each instruction executed. Some instructions will further modify this counter, like for instance the `JUMP` instruction.

### Example 1: while x > 0 yield ...

While the variable x is greater than zero, decrement `x` by one and yield to the
scheduler, letting other tasks run. Eventually return.

```py
def main():
  x = 5
  while (x > 0):
    x = x - 1
    yield
  return
```

Assembly:

```asm
define main 0
  CONST 5           ; K(0) = 5
  CONST 0           ; K(1) = 0
  CONST 1           ; K(2) = 1
  entry:
  LOADK  0  0       ; R(0) = K(0)
  LE     0  0  256  ; (0 == RK(k+1) < RK(0)) ? continue else PC++
  JUMP   3          ; PC += 3 to RETURN
  SUB    0  0  257  ; R(0) = R(0) - RK(k+1)
  YIELD  0  0  0    ; yield A=type=sched
  JUMP   -5         ; PC -= 5 to LE
  RETURN 0  0       ; return
```

Output when running in debug mode:

    $ build/debug/bin/sol
    Sol 0.1.0 x64
    [vm] ______________ ______________ __________ _______ ____ ______________
    [vm] Task           Function       PC         Op      Values
    [vm] 0x7fdf28c03c00 0x7fdf28c000e0 0          LOADK   AB:    0,   0
    [vm] 0x7fdf28c03c00 0x7fdf28c000e0 1          LE      ABC:   0,   0, 256
    [vm] 0x7fdf28c03c00 0x7fdf28c000e0 3          SUB     ABC:   0,   0, 257
    [vm] 0x7fdf28c03c00 0x7fdf28c000e0 4          YIELD   ABC:   0,   0,   0
    [vm] ______________ ______________ __________ _______ ____ ______________
    [vm] Task           Function       PC         Op      Values
    [vm] 0x7fdf28c03c00 0x7fdf28c000e0 5          JUMP    Bss:       -5
    [vm] 0x7fdf28c03c00 0x7fdf28c000e0 1          LE      ABC:   0,   0, 256
    [vm] 0x7fdf28c03c00 0x7fdf28c000e0 3          SUB     ABC:   0,   0, 257
    [vm] 0x7fdf28c03c00 0x7fdf28c000e0 4          YIELD   ABC:   0,   0,   0
    [vm] ______________ ______________ __________ _______ ____ ______________
    ...three more execution iterations identical to the above block...
    [vm] ______________ ______________ __________ _______ ____ ______________
    [vm] Task           Function       PC         Op      Values
    [vm] 0x7fdf28c03c00 0x7fdf28c000e0 5          JUMP    Bss:       -5
    [vm] 0x7fdf28c03c00 0x7fdf28c000e0 1          LE      ABC:   0,   0, 256
    [vm] 0x7fdf28c03c00 0x7fdf28c000e0 2          JUMP    Bss:        3
    [vm] 0x7fdf28c03c00 0x7fdf28c000e0 6          RETURN  AB:    0,   0
    Scheduler runloop exited.

### Example 2: Function calls and timers

This program uses two functions. The entry point is the `main` function which simply
calls the `kitten` function with one argument '500'. The `kitten` function "sleeps" for
the number of milliseconds passed to it (as the first argument.) The `kitten` function
then returns the number "123" to the caller—the `main` function—which dumps register values and
finally returns, causing the task to exit and subsequently the scheduler and the VM too to exit.

Assembly:

```asm
define kitten 1     ; Arguments: (R(0)=sleep_ms)
  CONST  123        ; K(0) = 123
  entry:
  YIELD  1  0  0    ; yield A=type=timer, RK(B)=R(0)=arg0
  LOADK  0  0       ; R(0) = K(0) = 123
  RETURN 0  1       ; return R(0)..R(0) = R(0) = 123

define main 0       ; Arguments: ()
  CONST  @kitten    ; K(0) = <func kitten>
  CONST  500        ; K(1) = 500
  entry:
  LOADK  0  0       ; R(0) = K(0) = the kitten function
  LOADK  1  1       ; R(1) = K(1) = 500
  CALL   0  1  1    ; R(0)..R(0) = R(0)(R(1)..R(1)) = a(R(1))
  DBGREG 0  1  0    ; VM debug function that dumps register values
  RETURN 0  0       ; return
```

Output when running in debug mode:

    $ time build/debug/bin/sol
    Sol 0.1.0 x64
    [vm] ______________ ______________ __________ _______ ____ ______________
    [vm] Task           Function       PC         Op      Values
    [vm] 0x7f8c9bc03bf0 0x7f8c9bc03910 0          LOADK   AB:    0,   0
    [vm] 0x7f8c9bc03bf0 0x7f8c9bc03910 1          LOADK   AB:    1,   1
    [vm] 0x7f8c9bc03bf0 0x7f8c9bc03910 2          CALL    ABC:   0,   1,   1
    [vm] 0x7f8c9bc03bf0 0x7f8c9bc000e0 1          YIELD   ABC:   1,   0,   0
    D Timer scheduled to trigger after 500.000000 ms (sched.c:81)
    # ...time passes and in this case the scheduler is idling...
    D Timer triggered -- scheduling task (sched.c:57)
    [vm] ______________ ______________ __________ _______ ____ ______________
    [vm] Task           Function       PC         Op      Values
    [vm] 0x7f8c9bc03bf0 0x7f8c9bc000e0 2          LOADK   AB:    0,   0
    [vm] 0x7f8c9bc03bf0 0x7f8c9bc000e0 3          RETURN  AB:    0,   1
    [vm] 0x7f8c9bc03bf0 0x7f8c9bc03910 3          DBGREG 
    D [vm] R(0) = 123.000000 (sched_exec.h:214)
    D [vm] R(1) = 500.000000 (sched_exec.h:215)
    D [vm] R(0) = 123.000000 (sched_exec.h:216)
    [vm] 0x7f8c9bc03bf0 0x7f8c9bc03910 4          RETURN  AB:    0,   0
    Scheduler runloop exited.

    real  0m0.504s
    user  0m0.001s
    sys   0m0.001s

### Example 3: Multitasking

Here we run three tasks, each running the program in *Example 1*:

    $ build/debug/bin/sol
    Sol 0.1.0 x64
    [sched 0x7fc219403930] run queue:
      [task 0x7fc219403c00] -> [task 0x7fc219403cd0] -> [task 0x7fc219403da0]
    [vm] ______________ ______________ __________ _______ ____ ______________
    [vm] Task           Function       PC         Op      Values
    [vm] 0x7fc219403c00 0x7fc2194000e0 0          LOADK   AB:    0,   0
    [vm] 0x7fc219403c00 0x7fc2194000e0 1          LE      ABC:   0,   0, 256
    [vm] 0x7fc219403c00 0x7fc2194000e0 3          SUB     ABC:   0,   0, 257
    [vm] 0x7fc219403c00 0x7fc2194000e0 4          YIELD   ABC:   0,   0,   0
    [vm] ______________ ______________ __________ _______ ____ ______________
    [vm] Task           Function       PC         Op      Values
    [vm] 0x7fc219403cd0 0x7fc2194000e0 0          LOADK   AB:    0,   0
    [vm] 0x7fc219403cd0 0x7fc2194000e0 1          LE      ABC:   0,   0, 256
    [vm] 0x7fc219403cd0 0x7fc2194000e0 3          SUB     ABC:   0,   0, 257
    [vm] 0x7fc219403cd0 0x7fc2194000e0 4          YIELD   ABC:   0,   0,   0
    [vm] ______________ ______________ __________ _______ ____ ______________
    [vm] Task           Function       PC         Op      Values
    [vm] 0x7fc219403da0 0x7fc2194000e0 0          LOADK   AB:    0,   0
    [vm] 0x7fc219403da0 0x7fc2194000e0 1          LE      ABC:   0,   0, 256
    [vm] 0x7fc219403da0 0x7fc2194000e0 3          SUB     ABC:   0,   0, 257
    [vm] 0x7fc219403da0 0x7fc2194000e0 4          YIELD   ABC:   0,   0,   0
    [vm] ______________ ______________ __________ _______ ____ ______________
    [vm] Task           Function       PC         Op      Values
    [vm] 0x7fc219403c00 0x7fc2194000e0 5          JUMP    Bss:       -5
    [vm] 0x7fc219403c00 0x7fc2194000e0 1          LE      ABC:   0,   0, 256
    [vm] 0x7fc219403c00 0x7fc2194000e0 3          SUB     ABC:   0,   0, 257
    [vm] 0x7fc219403c00 0x7fc2194000e0 4          YIELD   ABC:   0,   0,   0
    [vm] ______________ ______________ __________ _______ ____ ______________
    ...The above block of instruction is repeated three times in interleved
       round-robin order for each task. Then:
    [vm] ______________ ______________ __________ _______ ____ ______________
    [vm] Task           Function       PC         Op      Values
    [vm] 0x7fc219403c00 0x7fc2194000e0 5          JUMP    Bss:       -5
    [vm] 0x7fc219403c00 0x7fc2194000e0 1          LE      ABC:   0,   0, 256
    [vm] 0x7fc219403c00 0x7fc2194000e0 2          JUMP    Bss:        3
    [vm] 0x7fc219403c00 0x7fc2194000e0 6          RETURN  AB:    0,   0
    [vm] ______________ ______________ __________ _______ ____ ______________
    [vm] Task           Function       PC         Op      Values
    [vm] 0x7fc219403cd0 0x7fc2194000e0 5          JUMP    Bss:       -5
    [vm] 0x7fc219403cd0 0x7fc2194000e0 1          LE      ABC:   0,   0, 256
    [vm] 0x7fc219403cd0 0x7fc2194000e0 2          JUMP    Bss:        3
    [vm] 0x7fc219403cd0 0x7fc2194000e0 6          RETURN  AB:    0,   0
    [vm] ______________ ______________ __________ _______ ____ ______________
    [vm] Task           Function       PC         Op      Values
    [vm] 0x7fc219403da0 0x7fc2194000e0 5          JUMP    Bss:       -5
    [vm] 0x7fc219403da0 0x7fc2194000e0 1          LE      ABC:   0,   0, 256
    [vm] 0x7fc219403da0 0x7fc2194000e0 2          JUMP    Bss:        3
    [vm] 0x7fc219403da0 0x7fc2194000e0 6          RETURN  AB:    0,   0
    Scheduler runloop exited.

## Building

Initial configuration

  deps/libev-configure.s

Build Sol and run tests (when in the same directory as this README file):

    make

Build Sol in debug mode

    make DEBUG=1 sol

Run the debug version of Sol

    ./build/debug/bin/sol

Build and run tests (potentially building Sol too):

    make test

### Build targets

- (default) — Alias for "sol test"
- `sol` — Build Sol
- `test` — Build and run all unit tests
- `clean` — Clean tests and clean the target type for sol (debug or not). Note that you need to pass the `DEBUG=1` flag to `make clean` to cause cleaning of debug builds. To remove everything that has been generated, simply `rm -rf ./build`.

Sol specific targets (i.e. for `make -C ./sol`):

- `llvm_ir` — Compile all source files to LLVM IR assembly, placed in `<BUILD_PREFIX>/debug/sol-asm/<name>.ll`
- `asm` — Compile all source files to target assembly, placed in `<BUILD_PREFIX>/debug/sol-asm/<name>.s`

### Build flags

Special flags that can be passed to make (e.g. `make FLAG=VALUE ...`):

- `DEBUG=1|0` — When set to "1", build without optimizations, with debug symbols, with debug logging and with assertions. Defaults to "0", which causes building of "release" products (optimizations enabled, no debug logging and no assertions).

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
