/** @file fileParse.h
 *  @brief 
 *
 *
 *
 *  @author root
 *  @bug No known bugs.
 *
 *  Created on: Mar 2, 2019
 *
 */

#ifndef FILEPARSE_H_
#define FILEPARSE_H_

typedef struct{
	char *line;
	char **words;
	size_t num_words;
} LINE;

void *parseFile(char *resourceTable);
size_t getLines(FILE *fptr, LINE **lines);
void tokenizeLines(LINE *lines, size_t num_lines);
void *getCarveoutAddress(LINE *lines, size_t num_lines);
void printLines(LINE *lines, size_t num_lines);
void printTokensPerLine(LINE *lines, size_t num_lines);
void freeLines(LINE **lines, size_t num_lines);

#endif /* FILEPARSE_H_ */
