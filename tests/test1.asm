#   Copyright (C) 2025  filipemd
#
#   This file is part of IAS Assembler.
#
#   IAS Assembler is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   IAS Assembler is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with IAS Assembler.  If not, see <http://www.gnu.org/licenses/>.

LOAD bar
LOAD MQ, bar
LOAD |bar|
LOAD -bar
LOAD MQ

.set VAL 0x20
.set SPAM VAL
.org VAL

STOR foo

JUMP label, 0:19
JUMP label, 20:39

JUMP+ label, 0:19
JUMP+ label, 20:39

STOR other_label, 8:19
STOR other_label, 28:39

other_label:
ADD foo
ADD |foo|

SUB foo
SUB |foo|

LSH
RSH

label:
    LOAD bar

HALT

foo:
    .word 0x32
bar:
    .word 0x21
vec:
    .wfill VAL, foo