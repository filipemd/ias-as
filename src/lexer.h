/* Copyright (C) 2025  filipemd

   This file is part of IAS Assembler.

   IAS Assembler is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   IAS Assembler is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with IAS Assembler.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <regex.h>
#include <stdbool.h>

struct lexer_token {
	unsigned int line;
	enum token_type {
		TOKEN_NONE=-1,
		INSTRUCTION_LOAD,
		INSTRUCTION_STOR,
		INSTRUCTION_JUMP,
		INSTRUCTION_JUMP_PLUS,
		INSTRUCTION_ADD,
		INSTRUCTION_SUB,
		INSTRUCTION_MUL,
		INSTRUCTION_DIV,
		INSTRUCTION_HALT,
		INSTRUCTION_LSH,
		INSTRUCTION_RSH,
		
		REGISTER_MQ,

		PARAM_0_TO_19,
		PARAM_20_TO_39,
		PARAM_8_TO_19,
		PARAM_28_TO_39,

		KEYWORD_ORG,
		KEYWORD_WORD,
		KEYWORD_WFILL,
		KEYWORD_SET,
		//COMPAT_USELESS_KEYWORD_ALIGN, // Inútil

		NUMBER_DECIMAL,
		NUMBER_HEX,

		LABEL_DECLARATION,
		LABEL_USAGE,

		TOKEN_ABSOLUTE,
		TOKEN_NEGATIVE
	} type;
	char* string;
};

struct lexer_token determine_token(const char* word, unsigned int line); 

struct lexer_tokens_list {
	struct lexer_token* data;
	size_t size, capacity;
};

struct lexer_tokens_list lexer_tokens_list_init(void);
void lexer_tokens_list_add(struct lexer_tokens_list* list, struct lexer_token tok);
void lexer_tokens_list_free(struct lexer_tokens_list* list);

void lexer_init(void);
struct lexer_tokens_list lexer_lex(FILE* fptr, bool* error);
void lexer_end(void);

#endif
