#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *path = "/bin";
char error_message[30] = "An error has occurred\n";

void error() {
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit(EXIT_FAILURE);
}

char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator

    if (result == NULL) {
        write(STDERR_FILENO, error_message, strlen(error_message));
	} else {
        strcpy(result, s1);
        strcat(result, s2);
    }

    return result;
}

void parse_command(char *buffer)
{
    // define the required variables
	char *token, *current_path, *path_copy, *exe_path, *last_token, *last_path;
	const char *token_sep = " \t";
    const char *path_sep = ":";
    char *args[100];
    int status;    
    
    // parse the tokens into args array
    int i = 0;
    // iterate each token
    for(token = strtok_r(buffer, token_sep, &last_token); token; token = strtok_r(NULL, token_sep, &last_token)) {
        args[i] = token;
        i++;
    }

    // terminate the list of args
    args[i] = NULL;

    // make a copy of the path to modify during search
    path_copy = calloc(strlen(path)+1, sizeof(char));
    strcpy(path_copy, path);
    // search for the executable in path
    for(current_path = strtok_r(path_copy, path_sep, &last_path); current_path; current_path = strtok_r(NULL, path_sep, &last_path)) {
        // combine path with first arg (executable)
        exe_path = concat(current_path, concat("/", args[0]));

        // check if executable exists
        if (access(exe_path, X_OK) == 0) {
            // execute the command
            if (fork() == 0)
                execv(exe_path, args);
            else
                wait(&status);

            break;
        }
        
    }

}

int main(int argc, char *argv[])
{

	char *buffer;
	size_t characters;
    size_t buffer_size = 32;
    FILE *input = stdin;
    
	// initialize buffer
	buffer = (char *)malloc(buffer_size * sizeof(char));
	
    if (buffer == NULL || argc > 2)
        error();

    // switch to batch mode if given file
    if (argc == 2) {
        char **pargv = argv+1;
        // check if file can be accessed
        if (access(*pargv, R_OK) == 0) {
            // open the file
            input = fopen(*pargv, "r");
        }

        if (input == NULL || input == stdin) {
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(EXIT_FAILURE);
        }
    }

    // run commands
    while (1) {
        if (input == stdin)
            printf("dash> ");

        // use getline() to get the input string
        characters = getline(&buffer, &buffer_size, input);

        // check if user entered EOF (Ctrl+D) or the first token was exit
        if(characters == -1 || strncmp("exit", buffer, 4) == 0)
            break;
        
        // remove newline char at end of user input
        char sub_buffer[characters];
        strncpy(sub_buffer, buffer, characters - 1);
        sub_buffer[characters - 1] = '\0';

        // give input from stdin or file to parse_command
        parse_command(sub_buffer);
    }

    if (input != stdin)
        fclose(input);

    if (buffer)
        free(buffer);

    exit(EXIT_SUCCESS);
}

