# Zero-Clause BSD
# =============
#
# Permission to use, copy, modify, and/or distribute this software for
# any purpose with or without fee is hereby granted.
#
# THE SOFTWARE IS PROVIDED “AS IS” AND THE AUTHOR DISCLAIMS ALL
# WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE
# FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY
# DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
# AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
# OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

.set BEGIN 0x000
.org BEGIN

_start:
    LOAD MQ, g
    MUL x
    
    LOAD MQ
    STOR v_squared
    
square_root:
    RSH
    STOR k

loop:
    LOAD v_squared
    DIV k
    LOAD MQ
    STOR tmp
    
    LOAD k
    ADD tmp
    
    RSH
    STOR k
    
    # i--
    LOAD i
    SUB one
    STOR i
    
    JUMP+ loop, 0:19
    
    LOAD k
    STOR v
    
    # Fim
    JUMP 0x400, 0:19

g: .word 10
v: .word 0

v_squared: .word 0
k: .word 0

two: .word 2
one: .word 1

tmp: .word 0
i: .word 10

# Endereço no qual será feito os testes
.org 0x105
x: .word 3500
