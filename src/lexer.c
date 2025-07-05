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

#define ARRAY_SIZE(array) sizeof(array)/sizeof((array)[0])

// Maneira preguiçosa mas extremamente funcional: REGEX.

// Sim, eu poderia ter usado algo como lex, yacc, bla, bla, bla. Mas se funciona bem, funciona bem.
// Apesar disso, estou pensando em usar re2c no futuro.
static const char* token_patterns[] = {
	// Instruções
	"^LOAD$",
	"^STOR$",
	"^STOR\\+$",
	"^JUMP$",
	"^JUMP\\+$",
	"^ADD$",
	"^SUB$",
	"^MUL$",
	"^DIV$",
	"^LSH$",
	"^RSH$",
	// Registradores
	"^MQ$",
	// Parâmetros de instruçōes
	"^0:19$",
	"^20:39$",
	"^8:19$",
	"^28:39$",
	// Palavras-chave (dots escaped)
	"^\\.org$",
	"^\\.word$",
	"^\\.wfill$",
	"^\\.set$",
	// "^\\.align$",
	// Números
	"^([0-9]+)$",          // Número decimal
	"^0[xX]([0-9a-fA-F]+)$", // Número hexadecimal
	// "Labels"
	"^([a-zA-Z_][a-zA-Z0-9_]*)\\:$", // Declaração de uma label
	"^([a-zA-Z_][a-zA-Z0-9_]*)$",      // Uso de uma label
	// Números e labels negativos e absolutos. O valor pego da regex deve ser passado
	// novamente por `determine_token`. 
	"^\\|(.*)\\|$", // Absoluto
	"^-(.*)$" // Negativo
};

static regex_t token_regex[ARRAY_SIZE(token_patterns)];

void lexer_init(void) {
	for(size_t i=0; i<ARRAY_SIZE(token_patterns); i++) {
		/*if (regcomp(&token_regex[i], token_patterns[i], REG_EXTENDED) == 0) {
			// Compilação feita com sucesso
		} else {
			fprintf(stderr, "Error parsing regex pattern %s.\n", token_patterns[i]);
			exit(1);
		}*/
		assert(regcomp(&token_regex[i], token_patterns[i], REG_EXTENDED) == 0);
	}
}

// Atenção! Esta função chega aos cinco níveis de indentação. Se for Linus Torvalds, saia imediatamente!
struct lexer_token determine_token(const char* word, unsigned int line) {
	for(size_t i=0; i<ARRAY_SIZE(token_regex); i++) {
		regmatch_t matches[2]={{-1,-1},{-1,-1}}; // Dois matches porque o primeiro é o total
		int value = regexec(&token_regex[i], word, ARRAY_SIZE(matches), matches, 0);
		if (value==0) {
			char* string=NULL;
			if (matches[1].rm_so>=0) {
				int length=matches[1].rm_eo-matches[1].rm_so;

				string=malloc(length+1);
				if (string == NULL) {
					perror("malloc");
					exit(EXIT_FAILURE);
				}

				strncpy(string, word+matches[1].rm_so, length); // Cria a substring
						
				string[length] = '\0';
			}

			return (struct lexer_token){.line=line, .type=i, .string=string};
		} else if (value!=REG_NOMATCH) {
			fprintf(stderr, "An error parsing token %s occured.\n", word);
			exit(1);
		}
	}

	fprintf(stderr, "ERROR in line %d: no token \"%s\".\n", line, word);

	return (struct lexer_token){.line=line, .type=TOKEN_NONE, .string=NULL};
}

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

		char* save_ptr;
		char* word = strtok_r(line_buffer, TOKEN_SEPARATORS, &save_ptr);
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

			word=strtok_r(NULL, TOKEN_SEPARATORS, &save_ptr);
		}

		line++;
	}

	return result;
}

void lexer_end(void) {	
	for(size_t i=0; i<ARRAY_SIZE(token_regex); i++) {
		regfree(&token_regex[i]);
	}
}
