// COMP2521 21T2 Assignment 1 z5363476
// tw.c serves to compute top 'Nwords' most frequent words from an input file.
// It does so by preprocessing the file which include normalising, tokenising, 
// filtering  out any words from the 'stopwords' file and stemming each word
// before inserting them into a dictionary ADT. From here, tw.c prints the 
// 'nWords' most frequent by utilising the functions from Dict.c.
// Usage: ./tw [Nwords] File

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Dict.h"
#include "stemmer.h"
#include "WFreq.h"

#define MAXLINE 1000
#define MAXWORD 100
#define STOPWORDCOUNT 654
#define BEGINLENGTH 13
#define ENDLENGTH 11
#define isWordChar(c) (isalnum(c) || (c) == '\'' || (c) == '-')

static bool isStart(char line[MAXWORD + 1], char beginning[BEGINLENGTH]);
static bool isFinish(char line[MAXWORD + 1], char ending[ENDLENGTH]);
static void stopwordsArray(char stopwords[STOPWORDCOUNT][MAXWORD + 1]);
static Dict fileProccessing(FILE *fp, char stopwords[STOPWORDCOUNT][MAXWORD + 1]);
static void lineProcessing(Dict wordCounter, char line[MAXLINE + 1], 
char stopwords[STOPWORDCOUNT][MAXWORD + 1]);
static void tokeniseAndNormalise(char line[MAXWORD + 1]);
static bool isStopword(char stopwords[STOPWORDCOUNT][MAXWORD + 1], 
int start_index, int end_index, char *word);
static void printTopNWord(Dict WordCounter, int nWords);

int main(int argc, char *argv[]) {
	int nWords;    // number of top frequency words to show
	char *fileName;  // name of file containing book text
	// process command-line args
	switch (argc) {
		case 2:
			nWords = 10;
			fileName = argv[1];
			break;
		case 3:
			nWords = atoi(argv[1]);
			if (nWords < 10) nWords = 10;
			fileName = argv[2];
			break;
		default:
			fprintf(stderr, "Usage: %s [Nwords] File\n", argv[0]);
			exit(EXIT_FAILURE);
	}
	FILE *fp;
	fp = fopen(fileName, "r");
	if (fp == NULL) {
		fprintf(stderr, "Can't open %s\n", fileName);
		exit(EXIT_FAILURE);
	}
	
	// Initialising an array which stores the words from the stopwords file
	char stopwords[STOPWORDCOUNT][MAXWORD + 1];
	stopwordsArray(stopwords);
	Dict wordCounter = fileProccessing(fp, stopwords);
	printTopNWord(wordCounter, nWords);
	DictFree(wordCounter);
	fclose(fp);
	return 0;
}

// Determines where the text begins
// Returns true if input string is the start
// Returns false otherwise
static bool isStart(char line[MAXLINE + 1], char beginning[BEGINLENGTH]) {
	for (int i = 0; i < BEGINLENGTH - 1; i++) {
		if (line[i] != beginning[i]) {
			return false;
		}
	}
	return true;
}

// Determines where the text ends
// Returns true if input string is the end
// Returns false otherwise
static bool isFinish(char line[MAXLINE + 1], char ending[ENDLENGTH]) {
	for (int i = 0; i < ENDLENGTH - 1; i++) {
		if (line[i] != ending[i]) {
			return false;
		}
	}
	return true;
}

// This function loads the words from the stopwords file into an array
static void stopwordsArray(char stopwords[STOPWORDCOUNT][MAXWORD + 1]) {
	// Opening the stopwords file
	FILE *sw;
	sw = fopen("stopwords", "r");
	if (sw == NULL) {
		fprintf(stderr, "Can't open stopwords\n");
		exit(EXIT_FAILURE);
	}
	char stopword[MAXWORD + 1];
	int i = 0;
	while (fscanf(sw, "%s", stopword) != EOF) {
		strcpy(stopwords[i], stopword);
		i++;
	}
	fclose(sw);
}

// This function prints error messages when reading through the file and also 
// scans the file line by line which are then fed to the LineProcessing function
static Dict fileProccessing(FILE *fp, char stopwords[STOPWORDCOUNT][MAXWORD + 1]) {
	Dict wordCounter = DictNew();
	char line[MAXLINE + 1];
	char beginning[BEGINLENGTH] = "*** START OF";
	char ending[ENDLENGTH] = "*** END OF";
	bool isText = false;
	while (fgets(line, MAXLINE + 1, fp) != NULL) {
		if (isText && isFinish(line, ending)) {
		    // When "*** END OF" appears after "*** START OF"
			// This program breaks out of while loop since there is no need to
			// process any words from the file after the text ends.
			isText = false;
			break;
		} else if (isFinish(line, ending) && !isText) {
			// If the "*** END OF" line appears without having encountered 
			// the "*** START OF" line
			fprintf(stderr, "Not a Project Gutenberg book\n");
			exit(EXIT_FAILURE);
		}
		if (isText) {
			lineProcessing(wordCounter, line, stopwords);
		}
		if (isStart(line, beginning)) {
			isText = true;
		}
	}
	if (isText) {
		// If the entire book has been read and "*** START OF" never appears
		fprintf(stderr, "Not a Project Gutenberg book\n");
		exit(EXIT_FAILURE);
	}
	return wordCounter;
}

// This function preproccesses each given line and then inserts the 
// stemmed words into the dict ATD
static void lineProcessing(Dict wordCounter, char line[MAXLINE + 1], 
char stopwords[STOPWORDCOUNT][MAXWORD + 1]) {
	tokeniseAndNormalise(line);
	// Separating line into 'word' substrings
	char *word = strtok(line, " ");
	while (word != NULL) {
		if (!isStopword(stopwords, 0, STOPWORDCOUNT - 1, word) && strlen(word) > 1) {
			// If the word is not a stopword and is not a single character
			// We stem the word and then insert into the dictionary
			stem(word, 0, strlen(word) - 1);
			DictInsert(wordCounter, word);
		}
		word = strtok(NULL, " ");
	}
}

// This function replaces any character which isn't valid
// with a space and changes all uppercase characters into lowercase
static void tokeniseAndNormalise(char line[MAXLINE + 1]) {
	int index = 0;
	while (line[index] != '\0') {
		if (!isWordChar(line[index])) {
			line[index] = ' ';
		}
		line[index] = tolower(line[index]);
		index++;
	}
}

// Conducts a binary search of the stopwords array to determine if 'line' is a
// stopword
// Returns true if line is a stopword or else false. The algorithm for this
// binary search was found online with this link
// https://www.tutorialspoint.com/data_structures_algorithms/binary_search_algorithm.htm
static bool isStopword(char stopwords[STOPWORDCOUNT][MAXWORD + 1], 
int start_index, int end_index, char *word) {
	if (end_index >= start_index) {
		int middle = start_index + (end_index - start_index) / 2;
		// comparison ensures that we do not have to run strcmp multiple times
		int comparison = strcmp(stopwords[middle], word);
		if (comparison == 0) {
			return true;
		}
		if (comparison > 0) {
			return isStopword(stopwords, start_index, middle - 1, word);
		}
		// if  comparison < 0
		return isStopword(stopwords, middle + 1, end_index, word);
	}
	return false;
}

// This function initialises a wfs array which stores the top 'nWords' words
// which is then utilised to print these words out
static void printTopNWord(Dict wordCounter, int nWords) {
	WFreq *wfs = malloc(sizeof(struct WFreq) * nWords);
	int wfsSize = DictFindTopN(wordCounter, wfs, nWords);
	if (wfsSize < nWords) {
		// If nWords is greater than the unique word count of the text file
		// then we make nWords equal the unique word count
		nWords = wfsSize;
	}
	for (int i = 0; i < nWords; i++) {
		// Printing the first n elements from wfs which are the top n words
		printf("%d %s\n", wfs[i].freq, wfs[i].word);
	}
	free(wfs);
}