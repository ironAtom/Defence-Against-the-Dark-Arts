/*
 * 14-735 Secure Coding Assignment4
 * Qinyu Tong <qtong@andrew.cmu.edu>
 * */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <cilk/cilk.h>
#include <cilk/reducer.h>

#include "dictionary.h"


typedef struct {
	char **results;
	int count;
	int capacity;
} results_t;



// Assumes results is uninitialized. Do not call this twice on the same results object.
void results_init(results_t *results) {
	if (results == NULL) {
		return;
	}

	results->results = NULL;
	results->count = 0;
	results->capacity = 0;
}

void results_free(results_t *results) {
	if (results != NULL) {
		if (results->results != NULL) {
			for (int i = 0; i < results->count; i++) {
				free(results->results[i]);
			}
			free(results->results);
			results->count = 0;
			results->capacity = 0;
		}
	}
}

void results_append(results_t *results, const char *word) {
	if (results->capacity == results->count) {
		int new_capacity = (results->capacity == 0) ? 1 : results->capacity * 2;
		results->results = realloc(results->results, sizeof(char *) * new_capacity);
		results->capacity = new_capacity;
	}

	results->results[results->count] = strdup(word);
	results->count++;
}


void results_print(results_t *results) {
	for (int i = 0; i < results->count; i++) {
		printf("%s\n", results->results[i]);
	}
}

//my funcitions

void result_concat(results_t* left, results_t* right){
	for (int i = 0; i < right->count; i++){
		results_append(left, right->results[i]);
	}
	results_free(right);
}

void identity_results(void *reducer, void* results){
	results_init((results_t*)results);
}

void reduce_results(void *reducer, void *left, void *right){
	result_concat((results_t*)left, (results_t*)right);
}

void swap(char *s, int i, int j) {
	char t = s[i];
	s[i] = s[j];
	s[j] = t;
}

CILK_C_DECLARE_REDUCER(results_t) result_reducer =
	    CILK_C_INIT_REDUCER(results_t,
	    					reduce_results,
							identity_results,
							__cilkrts_hyperobject_noop_destroy);
							//initial value omitted


// finds anagrams by considering all permutations of input_word starting with pos.
void serial_find_anagrams_with_length(dict_t *dict, char *input_word, results_t *results, int word_length, int pos) {
	if (pos == word_length) {
		if (find_word(dict, input_word, pos)) {
			results_append(results, input_word);
		}
	}

	for (int i = pos; i < word_length; i++) {
		swap(input_word, pos, i);

		// We can't have a full anagram yet, since we've still got more characters to go, so don't bother checking.
		serial_find_anagrams_with_length(dict, input_word, results, word_length, pos+1);
		swap(input_word, pos, i);
	}
}

void serial_find_anagrams(dict_t *dict, char *input_word, results_t *result) {
	int word_length = strlen(input_word);

	serial_find_anagrams_with_length(dict, input_word, result, word_length, 0);
}

void parallel_find_anagrams_with_length(dict_t *dict, char *input_word, results_t *results, int word_length, int pos) {
	if (pos == word_length) {
		if (find_word(dict, input_word, pos)) {
			results_append(&REDUCER_VIEW(result_reducer), input_word);
		}
	}
		for (int i = pos; i < word_length; i++) {
				swap(input_word, pos, i);

				// We can't have a full anagram yet, since we've still got more characters to go, so don't bother checking.
				parallel_find_anagrams_with_length(dict, input_word, results, word_length, pos+1);
				swap(input_word, pos, i);
		}
}

void parallel_find_anagrams(dict_t *dict, char *input_word, results_t *result) {
	int word_length = strlen(input_word);
	if (word_length < 3){
		serial_find_anagrams(dict, input_word, result);
	}
	else{
		results_init(&REDUCER_VIEW(result_reducer));
		CILK_C_REGISTER_REDUCER(result_reducer);

		//expand previous two layer of recursive, create a larger set for cilk_for to split the work
		//to each processor
		results_t *premu_words = malloc(sizeof(results_t));
		results_init(premu_words);
		for (int i = 0; i < word_length; i++){
			swap(input_word, 0, i);
			results_append(premu_words,input_word);
			swap(input_word, i, 0);
		}

		int count = premu_words->count;
		for (int i = 0; i < count; i++){
			for (int j = 2; j < word_length; j++){
				swap(premu_words->results[i], 1, j);
				results_append(premu_words,premu_words->results[i]);
				swap(premu_words->results[i], j, 1);
			}
		}

		cilk_for (int i = 0; i < premu_words->count; i++){
			parallel_find_anagrams_with_length(dict, premu_words->results[i]  , result, word_length, 2);
		}

		results_free(premu_words);

		CILK_C_UNREGISTER_REDUCER(result_reducer);
		*result = REDUCER_VIEW(result_reducer);
	}
}



int main (int argc, char *argv[]) {
	if (argc < 2) {
		printf("Usage: anagrams input-word");
		exit(0);
	}

	char *input_word = argv[1];

	// Read the dictionary
	dict_t *dict = create_dictionary();

	// Find anagrams in serial
	results_t *results = malloc(sizeof(results_t));
	results_init(results);

	struct timeval start_time;
	gettimeofday(&start_time, NULL);
	serial_find_anagrams(dict, input_word, results);

	struct timeval end_time;
	gettimeofday(&end_time, NULL);
	double serial_elapsed_time = (end_time.tv_sec - start_time.tv_sec) + ((double)end_time.tv_usec - (double)start_time.tv_usec) / 1000000.0;

	printf("serial time required: %lf\n", serial_elapsed_time);
	printf("serial results:\n");
	results_print(results);
	results_free(results);
	free(results);

	printf("\n"); // separate sections with whitespace

	// Find anagrams in parallel
	results = malloc(sizeof(results_t));
	results_init(results);

	gettimeofday(&start_time, NULL);
	parallel_find_anagrams(dict, input_word, results);
	gettimeofday(&end_time, NULL);
	double parallel_elapsed_time = (end_time.tv_sec - start_time.tv_sec) + ((double)end_time.tv_usec - (double)start_time.tv_usec) / 1000000.0;
	printf("parallel time required: %lf\n", parallel_elapsed_time);

	printf("parallel results:\n");
	results_print(results);

	printf("\n");
	printf("Parallel speedup: %lf\n", serial_elapsed_time / parallel_elapsed_time);
	results_free(results);
	free(results);

	free_dictionary(dict);

    return 0;
}
