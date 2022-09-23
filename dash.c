#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

char *path = "/bin";
char error_message[30] = "An error has occurred\n";
char built_in_commands[3][5] = {
	"exit",
	"cd",
    "path"
};

void error() {
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit(EXIT_FAILURE);
}

char *trim_string(char const *str) {
    int l = strlen(str);
    int i = 0;
    //find first index from left
    while (i < l) {
        if (isspace(str[i]) == 1) {
            i++;
        }
        break;
    }
    if (i == l)
        return "";
    int l_index = i;
    //find first index from right
    i = l-1;
    while (i > l_index) {
        if (isspace(str[i]) == 1)
        {
            i--;
        }
        break;
    }
    int r_index = i;
    //copy from l_index to r_index and return
    char *trimmed_str = malloc(sizeof(char) * (r_index - l_index + 2));
    strncpy(trimmed_str, str + l_index, r_index - l_index + 1);
    return trimmed_str;
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

void execute_built_in_command(char *args[], int number_of_args, int command_index) {
    switch (command_index)
    {
    case 0:
        //exit already implemented
        break;
    case 1:
        //implement cd
        if (number_of_args != 2) {
            error();
        }
        else {
            if (chdir(args[1]) == 0) {
                char s[100];
                printf("%s is the current working directory.\n", getcwd(s,100));
            }
            else
                error();
        }
        break;
    case 2:
        //implement path
        path = "";
        int space_required = 0;
        for (int i = 1; i < number_of_args; i++) {
            space_required += strlen(args[i]) + 1;
        }
        path = malloc(space_required * sizeof(char));
        for (int i = 1; i < number_of_args; i++) {
            if (i != 0)
                strcat(path, ":");
            strcat(path, args[i]);
        }
        printf("Paths updated!\n");
        break;
    default:
        break;
    }
}

void run_command(char *buffer)
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

    int number_of_args = i;
    for (i=0; i<3; i++) {
        if (strcmp(args[0],built_in_commands[i]) == 0) {
            execute_built_in_command(args, number_of_args, i);
        }
    }

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

void parse_command(char *command) {
    char *delimiter_address = strchr(command, '&');
    int command_length = strlen(command);
    if (command_length == 0)
        return;
    int index = (delimiter_address == NULL ? -1 : delimiter_address - command);
    char *current_command;
    if (index == -1) {
        current_command = malloc(sizeof(char) * (command_length + 1));
        strcpy(current_command, command);
    }
    else {
        current_command = malloc(sizeof(char) * (index + 1));
        strncpy(current_command, command, index);
        char *parallel_commands = malloc(sizeof(char) * (command_length - index));
        strcpy(parallel_commands, command + index + 1);
        parse_command(parallel_commands);
    }
    printf("%s\n", trim_string(current_command));
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

        // give input from stdin or file to run_command
        parse_command(sub_buffer);
    }

    if (input != stdin)
        fclose(input);

    if (buffer)
        free(buffer);

    exit(EXIT_SUCCESS);
}

