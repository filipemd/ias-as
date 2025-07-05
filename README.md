# Assembler for the IAS

This is an assembler for the IAS that I created for [this](https://www.ic.unicamp.br/~edson/disciplinas/mc404/2017-2s/abef/IAS-sim/) simulator.

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

- **Absence of the `.align` Keyword**

In the other assembler, the `.align` keyword was used to declare words in memory with the `.word` or `.wfill` keywords, but it only generated an error if you didn't include it, without making any difference to the number placed after it. So I decided to remove it.

- **Bug Fix**

In the original assembler, the instructions `STOR M(foo, 8:19)` and `STOR M(foo, 28:39)` were interpreted as `STOR M(foo)`.

## Compiling

The only requirements are a C compiler compatible with ANSI C 99, a GNU Makefile, and a system that supports POSIX (or at least its Regex library)—that is, Windows is not supported without something like Cygwin or MSYS2.

## License

The program is licensed under the GPL version 3 or later, and it also includes code originally licensed under the BSD three-clause license (`sc_map.h` and `sc_map.c`). The codes in `examples/` are licensed under the zero-clause BSD license, so you can use them even without giving credit.