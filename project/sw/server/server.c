/* 
 *
 * Citing Sources: 
 *
 * https://www.linuxtopia.org/online_books/programming_books/gnu_c_programming_tutorial/argp-example.html 
 * 
 */

#include <stdio.h>
#include "../source/parse_args.h"
#include <arpa/inet.h>  //inet_aton
#include <netinet/in.h> //sockaddr_in
#include "../source/func.h"
#include "../source/headers.h"
#include <sys/time.h>

#ifndef MAX_NUM_OF_FILES
    #define MAX_NUM_OF_FILES 10
#endif

#ifndef MAX_NUM_OF_LINES
    #define MAX_NUM_OF_LINES 2048
#endif

#ifndef RECEIVE_BUFFER_SIZE
    #define RECEIVE_BUFFER_SIZE 8500
#endif



typedef struct received_lines_records {
    unsigned int mask : MAX_NUM_OF_FILES;
} received_lines_records_t;



typedef enum {
    LISTEN_FOR_INIT,
    SEND_INIT_ACK,
    RECEIVE_DATA,
    SEND_FIN_DATA,
    RECEIVE_FIN_ACK,
    SEND_FIN_ACK,
} fsm_state_t;



int main (int argc, char **argv)
{
    received_lines_records_t received_lines_record ;
    received_lines_record.mask = 0;
    received_lines_records_t expected_lines_record ;
    expected_lines_record.mask = 0;

    struct server_arguments arguments;
    
    //argument defaults:
    arguments.source_ip   = "";
    arguments.source_port = "";
    arguments.verbose = 0;
    arguments.debug   = 0;
    
    //Parse the options:
    argp_parse(&server_argp, argc, argv, 0, 0, &arguments);
    
    if (arguments.verbose != 0){ 
        printf ( "\n");
        printf ( "These arguments received:\n");
        printf ( "  --source-ip:    \"%s\"\n",   arguments.source_ip);
        printf ( "  --source-port:  \"%s\"\n", arguments.source_port);
    } 
    
    //Make sure dest ip and dest port are provided via options:
    if (arguments.source_ip == "" || arguments.source_port == "") {
        fprintf(stderr, "Make sure to provide --source-ip and --source-port\nUse --help for more information.\n");
        return -1;
    }  
    
    struct sockaddr_in adr_inet;
    
    //Check the IP Addresses to be 4 ascii octets ("127.0.0.1"):
    if ( !inet_aton(arguments.source_ip, &adr_inet.sin_addr) ) { printf("BAD ADDRESS %s\n", arguments.source_ip); return -1;}
    if ( !inet_aton(arguments.source_port, &adr_inet.sin_port) ) { printf("Bad Port: %s\n", arguments.source_port); return -1;}
    
    if (arguments.debug != 0) {
        printf( "The arguments.source_ip=%s\n",   arguments.source_ip);
        printf( "The arguments.source_port=%d\n\n", atoi(arguments.source_port) );
    }
    
    
    //##################### UDP RECEIVER STARTS HERE:
    
    #define MAX_CHARS_PER_LINE 512
    
    int test;
    struct sockaddr_in rx_local_address; // AF_INET
    struct sockaddr_in rx_from_address; // AF_INET
    
    memset(&rx_local_address, 0, sizeof(rx_local_address));
    
    rx_local_address.sin_family      = AF_INET;
    rx_local_address.sin_addr.s_addr = inet_addr(arguments.source_ip);  
    rx_local_address.sin_port        = htons(atoi(arguments.source_port));
    if (rx_local_address.sin_addr.s_addr == INADDR_NONE) bail("bad source address");
    
    if (arguments.debug != 0) {
        printf( "rx_local_address.sin_family      = AF_INET\n");
        printf( "rx_local_address.sin_addr.s_addr = 0x%08x\n",   rx_local_address.sin_addr.s_addr);
        printf( "rx_local_address.sin_port        = 0x%04x\n\n", rx_local_address.sin_port);
    }
 
    struct timeval timeout={2,0}; //set timeout for 2 seconds
   
    //OPEN A SOCKET AND CATCH THE FD:
    int rx_socket_fd = -1;
    rx_socket_fd = socket(AF_INET,SOCK_DGRAM,0);
    if (rx_socket_fd == -1) bail("Failed to create a socket; see line: rx_socket_fd=socket(AF_INET,SOCKET_DGRAM,0);");

    setsockopt(rx_socket_fd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval)); 
   
    if (arguments.debug != 0) printf("FD returned by socket(): %d\n", rx_socket_fd);
    
    //BIND THE SOCKET TO A GIVEN IP/PORT:
    test = bind( rx_socket_fd,                          \
                 (struct sockaddr *) &rx_local_address, \
                 sizeof(rx_local_address)               );
    if (test == -1) bail("bind()");
    
    //system("netstat -neopa | grep Recv-Q ");
    //system("netstat -neopa | grep dgram ");
    
    char receiveDgramBuffer[MAX_CHARS_PER_LINE]; // Receive Buffer
    

    //OPEN A SOCKET AND CATCH THE FD:
    int tx_socket_fd = -1;
    tx_socket_fd = -1;
    tx_socket_fd = socket(AF_INET,SOCK_DGRAM,0);
    if (tx_socket_fd < 0) { printf("error: failed to open datagram socket\n"); return -1; }

    fsm_state_t STATE = LISTEN_FOR_INIT;

    file_x_app_layer_t * app_layer = NULL;
    int txSockLen = sizeof(rx_from_address);
    char * init_data = NULL;
    char *destination_file_name = NULL;
    file_info_t * file = NULL; // FIXME: create a union of two strucs for app_layer and rename the total_liens to total_files
    uint16_t total_files = 0;
    int current_file = 0;
    int all_done = 0;
    uint16_t current_line=0;
    uint16_t number_of_lines_received=0;
    uint16_t number_of_files_received=0;
    char * file_data;
    FILE *outfile_fd = NULL;
    char * consolidated_file=NULL;
    int file_size=0;
    int client_port = 0;
    

    while(1) {



        //switch(STATE) {
            //case LISTEN_FOR_INIT:

          if (STATE == LISTEN_FOR_INIT) {
                printf("LISTEN_FOR_INIT\n");

                test = recvfrom( rx_socket_fd,                         \
                                 receiveDgramBuffer,                   \
                                 sizeof(receiveDgramBuffer),           \
                                 0,                                    \
                                 (struct sockaddr *) &rx_from_address, \
                                 &txSockLen                            );
                //time out occured or other error:
                if (test < 0) { STATE=LISTEN_FOR_INIT; continue; }

                
                app_layer = (file_x_app_layer_t *) receiveDgramBuffer;

                printf("(*app_layer).init: %d\n",(*app_layer).init);
                printf("(*app_layer).ack: %d\n",(*app_layer).ack);
                printf("(*app_layer).fin: %d\n",(*app_layer).fin);

                //printf("RECEIVE_ACK: (*app_layer).init: %d\n", (*app_layer).init );
                if ( (*app_layer).init == 0) {STATE = LISTEN_FOR_INIT; break;};
                STATE = SEND_INIT_ACK;
                //################ RECEIVE INIT PACKET ###################
                //Allocate the memory to store: the application header + the data following the application header + 1 byte for NULL:
                app_layer = (file_x_app_layer_t *) malloc(test + 1);
                if (app_layer == NULL) {perror("server(): failed to allocate memory"); exit(EXIT_FAILURE);}

                //Copy the header and the data to the allocated memory:
                memcpy(app_layer, receiveDgramBuffer, test);
                if ( (*app_layer).init == 0 ) { printf("Expected INIT packet but got NON-INIT packet!!!\n"); return -1; }
                //Get the data part of the packet containing the file name::
                init_data = (char *)  ( ((char *) app_layer) + sizeof(file_x_app_layer_t) );
                //Terminate the string with null:
                *(init_data + test-sizeof(file_x_app_layer_t) ) = NULL;
                //Point to the destination file's name at the tail of the init packet data:
                destination_file_name = init_data + (MAX_NUM_OF_FILES*2);
                

                //file_info_t file[10];
                file = (file_info_t *) calloc( (*app_layer).total_lines , sizeof(file_info_t) ); // FIXME: create a union of two strucs for app_layer and rename the total_liens to total_files
                if (file == NULL) {perror("server(): failed to allocate memory"); exit(EXIT_FAILURE);}
                total_files =(*app_layer).total_lines;
                client_port =(*app_layer).current_line;



                //Copy the file_ids to each file data structure:
                for (current_file=0; current_file<(*app_layer).total_lines; current_file++){
                    file[current_file].file_id = (uint16_t *) *(init_data+(current_file*2));
                    file[current_file].done = 0;
                    file[current_file].number_of_lines_in_file = 0;
                    file[current_file].received_line_record = NULL;
                    //printf("file[%d].file_id=0x%04x\n",current_file, file[current_file].file_id);
                    //printf("file[%d].file_id=%d\n",current_file, file[current_file].file_id);
                }
                
                printf("\nDestination File:%s\n", destination_file_name );
            
                //printBytes(app_header_n_data, test);
                //printf("(*app_layer): 0x%08x\n",(*app_layer));
                printf("(*app_layer).file_id: %d\n",(*app_layer).file_id);
                printf("(*app_layer).file_number: %d\n",(*app_layer).file_number);
                printf("(*app_layer).current_line: %d\n",(*app_layer).current_line);
                printf("(*app_layer).total_lines: %d\n",(*app_layer).total_lines);
                printf("(*app_layer).init: %d\n",(*app_layer).init);
                printf("(*app_layer).ack: %d\n",(*app_layer).ack);
                //printf("(*app_layer).reserved: %d\n",(*app_layer).reserved);
                
                //free(app_layer);
                app_layer = NULL;
            }
            //case SEND_INIT_ACK:
            if (STATE == SEND_INIT_ACK){
                STATE = RECEIVE_DATA;

                printf("SEND_INIT_ACK\n");

                //################ REPLY TO INIT BEGIN ###################
                app_layer = (file_x_app_layer_t *) calloc(1,test + 1);
                if (app_layer == NULL) {perror("server(): failed to allocate memory"); exit(EXIT_FAILURE);}
                (*app_layer).ack = 1;
                (*app_layer).init = 1;
                
                rx_from_address.sin_port        = htons(client_port);
            
                //Send the init packet:
                test=sendto( tx_socket_fd,                          \
                             app_layer,                             \
                             sizeof(file_x_app_layer_t),            \
                             0,                                     \
                             (struct sockaddr *) &rx_from_address,  \
                             sizeof(rx_from_address)               );
            
                if ( test < 0) printf("Failed to send line.\n");
            
                //free(app_layer);
                free(app_layer);
                app_layer = NULL;
                //################ REPLY TO INIT END ###################
                continue;
            }
            //case RECEIVE_DATA: // and send FIN
            if (STATE == RECEIVE_DATA){
                printf("RECEIVE_DATA\n");



                STATE = SEND_FIN_DATA;

                #ifdef TEST_CLIENT_ABORT
                    STATE = SEND_FIN_ACK;
                #endif

                //################ RECEIVE PACKETS OR REPLY TOi INIT WITH ACK BEGIN ###################
                all_done = 0;
                current_line=0;
                number_of_lines_received=0;
                number_of_files_received=0;

                //while (all_done < (5*10)){
                //while (!all_done){
                while (1){
            

                    test = recvfrom( rx_socket_fd,                         \
                                     receiveDgramBuffer,                   \
                                     sizeof(receiveDgramBuffer),           \
                                     0,                                    \
                                     (struct sockaddr *) &rx_from_address, \
                                     &txSockLen                            );

                    //time out occured or other error:
                    if (test < 0) { STATE=SEND_INIT_ACK; continue; }

                    // get the header:
                    app_layer = (file_x_app_layer_t *) receiveDgramBuffer;

                    printf("(*app_layer).init: %d\n",(*app_layer).init);
                    printf("(*app_layer).ack: %d\n",(*app_layer).ack);
                    printf("(*app_layer).fin: %d\n",(*app_layer).fin);
                    printf("(*app_layer).file_id: %d\n",(*app_layer).file_id);
                    printf("(*app_layer).file_number: %d\n",(*app_layer).file_number);
                    printf("(*app_layer).current_line: %d\n",(*app_layer).current_line);
                    printf("(*app_layer).total_lines: %d\n\n",(*app_layer).total_lines);

                    if ( (*app_layer).fin == 1 || (*app_layer).ack == 1 ) {STATE=SEND_FIN_ACK; break;}
            
                    // make sure the packet is not an init or a fin packet; if init, resend an ACK:
                    if ( (*app_layer).init == 1 ){
                        STATE = SEND_INIT_ACK;
                         printf("Respond with ACK!!!\n"); 


            
                        (*app_layer).ack = 1;
                    
                        //Send the init packet:
                        test=sendto( tx_socket_fd,                          \
                                     app_layer,                             \
                                     sizeof(file_x_app_layer_t),            \
                                     0,                                     \
                                     (struct sockaddr *) &rx_from_address,  \
                                     sizeof(rx_from_address)               );
                        if ( test < 0) printf("Failed to send line.\n");
            
                        continue;
                    }
            
                    //FIXME: make sure to send FIN here and terminate:
                    if ( (*app_layer).fin == 1 && (*app_layer).ack == 1 ) printf("Did not send the FIN yet. Sending FIN and terminating prematurely!!!\n"); 
            
                    //Allocate the memory to store: the application header + the data following the application header + 2 bytes for \n and NULL:
                    //app_layer = (file_x_app_layer_t *) malloc(test + 1);
                    app_layer = (file_x_app_layer_t *) malloc(sizeof(file_x_app_layer_t));
                    if (app_layer == NULL) {perror("server(): failed to allocate memory"); exit(EXIT_FAILURE);}
                    file_data = malloc(test - sizeof(file_x_app_layer_t) + 2);
                    if (file_data == NULL) {perror("server(): failed to allocate memory"); exit(EXIT_FAILURE);}
                    //append new line character that was stripped right before the string was transmitted by the sender:
                    *(file_data + test - sizeof(file_x_app_layer_t)) = '\n';
                    //terminate the string with NULL:
                    *(file_data + test - sizeof(file_x_app_layer_t) + 1) = '\0';
                    //Copy the header and the data to the allocated memory:
                    memcpy(app_layer, receiveDgramBuffer, sizeof(file_x_app_layer_t));
                    memcpy(file_data, receiveDgramBuffer+sizeof(file_x_app_layer_t) , test-sizeof(file_x_app_layer_t) );
            
                    //Check if a packet corresponding to this file was already received:
                    if (file[(*app_layer).file_number].received_line_record == NULL){
                        file[(*app_layer).file_number].received_line_record = (char *) calloc( (*app_layer).total_lines, sizeof(char));
                        if (file[(*app_layer).file_number].received_line_record == NULL) {perror("server(): failed to allocate memory"); exit(EXIT_FAILURE);}
                        file[(*app_layer).file_number].number_of_lines_in_file = (*app_layer).total_lines;
                    }
            
                    //Check if this file's line has already been received (this packet is a duplicate):
                    if ( *( file[(*app_layer).file_number].received_line_record + (*app_layer).current_line ) == 1 ){
                        printf("GOT DUPLICATE\n");
                        continue;
                    }
            
                    //Get the data part of the packet containing the file name::
                    file[(*app_layer).file_number].text_line[(*app_layer).current_line] = file_data;
            
                    //Mark the record to indicate that this line of this file is received:
                    *(file[(*app_layer).file_number].received_line_record + (*app_layer).current_line) = 1;
            
                    //Check if all lines received for this file:
                    for (current_line=0, number_of_lines_received=0; current_line < file[(*app_layer).file_number].number_of_lines_in_file; current_line++) {
                        
                        if ( *(file[(*app_layer).file_number].received_line_record + current_line) == 1 )  number_of_lines_received++;
                    }
                    if ( number_of_lines_received == file[(*app_layer).file_number].number_of_lines_in_file ) file[(*app_layer).file_number].done = 1;
                    
                    /*
                    printf("\n\n\n(*app_layer): 0x%08x\n",(*app_layer));
                    printf("(*app_layer).file_id: %d\n",(*app_layer).file_id);
                    printf("(*app_layer).file_number: %d\n",(*app_layer).file_number);
                    printf("(*app_layer).current_line: %d\n",(*app_layer).current_line);
                    printf("(*app_layer).total_lines: %d\n",(*app_layer).total_lines);
                    printf("(*app_layer).init: %d\n",(*app_layer).init);
                    printf("(*app_layer).ack: %d\n",(*app_layer).ack);
                    printf("(*app_layer).reserved: %d\n",(*app_layer).reserved);
                    printf("file[%d].line[%d]: %s", (*app_layer).file_number, (*app_layer).current_line, \
                                                    file[(*app_layer).file_number].text_line[(*app_layer).current_line] );
                    */
            
                    // Check if data transfer is complete:
                    for (current_file=0, number_of_files_received=0; current_file<total_files; current_file++){
                        if ( file[current_file].done == 1 )  number_of_files_received++;
                    }
                    if (number_of_files_received == total_files){
                        //printf("received files: %d\n",number_of_files_received);
                        //printf("total_files: %d\n",total_files);
                        break;
                    }
            
                }
            
                //##################### OPEN FILE BEGIN: ##########################


                if ( (outfile_fd=fopen(destination_file_name,"w")) == NULL ) {
                    printf(stderr, "Unable to open file %s; Use --input-file option, and make sure the file is present.\n", destination_file_name); 
                    return -1; }
                else  {
                    fflush(outfile_fd);
                    for(current_file=0;current_file<10;current_file++) {
                        for(current_line=0;current_line<file[current_file].number_of_lines_in_file;current_line++) 
                            fputs( file[current_file].text_line[current_line], outfile_fd);
                            //fputs( file[0].text_line[0], outfile_fd);
            
                            //if (test != 0) return -1;
                    }
                    fclose(outfile_fd);
                }  
            }

            //case SEND_FIN_DATA:
            if (STATE ==  SEND_FIN_DATA) {
                STATE = RECEIVE_FIN_ACK;
                printf("SEND_FIN_DATA\n");
                //########FIXME: send FIN with the content of destination_file_name!
            
                app_layer = malloc( sizeof(file_x_app_layer_t) );
                if (app_layer == NULL) {perror("server(): failed to allocate memory"); exit(EXIT_FAILURE);}
                //(*app_layer).file_id = file[current_file].file_id ;
                (*app_layer).file_number = 0;
                (*app_layer).current_line = 0;
                (*app_layer).total_lines = file[current_file].number_of_lines_in_file;
                (*app_layer).init = 0;
                (*app_layer).ack = 0;
                (*app_layer).fin = 1;
                (*app_layer).reserved = 0;
            
                /*
                printf("\n\napp_layer: 0x%08x\n",(*app_layer));
                printf("(*app_layer).file_id: %d\n",(*app_layer).file_id);
                printf("(*app_layer).file_number: %d\n",(*app_layer).file_number);
                printf("(*app_layer).current_line: %d\n",(*app_layer).current_line);
                printf("(*app_layer).total_lines: %d\n",(*app_layer).total_lines);
                printf("(*app_layer).init: %d\n",(*app_layer).init);
                printf("(*app_layer).ack: %d\n",(*app_layer).ack);
                printf("(*app_layer).fin: %d\n",(*app_layer).fin);
                printf("(*app_layer).reserved: %d\n",(*app_layer).reserved);
                */
            
                //char * consolidated_file=NULL;
                consolidated_file=(char *) app_layer;
                file_size=sizeof(file_x_app_layer_t);
                //printf("app size=%d\n", sizeof(file_x_app_layer_t) );
                
                for(current_file=0;current_file<10;current_file++) {
                    for(current_line=0;current_line<file[current_file].number_of_lines_in_file;current_line++) {
                        file_size = concat_bytes_append(consolidated_file, \
                                                        file_size, \
                                                        file[current_file].text_line[current_line], \
                                                        strlen(file[current_file].text_line[current_line]));
                        //printf("file_size=%d\n", file_size);
                        //printf("ALL IN ONE(size=%d)--%s", strlen(file[current_file].text_line[current_line]), file[current_file].text_line[current_line] );
                        //printf("strlen(file[current_file].text_line[current_line])=%d\n\n", strlen(file[current_file].text_line[current_line]) );
                 
                    }
                } 
                rx_from_address.sin_port        = htons(client_port);
                test=sendto(tx_socket_fd,                          \
                            consolidated_file,                     \
                            file_size,                             \
                            0,                                     \
                            (struct sockaddr *) &rx_from_address,  \
                            sizeof(rx_from_address)               );
                if ( test < 0) printf("Failed to send line.\n");
                continue;
            }


           //case RECEIVE_FIN_ACK:
            if (STATE == RECEIVE_FIN_ACK) {
                printf("RECEIVE_FIN_ACK\n");

                test = recvfrom( rx_socket_fd,                         \
                                 receiveDgramBuffer,                   \
                                 sizeof(receiveDgramBuffer),           \
                                 0,                                    \
                                 (struct sockaddr *) &rx_from_address, \
                                 &txSockLen                            );

                //time out occured or other error:
                if (test < 0) { STATE=SEND_FIN_DATA; continue; }

                app_layer = (file_x_app_layer_t *) receiveDgramBuffer;

                    printf("(*app_layer).init: %d\n",(*app_layer).init);
                    printf("(*app_layer).ack: %d\n",(*app_layer).ack);
                    printf("(*app_layer).fin: %d\n",(*app_layer).fin);

                STATE = SEND_FIN_DATA;
                if ( (*app_layer).ack == 1 || (*app_layer).fin == 1) STATE = SEND_FIN_ACK; 
            }
           //case SEND_FIN_ACK:
            if (STATE == SEND_FIN_ACK) {
                printf("SEND_FIN_ACK\n");

                //printf("(*app_layer).ack: %d\n",(*app_layer).ack);
                //printf("(*app_layer).fin: %d\n",(*app_layer).fin);
                
                app_layer = (file_x_app_layer_t *) malloc(sizeof(file_x_app_layer_t));
                if (app_layer == NULL) {perror("server(): failed to allocate memory"); exit(EXIT_FAILURE);}
                (*app_layer).fin = 1;
                (*app_layer).ack = 1;
                rx_from_address.sin_port        = htons(client_port);
                test=sendto(tx_socket_fd,                          \
                                app_layer,                             \
                                sizeof(file_x_app_layer_t),            \
                                0,                                     \
                                (struct sockaddr *) &rx_from_address,  \
                                sizeof(rx_from_address)               );
                if ( test < 1) {printf("Failed to send line.\n");  return -1;}
                printf("DONE\n");

                return 0;
            }



    }


    return 0;
}
