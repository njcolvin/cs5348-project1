#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>

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
        //implement exit
        exit(EXIT_SUCCESS);
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
	char *buffer_copy, *token, *current_path, *path_copy, *exe_path, *last_token, *last_path, *index_of_redirect, *output_file;
    const char *redirect = ">";
	const char *token_sep = " \t";
    const char *path_sep = ":";
    // int status;    
    int i = 0;

    // see if the command contains a redirect
    index_of_redirect = strstr(buffer, redirect);
    if (index_of_redirect != NULL) {
        // get the index of the redirect character
        int index = index_of_redirect - buffer;
        int length = strlen(buffer);
                    
        // if index is the first or last symbol of the command then it is an error
        if (index == 0 || index == length - 1)
            error();
        else {
            // trim the command up to the redirect symbol and set output_file to the trimmed susbtring after the redirect symbol
            output_file = (char *) malloc((strlen(index_of_redirect)) * sizeof(char));
            strncpy(output_file, index_of_redirect + 1, length - index);
            output_file[length - index + 1] = '\0';
            buffer[index] = '\0';
            while(isspace(output_file[0]))
                output_file = trim_string(output_file);
        }
    }
    

    // parse the tokens into args array

    // copy the buffer as it will be modified during iteration
    buffer_copy = calloc(strlen(buffer)+1, sizeof(char));
    strcpy(buffer_copy, buffer);

    // iterate each token to get the size of the array
    for(token = strtok_r(buffer_copy, token_sep, &last_token); token; token = strtok_r(NULL, token_sep, &last_token))
        i++;


    // initialize args
    char *args[i + 1];

    // iterate each token again to add them to the array
    i = 0;
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
            return;
        }
    }
    int command_exists = 0;
    // make a copy of the path to modify during search
    path_copy = calloc(strlen(path)+1, sizeof(char));
    strcpy(path_copy, path);
    // search for the executable in path
    for(current_path = strtok_r(path_copy, path_sep, &last_path); current_path; current_path = strtok_r(NULL, path_sep, &last_path)) {
        if (command_exists == 1) 
            break;
        // combine path with first arg (executable)
        exe_path = concat(current_path, concat("/", args[0]));

        // check if executable exists
        if (access(exe_path, X_OK) == 0) {
            // execute the command
            if (index_of_redirect != NULL) {
                // redirect the command output
                int fd = open(output_file, O_CREAT | O_WRONLY, 0777);
                // redirect stdout
                dup2(fd, 1);
                //redirect stderr
                dup2(fd, 2);
                // file handles set; file can be closed
                close(fd);
            }
            command_exists = 1;
            execv(exe_path, args);
        } 
    }
    if (command_exists == 0) {
        error();
    }
}

void parse_command(char *command) {
    int status;
    pid_t process_id = fork();
    if (process_id == 0) {
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
        run_command(trim_string(current_command));
    }
    else {
        wait(&status);
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
        else {
            printf("File can't be accessed\n");
        }

        if (input == NULL || input == stdin)
            error();
    }

    // run commands
    while (1) {
        if (input == stdin)
            printf("dash> ");

        // use getline() to get the input string
        characters = getline(&buffer, &buffer_size, input);

        // check if user entered EOF (Ctrl+D) or the first token was exit
        if(characters == -1)
            break;
        
        // remove newline char at end of user input
        char sub_buffer[characters];
        strncpy(sub_buffer, buffer, characters - 1);
        sub_buffer[characters - 1] = '\0';

        // give input from stdin or file to run_command
        parse_command(sub_buffer);
        if (strstr(sub_buffer,"exit") != NULL)
            break;
    }

    if (input != stdin)
        fclose(input);

    if (buffer)
        free(buffer);

    exit(EXIT_SUCCESS);
}

