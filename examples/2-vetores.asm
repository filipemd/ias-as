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

.set DEFAULT_VEC_SIZE 16

_start:
loop:
    LOAD vec1_addr
    STOR _1, 28:39
    
    LOAD MQ, one
_1:
    # Multiplica pelo item do vetor
    MUL 0x000
    
    LOAD vec2_addr
    STOR _2, 8:19
_2:
    # Multiplica pelo item do vetor
    MUL 0x000
    LOAD MQ
    STOR tmp
    
    LOAD result
    ADD tmp
    STOR result
    
    # vec1_addr++
    LOAD vec1_addr
    ADD one
    STOR vec1_addr
    
    # vec2_addr++
    LOAD vec2_addr
    ADD one
    STOR vec2_addr
    
    # vectors_size--
    LOAD vectors_size
    SUB one
    STOR vectors_size
    
    # if vectors_size >= 0 then goto loop
    JUMP+ loop, 0:19
    
    # Pro teste
    LOAD result
    
    # Fim
    JUMP 0x400, 0:19

result: .word 0x000
tmp: .word 0x000
one: .word 0x001

vec1: .wfill DEFAULT_VEC_SIZE, 0x003
vec2: .wfill DEFAULT_VEC_SIZE, 0x002

# Valores pro teste
.org 0x3FD
vec1_addr: .word vec1
vec2_addr: .word vec2
vectors_size: .word DEFAULT_VEC_SIZE
