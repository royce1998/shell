#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int parseAndRun(char *str, int outfilestats);
int prepareCMD(char *cmd_buff);

void myPrint(char *msg)
{
    write(STDOUT_FILENO, msg, strlen(msg));
}

/*char* trim(char *str){
    int counter, i;
    for (i = 0; i < strlen(str); i++){
        if (str[i] != ' ' && str[i] != '\t' && str[i] != '\n')
            counter++;
    }
    char *trimmed = (char *)malloc(sizeof(char) * counter);
    int counter2 = 0;
    for (i = 0; i < strlen(str); i++){
        if (str[i] != ' ' && str[i] != '\t' && str[i] != '\n'){
            trimmed[counter2] = str[i];

        }
    }
}*/

#define delim ">+ \t\n"
int main(int argc, char *argv[]) 
{
	int reprompt = 1;
    char *cmd_buff = (char *)malloc(515 * sizeof(char));
	cmd_buff[514] = '&';
    char *pinput;
	int stat = 1;
	size_t len = 515;
	
	FILE *batch;
	if (argc > 1){
		argv[2] = NULL;
		batch = fopen(argv[1], "r");
		if (!batch){ // if file is null
			char error_message[30] = "An error has occurred\n";
			write(STDOUT_FILENO, error_message, strlen(error_message));
		}
		while(getline(&cmd_buff, &len, batch) != -1){
			int allspaces = 1;
			int i;
			for (i = 0; i < strlen(cmd_buff) - 1; i++){
				if (cmd_buff[i] != ' ' && cmd_buff[i] != '	' && cmd_buff[i] != '\n')
					allspaces = 0;
			}
			if (!allspaces && cmd_buff[0] != '\n'){
				myPrint(cmd_buff);
			}
			if ('e' == cmd_buff[0] && 'x' == cmd_buff[1] && 'i' == cmd_buff[2] && 't' == cmd_buff[3]){
				if (cmd_buff[4] == '\n'){
					exit(0);
				}
			}
			if (cmd_buff[514] == '&')
				prepareCMD(cmd_buff);
			else{
				char error_message[30] = "An error has occurred\n";
				write(STDOUT_FILENO, error_message, strlen(error_message));
				cmd_buff[514] = '&';
			}
		}
		stat = 0;
	}
	
    while (stat){
		if (reprompt){
			myPrint("myshell> ");
		}
		int torun = 1;
		if (argc < 2){
			if (cmd_buff[514] == '&')
				pinput = fgets(cmd_buff, 515, stdin);
			else{
				char error_message[30] = "An error has occurred\n";
				write(STDOUT_FILENO, error_message, strlen(error_message));
				cmd_buff[514] = '&';
				torun = 0;
				continue;
			}
		}
		if (torun){
			if (!pinput || ('e' == cmd_buff[0] && 'x' == cmd_buff[1] && 'i' == cmd_buff[2] && 't' == cmd_buff[3])) {
				myPrint(cmd_buff);
			}
			if ('e' == cmd_buff[0] && 'x' == cmd_buff[1] && 'i' == cmd_buff[2] && 't' == cmd_buff[3]){
				if (cmd_buff[4] == '\n'){
					exit(0);
				}
				char error_message[30] = "An error has occurred\n";
				write(STDOUT_FILENO, error_message, strlen(error_message));
				cmd_buff[0] = '\n';
			}
			int allspaces = 1;
			int i;
			for (i = 0; i < strlen(cmd_buff) - 1; i++){
				if (cmd_buff[i] != ' ' && cmd_buff[i] != '	' && cmd_buff[i] != '\n')
					allspaces = 0;
			}
			if (!allspaces && cmd_buff[0] != '\n'){
				reprompt = 1;
				myPrint(cmd_buff);
				stat = prepareCMD(cmd_buff);
			}
		}
	}
//	free(cmd_buff);
	return 0;
}

int validcarrots(char *str){
	//trim string of whitespace
	int carrotcounter = 0;
	int i;
	for (i = 0; i < strlen(str); i++){
		if (str[i] == '>'){
			carrotcounter++;
			if (i == (strlen(str) - 2))
				return 0;
		}
	}
	if (carrotcounter > 1 || '+' == str[strlen(str) - 2])
		return 0;
	return 1;
}

int prepareCMD(char *cmd_buff){
	int counter = 0;
	char **commands = (char **)malloc(sizeof(char*) * 64);
	char *token;
	token = strtok(cmd_buff, ";"); // break up commands by ;
	while (token != NULL){
		commands[counter] = token;
		counter++;
		token = strtok(NULL, ";");
	}
//	myPrint("6");
	int stat = 1;
	int j;
	char *ret;
	int outfilestats = 0;
	for (j = 0; j < counter; j++){
		if (!validcarrots(cmd_buff)){
			char error_message[30] = "An error has occurred\n";
			write(STDOUT_FILENO, error_message, strlen(error_message));
			continue;
		}
		ret = strpbrk(commands[j], ">+");
		if (ret != NULL && *ret == '>'){
//			myPrint("hi");
			outfilestats++;
			if (ret + 1 != NULL && *(ret + 1) == '+')
				outfilestats++;
//			myPrint("hi2");
		}
		int allspaces = 1;
		int i;
		for (i = 0; i < strlen(commands[j]) - 1; i++){
			if (commands[j][i] != ' ' && commands[j][i] != '	' && commands[j][i] != '\n')
				allspaces = 0;
		}
		if (!allspaces && commands[j][0] != '\n'){
			stat = parseAndRun(commands[j], outfilestats); // parse and run each line
		}
		if (stat == 2){
//			free(commands);
			continue;
		}
		if (stat == 0){
//			free(commands);
			return 1;
		}
		ret = NULL;
		outfilestats = 0;
	}
//	free(commands);
	return stat;
}
//////////////////////////////////////////////////////////////////////
int parseAndRun(char *str, int outfilestats){
	char **tokens = (char **)malloc(sizeof(char*) * 64);
	int counter = -1;
	char *token;
	token = strtok(str, delim);
	if (token != NULL)
		counter++;
	while (token != NULL){
		tokens[counter] = token;
		counter++;
		token = strtok(NULL, delim);
	}
	tokens[counter] = NULL;
	int k;
	for (k = 0; k < counter - 1; k++){
		if (strcmp(tokens[k], "/") == 0){
			char error_message[30] = "An error has occurred\n";
			write(STDOUT_FILENO, error_message, strlen(error_message));
			return 1;
		}
	}
	int saved_stdout = dup(1);
	int fd;
	char readfile[5000];
	readfile[0] = '&';
	if (outfilestats == 1){ // for > operation
		if (counter > 0)
			token = tokens[counter - 1];
		else{
			char error_message[30] = "An error has occurred\n";
			write(STDOUT_FILENO, error_message, strlen(error_message));
			return 2;
		}
		if (counter > 1){
			tokens[counter - 1] = NULL;
		}
		if (access(token, F_OK) != -1 || strcmp(tokens[0], "cd") == 0 || strcmp(tokens[0], "pwd") == 0 || strcmp(tokens[0], "exit") == 0){
			char error_message[30] = "An error has occurred\n";
			write(STDOUT_FILENO, error_message, strlen(error_message));
//			myPrint("Error 1");
			return 0;
		}
		fd = creat(token, 0644);
		if (fd == -1){
			char error_message[30] = "An error has occurred\n";
			write(STDOUT_FILENO, error_message, strlen(error_message));
			return 0;
		}
//		myPrint("TOKEN IS:......");
//		myPrint(token);
//		myPrint("12");
		dup2(fd, fileno(stdout));
		close(fd);
	}
	else if (outfilestats == 2){ // for >+ operaion
		if (counter > 0)
			token = tokens[counter - 1];
		else{
			char error_message[30] = "An error has occurred\n";
			write(STDOUT_FILENO, error_message, strlen(error_message));
			return 2;
		}
		if (counter > 1)
			tokens[counter - 1] = NULL;
		if (strcmp(tokens[0], "cd") == 0 || strcmp(tokens[0], "pwd") == 0 || strcmp(tokens[0], "exit") == 0){
			char error_message[30] = "An error has occurred\n";
			write(STDOUT_FILENO, error_message, strlen(error_message));
			return 0;
		}
		fd = open(token, O_RDWR | O_CREAT, 0666);
		dup2(fd, fileno(stdout));
		if (read(fd, readfile, 5000) < 0){
			char error_message[30] = "An error has occurred\n";
			write(STDOUT_FILENO, error_message, strlen(error_message));
			return 2;
		}
		close(fd);
		fd = open(token, O_RDWR | O_CREAT | O_TRUNC, 0666);
		dup2(fd, fileno(stdout));
		close(fd);
	}
	
	if (strlen(str) > 1 && strcmp(tokens[0], "cd") == 0){ // cd operation
//		myPrint("10");
		if (tokens[1] == NULL){ // if no path specified
//			myPrint("11");
			chdir(getenv("HOME"));
		}
		else{
//			myPrint("12");
			if (chdir(tokens[1]) == -1){
//				myPrint("13");
				char error_message[30] = "An error has occurred\n";
				write(STDOUT_FILENO, error_message, strlen(error_message));
//				write(1, "Error 3", 7);
			}
//			myPrint("14");
		}
//		myPrint("15");
	}
	else if (strlen(str) > 1 && strcmp(tokens[0], "pwd") == 0){ // pwd operaion
		if (tokens[1] != NULL){
			char error_message[30] = "An error has occurred\n";
			write(STDOUT_FILENO, error_message, strlen(error_message));
			return 1;
		}
		char *buf = (char *)malloc(sizeof(char*) * 128);
		getcwd(buf, (size_t)128);
		myPrint(buf);
		myPrint("\n");
	}
	else if (strlen(str) > 1){ // other program operation
//		myPrint("10");
		pid_t pid;
		int stat = 0;
		
		pid = fork();
		//Child
		if (pid == 0){
			if (execvp(tokens[0], tokens) == -1){
				char error_message[30] = "An error has occurred\n";
				write(STDOUT_FILENO, error_message, strlen(error_message));
//				write(1, "Error 4", 7);
			}
			exit(0);
		}
		else{
			do{
				waitpid(pid, &stat, WUNTRACED);
				// referenced manual page https://linux.die.net/man/2/waitpid
			} while(!WIFSIGNALED(stat) && !WIFEXITED(stat));
		}
	}
//	myPrint("10.5");
//	free(tokens);
	dup2(saved_stdout, 1);
//	myPrint("11");
	if (readfile[0] != '&'){ // if user used >+ (readfile string was initiated with & at beginning for testing)
		fd = open(token, O_WRONLY | O_APPEND);
		if (fd < 0)
			return 1;
		if (write(fd, readfile, strlen(readfile)) < 0){
			char error_message[30] = "An error has occurred\n";
			write(STDOUT_FILENO, error_message, strlen(error_message));
		}
	}
	return 1;
}
