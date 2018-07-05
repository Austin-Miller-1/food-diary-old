#include <math.h>
#include <stdio.h>	//For printing
#include <stdlib.h>	//For exit
#include <sys/types.h> 	//For opendir, closedir
#include <dirent.h>	//For opendir, closedir and readdir
#include <string.h>	//For string literals & string functions
#include <errno.h>	//For errno
#include <sys/ioctl.h>	//For access to terminal's size

void list_dir(char *dirName, int printHidden, int sortedL, int sortedLR);	//Prints directory entries. Takes name of dir, flag for hidden files, flag for sorting lexographically and flag for printing  reverse lexographically
void print_row(struct dirent* entries[], int entryNameLengths[], int colNum, int entriesPerCol, int startIndex, int isReverse, int maxNameLength, int totalEntries, int rowLength);	//Prints each row of dir-entries. Uses pointers to entries, pointers to string lengths, num of cols and num of elements per col
int entry_LexCompare(const void* ent1, const void* ent2);			//Compares the names of two directory entries. Is used in qsort to sort entries.
int get_entry_name_length(struct dirent* entry);	//Returns the length of the entrie's name
int get_terminal_width();	//Gets the width of the terminal's window


int main(int ac, char* args[]){
	char *dirName;	//Points to name of directory
	int a, s, r;	//Flags for the switches. a -> print hidden files, s -> print in lexographic order, r -> print in reverse lexographic order
	a = s = r = 0;
	int dirCount = ac-1;	//Number of directories to print. Since first arg is command name, there are (at most) ac-1 dir to list.

	//Check if the user included switches. If they did, it would have to be in the second argument.
	if(ac>1 && args[1][0] == '-'){
		dirCount--; 	//If first argument is for switches, remove it from dirCount.

		//Set the switch flags. Go through the switch string char by char.
		char currentChar;
		int charCount = 1;
		while( (currentChar = args[1][charCount++]) != '\0' ){
			switch(currentChar){
				case 'a': a = 1; break;
				case 's': s = 1; break;
				case 'r': r = 1; break;
				default: break;
			}
		}
	}

	//If no directories were given by user, list the current directory.
	//Otherwise, go through the argument array and list each directory that is given.
	if(dirCount == 0){ list_dir(".", a, s, r); }
	else{
		while( dirCount-- ){ list_dir( args[ac-1-dirCount], a, s, r); if(dirCount>0){ printf("\n");} } //For each directory, find the corresponding path in args. Print the directory.
	}
	return 0;
}

//Prints a directory's entries. Takes a directory's name, and flags for printing hidden files, printing in lex. order and printing in reverse lex. order.
/*
	Function opens directory, then reads each entry. Function saves pointers to each dirent struct in array so that they can be sorted.
	Since the entries will be printed in columns, and this amount depends on the name lengths, the lengths of each entry name will also be
	kept in an array.
	After adding all entries to the array, the entries are sorted if proper flags are given.
	After sorting, determine the max number of columns that can be used.
	This depends on the number of entries, their individual name sizes and the terminal window size.
	Then, print the name of each entry in the sorted/nonsorted order in columns.
*/
void list_dir(char* dirName, int printHidden, int sortedL, int sortedLR){
	DIR *dir_ptr;				//The directory

	struct dirent *entry_ptr = NULL;	//Each entry in the directory
	int entryTotal = 0;			//Total number of entries

	int const MAX_ENTRIES = 150;		//Max number of directory entries.
	struct dirent* entryList[MAX_ENTRIES];	//Array that will hold each directory entry. Will be sorted if proper flags are active.
	int entNameLengths[MAX_ENTRIES];	//Array that will hold the lengths of each directory entry name

	int maxNameLength = -1;
	int currentNameLength = -1;
	int colNum = -1;	//Will be the number of columns to print entries over.
	int terminalWidth = get_terminal_width(); //Width of terminal window
	int entriesPerCol = -1; //The number of entries per column
	int currentEnt = 0;

	//Open directory. If directory fails to open (if opendir returns null), print error, and exit program.
	// Open each entry. If entry is a hidden file and hidden files should be ignored, do not add to array.
	//Otherwise, read all entries in directory, place dirent struct pointers in array, sort (if wanted), and print
	//Also keep track of each entries' name-lengths and the max name length.
	if((dir_ptr = opendir(dirName)) == NULL ){ fprintf(stderr, "Could not open directory '%s': %s\n", dirName, strerror(errno)); exit(EXIT_FAILURE); }
	else{
		while( (entry_ptr = readdir(dir_ptr) ) != NULL ){
			if(printHidden || entry_ptr->d_name[0]!='.'){
				//currentNameLength = get_entry_name_length(entry_ptr);
				//entNameLengths[entryTotal] = currentNameLength;
				//if(currentNameLength > maxNameLength){ maxNameLength = currentNameLength;}
				entryList[entryTotal++] = entry_ptr; //Get entry, add to list. Continue until no entries remain. Keep track of number of entries
			}
		}


		//Sort if sort flags are active.
		if(sortedL || sortedLR){ qsort(entryList, entryTotal, sizeof(struct dirent *), entry_LexCompare); }

		//Get lengths of each and put in array
		for(currentEnt = 0; currentEnt < entryTotal; currentEnt++){
			entry_ptr = entryList[currentEnt];
			currentNameLength = get_entry_name_length(entry_ptr);
			entNameLengths[currentEnt] = currentNameLength;
			if(currentNameLength > maxNameLength){ maxNameLength = currentNameLength;}

		}
		//Get column number	and number of entries per column.
		colNum = terminalWidth / (maxNameLength+2);
		if(colNum <= 0) colNum = 1;

		entriesPerCol = (int) ceil(entryTotal / (double) colNum);

		//printf("%d %d %d %d", terminalWidth, maxNameLength, colNum, entriesPerCol);
		//Print entries.
		int currentRow; int spaceCount;
		struct dirent* currentEntry = NULL;
		int entryInRow;
		int index; int indexAdditive; int nameLengthDif;
		char row[terminalWidth+10];
		int newLength = 0;
		if(sortedLR){	//Print in reverse lexographic order (use lexographic ordered array, but print from the bottom to the top)
			for(currentRow = 0; currentRow < entriesPerCol; currentRow++){
					row[0] = '\0';
					for( entryInRow = 0; entryInRow < colNum; entryInRow++ ){
						indexAdditive = entryInRow*entriesPerCol;
						index = entryTotal - 1 - currentRow - indexAdditive;
						//printf("%d %d %d \n", currentRow, index, indexAdditive);
						if(index >= 0){
							currentEntry = entryList[index];
							strcat(row, currentEntry->d_name);

							nameLengthDif = maxNameLength - entNameLengths[index];
							for(spaceCount = 0; spaceCount < nameLengthDif; spaceCount++){strcat(row," ");}
							strcat(row,"  ");
						}
					}
					printf("%s\n", row);
				//print_row(entryList, entNameLengths, colNum, entriesPerCol, currentRow, 1, maxNameLength, entryTotal, terminalWidth+20);
			}
		}
		else{		//Print in lexographic order
			for(currentRow = 0; currentRow < entriesPerCol; currentRow++){
				row[0] = '\0';
				for( entryInRow = 0; entryInRow < colNum; entryInRow++ ){
					indexAdditive = entryInRow*entriesPerCol;
					index = currentRow + indexAdditive;

					if(index < entryTotal){
						currentEntry = entryList[index];
						strcat(row, currentEntry->d_name);
						//newLength = strlen(currentEntry->d_name);

						nameLengthDif = maxNameLength - entNameLengths[index];
						for(spaceCount = 0; spaceCount < nameLengthDif; spaceCount++){strcat(row," ");}
						strcat(row,"  ");
						//newLength += nameLengthDif;
					}
					//printf("name: %s, col length: %d, calculated new length: %d\n", currentEntry->d_name, maxNameLength, newLength);
				}
				printf("%s\n", row);
				//print_row(entryList, entNameLengths, colNum, entriesPerCol, currentRow, 0, maxNameLength, entryTotal, terminalWidth+20);
			}
		}
		closedir(dir_ptr);	//Close directory
	}
}
	//Prints row of dir entries//Prints each row of dir-entries. Uses pointers to entries, pointers to string lengths, num of cols and num of elements per col
	void print_row(struct dirent* entries[], int entryNameLengths[], int colNum, int entriesPerCol, int startIndex, int isReverse, int maxNameLength, int totalEntries, int rowLength){
		int entryInRow;
		int index;
		int nameLengthDif;
		struct dirent* currentEntry;
		char* currentEntryName;
		int spaceCount;
		int indexAdditive = 0;

		char* SPACE = " ";
		char* TWO_SPACE = "  ";
		char row[rowLength];
		row[0] = ' ';
		int stringLength = 0;

		for( entryInRow = 0; entryInRow < colNum; entryInRow++ ){
			indexAdditive = entryInRow*entriesPerCol;
			if(isReverse){indexAdditive *= -1;}
			index = startIndex + indexAdditive;
			printf("%d %d %d %d\n", startIndex, index, indexAdditive, isReverse);

			if(index < totalEntries-1 && index > 0){
				currentEntry = entries[index];

			//	stringLength += strlen(currentEntry->d_name);
				strcat(row, currentEntry->d_name);

				nameLengthDif = maxNameLength - entryNameLengths[index];
				for(spaceCount = 0; spaceCount < nameLengthDif; spaceCount++){strcat(row,SPACE);}
				strcat(row,TWO_SPACE);
				//printf("%d %d\n", rowLength, stringLength);
				//stringLength += nameLengthDif * strlen(SPACE);
				//printf("%d %d\n", rowLength, stringLength);
				//stringLength += strlen(TWO_SPACE);
				//printf("%d %d\n", rowLength, stringLength);
			}
		}

		//printf("%s\n", row);
	}

	//Compare two directory entries (two dirent structs). This function is used in qsort, so it takes two void pointers to the two dirent stuct pointers.
	//	Function converts the void pointers to pointers to dirent struct pointers. Then, it compares the names of these structs lexographically
	//	using strcmp.
	int entry_LexCompare(const void * ent1, const void * ent2){
		const struct dirent ** entry1 = (const struct dirent **) ent1;
		const struct dirent ** entry2 = (const struct dirent **) ent2;
		return strcmp( (*entry1)->d_name, (*entry2)->d_name);
	}

	//Gets the length of a directory-entry's name
	int get_entry_name_length(struct dirent *entry){	return strlen(entry->d_name); }

	//Get the width of the terminal window (in number of columns/characters). Exit if could not access.
	int get_terminal_width(){
		struct winsize window;

		if( ioctl(0, TIOCGWINSZ, &window) == -1){ perror("Terminal window size could not be accessed!"); exit(EXIT_FAILURE); }
		else{	return window.ws_col; }
	}
