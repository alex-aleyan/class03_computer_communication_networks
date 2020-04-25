// if ack is unset, indicates lines to be tx'd or line tag of tx'd line, if ack set indicates the line the server is requesting
#ifndef MAX_NUM_OF_LINES
    #define MAX_NUM_OF_LINES 1024
#endif

typedef struct file_x_app_layer{
    unsigned int file_id       : 16;
    unsigned int file_number   : 8;
    unsigned int current_line  : 16; 
    unsigned int total_lines   : 16;
    unsigned int ack           : 1; 
    unsigned int init          : 1; 
    unsigned int fin           : 1; 
    unsigned int reserved      : 5;
} file_x_app_layer_t;

typedef struct file_info{
    unsigned int file_id                 : 16;
    unsigned int file_number             : 8;
    unsigned int number_of_lines_in_file : 16;
    char * received_line_record;
    char done;
    //char * text_line[]; //<-- won't work; must know the array size at instantation.
    char * text_line[MAX_NUM_OF_LINES];
} file_info_t;
