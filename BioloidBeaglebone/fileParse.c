/** @file fileParse.c
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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <err.h>
#include <errno.h>
#include <sys/mman.h>

#include "fileParse.h"

void *parseFile(char *resourceTable)
{
	FILE *fptr;
	LINE *lines = NULL;
	size_t num_lines = 0;
	void *carveoutAddress = NULL;

	fptr = fopen(resourceTable, "r");
	if(fptr != NULL)
	{
		num_lines = getLines(fptr, &lines);
		tokenizeLines(lines, num_lines);
		printTokensPerLine(lines, num_lines);
		printf("\n");
		carveoutAddress = getCarveoutAddress(lines, num_lines);
		freeLines(&lines, num_lines);
	}
	fclose(fptr);

	return carveoutAddress;
}

size_t getLines(FILE *fptr, LINE **lines)
{
	char *tmp = NULL;
	size_t len = 0;
	ssize_t nread = 0;
	size_t num_lines = 0;

	while((nread=getline(&tmp, &len, fptr)) != -1)
	{
		*lines = realloc(*lines, ++num_lines * sizeof(LINE));
		(*lines)[num_lines - 1].line = strdup(tmp);
		(*lines)[num_lines - 1].words = NULL;
		(*lines)[num_lines - 1].num_words = 0;
	}

	free(tmp);
	return num_lines;
}

void tokenizeLines(LINE *lines, size_t num_lines)
{
	for(int counter = 0; counter < num_lines; counter++)
	{
		LINE *line = &lines[counter];
		for(char *end=line->line; *end!='\0'; end++)
		{
			while(isspace(*end))end++;
			if(*end=='\0')break;
			char *start=end;
			size_t charactersInWord=0;
			while((!isspace(*end)) && (*end != '\0'))
			{
				charactersInWord++;
				end++;
			}
			line->words=realloc(line->words, ++(line->num_words) * sizeof(char *));
			line->words[line->num_words - 1] = malloc(charactersInWord + 1);
			memcpy(line->words[line->num_words - 1], start, charactersInWord);
			line->words[line->num_words - 1][charactersInWord] = '\0';
		}
	}
}

void *getCarveoutAddress(LINE *lines, size_t num_lines)
{
	void *map_base, *virt_addr;
	off_t target;
	size_t page_size, mapped_size, offset_in_page;
	int fd;

	for(int counter = 0; counter < num_lines; counter++)
	{
		LINE *line = &lines[counter];
		if(line->num_words==3)
		{
			if(strcmp(line->words[0], "Physical") == 0 && strcmp(line->words[1], "Address") == 0)
			{
				target = strtoull(line->words[2], NULL, 0);
				line = &lines[++counter];
				mapped_size = strtoull(line->words[1], NULL, 0);
				fd = open("/dev/mem", (O_RDWR | O_SYNC));
				page_size = getpagesize();
				offset_in_page = (unsigned)target & (page_size - 1);
				map_base = mmap(NULL,
								mapped_size + offset_in_page,
								(PROT_READ | PROT_WRITE),
								MAP_SHARED,
								fd,
								target & ~(off_t)(page_size - 1));
				if(map_base == MAP_FAILED) exit(1);
				virt_addr = (char *)map_base + offset_in_page;
				return virt_addr;
			}
		}
	}
	return 0;
}

void printLines(LINE *lines, size_t num_lines)
{
	printf("In function printLines %x/n",lines);
	for(int counter = 0; counter < num_lines; counter++)
	{
		printf("%s", lines[counter].line);
	}
}

void printTokensPerLine(LINE *lines, size_t num_lines)
{
	for(int counter = 0; counter < num_lines; counter++)
	{
		for(int wordCounter = 0; wordCounter < lines[counter].num_words; wordCounter++)
		{
			printf("%s ", lines[counter].words[wordCounter]);
		}
		printf("\n");
	}
}

void freeLines(LINE **lines, size_t num_lines)
{
	for(int counter = 0; counter < num_lines; counter++)
	{
		for(int wordCounter = 0; wordCounter < (*lines)[counter].num_words; wordCounter++)
		{
			free((*lines)[counter].words[wordCounter]);
		}
		free((*lines)[counter].words);
		free((*lines)[counter].line);
	}

	free(*lines);
}


//void parseFile()
//{
//	FILE *fptr;
//	char *resourceTable=(char *)malloc(1);
//	char *tmp=NULL;
//	size_t len;
//	ssize_t nread;
//
//	if(resourceTable!=NULL)
//	{
//		*resourceTable='\0';
//	}
//	else
//	{
//		exit(1);
//	}
//
//	fptr = fopen("/sys/kernel/debug/remoteproc/remoteproc2/resource_table", "r");
//	if(fptr != NULL)
//	{
//		while((nread=getline(&tmp, &len, fptr)) !=-1)
//		{
//			resourceTable = realloc(resourceTable, strlen(resourceTable) + len + 1);
//			strcat(resourceTable, tmp);
//		}
//
//
//		char *newline = resourceTable;
//		size_t newlineCount = 0;
//		for(;*newline!='\0';newline++)
//		{
//			if(*newline=='\n') newlineCount++;
//		}

//		free(tmp);
//		free(resourceTable);
//	}
//	fclose(fptr);
//}
