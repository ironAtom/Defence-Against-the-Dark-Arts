/*
 * dictionary.c
 *
 *  Created on: Mar 13, 2015
 *      Author: user
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dictionary.h"

dict_t *create_dictionary()
{
	FILE *fp = fopen("/usr/share/dict/words", "r");
	if (fp == NULL) {
		printf("Error opening words file\n");
		return NULL;
	}

	int words_capacity = 1024;
	char **words = malloc(sizeof(char *) * words_capacity);
	int words_read = 0;


	char word[65];
	while (!feof(fp) && fscanf(fp, "%64s", word)) {

		if (words_capacity <= words_read) {
			words_capacity *= 2;
			words = realloc(words, words_capacity * sizeof (char *));
		}
		words[words_read] = strndup(word, 64);
		words_read++;
	}

	fclose(fp);

	dict_t *result = (dict_t *)malloc(sizeof(dict_t));
	result->count = words_read;
	result->words = words;

	printf("read %d words from the dictionary\n", words_read);
	return result;
}

void free_dictionary(dict_t *dict)
{
	if (dict != NULL) {
		free(dict->words);
		free(dict);
	}
}

bool find_word(dict_t *dict, const char *word, int word_length)
{
	int left = 0;
	int right = dict->count - 1;

	while (left <= right) {
		int mid = (right + left) / 2;
		char *dict_word = dict->words[mid];

		int comp = strncmp(dict_word, word, word_length);
		if (comp == 0 && strlen(dict_word) > word_length) {
			comp = 1;
		}
		if (comp > 0) {
			right = mid - 1;
		}
		else if (comp < 0) {
			// check prefix-ness
			left = mid + 1;
		}
		else {
			return true;
		}
	}
	return false;
}
