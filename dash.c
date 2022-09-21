#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * This program counts the number of sentences in a given string and
 * the number of tokens each in sentence.
 * Terminates when user enters BYE
 */

size_t buffer_size = 32;

void parse_command(char *buffer, size_t characters)
{
    // define the required variables
	char *sentence, *token, *last_sentence, *last_token;
	char *sentence_sep = ".?!";
	char *token_sep = " ";    
    int num_sentences = 0;
	int num_tokens = 0;

    // The input string consists of several sentences separated by '.'
    // Each sentence consists of several tokens separated by ' ' (space).
    // Using strtok_r() find the number of sentences and the number of tokens
    // in each sentence. HINT: man strtok and look at the sample program at the end.
    // Print the results.
    // If the first token is BYE, break from the while loop (check strcmp/strncmp)

    // check if user entered EOF (Ctrl+D) or the first token was BYE
    if (characters == -1 || strncmp("exit", buffer, 4) == 0)
        exit(EXIT_SUCCESS);

    // iterate each sentence
    for(sentence = strtok_r(buffer, sentence_sep, &last_sentence); sentence; sentence = strtok_r(NULL, sentence_sep, &last_sentence)) {
        num_sentences++;
        
        // check if sentence is newline aka end of user input
        if (strcmp("\n", sentence) == 0)
            break;

        printf("sentence %d: ", num_sentences);

        // iterate each token in sentence
        for (token = strtok_r(sentence, token_sep, &last_token); token; token = strtok_r(NULL, token_sep, &last_token))
            num_tokens++;
        
        printf("tokens %d\n", num_tokens);

        // reset token counter
        num_tokens = 0;
    }

    // reset sentence counter
    num_sentences = 0;
}


int main(int argc, char *argv[])
{

	
	char *buffer;
	size_t characters;
    
	// initialize buffer
	buffer = (char *)malloc(buffer_size * sizeof(char));
	if (buffer == NULL) {
		perror("unable to allocate buffer");
		exit(EXIT_FAILURE);
	}

    if (argc == 2) {
        char **pargv = argv+1;
        // check if file can be accessed
        if (access(*pargv, R_OK) == 0) {
            FILE *fptr;
            ssize_t read;

            // open the file
            fptr = fopen(*pargv, "r");
            if (fptr == NULL)
                exit(EXIT_FAILURE);
            
            // read line by line
            while ((read = getline(&buffer, &characters, fptr)) != -1) {
                parse_command(buffer, characters);
            }

            // close file
            fclose(fptr);

        }

    } else {

        while (1) {
            printf("dash> ");

            // use getline() to get the input string
            characters = getline(&buffer, &buffer_size, stdin);

            // give input from stdin to parse_command
            parse_command(buffer, characters);
            
        }

    }

    if (buffer)
        free(buffer);

    exit(EXIT_SUCCESS);
}

