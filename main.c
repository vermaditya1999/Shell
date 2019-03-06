/* A shell program.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


const int BUF_SIZE = 1024;
const int MAX_TOKENS = 128;
const int MAX_CMDS = 16;
const int CMD_SIZE = 16;


void input(char* argv[], char buf[]);
void sigint_handler(int SIGNAL);
int partition_tokens(char *tokens[], char *cmds[MAX_CMDS][CMD_SIZE]);


int
main(void)
{
    // Set signal handler for SIGINT
    signal(SIGINT, sigint_handler);

    while (1)
    {
        int num_cmds;
        pid_t pid;
        char buf[BUF_SIZE];
        char *tokens[MAX_TOKENS];
        char *cmds[MAX_CMDS][CMD_SIZE];

        // Display shell prompt
        write(1, "(ash) $ ", 8);

        // Take user input
        input(tokens, buf);

        // Partition tokens into cmds array
        num_cmds = partition_tokens(tokens, cmds);

        if (num_cmds > 0)
        {
            // Exit if exit command is entered
            if (strcmp(tokens[0], "exit") == 0)
            {
                exit(0);
            }

            // Create child process
            if ((pid = fork()) > 0)
            {
                wait(NULL);
            }
            else if (pid == 0)
            {
                execvp(tokens[0], tokens);
                printf("%s: Command not found\n", tokens[0]);
                exit(0);
            }
            else
            {
                printf("Fork Error!\n");
            }
        }      
    }
}


/* Takes input from user and splits it in
   tokens into argv. The last element in
   argv will always be NULL. */
void
input(char* tokens[], char buf[])
{    
    buf[0] = '\0';
    fgets(buf, BUF_SIZE, stdin);

    tokens[0] = strtok(buf, "  \n\0");
    for (int i = 1; tokens[i] != NULL; ++i)
    {
        tokens[i] = strtok(NULL, "  \n\0");
    }
}


/* Parition the array of tokens into NULL terminated
   arrays of strings into the cmds array. The paritioning
   is done using the delimiter pipe "|" and NULL.
   The argument tokens must be a NULL terminated array
   of strings.
   The function returns the total number of cmds. */ 
int
partition_tokens(char *tokens[], char *cmds[MAX_CMDS][CMD_SIZE])
{
    if (tokens[0] == NULL) return 0;

    int num_cmds = 0;
    int cmds_i = 0;
    int col_i = 0;
    int tok_i = 0;

    while (1)
    {
        if (tokens[tok_i] == NULL)
        {
            cmds[cmds_i][col_i] = NULL;
            ++num_cmds;
            break;
        }
        else if (!strcmp(tokens[tok_i], "|"))
        {
            cmds[cmds_i][col_i] = NULL;
            ++cmds_i;
            ++tok_i;
            col_i = 0;
            ++num_cmds;
        }
        else {
            cmds[cmds_i][col_i] = tokens[tok_i];
            ++col_i;
            ++tok_i;
        }
    }

    return num_cmds;
}


/* The child processes created by execvp will
   have their default signal handlers and
   will close on SIGINT. However, we need to
   catch SIGINT from the top most parent
   process shell for which we will set an empty
   handler for SIGINT */
void
sigint_handler(int SIGNAL)
{
    // Do nothing
}
