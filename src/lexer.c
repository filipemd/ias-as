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

#include "lexer.h"

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

// Parsing preguiçoso -- permite sintaxes boas, como `LOAD MQ foo` e `LOAD MQ, foo`, mas também 
// atrocidades como `LOAD,MQ,foo`
#define TOKEN_SEPARATORS " \t,"

#define LINE_SIZE 255

struct lexer_tokens_list lexer_tokens_list_init(void) {
	struct lexer_tokens_list list;

	list.data = NULL;
	list.size = 0;
	list.capacity = 0;

	return list;
}

void lexer_tokens_list_add(struct lexer_tokens_list* list, struct lexer_token tok) {
	if (list->size >= list->capacity) {
		size_t new_capacity = list->capacity == 0 ? 4 : list->capacity * 2;
		struct lexer_token* new_data = realloc(list->data, new_capacity * sizeof(struct lexer_token));
		if (!new_data) {
			perror("realloc");
			exit(EXIT_FAILURE);
		}
		list->data = new_data;
		list->capacity = new_capacity;
	}

	list->data[list->size++] = tok;
}

void lexer_tokens_list_free(struct lexer_tokens_list* list) {
	if (!list) return;

	// Free each string inside the token
	for (size_t i = 0; i < list->size; ++i) {
		free(list->data[i].string);
	}

	// Free the array of tokens
	free(list->data);

	// Reset the list structure
	list->data = NULL;
	list->size = 0;
	list->capacity = 0;
}

struct lexer_tokens_list lexer_lex(FILE* fptr, bool* error) {
	struct lexer_tokens_list result = lexer_tokens_list_init();

	char line_buffer[LINE_SIZE];

	unsigned int line=1;

	while (fgets(line_buffer, LINE_SIZE, fptr)) {	
		line_buffer[strcspn(line_buffer, "\n")] = '\0';	

		char* word = strtok(line_buffer, TOKEN_SEPARATORS);
		while(word != NULL) {
			// Ignora comentários. Eu sei que a maioria dos assemblers usam ponto e
			// vírgula para isso, mas o que eu estou me inspirando não.
			if (word[0]=='#') {
				break;
			}

			const struct lexer_token token=determine_token(word, line);

			if (token.type == TOKEN_NONE) {
				*error=true;
			}
			
			lexer_tokens_list_add(&result, token); 		

			word=strtok(NULL, TOKEN_SEPARATORS);
		}

		line++;
	}

	return result;
}
