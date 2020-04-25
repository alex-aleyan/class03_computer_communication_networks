/* 
 *
 * Citing Sources: 
 *
 * https://www.linuxtopia.org/online_books/programming_books/gnu_c_programming_tutorial/argp-example.html 
 * 
 */

#include <stdio.h>
#include <argp.h>


const char *argp_program_version = "argex 1.0";
const char *argp_program_bug_address = "<bug-gnu-utils@gnu.org>";


struct arguments
{
  char *args[10];            /* ARG1 and ARG2 */
  int verbose, debug;              /* The -v flag */
  char *outfile;            /* Argument for -o */
  char *source_ip, *source_port;
  char *dest_ip,   *dest_port;  /* Arguments for -a and -b */
};

struct server_arguments
{
  char *args[2];            /* ARG1 and ARG2 */
  int verbose, debug;              /* The -v flag */
  char *source_ip, *source_port;
};

static struct argp_option options[] =
{
  {"verbose",        'v',	0              , 0, "Produce verbose output"},
  {"debug",          'x',	0              , 0, "Produce verbose output"},
  {"dest-ip",        'd', "<XX.XX.XX.XX>", 0, "Provide Destination IP/hostname"},
  {"dest-port",      'b', "<XXXX>"						 , 0, "Provide destination port number"},
  {"source-ip",      's', "<XX.XX.XX.XX>", 0, "Provide source IP address of you NIC"},
  {"source-port",    'p', "<XXXX>"			 , 0, "Provide source port number > 1025"},
  {"output-file",    'o', "OUTFILE"					 , 0, "Output to OUTFILE instead of to standard output"},
  {0}
};

static struct argp_option server_options[] =
{
  {"verbose",        'v',	0              , 0, "Produce verbose output"},
  {"debug",          'x',	0              , 0, "Produce verbose output"},
  {"source-ip",      's', "<XX.XX.XX.XX>", 0, "Provide source IP address of you NIC"},
  {"source-port",    'p', "<XXXX>"			 , 0, "Provide source port number > 1025"},
  {0}
};

static char args_doc[] = "ARG1 ARG2";
static char doc[] = "Heading for the --help version\n";


static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
    struct arguments *arguments = state->input;
  
    switch (key)
    {
        case 'v':
            arguments->verbose = 1;
            break;
        case 'x':
            arguments->debug = 1;
            break;
        case 'd':
            arguments->dest_ip = arg;
            break;
        case 'b':
            arguments->dest_port = arg;
            break;
        case 's':
            arguments->source_ip = arg;
            break;
        case 'p':
            arguments->source_port = arg;
            break;
        case 'o':
            arguments->outfile = arg;
            break;
        /*
        case ARGP_KEY_ARG:
          return 0;
        */
        case ARGP_KEY_ARG:
            if (state->arg_num >= 10) argp_usage(state);
            arguments->args[state->arg_num] = arg;
            break;
        case ARGP_KEY_END:
            if (state->arg_num < 10) argp_usage(state);
            break;
        default:
          return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static error_t server_parse_opt (int key, char *arg, struct argp_state *state)
{
    struct server_arguments *arguments = state->input;
  
    switch (key)
    {
        case 'v':
            arguments->verbose = 1;
            break;
        case 'x':
            arguments->debug = 1;
            break;
        case 's':
            arguments->source_ip = arg;
            break;
        case 'p':
            arguments->source_port = arg;
            break;
        case ARGP_KEY_ARG:
          return 0;
        default:
          return ARGP_ERR_UNKNOWN;
    }

    return 0;
}

static struct argp argp        = {options,        parse_opt,        args_doc, doc};
static struct argp server_argp = {server_options, server_parse_opt, args_doc, doc};

