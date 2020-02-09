// Shell starter file
// You may make any changes to any part of this file.

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>

#define COMMAND_LENGTH 1024
#define NUM_TOKENS (COMMAND_LENGTH / 2 + 1)
#define HISTORY_DEPTH 10
char history[HISTORY_DEPTH][COMMAND_LENGTH];
int arguments = 0 ; 
int Index_history = -1 ; 
pid_t var_pid = -1;
/**
 * Command Input and Processing
 */
/*
 * Tokenize the string in 'buff' into 'tokens'.
 * buff: Character array containing string to tokenize.
 *       Will be modified: all whitespace replaced with '\0'
 * tokens: array of pointers of size at least COMMAND_LENGTH/2 + 1.
 *       Will be modified so tokens[i] points to the i'th token
 *       in the string buff. All returned tokens will be non-empty.
 *       NOTE: pointers in tokens[] will all point into buff!
 *       Ends with a null pointer.
 * returns: number of tokens.
 */

void add_command(int *var, char* command[], bool background){
	char arg_char[16]; 
	sprintf(arg_char, "%d", ++(*var));
	//preping command 
	char cmd[1024];

	strcpy(cmd, arg_char);
	strcat(cmd, " ");
	strcat(cmd, " ");
	strcat(cmd, " ");
	strcat(cmd, " ");
	int  i  =0 ;
	int k = 4 ; 
	// FIXING CMD;
	while(command[i] != NULL ){
		int j = 0 ;
		for ( ; j < strlen(command[i]); j++){
			cmd[k++] = command[i][j];
		}
		cmd[k++] = ' '; 
		i++; 
	}
	if ( background){
		cmd[k] = '&';
		cmd[k+1] = '\0';
	}
	else{
		cmd[k] = '\0'; 
	}

	++Index_history; 
	// printf("\n INDEX HISTORY IN ADD IS %d\n", Index_history);

	if ( Index_history < HISTORY_DEPTH){
		int i = 0 ;
		for( ; i < strlen(cmd) ; i++){
			history[Index_history][i] = cmd[i];
		}
		history[Index_history][i] = '\0'; 
	}

	else {
		for( int i = 0 ; i < HISTORY_DEPTH - 1 ; i++){
			int j = 0 ;
			for(  ; history[i+1][j] != '\0' ; j++){
				history[i][j] = history[i+1][j];
			}
			history[i][j] = '\0';
		}
		int i = 0;
		for( ; i < strlen(cmd) ; i++){
			history[HISTORY_DEPTH-1][i] = cmd[i];
		}
		history[HISTORY_DEPTH-1][i] = '\0';
	}
	// printf("\n");
	// write(STDOUT_FILENO, history[0], strlen(history[0]));
	// printf("\n");
	// printf("cmd is  -- %s\n", cmd);

}
void print_history(){
	if ( Index_history < 0){
		write(STDOUT_FILENO, " \nNO COMANDS YET \n", strlen(" \nNO COMANDS YET \n"));
	}
	else if ( Index_history < HISTORY_DEPTH){
		for( int i = Index_history; i >=0 ; i-- ){
			write(STDOUT_FILENO, history[i], strlen(history[i]));
			write(STDOUT_FILENO, "\n", strlen("\n"));
		}
	}
	else{
		for( int i =  HISTORY_DEPTH-1; i >= 0 ; i--){
			write(STDOUT_FILENO, history[i], strlen(history[i]));
			write(STDOUT_FILENO, "\n", strlen("\n"));
		}
	}


}
int tokenize_command(char *buff, char *tokens[])
{
	int token_count = 0;
	_Bool in_token = false;
	int num_chars = strnlen(buff, COMMAND_LENGTH);

	for (int i = 0; i < num_chars; i++) {
		switch (buff[i]) {
		// Handle token delimiters (ends):
		case ' ':
		case '\t':
		case '\n':
			buff[i] = '\0';
			in_token = false;
			break;

		// Handle other characters (may be start)
		default:
			if (!in_token) {
				tokens[token_count] = &buff[i];
				token_count++;
				in_token = true;
			}
		}
	}
	tokens[token_count] = NULL;
	return token_count;
}

/**
 * Read a command from the keyboard into the buffer 'buff' and tokenize it
 * such that 'tokens[i]' points into 'buff' to the i'th token in the command.
 * buff: Buffer allocated by the calling code. Must be at least
 *       COMMAND_LENGTH bytes long.
 * tokens[]: Array of character pointers which point into 'buff'. Must be at
 *       least NUM_TOKENS long. Will strip out up to one final '&' token.
 *       tokens will be NULL terminated (a NULL pointer indicates end of tokens).
 * in_background: pointer to a boolean variable. Set to true if user entered
 *       an & as their last token; otherwise set to false.
 */
void read_command(char *buff, char *tokens[], _Bool *in_background)
{
	*in_background = false;

	// Read input
	int length = read(STDIN_FILENO, buff, COMMAND_LENGTH-1);

	// if (length < 0) {
	// 	perror("Unable to read command from keyboard. Terminating.\n");
	// 	exit(-1);
	// }
	if ( (length < 0) && (errno !=EINTR) ){
    perror("Unable to read command. Terminating.\n");
    exit(-1);  /* terminate with error */
	}
	// Null terminate and strip \n.
	buff[length] = '\0';
	if (buff[strlen(buff) - 1] == '\n') {
		buff[strlen(buff) - 1] = '\0';
	}

	// Tokenize (saving original command string)
	int token_count = tokenize_command(buff, tokens);
	if (token_count == 0) {
		return;
	}

	// Extract if running in background:
	if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
		*in_background = true;
		tokens[token_count - 1] = 0;
	}
}

void handle_SIGINT(){
	write(STDOUT_FILENO, "\n", strlen("\n"));
	print_history(); 
	// exit(0);
	// return;
}
/**
 * Main and Execute Commands
 */
int main(int argc, char* argv[])
{
	char input_buffer[COMMAND_LENGTH];
	char *tokens[NUM_TOKENS];
	// int arguments = 0; 
	
	int run_prev_commmand = false; 
	char prev_command[1024]; 
	struct sigaction handler; 
	handler.sa_handler = handle_SIGINT;
	handler.sa_flags = 0; sigemptyset(&handler.sa_mask);
	sigaction(SIGINT, &handler, NULL);


	while (true) {
		// signal(SIGINT, handle_SIGINT);
		// Get command
		// Use write because we need to use read() to work with
		// signals, and read() is incompatible with printf().
		// write(STDOUT_FILENO, "$ ", strlen("$ "));
		_Bool in_background = false;

		signal(SIGINT, handle_SIGINT);
		

		if ( in_background == false){
			while(waitpid(var_pid, NULL, WNOHANG) == 0){
						;
			}
		}

		if ( run_prev_commmand == true){
			int token_count =  tokenize_command(prev_command, tokens);
			// write(STDOUT_FILENO, "token is \n", strlen("token is \n"));
			// write(STDOUT_FILENO, tokens[0], strlen(tokens[0]));
			// Extract if running in background:
			// write(STDOUT_FILENO, "\nPREV executed\n", strlen("\nPREV executed\n"));
			if (token_count > 0 && strcmp(tokens[token_count - 1], "&") == 0) {
				in_background = true;
				write(STDOUT_FILENO, tokens[token_count-1], strlen(tokens[token_count-1]));

				tokens[token_count-1] = 0;
			}
			run_prev_commmand = false;
		}
		else { 
			char current_dir[FILENAME_MAX];
			write(STDOUT_FILENO, getcwd(current_dir, sizeof(current_dir)), strlen(getcwd(current_dir, sizeof(current_dir))) );
			write(STDOUT_FILENO, "$ ", strlen("$ "));
			in_background = false;
			// write(STDOUT_FILENO, "\noRIG executed\n", strlen("\noRIG executed\n"));
			read_command(input_buffer, tokens, &in_background);
		}

		// DEBUG: Dump out arguments:
		for (int i = 0; tokens[i] != NULL; i++) {
			write(STDOUT_FILENO, "   Token: ", strlen("   Token: "));
			write(STDOUT_FILENO, tokens[i], strlen(tokens[i]));
			write(STDOUT_FILENO, "\n", strlen("\n"));
		}
		if (in_background) {
			write(STDOUT_FILENO, "Run in background.", strlen("Run in background."));
		}
		// write(STDOUT_FILENO, getuid(), strlen(getuid()));
		printf("\n uid is %d\n", getuid());
		// itoa( arguments, arg_char, 10 ); 
		
		// printf("val of arguments is - --%d\n", arguments);
	
		/**
		 * Steps For Basic Shell:
		 * 1. Fork a child process
		 * 2. Child process invokes execvp() using results in token array.
		 * 3. If in_background is false, parent waits for
		 *    child to finish. Otherwise, parent loops back to
		 *    read_command() again immediately.
		 */
		//HISTORY COMMANDS :::::::::
		if ( strcmp(tokens[0], "history")== 0){
			// printf(" index_history is  -- %d", Index_history); 
			add_command(&arguments, tokens, in_background);
			print_history(); 
		}
		else if (tokens[0][0] == '!' ){
			if ( tokens[0][1] == '\0'){
				write(STDOUT_FILENO, "No Command Number Entered\n", strlen("No Command Number Entered\n"));
			}
			else if ( tokens[0][1] == '!'){
				if ( Index_history >= 0){
					char temp[1024];
					int i = 0 ;
					for( ; history[Index_history][i+4] != '\0'; i++){
						temp[i] = history[Index_history][i+4] ;
					}
					temp[i] = '\0';
					run_prev_commmand = true ;
					write(STDOUT_FILENO, "\n", strlen("\n"));
					write(STDOUT_FILENO, temp, strlen(temp));
					write(STDOUT_FILENO, "\n", strlen("\n"));
					strcpy(prev_command, temp);
				}
				else{
					write( STDOUT_FILENO, "No Commands Entered Yet\n", strlen("No Commands Entered Yet\n"));
				}
			}
			else {
				char arr[10]; 
				int i =1 ;
				for( int i =1 ; tokens[0][i] != '\0'; i++){
					arr[i-1] = tokens[0][i];
				}
				arr[i+1] = '\0';
				// printf( "\n arr is %s", arr);
				int num = atoi( arr);
				if ( num < arguments && num > 0 && Index_history >= 0 && num > arguments -10){
					char temp[1024];
					if ( Index_history < HISTORY_DEPTH){
						int i = 0 ;
						for( ; history[num-1][i+4] != '\0'; i++){
							temp[i] = history[num-1][i+4] ;
						}
						temp[i] = '\0';
						// printf(" temp is %s\n", temp);
					}
					else{
						// printf("\n num is %d\n", num);
						int num_to_use = arguments - num ;
						num_to_use = HISTORY_DEPTH - num_to_use;
						int i = 0 ;
						// printf("\n num TO USE is %d\n", num_to_use);
						for( ; history[num_to_use-1][i+4] != '\0'; i++){
							temp[i] = history[num_to_use-1][i+4] ;
						}
						temp[i] = '\0';
					}
					//Prev Command Run:::::::::::
					run_prev_commmand = true ;
					strcpy(prev_command, temp);
				}
				else{
					write(STDOUT_FILENO, "Invalid Command Number", strlen("Invalid Command Number"));
					write(STDOUT_FILENO, "\n", strlen("\n"));
				}

			}
			
			
			// printf( "\n num is %d", num);

		}
		//Handling Internal Comands:::::

		else if (strcmp( tokens[0], "exit") == 0 ){
			if ( tokens[1] != '\0'){
				write( STDOUT_FILENO ," NO ARGUMENTS FOR exit ", strlen(" NO ARGUMENTS FOR exit ")); 
				write(STDOUT_FILENO, "\n", strlen("\n"));
			}
			else {
				add_command(&arguments, tokens, in_background);
				exit(0); 
			}
		}
		else if ( strcmp( tokens[0], "pwd") == 0){
			char cwd [FILENAME_MAX];
			if ( tokens[1] != '\0'){
				write( STDOUT_FILENO ," NO ARGUMENTS FOR pwd ", strlen(" NO ARGUMENTS FOR pwd ")); 
				write(STDOUT_FILENO, "\n", strlen("\n"));
			}
			else{
				if ( getcwd( cwd , sizeof(cwd)) != NULL){ 
					write( STDOUT_FILENO, cwd, strlen(cwd) );
					write(STDOUT_FILENO, "\n", strlen("\n"));
					add_command(&arguments, tokens, in_background);
				}
				else{
					perror("getcwd() error"); 
				}
			}
			
		}
		else if( strcmp( tokens[0], "cd") == 0){ 
			if ( tokens[1] == '\0'){
				chdir(getenv("HOME"));
			}
			else if( tokens[2] != '\0'){
				write( STDOUT_FILENO ," NO 2 ARGUMENTS FOR cd ", strlen(" NO 2 ARGUMENTS FOR cd ")); 
				write(STDOUT_FILENO, "\n", strlen("\n"));
			}
			else if ( tokens[1][0]  == '~') {
				if ( strlen(tokens[1]) == 1){
					printf("\nhahahah\n");
					chdir(getenv("HOME"));
				}
				else{
					char arr[FILENAME_MAX]; 
					int i = 0 ; 
					for( ; tokens[1][i+2] != '\0'; i++){
						arr[i] = tokens[1][i+2];
					}
					arr[i]= '\0';
					struct passwd *user_dir;
					if ((user_dir = getpwuid(getuid())) == NULL)
					    perror("getpwuid() error");
					else {
					    printf("  pw_name  : %s\n",       user_dir->pw_name);
					    chdir(getenv("HOME"));
					    chdir(user_dir->pw_name);
					    chdir(arr);
					}  

				}
				
			}
			else if ( strcmp(tokens[1], "-") == 0 ) {
				add_command(&arguments, tokens, in_background);
				int var = chdir("..");
				if ( var == -1){
					perror("chdir failed"); 
				}
			}
			else if ( chdir(tokens[1]) == -1 ){
				printf(" 11 is  %s\n", tokens[1]);
				perror("chdir failed");

			}
			else{
				add_command(&arguments, tokens, in_background);
				chdir("tokens[1]"); // ASK ABOUT MOVING UP A DIRECTORY ????????????????????????????????????
				// chdir("..");
			}
		}
		else if (strcmp ( tokens[0], "help") == 0){
			if( tokens[1] == '\0'){
				add_command(&arguments, tokens, in_background);
				write(STDOUT_FILENO, " 'exit' is a builtin command for exiting the shell.\n", strlen(" 'exit' is a builtin command for exiting the shell.\n"));
				write(STDOUT_FILENO, " 'pwd' is a builtin command for displaying the current working DIRECTORY.\n", strlen(" 'pwd' is a builtin command for displaying the current working DIRECTORY.\n"));
				write(STDOUT_FILENO, " 'cd' is a builtin command for changing the current working directory.\n", strlen(" 'cd' is a builtin command for changing the current working directory.\n"));
				write(STDOUT_FILENO, " 'help' is a builtin command to get short summary on commands.\n", strlen(" 'help' is a builtin command to get short summary on commands.\n"));
			}
			else if(tokens[2] != '\0'){
				write( STDOUT_FILENO ,"\nNO 2 ARGUMENTS FOR help\n", strlen("\nNO 2 ARGUMENTS FOR help\n")); 
				write(STDOUT_FILENO, "\n", strlen("\n"));
			}
			else{
				if( strcmp(tokens[1], "exit") == 0 ){
					add_command(&arguments, tokens, in_background);
					write(STDOUT_FILENO, " 'exit' is a builtin command for exiting the shell.\n", strlen(" 'exit' is a builtin command for exiting the shell.\n"));
				} 
				else if ( strcmp(tokens[1], "pwd") == 0 ){
					add_command(&arguments, tokens, in_background);
					write(STDOUT_FILENO, " 'pwd' is a builtin command for displaying the current working DIRECTORY.\n", strlen(" 'pwd' is a builtin command for displaying the current working DIRECTORY.\n"));

				}
				else if ( strcmp( tokens[1], "cd") == 0){
					add_command(&arguments, tokens, in_background);
					write(STDOUT_FILENO, " 'cd' is a builtin command for changing the current working directory.\n", strlen(" 'cd' is a builtin command for changing the current working directory.\n"));
				}
				else if ( strcmp( tokens[1], "help") == 0){
					add_command(&arguments, tokens, in_background);
					write(STDOUT_FILENO, " 'help' is a builtin command to get short summary on commands.\n", strlen(" 'help' is a builtin command to get short summary on commands.\n"));
				}
				else{
					char happy[strlen(" is an external command or application.")] = " is an external command or application.";
					strcat(tokens[1], happy);
					write( STDOUT_FILENO, tokens[1], strlen(tokens[1]) );
					write(STDOUT_FILENO, "\n", strlen("\n"));
					add_command(&arguments, tokens, in_background);
				}
			}
		}
		else {
			var_pid = fork();
			add_command(&arguments, tokens, in_background);
			if ( var_pid < 0 ){
				write(STDOUT_FILENO, "\nfork failed\n", strlen("\nfork failed\n"));
				exit(-1); 
			}
			else if( var_pid == 0 ){   ///// NO FUNCTION CALLS WORK FOR THIS..... !!!!!!REMEMBER pid == 0 !!!!!!
				// add_command(&arguments, tokens);
				write(STDOUT_FILENO, "\nforking child\n", strlen("\nforking child\n"));
				write(STDOUT_FILENO, "\n", strlen("\n"));
				// add_command(&arguments, tokens);
				execvp(tokens[0], tokens);
				perror(" execvp failed ");
				exit(1);
			}
			else {
				while ( waitpid(-1, NULL, WNOHANG) > 0){
					;//don nithing waitng for any child processes 
				}
			}
		}
		
	}
	return 0;
}

