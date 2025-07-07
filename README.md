# Assembler for the IAS

This is an assembler for the IAS that I created for [this](https://www.ic.unicamp.br/~edson/disciplinas/mc404/2017-2s/abef/IAS-sim/) simulator.

If you want to know the machine's instructions, see [this](https://www.ic.unicamp.br/~edson/disciplinas/mc404/2017-2s/abef/) PDF (it is in Portuguese though). And if you want to learn more about the machine itself, there is the [Wikipedia page](https://en.wikipedia.org/wiki/IAS_machine).

It was inspired by [this](https://www.ic.unicamp.br/~edson/disciplinas/mc404/2017-2s/abef/IAS-Assembler/assembler.html) assembler, but with the following differences:

- **Different Syntax**

While my assembler has a syntax like this:

```
LOAD MQ, foo

STOR bar, 8:19
```

The original one had this syntax:

```
LOAD MQ, M(foo)

STOR M(bar, 8:19)
```

This change was less about having a better syntax and more about making it much easier to create a lexer this way—it's just a matter of separating the tokens with spaces and commas.

* **Optional parameter in STOR and JUMP instructions**

In the IAS machine, two instructions share the same memory address, so you would use:

```
JUMP foo, 0:19
```

To execute the instruction on the right-hand side of the address, and:

```
JUMP foo, 20:39
```

To execute the instruction on the left-hand side. However, in this *assembler*, that parameter is optional, so you can simply write:

```
JUMP foo
```

A similar rule applies to the STOR instruction. However, STOR without any parameter modifies the entire address. To handle this, the `STOR+` instruction is used.

- **Absence of the `.align` Keyword**

In the other assembler, the `.align` keyword was used to declare words in memory with the `.word` or `.wfill` keywords, but it only generated an error if you didn't include it, without making any difference to the number placed after it. So I decided to remove it.

- **Bug Fix**

In the original assembler, the instructions `STOR M(foo, 8:19)` and `STOR M(foo, 28:39)` were interpreted as `STOR M(foo)`.

## Compiling

~~The only requirements are a C compiler compatible with ANSI C 99, a GNU Makefile, and a system that supports POSIX (or at least its Regex library)—that is, Windows is not supported without something like Cygwin or MSYS2.~~

As the assembler stopped using the POSIX Regex library and started using re2c, the only requirements now are:

- A C compiler compatible with ANSI C 99;
- GNU Makefile;
- [re2c](https://re2c.org/).

### Compiling for Windows on Linux

Install MinGW using your GNU/Linux distro's package manager and run:

```
make CC=x86_64-w64-mingw32-gcc
```

The executable will be generated at `build/ias-as.exe`. To run it, use [Wine](https://www.winehq.org/).

The test script also supports Wine. To test the Windows build, run `./run_tests.sh windows`.

### Compiling on Windows

The recommended way is using [MSYS2](https://www.msys2.org/), which lets you install the C compiler and re2c and compile just like on Linux.

However, it's also possible to compile on Windows without it. You’ll need MinGW and re2c in your PATH, and run `mingw32-make CC=gcc`. Note that the Makefile may not work correctly due to differences between the POSIX shell and Windows shells (CMD or PowerShell).

## License

The program is licensed under the GPL version 3 or later, and it also includes code originally licensed under the BSD three-clause license (`sc_map.h` and `sc_map.c`). The codes in `examples/` are licensed under the zero-clause BSD license, so you can use them even without giving credit.