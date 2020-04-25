/* 
 *
 * Citing Sources: 
 *
 * https://www.linuxtopia.org/online_books/programming_books/gnu_c_programming_tutorial/argp-example.html 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include "../source/func.h"

int getLinesPerFile(FILE *fp){
    int line_count = 0;
    char current_char;

    rewind(fp);
    
    for (current_char = getc(fp); current_char != EOF; current_char = getc(fp))	
    if (current_char == '\n') line_count++;
    
    printf("\ngetLinesPerFile: %d\n\n", line_count);
    
    rewind(fp);
    
    return line_count;
}

char ** getData(FILE *fd, int * number_of_lines, int debug, char * line_ptr2[])
{

    // Get number of lines:
    char current_char;
    int line_count=0;
    for (current_char = getc(fd), line_count=0; current_char != EOF; current_char = getc(fd))  if (current_char == '\n') line_count++;
    if (debug != 0) printf("\nNumber of lines (line_count): %d\n\n", line_count);
    if (line_count == 0){ perror("Error: file-in is empty!\n\n"); return -1; };
    (*number_of_lines)=line_count;

    char * line_ptr[line_count];
  
    rewind(fd);

    // get number of chars per line:
    int char_count=0;
    int current_line=0;
    for (current_char = getc(fd), current_line=0; current_char != EOF; current_char = getc(fd))
    {
        char_count++;
        if (current_char == '\n')
        {
            // allocate just enough memory to hold the characters of current line:
            if (debug != 0) {
            printf("Number of char per line %d: %d\n", current_line, char_count);
            printf("Allocating: line=%d; bytes=%d: %d\n", current_line, char_count*sizeof(char)+1);
            }
  
            line_ptr[current_line] = malloc(char_count*sizeof(char)+1);
            if (line_ptr[current_line] == NULL) {
                perror("getData(): failed to allocate memory"); 
                exit(EXIT_FAILURE);}
            line_ptr2[current_line] = line_ptr[current_line];
            if (debug != 0) printf("Line=%d stored at 0x%x\n\n", current_line, line_ptr[current_line]);
  
            char_count = 0;
            current_line++;
        }
    }
  
  	rewind(fd);
  
    printf("\n");


  
    //copy file content to allocated memory line by line one charachter at the time:
    for (current_char = getc(fd), current_line=0, char_count;
         current_char != EOF;
         current_char = getc(fd)) 
    {
       if (debug != 0) if (char_count == 0) printf("Line(%d):", current_line);
  
        *(line_ptr[current_line]+char_count)=current_char;
        char_count++;
        if (current_char == '\n')
        {
            //terminate with NULL:
            *(line_ptr[current_line]+char_count)='\0';
            if (debug != 0) printf("%s", line_ptr[current_line]);
            char_count=0;
            current_line++;
        }
    }
  
    printf("\n");

    rewind(fd);

    return line_ptr;
}


int getFileInfo( file_info_t *file, char * file_name, uint8_t current_file, int debug)
{
    int NUM_OF_LINES;
    uint16_t current_line=0;


    (*file).file_id = (packet_id() | current_file);
    (*file).file_number = current_file;
    FILE *fd = NULL;

    fd=fopen(file_name,"r");
    if (fd == NULL) {
        fprintf(stderr, "Unable to open file:%s\nUse --input-file option, and make sure the file is present.\n", file_name); 
        return -1; 
    }
    
    (*file).number_of_lines_in_file = getLinesPerFile(fd);
    if (!(*file).number_of_lines_in_file) {printf("Error:%s file is empty!\n", file_name); exit(EXIT_FAILURE);}
    if (!(*file).number_of_lines_in_file > MAX_NUM_OF_LINES) {printf("Error:%s has too many line; modify MAX_NUM_OF_LINES and recompile!\n", file_name); exit(EXIT_FAILURE);}
    getData(fd, &NUM_OF_LINES, 1, (*file).text_line);
    (*file).number_of_lines_in_file = NUM_OF_LINES;
    
    if (debug != 0) {
        printf("(*file).file_id:                 0x%04x\n", (*file).file_id);
        printf("(*file).number_of_lines_in_file: 0x%04x\n", (*file).number_of_lines_in_file);
        for(current_line=0; current_line<(*file).number_of_lines_in_file; current_line++){
            printf("(*file).text_line[%d]:            %s",  current_line, (*file).text_line[current_line]);
        }   
    } 
    close(fd);
    return 0;
}
