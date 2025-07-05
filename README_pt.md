# Assembler para o IAS

Esse é um _assembler_ para o IAS que eu criei para [este](https://www.ic.unicamp.br/~edson/disciplinas/mc404/2017-2s/abef/IAS-sim/) simulador.

Ele foi inspirado [neste](https://www.ic.unicamp.br/~edson/disciplinas/mc404/2017-2s/abef/IAS-Assembler/assembler.html) _assembler_, mas com essas diferenças:

- **Sintaxe diferente**

Enquanto o meu assembler tem uma sintaxe assim:

```
LOAD MQ, foo

STOR bar, 8:19
```

A do original era assim:

```
LOAD MQ, M(foo)

STOR M(bar, 8:19)
```

Isso foi menos uma questão de ser uma sintaxe melhor, mas sim porque é muito mais fácil criar um _lexer_ desse jeito -- é só separar os _tokens_ com espaço e vírgula.

- **Inexistência da palavra-chave `.align`**

No outro assembler, a palavra-chave `.align` serve para declarar na memória palavras com as palavras-chave `.word` ou `.wfill`, mas a única coisa que fazia era dar um erro se você não a colocasse, não fazendo diferença o número colocado depois dela. Então decidi tirá-la.

- **Correção de um bug**

No _assembler_ original, as instruções `STOR M(foo, 8:19)` e `STOR M(foo, 28:39)` eram interpretadas como sendo `STOR M(foo)`.

## Compilando

Os únicos requisitos são um compilador C com compatibilidade com o ANSI C 99, GNU Makefile e um sistema que suporte POSIX (ou pelo menos a biblioteca de Regex dele) -- ou seja: Windows não é suportado sem algo como o Cygwin ou MSYS2.