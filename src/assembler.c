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

#include "assembler.h"
#include "lexer.h"

#include "sc_map.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Sim, uma palavra no IAS tem 40 bits. Usando 64 bits porque é mais fácil
typedef uint64_t ias_word_t;

// Lista de endereços de diretivas .org
struct address_list {
	uint16_t* data;
	size_t size, capacity;
};

static struct address_list address_list_init(void) {
	struct address_list result;

	result.data=NULL;
	result.size=0;
	result.capacity=0;

	return result;
}

static void address_list_add(struct address_list* list, uint16_t value) {
	if (list->size >= list->capacity) {
		size_t new_capacity = list->capacity == 0 ? 4 : list->capacity * 2;
		uint16_t* new_data = realloc(list->data, new_capacity * sizeof(uint16_t));
		if (!new_data) {
			perror("realloc");
			exit(EXIT_FAILURE);
		}
		list->data = new_data;
		list->capacity = new_capacity;
	}

	list->data[list->size++] = value;
}

static void address_list_free(struct address_list* list) {
	free(list->data);

	list->data=NULL;
	list->size=0;
	list->capacity=0;
}

static inline bool is_token_number(struct lexer_token token) {
	return token.type==NUMBER_DECIMAL || token.type==NUMBER_HEX;
}

static uint16_t get_number_from_token(struct lexer_token token, bool* error) {
	if (!is_token_number(token)) {
		fprintf(stderr, "ERROR in line %d: argument should be number.\n", token.line);
		*error = true;
	}
	
	return strtoul(token.string, NULL, token.type==NUMBER_DECIMAL? 10 : (token.type==NUMBER_HEX ? 16 : -1));
}

// Literalmente um Ctrl. C + Ctrl.V da função de cima, mas C não tem generics.
// Impressionante como Go até 2022 também não tinha.
static ias_word_t get_word_from_token(struct lexer_token token, bool* error) {
	if (!is_token_number(token)) {
		fprintf(stderr, "ERROR in line %d: argument should be number.\n", token.line);
		*error = true;
	}
	
	// Minto, tem um 'l' extra -- ao invés de ser `strtoul` é `strtoull`.
	return strtoull(token.string, NULL, token.type==NUMBER_DECIMAL? 10 : (token.type==NUMBER_HEX ? 16 : -1));
}

// Essas funções (`get_actual_address` e `set_actual_address`) que não consegui nomear de forma melhor
// servem para converter os endereços reais do IAS na representação utilizada. Isso acontece porque, no
// IAS, podem haver duas instruções no mesmo endereço.
static inline uint16_t get_actual_address(uint16_t addr) {
	return addr/2;
}

static inline uint16_t set_actual_address(uint16_t addr) {
	return addr*2;
}

struct sc_map_su labels;

// Lista de endereços de diretivas .org
struct address_list assembler_addresses;

void assembler_init(void) {
	sc_map_init_su(&labels, 0, 0);
	assembler_addresses = address_list_init();
}

static void add_label(const char* name, unsigned int value, unsigned int line, bool* error) {
	sc_map_get_su(&labels, name);
	if (sc_map_found(&labels)) {
		fprintf(stderr, "ERROR in line %d: name %s already exists!\n", line, name);
		*error = true;
	}

	sc_map_put_su(&labels, name, value);
}

static uint16_t get_label_value(const char* name, unsigned int line, bool* error) {
	unsigned int value = sc_map_get_su(&labels, name);
	if (!sc_map_found(&labels)) {
		fprintf(stderr, "ERROR in line %d: name %s does not exists!\n", line, name);
		*error = true;
	}

	return value;
}

static inline bool is_token_number_or_label(struct lexer_token token) {
	return is_token_number(token) || token.type==LABEL_USAGE;
}

static uint16_t get_value_from_label_or_number(struct lexer_token token, bool* error) {
	if (token.type == LABEL_USAGE) {
		return get_actual_address(get_label_value(token.string, token.line, error));
	} else if (is_token_number(token)) {
		return get_number_from_token(token, error);
	} else {
		fprintf(stderr, "ERROR in line %d: argument should be number or name.\n", token.line);
		*error=true;
		return 0;
	}
}

static struct lexer_token get_subtoken_from_token(struct lexer_token token) {
	struct lexer_token result = determine_token(token.string, token.line);
	return result;
}

/*
	Este vai pegar o endereço de cada coisa e armazenar
*/
static void pass1(struct lexer_tokens_list* tokens, bool* error) {
	// Macros são "ruins", mas repetir código é pior
	#define ASSIGN_LABELS() \
		/* 
			Macros em C são horríveis, tenho que colocar o caracter "\" no fim, o que confunde
			também os comentários em estilo C++/C99 "//", que comentam a próxima linha, por isso
			estou usando comentários de bloco.
		*/ \
		/* Não pode ser size_t porque tem que ser menor que zero */ \
		for (ptrdiff_t j=i-1; j >= 0 && tokens->data[j].type == LABEL_DECLARATION; j--) { \
			add_label(tokens->data[j].string, address, tokens->data[j].line, error); \
		}

	// ATENÇÃO: para encontrar o endereço REAL que será escrito, divida por dois. Isso acontece 
	// porque o IAS consegue armazenar DUAS instruções em um mesmo endereço
	uint16_t address=0;

	for (size_t i=0; i<tokens->size; i++) {
		switch (tokens->data[i].type) {
		case KEYWORD_ORG:
			/*if (!(is_token_number(tokens->data[i+1]) || tokens->data[i+1].type == LABEL_USAGE)) {
				fprintf(stderr, "ERROR in line %d: keyword \".org\" should be followed by a number or name.\n", tokens->data[i].line);
				exit(1);
			}*/

			// EU ATÉ AGORA NÃO ENTENDI, mas, por algum motivo, tenho que fazer isso pro compilador não
			// dar o aviso "a label can only be part of a statement and a declaration is not a statement".
			;

			// Se for um número
			const uint16_t next_address=set_actual_address(get_value_from_label_or_number(tokens->data[i+1], error));
			if (next_address < address) {
				fprintf(stderr, "ERROR in line %d: address 0x%x in \".org\" overlaps the address 0x%x.\n", tokens->data[i+1].line, get_actual_address(next_address), get_actual_address(address));
				*error=true;
			}
			address=next_address;
			address_list_add(&assembler_addresses, address);

			i++;
			break;
		case KEYWORD_WORD:
			/*if (!is_token_number(tokens->data[i+1])) {
				fprintf(stderr, "ERROR in line %d: keyword \".word\" should be followed by a number.\n", tokens->data[i].line);
				exit(1);
			}*/

			// Se for ímpar
			if (address % 2 == 1) {
				address++;
			}

			ASSIGN_LABELS();

			address++;

			i++;
			break;
		case KEYWORD_SET:
			if (!(tokens->data[i+1].type == LABEL_USAGE)) {
				fprintf(stderr, "ERROR in line %d: keyword \".set\" should be followed by a name.\n", tokens->data[i].line);
				*error=true;
			}

			// Gambiarra em que o valor do macro é multiplicado por dois para não ser tratado como
			// uma label normal.
			add_label(
				tokens->data[i+1].string, 
				set_actual_address(get_value_from_label_or_number(tokens->data[i+2], error)), 
				tokens->data[i+1].line, error
			);
			
			i+=2;
			
			break;
		case KEYWORD_WFILL:
			if (!(is_token_number_or_label(tokens->data[i+1]) && is_token_number_or_label(tokens->data[i+2]))) {
				fprintf(stderr, "ERROR in line %d: keyword \".wfill\" should be followed by two numbers or names.\n", tokens->data[i].line);
				*error=true;
			}

			// Se for ímpar
			if (address % 2 == 1) {
				address++;
			}

			ASSIGN_LABELS();

			address+=set_actual_address(get_value_from_label_or_number(tokens->data[i+1], error));

			i+=2;
			break;
		default:
			if (tokens->data[i].type >= INSTRUCTION_LOAD && tokens->data[i].type <= INSTRUCTION_RSH) {
				ASSIGN_LABELS();

				address++;
			}

			break;
		};

		if (get_actual_address(address) > 0x3FF) {
			fprintf(stderr, "ERROR: the size of the program exceeded 0x3FF -- maxium supported by the IAS machine.\n");
			*error=true;
		}
	}

	// Percebi que não fiz isso enquanto estava na escola
	#undef ASSIGN_LABELS
}

struct assembler_halfword {
	uint8_t opcode;
	uint16_t value;
};

static struct assembler_halfword get_instruction(struct lexer_tokens_list* tokens, size_t* index, bool* error) {
	struct assembler_halfword result = {
		.opcode = 0x00,
		.value = 0x00
	};

	size_t i=*index;
	switch (tokens->data[i].type) {
	case INSTRUCTION_LOAD:
		i++;
		if (tokens->data[i].type == REGISTER_MQ) {
			if (is_token_number_or_label(tokens->data[i+1])) {
				i++;
				result.opcode=0x09;
				result.value=get_value_from_label_or_number(tokens->data[i], error);
			} else {
				result.opcode=0x0A;
			}
		} else if (is_token_number_or_label(tokens->data[i]))  {
			result.opcode=0x01;
			result.value=get_value_from_label_or_number(tokens->data[i], error);
		} else if (tokens->data[i].type==TOKEN_NEGATIVE) {
			result.opcode=0x02;
			result.value=get_value_from_label_or_number(get_subtoken_from_token(tokens->data[i]), error);
		} else if (tokens->data[i].type==TOKEN_ABSOLUTE) {
			result.opcode=0x03;
			result.value=get_value_from_label_or_number(get_subtoken_from_token(tokens->data[i]), error);
		} else {
			fprintf(stderr, "ERROR in line %d: instruction LOAD should be followed by a number, name, or its absolute or negative counterparts.\n", 
					tokens->data[i].line);
			*error=true;
		}
		break;
	case INSTRUCTION_STOR:
		i++;
		result.value=get_value_from_label_or_number(tokens->data[i], error);
		// STOR foo, 8:19
		if (tokens->data[i+1].type == PARAM_8_TO_19) {
			result.opcode=0x12;
			i++;
		// STOR foo, 28:39
		} else if (tokens->data[i+1].type == PARAM_28_TO_39) {
			result.opcode=0x13;
			i++;
		// STOR foo
		} else {
			result.opcode=0x21;
		}
		break;
	case INSTRUCTION_STOR_PLUS:
		i++;		
		uint16_t label_logical_address = get_label_value(tokens->data[i].string, tokens->data[i].line, error);
		result.value = get_actual_address(label_logical_address);

		if (label_logical_address % 2 == 0) {
			result.opcode=0x12;
		} else {
			result.opcode=0x13;
		}

		break;
	case INSTRUCTION_JUMP:
		i++;
		result.value=get_value_from_label_or_number(tokens->data[i], error);

		i++;
		if (tokens->data[i].type == PARAM_0_TO_19) {
			result.opcode=0x0D;
		} else if (tokens->data[i].type == PARAM_20_TO_39) {
			result.opcode=0x0E;
		} else if (tokens->data[i].type==LABEL_USAGE) {
			// Se a label estiver na direita, é 0x0D, senão, é 0x0E
			if (get_label_value(tokens->data[i].string, tokens->data[i].line, error)%2  == 0) {
				result.opcode=0x0D;
			} else {
				result.opcode=0x0E;
			}
		} else {
			fprintf(stderr, "ERROR in line %d: instruction JUMP should be followed by a number or name and 0:19 or 20:39.\n", tokens->data[i].line);
			*error=true;
		}
		break;
	case INSTRUCTION_JUMP_PLUS:
		i++;
		result.value=get_value_from_label_or_number(tokens->data[i], error);

		if (tokens->data[i+1].type == PARAM_0_TO_19) {
			i++;
			result.opcode=0x0F;
		} else if (tokens->data[i+1].type == PARAM_20_TO_39) {
			i++;
			result.opcode=0x10;
		} else if (tokens->data[i].type==LABEL_USAGE) {
			// Se a label estiver na direita, é 0x0F, senão, é 0x10
			if (get_label_value(tokens->data[i].string, tokens->data[i].line, error)%2  == 0) {
				result.opcode=0x0F;
			} else {
				result.opcode=0x10;
			}
		} else {
			fprintf(stderr, "ERROR in line %d: instruction JUMP+ should be followed by a number or name and 0:19 or 20:39.\n", tokens->data[i].line);
			*error=true;
		}
		break;
	case INSTRUCTION_ADD:
		i++;

		if (tokens->data[i].type == TOKEN_ABSOLUTE) {
			result.opcode = 0x07;
			result.value = get_value_from_label_or_number(get_subtoken_from_token(tokens->data[i]), error);
		} else {
			result.opcode = 0x05;
			result.value = get_value_from_label_or_number(tokens->data[i], error);
		}
		break;
	case INSTRUCTION_SUB:
		i++;

		if (tokens->data[i].type == TOKEN_ABSOLUTE) {
			result.opcode = 0x08;
			result.value = get_value_from_label_or_number(get_subtoken_from_token(tokens->data[i]), error);
		} else {
			result.opcode = 0x06;
			result.value = get_value_from_label_or_number(tokens->data[i], error);
		}
		break;
	case INSTRUCTION_MUL:
		i++;

		result.opcode = 0x0B;
		result.value = get_value_from_label_or_number(tokens->data[i], error);

		break;
	case INSTRUCTION_DIV:
		i++;

		result.opcode = 0x0C;
		result.value = get_value_from_label_or_number(tokens->data[i], error);

		break;
	case INSTRUCTION_HALT:
		// Pula para um endereço que não existe		
		result.opcode = 0x0D;
		result.value = 0xFFF;

		break;
	case INSTRUCTION_LSH:
		result.opcode = 0x14;
		break;
	case INSTRUCTION_RSH:
		result.opcode = 0x15;
		break;
	default:
		fprintf(stderr, "ERROR in line %d: misplaced token.\n", tokens->data[i].line);
		*error=true;
		break;
	}

	/*if (result.value > 0x3FF) {
		fprintf(stderr, "ERROR in line %d: max value should be 0x3FF in hexadecimal, 1024 in decimal.\n", tokens->data[i].line);
		*error=true;
	}*/

	*index=i;

	return result;
}

static void print_word(FILE* output, struct assembler_halfword word[2], uint16_t address, bool* error) {
	// Se estivermos no início de um endereço e não tiver dado nenhum erro.
	if (address % 2 == 0 && address > 0 && !*error) {
		fprintf(output, "%03X %02X %03X %02X %03X\n", 
			get_actual_address(address-1), word[0].opcode, word[0].value, word[1].opcode, word[1].value);
	}
}

static void number_to_word(ias_word_t num, struct assembler_halfword word[2]) {
	// Extract first 16 hex digits (most significant part)
	word[0].opcode = (num >> 60) & 0xF;     // First 4 bits (1 hex digit)
	word[0].value = (num >> 48) & 0xFFF;    // Next 12 bits (3 hex digits)

	// Extract next 2 hex digits (middle part)
	word[1].opcode = (num >> 40) & 0xFF;    // Next 8 bits (2 hex digits)

	// Extract last 3 hex digits (least significant part)
	word[1].value = num & 0xFFF;            // Last 12 bits (3 hex digits)
}

static void assemble(FILE* output, struct lexer_tokens_list* tokens, bool* error) {
	// Vai pro próximo endereço ao invés de sobrescrever o que já está.
	#define JUMP_TO_NEXT_ADDRESS() \
		if (address % 2 == 1) { \
			word[1]=(struct assembler_halfword){0,0}; \
			address++; \
			print_word(output, word, address, error); \
		}

	// Duas halfwords, uma word
	struct assembler_halfword word[2] = {0};
	
	uint16_t address=0;
	uint16_t org = 0;
	for(size_t i=0; i < tokens->size; i++) {
		switch(tokens->data[i].type) {
		case KEYWORD_ORG:
			JUMP_TO_NEXT_ADDRESS();

			address=assembler_addresses.data[org];
			org++;
			i++;
			continue;
			break;
		case KEYWORD_WORD:
			JUMP_TO_NEXT_ADDRESS();

			uint64_t word_value=0;
			i++;
			if (is_token_number(tokens->data[i])) {
				word_value=get_word_from_token(tokens->data[i], error);
			} else if (tokens->data[i].type == LABEL_USAGE) {
				word_value=get_actual_address(get_label_value(tokens->data[i].string, tokens->data[i].line, error));
			} else {
				fprintf(output, "ERROR in line %d: keyword \".word\" should be followed by a number or name.\n", tokens->data[i].line);
				*error=true;
			}

			number_to_word(word_value, word);

			address+=2;
			break;
		case KEYWORD_WFILL:
			JUMP_TO_NEXT_ADDRESS();

			i++;
			uint16_t amount = get_value_from_label_or_number(tokens->data[i], error);

			uint64_t wfill_value=0;
			i++;
			if (is_token_number(tokens->data[i])) {
				wfill_value=get_word_from_token(tokens->data[i], error);
			} else if (tokens->data[i].type == LABEL_USAGE) {
				wfill_value=get_actual_address(get_label_value(tokens->data[i].string, tokens->data[i].line, error));
			} else {
				fprintf(output, "ERROR in line %d: keyword \".wfill\" should be followed by two numbers or names.\n", tokens->data[i].line);
				*error=true;
			}

			number_to_word(wfill_value, word);
			for (uint16_t i=0; i<amount; i++) {
				address+=2;
				print_word(output, word, address, error);
			}
			break;
		case KEYWORD_SET:
			i+=2;
			break;
		case LABEL_DECLARATION:
			continue;
			break;
		default:
			if (tokens->data[i].type >= INSTRUCTION_LOAD && tokens->data[i].type <= INSTRUCTION_RSH) {
				word[address%2] = get_instruction(tokens, &i, error);

				address++;
			} else {
				fprintf(stderr, "ERROR in line %d: misplaced token.\n", tokens->data[i].line);
				*error=true;
			}
			break;
		}


		print_word(output, word, address, error);
	}

	#undef JUMP_TO_NEXT_ADDRESS
}

void assembler_assemble(FILE* output,struct lexer_tokens_list* tokens, bool* error) {
	pass1(tokens, error);

	// Para debugging apenas. Por favor não invente de dar push com isso descomentado como fiz em
	// https://github.com/filipemd/termo-x86_64-asm
	/*const char* key;
	unsigned int value;
	sc_map_foreach(&labels, key, value) {
		printf("Label %s address 0x%x.\n", key, get_actual_address(value));
	}

	for (size_t i=0; i < assembler_addresses.size; i++) {
		printf(".org address 0x%x.\n", get_actual_address(assembler_addresses.data[i]));
	}*/

	assemble(output, tokens, error);
}

void assembler_end(void) {
	sc_map_term_su(&labels);
	address_list_free(&assembler_addresses);
}
