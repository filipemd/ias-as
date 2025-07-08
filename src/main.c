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

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "lexer.h"
#include "assembler.h"

static const char* version="0.3.2";

static void show_version(const char* argv0) {
	printf(
		"%s version %s\n",
		argv0, version
	);
}

static void show_usage(const char* argv0) {
	printf(
		"Usage:\n"
		"%s <source file> [output file]\n",
		argv0
	);
}

int main(int argc, char* argv[]) {
	if (argc <= 1) {
		show_usage(argv[0]);
		return 1;
	}

	if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
		show_usage(argv[0]);
		return 0;
	}

	if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0) {
		show_version(argv[0]);
		return 0;
	}	

	FILE* input_file = fopen(argv[1], "r");

	if (input_file == NULL) {
		fprintf(stderr, "Error loading file: %s\n", argv[1]);
		return 1;
	}

	FILE* output_file = NULL;

	if (argc == 2) {
		output_file = stdout;
	} else {
		// No Windows, é \r\n, no Linux e MacOS, é \n. Então tem que estar unificado para não dar problema
		// nos testes
		output_file = fopen(argv[2], "wb");
		if (output_file == NULL) {
			fprintf(stderr, "Error loading file: %s\n", argv[2]);
			return 1;
		}
	}

	bool error=false;

	struct lexer_tokens_list tokens=lexer_lex(input_file, &error);	
	
	fclose(input_file);

	/*for(size_t i=0; i<tokens.size; i++) {
		printf("Line %d token %d string %s.\n", tokens.data[i].line, tokens.data[i].type, tokens.data[i].string);
	}*/

	assembler_init();

	assembler_assemble(output_file, &tokens, &error);

	if (error && output_file != stdout) {
		int status = remove(argv[2]);
		if (status != 0) {
			fprintf(stderr, "Error removing file %s.\n", argv[2]);
		}
	}

	assembler_end();

	lexer_tokens_list_free(&tokens);

	return error;
}
