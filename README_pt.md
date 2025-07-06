# Assembler para o IAS

Esse é um _assembler_ para o IAS que eu criei para [este](https://www.ic.unicamp.br/~edson/disciplinas/mc404/2017-2s/abef/IAS-sim/) simulador.

Caso queira saber as instruções da máquina, veja [este](https://www.ic.unicamp.br/~edson/disciplinas/mc404/2017-2s/abef/) PDF. E caso queira saber mais sobre a máquina em si, tem a [página da Wikipedia](https://pt.wikipedia.org/wiki/Computador_IAS).

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

- **Parâmetro nas instruções STOR e JUMP opcionais**

Na máquina IAS, duas instruções ocupam o mesmo endereço da memória, então se usaria:

```
JUMP foo, 0:19
```

Para executar a instrução no lado direito no endereço, e:

```
JUMP foo, 20:39
```

Para executar a instrução no lado esquerdo do endereço. Entretanto, nesse _assembler_, isso é opcional, você pode escrever só:

```
JUMP foo
```

Algo parecido também acontece com a instrução STOR. Entretanto, STOR sem nenhum parâmetro modifica o endereço todo. Então, pra isso, se usa a instrução STOR+.

- **Inexistência da palavra-chave `.align`**

No outro assembler, a palavra-chave `.align` serve para declarar na memória palavras com as palavras-chave `.word` ou `.wfill`, mas a única coisa que fazia era dar um erro se você não a colocasse, não fazendo diferença o número colocado depois dela. Então decidi tirá-la.

- **Correção de um bug**

No _assembler_ original, as instruções `STOR M(foo, 8:19)` e `STOR M(foo, 28:39)` eram interpretadas como sendo `STOR M(foo)`.

## Compilando

~~Os únicos requisitos são um compilador C com compatibilidade com o ANSI C 99, GNU Makefile e um sistema que suporte POSIX (ou pelo menos a biblioteca de Regex dele) -- ou seja: Windows não é suportado sem algo como o Cygwin ou MSYS2.~~

Como o _assembler_ deixou de usar a biblioteca de Regex do POSIX e passou a usar o re2c, os únicos requisitos agora são:

- Um compilador C compatível com o ANSI C 99;
- GNU Makefile;
- [re2c](https://re2c.org/).

## Licença

O programa é licenciado sob a GPL versão 3 ou posterior, e também inclui código originalmente licenciado sob a licença BSD de três cláusulas (`sc_map.h` e `sc_map.c`). Os códigos em `examples/` são licenciados sob a licença BSD de zero cláusulas, portanto, você pode usá-los mesmo sem dar crédito.