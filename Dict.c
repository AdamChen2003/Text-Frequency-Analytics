// COMP2521 21T2 Assignment 1 z5363476
// Dict.c serves to recieve any input strings and store them in
// a binary search tree which is organised lexicographically.
// The frequency of these input strings are also noted and will
// be utilised when placing them in an array containing the top
// 'n' occuring input strings.

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Dict.h"
#include "WFreq.h"

#define MAXWORDCOUNT 600000
#define MAXLINE 1000

typedef struct DictNode *Node;
typedef struct WFreq *Wfreq;

struct DictNode {
	Wfreq input;
	Node left;
	Node right;
};

struct DictRep {
	Node root;
};

static Node nodeNew(char *word);
static Wfreq WfreqNew(char *word);
static void treeFree(Node node);
static void WfreqFree(Wfreq wfreq);
static Node inserting(Node root, char *word);
static void nodeFind(Node root, char *word, int *count);
static int treeToArray(Node root, WFreq *allWords, int rank);
static int comparison(const void *pointer1, const void *pointer2);

// Creates a new Dictionary
Dict DictNew(void) {
	Dict newDict = malloc(sizeof(struct DictRep));
	newDict->root = NULL;
	return newDict;
}

// Creates a new node with an associated word
static Node nodeNew(char *word) {
	Node newNode = malloc(sizeof(struct DictNode));
	newNode->right = NULL;
	newNode->left = NULL;
	newNode->input = WfreqNew(word);
	return newNode;
}

// Creates a new WFreq pointer with an associated word
static Wfreq WfreqNew(char *word) {
	Wfreq newWfreq = malloc(sizeof(struct WFreq));
	newWfreq->freq = 1;
	newWfreq->word = malloc(strlen(word) + 1);
	strcpy(newWfreq->word, word);
	return newWfreq;
}

// Frees the given Dictionary
void DictFree(Dict d) {
	treeFree(d->root);
	free(d);
}

// Frees the root of the dictionary
static void treeFree(Node node) {
	if (node != NULL) {
		treeFree(node->left);
		treeFree(node->right);
		WfreqFree(node->input);
		free(node);
	}
}

// Frees the pointer to WFreq
static void WfreqFree(Wfreq wfreq) {
	free(wfreq->word);
	free(wfreq);
}

// Inserts an occurrence of the given word into the Dictionary
void DictInsert(Dict d, char *word) {
	d->root = inserting(d->root, word);
}

// Inserts an occurance of the given word into the root
static Node inserting(Node root, char *word) {
	if (root == NULL) {
	    // If the tree is empty or there is a new word
		root = nodeNew(word);
	} else {
		// Traversing the tree to search for existing words
		int comparison = strcmp(root->input->word, word);
		if (comparison == 0) {
			// If it's a duplicate word
			root->input->freq++;
		} else if (comparison < 0) {
			root->left = inserting(root->left, word);
		} else {
			// comparison > 0
			root->right = inserting(root->right, word);
		}
	}
	return root;
}

// Returns the occurrence count of the given word. Returns 0 if the word
// is not in the Dictionary.
int DictFind(Dict d, char *word) {
	int count = 0;
	int *pointer = &count;
	nodeFind(d->root, word, pointer);
	return count;
}

// Helper function for DictFind
static void nodeFind(Node root, char *word, int *count) {
	if (root != NULL) {
		int comparison = strcmp(word, root->input->word);
		if (comparison < 0) {
			nodeFind(root->right, word, count);
		} else if (comparison > 0) {
			nodeFind(root->left, word, count);
		} else {
		    // found the word
			*count = root->input->freq;
		}
	}
}

// Finds  the top `n` frequently occurring words in the given Dictionary
// and stores them in the given  `wfs`  array  in  decreasing  order  of
// frequency,  and then in increasing lexicographic order for words with
// the same frequency. Returns the number of WFreq's stored in the given
// array (this will be min(`n`, #words in the Dictionary)) in  case  the
// Dictionary  does  not  contain enough words to fill the entire array.
// Assumes that the `wfs` array has size `n`.
int DictFindTopN(Dict d, WFreq *wfs, int n) {
	WFreq *allWords = malloc(sizeof(struct WFreq) * MAXWORDCOUNT);
	int wordCount = treeToArray(d->root, allWords, 0);
	qsort(allWords, wordCount, sizeof(struct WFreq), comparison);
	if (wordCount < n) {
		n = wordCount;
	} // Loading the top n words from allWords to wfs
	for (int rank = 0; rank < n; rank++) {
		wfs[rank] = allWords[rank];
	}
	free(allWords);
	return n;
}

// This function loads all words from the binary search tree into
// the allWords array and returns the number of total words which have been
// inserted into the array
static int treeToArray(Node root, WFreq *allWords, int rank) {
	if (root == NULL) {
		return rank;
	}
	allWords[rank] = *(root->input);
	rank++;
	if (root->left != NULL) {
		rank = treeToArray(root->left, allWords, rank);
	}
	if (root->right != NULL) {
		rank = treeToArray(root->right, allWords, rank);
	}
	return rank;
}

// This is the comparison used in qsort
static int comparison(const void *pointer1, const void *pointer2) {
	WFreq wfreqA = *(struct WFreq *) pointer1;
	WFreq wfreqB = *(struct WFreq *) pointer2;
	// If the words have different frequencies, determine which word has
	// greater frequency
	if (wfreqA.freq != wfreqB.freq) {
		return wfreqB.freq - wfreqA.freq;
	}
	// If the words have the same frequency, determine which word is greater
	// lexicographically
	return strcmp(wfreqA.word, wfreqB.word);
}

// Displays the given Dictionary. This is purely for debugging purposes,
// so  you  may  display the Dictionary in any format you want.  You may
// choose not to implement this.
void DictShow(Dict d) {
}
