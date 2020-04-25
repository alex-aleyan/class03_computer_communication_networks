/* 
 *
 * Citing Sources: 
 *
 * https://www.linuxtopia.org/online_books/programming_books/gnu_c_programming_tutorial/argp-example.html 
 * 
 */

#include <stdio.h>
#include <arpa/inet.h>  //inet_aton
#include <netinet/in.h> //sockaddr_in
#include "../source/parse_args.h"
//#include "../source/func.h"
#include "../source/headers.h"
#include "../source/readfile.h"
#include "../source/packet_id.h"
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

#define member_size(type, member) sizeof(((type *)0)->member)

typedef enum {
    SEND_INIT,
    RECEIVE_INIT_ACK,
    SEND_DATA,
    RECEIVE_FIN_DATA,
    SEND_FIN_ACK,
    RECIEVE_FIN_ACK,
} fsm_state_t;

int main (int argc, char **argv)
{
  
    int test;

    //##################### OPARSE ARGS STARTS HERE:##################################
  
    struct arguments arguments;
  
    /* Set argument defaults */
    arguments.outfile = NULL;
    arguments.dest_ip = "";
    arguments.dest_port = "";
    arguments.verbose = 0;
    arguments.debug   = 0;
    int arg_i = 0 ; for (arg_i=0; arg_i<MAX_NUM_OF_FILES; arg_i++)  arguments.args[arg_i]=NULL;

  
    // Parse the Arguments:
    argp_parse (&argp, argc, argv, 0, 0, &arguments);
  
  
    if (arguments.verbose != 0){
        printf ( "\n");
        printf ( "These arguments received:\n");
        printf ( "  --dest-ip:    \"%s\"\n",   arguments.dest_ip);
        printf ( "  --dest-port:  \"%s\"\n", arguments.dest_port);
        printf ( "  --output-file: \"%s\"\n\n", arguments.outfile);
        //int arg_i = 0 ;
        for (arg_i=0; arg_i<MAX_NUM_OF_FILES; arg_i++)  printf ( "arguments.args[%d]:%s\n", arg_i, arguments.args[arg_i]);
    }

    //Make sure dest ip and dest port are provided via options:
    if (arguments.dest_ip == "" || arguments.dest_port == "" || arguments.args[0] == NULL || arguments.outfile == NULL) {
        fprintf(stderr, "Make sure to provide: \n\t--dest-ip\n\t--dest-port\n\t--input-file\n\t--output-file\n\nUse --help for more information.\n"); 
        return -1;
    } 
  
    struct sockaddr_in adr_inet;
  
    //Check the IP Addresses to be 4 ascii octets ("127.0.0.1"):
    if ( !inet_aton(arguments.dest_ip, &adr_inet.sin_addr) ) { printf("Bad IP Address %s\n", arguments.dest_ip); return -1;}
    if ( !inet_aton(arguments.dest_port, &adr_inet.sin_port) ) { printf("Bad Port: %s\n", arguments.dest_port); return -1;}

    if (arguments.debug != 0) {
        printf( "The arguments.dest_ip=%s\n",   arguments.dest_ip);
        printf( "The arguments.dest_port=%d\n\n", atoi(arguments.dest_port) );
    }


  
    //##################### OPEN FILE STARTS HERE:##################################
    
    int16_t current_file;
    int16_t current_line=0;
    char ** text_line;
    file_info_t file[MAX_NUM_OF_FILES];


    for (current_file=0; current_file<MAX_NUM_OF_FILES; current_file++){ 
        getFileInfo( &file[current_file], arguments.args[current_file], current_file, 1);
    }

    /*
    printf("#############\n##############\n##############");
    for (current_file=0; current_file<MAX_NUM_OF_FILES; current_file++){ 
        printf("file[%d].file_id:                 0x%04x\n", current_file, file[current_file].file_id);
        printf("file[%d].number_of_lines_in_file: 0x%04x\n", current_file, file[current_file].number_of_lines_in_file);
        for(current_line=0; current_line<file[current_file].number_of_lines_in_file; current_line++){
            printf("file[%d].text_line[%d]:            %s", current_file, current_line, file[current_file].text_line[current_line]);
            }

    }

    */
    //##################### CLOSE FILE STARTS HERE:##################################
    

    
    //##################### OPEN UDP SENDER BEGIN:##################################

    struct sockaddr_in tx_to_address;

    memset(&tx_to_address, 0, sizeof(tx_to_address));
  
    tx_to_address.sin_family      = AF_INET; // Receiving Socket Family
    tx_to_address.sin_addr.s_addr = inet_addr(arguments.dest_ip);// Receiving Socket IP Address
    tx_to_address.sin_port        = htons(atoi(arguments.dest_port)); // Receiving Socket Port Number

    if (arguments.debug != 0) {
        printf( "tx_to_address.sin_family      = AF_INET\n");
        printf( "tx_to_address.sin_addr.s_addr = 0x%08x\n", tx_to_address.sin_addr.s_addr);
        printf( "tx_to_address.sin_port        = 0x%04x\n\n", tx_to_address.sin_port);      }

    //OPEN A SOCKET AND CATCH THE FD:
    int tx_socket_fd = -1;
    tx_socket_fd     = socket(AF_INET,SOCK_DGRAM,0); 
    if (tx_socket_fd < 0) {perror("error: failed to open datagram socket\n"); exit(1); }
  
    //for(current_line=0; current_line<file[0].number_of_lines_in_file; current_line++) printf("%s",file[0].text_line[current_line]);
    for (current_file=0; current_file<MAX_NUM_OF_FILES; current_file++){ 
        printf("file[%d].file_id:                 0x%04x\n", current_file, file[current_file].file_id);
        printf("file[%d].number_of_lines_in_file: 0x%04x\n", current_file, file[current_file].number_of_lines_in_file);
        for(current_line=0; current_line<file[current_file].number_of_lines_in_file; current_line++){
            printf("file[%d].text_line[%d]:            %s", current_file, current_line, file[current_file].text_line[current_line]);  }

    }
    //##################### CLOSE UDP SENDER BEGIN:##################################


    
    //##################### OPEN UDP RECEIVER BEGIN:##################################
    char receiveDgramBuffer[RECEIVE_BUFFER_SIZE]; // Receive Buffer

    struct sockaddr_in rx_from_address; // AF_INET
    int rxSockLen = sizeof(rx_from_address);

    struct timeval timeout={2,0}; //set timeout for 2 seconds

    //RX SOCKET:
    struct sockaddr_in rx_local_address; // AF_INET
    memset(&rx_local_address, 0, sizeof(rx_local_address));
    rx_local_address.sin_family      = AF_INET;
    rx_local_address.sin_addr.s_addr = inet_addr(arguments.source_ip);
    rx_local_address.sin_port        = htons(atoi(arguments.source_port));
    if (rx_local_address.sin_addr.s_addr == INADDR_NONE) bail("bad source address");

    //OPEN A SOCKET AND CATCH THE FD:
    int rx_socket_fd = -1; 
    rx_socket_fd = socket(AF_INET,SOCK_DGRAM,0);
    if (rx_socket_fd == -1) bail("Failed to create a socket; see line: rx_socket_fd=socket(AF_INET,SOCKET_DGRAM,0);");

    setsockopt(rx_socket_fd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));

    test = bind( rx_socket_fd,                          \
                 (struct sockaddr *) &rx_local_address, \
                 sizeof(rx_local_address)               );
    if (test == -1) bail("bind()");
    //##################### CLOSE UDP RECEIVER BEGIN:##################################

    
        
    fsm_state_t STATE = SEND_INIT;
    char *app_header_n_data=NULL;
    FILE *outfile_fd = NULL;
    int current_char;
    file_x_app_layer_t * app_layer;
    app_layer = calloc(1, sizeof(file_x_app_layer_t) );
    if (app_layer == NULL) {perror("client(): failed to allocate memory"); exit(EXIT_FAILURE);}

    while(1) {
        //switch(STATE) {
            //case SEND_INIT:
            if (STATE ==  SEND_INIT){
                printf("SEND_INIT\n");
        
                STATE=RECEIVE_INIT_ACK;
                
                //##################### SEND INIT PACKET BEGIN:##################################
                // Build init packet:
                app_layer = calloc(1, sizeof(file_x_app_layer_t) );
                if (app_layer == NULL) {perror("client(): failed to allocate memory"); exit(EXIT_FAILURE);}

                (*app_layer).file_id = 0;
                (*app_layer).current_line = 0;
                (*app_layer).current_line = atoi(arguments.source_port);
                (*app_layer).total_lines = MAX_NUM_OF_FILES;
                (*app_layer).ack = 0;
                (*app_layer).init = 1;
                (*app_layer).fin = 0;
                (*app_layer).reserved = 0;
            
                //print the applicatin header:
                /*
                printf("SEND_INIT: \n\napp_layer: 0x%08x\n",(*app_layer));
                printf("SEND_INIT: app_layer.file_id: %d\n",(*app_layer).file_id);
                printf("SEND_INIT: (*app_layer).current_line: %d\n",(*app_layer).current_line);
                printf("SEND_INIT: (*app_layer).total_lines: %d\n",(*app_layer).total_lines);
                printf("SEND_INIT: (*app_layer).init: %d\n",(*app_layer).init);
                printf("SEND_INIT: (*app_layer).ack: %d\n",(*app_layer).ack);
                printf("SEND_INIT: (*app_layer).reserved: %d\n",(*app_layer).reserved);
                */
            
                //Allocate the memory to store the MAX_NUM_OF_FILES file ids with the destination file's name appended at the end:
                char * init_packet_payload = calloc(1, (MAX_NUM_OF_FILES*2) + strlen(arguments.outfile));
                if (init_packet_payload == NULL) {perror("client(): failed to allocate memory"); exit(EXIT_FAILURE);}
            
                //put all file_ids at the head of the allocated memory:
                for (current_file=0; current_file<MAX_NUM_OF_FILES; current_file++)
                    *(init_packet_payload+(current_file*2)) = file[current_file].file_id;
            
                //place the destination file's name at the tail of the allocated memory:   
                memcpy( (init_packet_payload+(MAX_NUM_OF_FILES*2)) , arguments.outfile , strlen(arguments.outfile) );
                
                //Concatinate the Application Header with the Payload:
                char * init_payload = concat_bytes_alloc(app_layer, sizeof(file_x_app_layer_t), init_packet_payload, (MAX_NUM_OF_FILES*2)+strlen(arguments.outfile) );
                if (init_payload == NULL) {perror("client(): failed to allocate memory"); exit(EXIT_FAILURE);}
            
                free(init_packet_payload);
                free(app_layer);
            
                //printBytes(init_payload, sizeof(file_x_app_layer_t) + strlen(arguments.outfile) + (2*MAX_NUM_OF_FILES) );
                //printf("SEND_INIT: NULL: %02x %02x \n", NULL, '\0');
                
                //Send the init packet:
                test=sendto( tx_socket_fd,                                                \
                             init_payload,                                                \
                             sizeof(file_x_app_layer_t)+strlen(arguments.outfile)+(MAX_NUM_OF_FILES*2), \
                             0,                                                           \
                             (struct sockaddr *) &tx_to_address,                          \
                             sizeof(tx_to_address)                                        );
            
                if ( test < 0) printf("SEND_INIT: Failed to send line.\n");
            
                free(init_payload);
            
                //##################### SEND INIT PACKET END:##################################
                //
            }
            //case RECEIVE_INIT_ACK:
            if (STATE ==  RECEIVE_INIT_ACK){
                printf("RECEIVE_INIT_ACK\n");

                test = recvfrom( rx_socket_fd,                             \
                         receiveDgramBuffer,                   \
                         sizeof(receiveDgramBuffer),           \
                         0,                                    \
                         (struct sockaddr *) &rx_from_address, \
                         &rxSockLen                            );

                // time out occured or other error:
                if (test < 0) { STATE=SEND_INIT; continue; }

                app_layer = (file_x_app_layer_t *) receiveDgramBuffer;

                printf("(*app_layer).fin: %d\n",(*app_layer).fin);
                printf("(*app_layer).init: %d\n",(*app_layer).init);
                printf("(*app_layer).ack: %d\n",(*app_layer).ack);
                STATE = SEND_DATA;

                #ifdef TEST_SERVER_ABORT
                    STATE = SEND_FIN_ACK;
                #endif

                if ( (*app_layer).ack != 1 || (*app_layer).init != 1 ) { 
                    STATE = SEND_INIT; 
                    continue;
                }

            }

            //case SEND_DATA:
            if (STATE ==  SEND_DATA){
                printf("SEND_DATA\n");
                STATE = RECEIVE_FIN_DATA;

                //##################### SEND DATA BEGIN:##################################
                //Send data:
                //    int test;

            
                app_layer = calloc(1, sizeof(file_x_app_layer_t) );
                if (app_layer == NULL) {perror("client(): failed to allocate memory"); exit(EXIT_FAILURE);}
                (*app_layer).init = 0;
                (*app_layer).ack = 0;
                (*app_layer).fin = 0;
                (*app_layer).reserved = 0;
            
                //    for(current_line=0, test=-1; current_line<number_of_lines_in_file[0]; current_line++)
                for(current_file=MAX_NUM_OF_FILES-1; current_file>=0; current_file--)
                //for(current_file=0; current_file<MAX_NUM_OF_FILES; current_file++)
                {
            
                    for(current_line=file[current_file].number_of_lines_in_file-1, test=-1; current_line>=0; current_line--)
                    //for(current_line=0, test=-1; current_line<file[current_file].number_of_lines_in_file; current_line++)
                    {
                
                        (*app_layer).file_id = file[current_file].file_id ;
                        (*app_layer).file_number = file[current_file].file_number ;
                        (*app_layer).current_line = current_line;
                        (*app_layer).total_lines = file[current_file].number_of_lines_in_file;

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
                
                        app_header_n_data = concat_bytes_alloc(app_layer, sizeof(file_x_app_layer_t), 
                                                         file[current_file].text_line[current_line], strlen(file[current_file].text_line[current_line])-1 );
                        
                        //printBytes(app_header_n_data, sizeof(file_x_app_layer_t) + strlen(file[current_file].text_line[current_line]) -1);
                
                        //SEND THE DATA: TX Socket, TX Data, TX Data Size, flags, RX Socket Info (DOMAIN, IP and PORT), size of RX Address Struct)
                        test=sendto( tx_socket_fd, 
                                     app_header_n_data,                                             \
                                     strlen(file[current_file].text_line[current_line])+sizeof(file_x_app_layer_t)-1,    \
                                     0,                                                             \
                                     (struct sockaddr *) &tx_to_address,                            \
                                     sizeof(tx_to_address)                                          );
                        if ( test < 0) printf("Failed to send line(%d).\n", current_line);
                        free(app_header_n_data);
                        
                    }

                }    
                free(app_layer);
            
                //##################### SEND DATA END:##################################

                //break;
            }
            //case RECEIVE_FIN_DATA: 
            if (STATE ==  RECEIVE_FIN_DATA){
                printf("RECEIVE_FIN_DATA\n");

                test = recvfrom( rx_socket_fd,                             \
                         receiveDgramBuffer,                   \
                         sizeof(receiveDgramBuffer),           \
                         0,                                    \
                         (struct sockaddr *) &rx_from_address, \
                         &rxSockLen                            );
                // time out occured or other error:
                if (test < 0) { STATE=SEND_DATA; continue; }
    
                app_layer = (file_x_app_layer_t *) receiveDgramBuffer;
                printf("(*app_layer).fin: %d\n",(*app_layer).fin);
                printf("(*app_layer).init: %d\n",(*app_layer).init);
                printf("(*app_layer).ack: %d\n",(*app_layer).ack);

                if ( (*app_layer).fin == 1 && (*app_layer).ack == 1 ) return -1;

                STATE = SEND_FIN_ACK;
                if ( (*app_layer).fin == 0 ) STATE = SEND_DATA;
    

            
                
                //##################### RECEIVE FIN PACKET END:##################################
                
                //##################### WRITE FILE BEGIN:##################################
            

                if ( (outfile_fd=fopen(arguments.outfile,"w")) == NULL ) { 
                    printf(stderr, "Unable to open file %s; Use --input-file option, and make sure the file is present.\n", arguments.outfile); 
                    return -1; }
                else  {
                    fflush(outfile_fd);
                    for(current_char=0;current_char<(test-sizeof(file_x_app_layer_t));current_char++) {
                        //printf("%c", app_layer + sizeof(file_x_app_layer_t) + current_char);
                        fwrite( (char *) app_layer+sizeof(file_x_app_layer_t)+current_char, 1, 1, outfile_fd);
                     }
                    
                    fclose(outfile_fd);
                } 
            
                //##################### WRITE FILE END SEND_FIN_ACK BEGIN:##################################
            }
            //case SEND_FIN_ACK:
            if (STATE ==  SEND_FIN_ACK){
                printf("SEND_FIN_ACK\n");
                STATE = RECIEVE_FIN_ACK;

                app_layer = calloc(1, sizeof(file_x_app_layer_t) );
                if (app_layer == NULL) {perror("client(): failed to allocate memory"); exit(EXIT_FAILURE);}

                (*app_layer).file_id = 0;
                (*app_layer).current_line = 0;
                (*app_layer).total_lines = 0;
                (*app_layer).ack = 1;
                (*app_layer).init = 0;
                (*app_layer).fin = 1;
                (*app_layer).reserved = 0;

                test=sendto( tx_socket_fd,                          \
                             app_layer,                             \
                             sizeof(file_x_app_layer_t),            \
                             0,                                     \
                             (struct sockaddr *) &tx_to_address,    \
                             sizeof(tx_to_address)                  );


                if ( test < 0) printf("Failed to send FIN|ACK\n");
                free(app_layer);
                //break;
            }
            //case RECIEVE_FIN_ACK:
            if (STATE ==  RECIEVE_FIN_ACK){
                printf("RECEIVE_FIN_ACK\n");

                test = recvfrom( rx_socket_fd,                             \
                         receiveDgramBuffer,                   \
                         sizeof(receiveDgramBuffer),           \
                         0,                                    \
                         (struct sockaddr *) &rx_from_address, \
                         &rxSockLen                            );
                // time out occured or other error:
                if (test < 0) { STATE=SEND_FIN_ACK; continue; }
                app_layer = (file_x_app_layer_t *) receiveDgramBuffer;

                printf("(*app_layer).fin: %d\n",(*app_layer).fin);
                printf("(*app_layer).init: %d\n",(*app_layer).init);
                printf("(*app_layer).ack: %d\n",(*app_layer).ack);

                if ( (*app_layer).fin == 1 && (*app_layer).ack == 1) {printf("DONE\n");return 0;}
                STATE = SEND_FIN_ACK;
                
                //continue;
            }

            //default: return -1;
        //}   


    }

  
    return 0;
}
