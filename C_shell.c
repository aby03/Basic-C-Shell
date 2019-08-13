/*	BASIC SHELL IN C
	Features:
		-history implementation
		-cd implementation
		-rest system calls are being passed to execvp after forking child process
	Owner: aby03
*/
#include  <stdio.h>
#include  <sys/types.h>

#define HISTORY_MAX_SIZE 100

// Global Variables
static char *history[HISTORY_MAX_SIZE];
static unsigned history_count = 0;

// Function Definitions
// Parse the input
void get_words(char *line, char **words)
{
	// Remove starting space
	while (*line == ' ' || *line == '\t'){
		line++;
	}
	// Parse
	int comma = 0;
	while (*line != '\0') {
		while (*line == ' ' || *line == '\t' || *line == '\n'){

			if (*line == '\\'){
				memmove(line, line+1, strlen(line));
			}
			*line++ = '\0';
		}
		*words++ = line;
		while (*line != '\0' && (*line != ' ' || comma == 1) && *line != '\t' && *line != '\n') {
			if (*line == '"'){
				memmove(line, line+1, strlen(line));
				comma = (comma+1)%2;
			}
			if (*line == '\\'){
				memmove(line, line+1, strlen(line));
			}
			line++;
		}
	}
	*words = '\0';
}

// For making system calls
void system_call(char **words){
	pid_t pid;
	int status;         // captures return status of child process
	pid = fork();

	if (pid < 0){
		// Process Creation failed
		printf("### ERROR: Child Process could not be created. ###");
		exit(1);
	}else if (pid == 0){
		// Child Process
		if (execvp(*words, words) < 0){
			printf("### ERROR: No Command Found ###");
			exit(1);
		}
	}else{
		// Parent Process
		while( wait(&status) != pid);
	}
}

// For storing commands to history
void add_command_to_history( const char *command )
{
   if (history_count < HISTORY_MAX_SIZE) {
        history[history_count++] = strdup( command );
   } else {
        free( history[0] );
        for (unsigned index = 1; index < HISTORY_MAX_SIZE; index++) {
            history[index - 1] = history[index];
        }
        history[HISTORY_MAX_SIZE - 1] = strdup( command );
    }
}

// Main Loop
int main(int argc, char **argv){
	char line[1000];				// Contains full typed line
	char *words[100];			// Contains pointers to each space separated word
	char *directory[100];	// Contains path to current directory from root(Directory from where code is being run)
	int dir_status = 0;		// Depth from root(Directory where code was executed) directory

	while(1){
		printf("aby03");
		for (int i=0; i<dir_status; i++){			// Print current directory from root
			printf("/");
			printf("%s", directory[i]);
		}
		printf(">");
		gets(line);         									// Get Full command
		add_command_to_history(line);         // Copy for history
		get_words(line, words);       				// Break it into words
		// Check system call vs other calls
		// Handling cd command
		if (strcmp(*words, "cd") == 0){
			char cwd[256]; 
			getcwd(cwd,sizeof(cwd));
			char *ptr = *(words+1);
			int go_back = 0;
			// Case 1: cd from root directory
			if (*ptr == '/'){
				ptr++;
				int qq = dir_status;
				while (qq > 0){
					strcat(cwd,"/..");
					qq--;
				}
				strcat(cwd,"/");
				strcat(cwd,*(words+1));
				if (chdir(cwd) == 0){
					dir_status = qq;
					char *ptr;
					ptr = *(words+1);
					char *prev_ptr;
					ptr++;
					prev_ptr = ptr;
					while (*ptr != '\0'){
						// printf(" $ %c $", *ptr);
						if (*ptr == '/'){
							*ptr = '\0';
							directory[dir_status++] = strdup( prev_ptr );
							// dir_status++;
							ptr++;
							prev_ptr = ptr;
						}else{
							ptr++;
						}
					}
					ptr = '\0';
					directory[dir_status++] = strdup( prev_ptr );
				}else{
					printf("### No such directory exists. ###");
				}
			}else if (*ptr == '.' && *(ptr+1) == '.'){
				// Case 2: Going back
				while (*ptr != '\0'){
					if (*ptr != '.'){
						go_back = 0;
						break;
					}
					ptr++;
					if (*ptr != '.'){
						go_back = 0;
						break;
					}
					ptr++;
					go_back++;
					if (*ptr == '\n' || *ptr == '\0'){
						break;
					}else if (*ptr == '/'){
						ptr++;
						continue;
					}else{
						go_back = 0;
						break;
					}
				}
				if (go_back > 0 && dir_status - go_back >= 0){
					int pp = go_back;
					while(pp>0){
						strcat(cwd,"/..");
						pp--;
					}
					if (chdir(cwd) == 0){
						directory[dir_status] = *(words+1);
						dir_status-=go_back;
					}else{
						printf("### Couldn't Execute ###");
					}
				}else{
					if (go_back == 0){
						printf("### Cant go back ###");
					}else{
						printf("### Couldnt go back ###");
					}
				}
			}else{
				// Case 3: Going forward
				strcat(cwd,"/");
				strcat(cwd,*(words+1));
				if (chdir(cwd) == 0){
					char *ptr;
					ptr = *(words+1);
					char *prev_ptr;
					prev_ptr = ptr;
					int jk = 0;
					while (*ptr != '\0'){
						if (*ptr == '/'){
							*ptr = '\0';
							directory[dir_status++] = strdup( prev_ptr );
							ptr++;
							prev_ptr = ptr;
							jk=1;
						}else{
							ptr++;
							jk++;
						}
					}
					ptr = '\0';
					directory[dir_status++] = strdup( prev_ptr );
				}else{
					printf("### No such directory exists. ###");
				}
			}
		}else if (strcmp(*words, "history") == 0){
			for (int i=0; i<history_count; i++){
				printf("%d\t", i);
				printf("%s", history[i]);
				printf("\n");
			}
		}else{
			system_call(words);
		}
		printf("\n");
	}
}
