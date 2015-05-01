/*
 * dictionary.h
 *
 *  Created on: Mar 13, 2015
 *      Author: user
 */

#ifndef DICTIONARY_H_
#define DICTIONARY_H_

#include <stdbool.h>

typedef struct {
	char **words;
	int count;
} dict_t;

dict_t *create_dictionary();
void free_dictionary(dict_t *dict);

bool find_word(dict_t *dict, const char *word, int word_length);


#endif /* DICTIONARY_H_ */
