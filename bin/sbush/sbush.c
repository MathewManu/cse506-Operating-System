#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define CMD_UNKNOWN      0
#define CMD_CD           1
#define CMD_CWD          2
#define CMD_LS           3
#define CMD_EXIT         4

typedef struct piped_commands {
  char *commands[50];
} piped_commands;

struct linux_dirent64 {
  unsigned long  d_ino;    /* 64-bit inode number */
  unsigned long  d_off;    /* 64-bit offset to next structure */
  unsigned short d_reclen; /* Size of this dirent */
  unsigned char  d_type;   /* File type */
  char           d_name[]; /* Filename (null-terminated) */
};


char **m_environ;


int do_execute(char *cmd, char *cmd_path[], char *env[]); 


char *getenv(const char *name);
int setenv(char *name, char *value, int overwrite);
char *getcwd(char *buf, size_t size);
int  puts_nonewline(const char *s);

int  get_command(char *cmd);
void handle_cd(char *path);
void handle_cwd();
void handle_ls();
void execute_non_builtin(char *cmd, char *cmd_arg);
void execute_command_line(char *cmd);
void execute_commands(char *cmd, char *cmd_arg);
void read_from_file(int num_tokens, char *cmd_tokens[]);

char ps1_variable[256] = "sbush>";

int execute_piped_commands(int num_pipes, piped_commands *cmds);

void print_prompt() {
  puts(ps1_variable);
}

int getdents64(int fd, struct linux_dirent64 *dirp, int count) {
  return syscall(__NR_getdents64, fd, dirp, count);
}

int puts_nonewline(const char *s) {
  int ret;
  while (*s) {
    if ((ret = putchar(*s)) != *s)
      return EOF;
    s++;
  } 
  return 0;
}

int tokenize(char *arg, char *argv[], int max_tokens, char *sep); 

int find_path_and_exe(char *cmd, char *argv[], char *env[]) {

  char *slash = strstr(cmd, "/");
  /* commands like ls */
  if (slash == 0) {

    return do_execute(cmd, argv, env);

  }
  /*complete path: eg: /bin/ls */

  if(execve(cmd, argv, env) < 0) {
   return -1;
  }
  return -1;
}

int do_execute(char *cmd, char *argv[], char *env[]) {

  char *path_env = getenv("PATH");
  char *paths[50] = {0};
  char exe_path[255] = {0};

  int i, len, ret = 1;
  int num_paths  = tokenize(path_env + 5, paths, 50, ":");
  /* 
   * iterate and find out the path of cmd
   */
  for (i = 0; i < num_paths; i++) {
    len = strlen(paths[i]);

    strncpy(exe_path, paths[i], len);    
    strncpy(exe_path + len, "/", 1);
    strcpy(exe_path + len + 1, cmd);

    if(execve(exe_path, argv, env) < 0) {
    //  printf("command to try : %s\n", exe_path);
      continue;
    }	
  }
    if (i == num_paths)
	ret = -1; 
  return ret;
}

int tokenize(char *arg, char *argv[], int max_tokens, char *sep) {
  int i = 0;
  char *saveptr;
  char arr[255] = {0};
  strcpy(arr, arg);
  char *token = strtok_r(arr, sep, &saveptr);
  while (token != NULL && i < max_tokens - 1) {
    argv[i] = malloc(strlen(token) + 1);
    strcpy(argv[i], token);
    token = strtok_r(NULL, sep, &saveptr);
    i++;
  }

  argv[i] = NULL;
  return i;
}

void build_argv(char *input, char *arg, char *argv[]) {
  argv[0] = malloc(strlen(input) + 1);
  strcpy(argv[0], input);
  tokenize(arg, &argv[1], 49, " ");
}


void handle_cd(char *path) {
  
  int ret;
  ret = chdir(path);
  if (ret != 0) {
    puts_nonewline("sbush: cd: ");
    puts_nonewline(path); 
    puts(": No such file or directory"); 
  }
}

void handle_cwd() {
  char buff[1024] = {0};
  if (getcwd(buff, sizeof(buff)) != NULL)
    puts(buff);
}

void handle_ls() {

	char buff[1024] = {0};
	if (getcwd(buff, sizeof(buff)) == NULL)
		return;

	int fd;
	int ret;
	int i = 0;
	char buf[1024];
	struct linux_dirent64 *d_ent;
	fd = open(buff, O_RDONLY);
	ret = getdents64(fd, (struct linux_dirent64 *)buf, 1024);

	while (i < ret) {
		d_ent = (struct linux_dirent64 *) (buf + i);
		if ((d_ent->d_name)[0] != '.')
			puts(d_ent->d_name);

		i += d_ent->d_reclen;
	}

}

int get_command(char *cmd) {

  /* built-in */
  if (strcmp(cmd, "cd") == 0)
    return CMD_CD;

  /* built-in */
  else if (strcmp(cmd, "exit") == 0)
    return CMD_EXIT;

  else
    return CMD_UNKNOWN;
}

void get_path_string(char *cmd, char *path_value) {

  char *ptr = NULL;
  if ((strstr(cmd, "$PATH")) != NULL) {
    /*
     * 2 cases. eg: PATH=$PATH:/bin:/usr/bin 
     * $PATH anywhere else. beginning or somewhere else
     */ 
    char *path = strstr(cmd, "=");
    path++; 
    char *temp = path;
    int len = strlen(path);

    ptr = strstr(path, "$PATH");

    char *sys_env = getenv("PATH");

    /* $PATH in the beginning */
    if (temp == ptr) {
      strncpy(path_value, sys_env, strlen(sys_env));
      if (len - 6 > 0) { //6 len of $PATH:
        strcpy(path_value+strlen(sys_env), ptr+5);
      } 
    } else {
      /* $PATH anywhere else */
      strncpy(path_value, temp, ptr-temp); 
      strncpy(path_value + (ptr - temp), sys_env, strlen(sys_env));
      strcpy(path_value + (ptr - temp) + strlen(sys_env), ptr + 5);
    }

  } else {
    /* eg: PATH=/usr/bin */ 
    ptr = strstr(cmd, "=");
    strcpy(path_value, ptr + 1); 
  }
}


int check_if_path_cmd(char *cmd) {
  return strncmp(cmd, "PATH=", 5) == 0 ? 1 : 0; 
}

int check_if_ps1_cmd(char *cmd) {
  return strncmp(cmd, "PS1=", 4) == 0 ? 1 : 0; 
}

/*
 * if & is found, replace with '\0' and return true
 */ 
int update_if_bg_cmdarg(char *cmd_arg) {
  char *amp;
  int ret = 0;
  if ((amp = strstr(cmd_arg, "&")) != NULL) {
    *amp = '\0'; 
    ret = 1;
  }

  return ret; 
}

/* Not shell built-in commands, call exec */
void execute_non_builtin(char *cmd, char *cmd_arg) {
  pid_t pid;
  int i, status, bg_process = 0;
  char *argv[50] = {0};
  char path_value[1024] = {0} ; 
  
  /* PATH variable set */
  if (check_if_path_cmd(cmd)) {
    get_path_string(cmd, path_value); 
    setenv("PATH", path_value, 1);
    return;
  }
  /* PS1 variable set */ 
  else if (check_if_ps1_cmd(cmd)) {
    strcpy(ps1_variable, strstr(cmd, "=") + 1);
    return;
  }
  /* command & handling, true if & is found in the command */
  else if (update_if_bg_cmdarg(cmd_arg)) {
    bg_process = 1;
  }
 
  build_argv(cmd, cmd_arg, argv);

  pid = fork();
  if (pid == 0) {
     if (find_path_and_exe(cmd, argv, m_environ) < 0) {
      puts_nonewline(cmd);
      puts(": command not found");
      exit(1);
    }
  } else {
    if (pid < 0) {
      exit(1);
    }
    else {
      if (!bg_process)
        waitpid(-1, &status, 0);
    }
  }

  /* Freeing */
  i = 0;
  while (argv[i]) {
    free(argv[i]);
    argv[i] = NULL;
    i++;
  }
}

void execute_command_line(char *cmd) {
  int i;
  char *str, *token, *saveptr;
  for (i = 1, str = cmd; ; i++, str = NULL) {

    token = strtok_r(str, " ", &saveptr);
    if (token == NULL)
      break;

    if (i == 1 && token[0] != '#')
      execute_commands(token, saveptr);
  }
}

void execute_commands(char *cmd, char *cmd_arg) {

  int cmd_id = get_command(cmd);
  switch (cmd_id) {

    case CMD_CD:
      handle_cd(cmd_arg);
      break;

    case CMD_EXIT:
      exit(0);

    case CMD_UNKNOWN:
      execute_non_builtin(cmd, cmd_arg);
      break;

    default:
      puts_nonewline(cmd);
      puts(": command not found");
      break;
  }
}

/* Read from the file one line at a time and execute */
void read_from_file(int num_tokens, char *cmd_tokens[]) {

  int file = open(cmd_tokens[1], O_RDONLY);
  char code[1024] = {0};
  size_t n = 0;
  char c;

  if (file == -1)
    return;

  while (read(file, &c, 1) > 0)
  {
    code[n++] = (char) c;
    if (c == '\n') {
      code[n - 1] = '\0';
      execute_command_line(code);    
      n = 0;
    }
  }

  code[n] = '\0'; 
  execute_command_line(code);    
}

void handle_piped_commands(char *arg) {
  char *argv[50] = {0};
  int i = 0;
  int num_cmds  = tokenize(arg, &argv[0], 50, "|");
  int num_pipes = num_cmds - 1;
  piped_commands cmds[num_cmds];
  memset(cmds, 0, sizeof(cmds));

  while (i < num_cmds) {
    tokenize(argv[i], cmds[i].commands, 50, " ");
    i++;
  }
 
  execute_piped_commands(num_pipes, cmds);

  /* Freeing */
  i = 0;
  while (i < num_cmds) {
    int j = 0;
    while(cmds[i].commands[j]) {
      free(cmds[i].commands[j]);
      cmds[i].commands[j] = NULL;
      j++;
    }
    i++;
  }
  i = 0;
  while(argv[i]) {
    free(argv[i]);
    argv[i] = NULL;
    i++;
  }
}

void read_from_stdin() {
  int cnt;
  char *str, *saveptr, *token;
  char buff[1024] = {0};
  while (read(0, buff, sizeof(buff)) > 0) {

    cnt = 1;
    size_t buff_length = strlen(buff);
    if (buff_length != 0 && buff[buff_length - 1] == '\n') {

      buff_length--;
      buff[buff_length] = '\0';
    }

    if (strstr(buff, "|")) {
      handle_piped_commands(buff);
    }  else {
      str = buff;
      while (1) {
        token = strtok_r(str, " ", &saveptr);
        if (token == NULL)
          break;

        if (cnt == 1)
          execute_commands(token, saveptr);

        cnt++;
        str = NULL;
      }
    }

    print_prompt();

    memset(buff, 0, sizeof(buff));
  }
}

int process_start(int input_fd, int output_fd, piped_commands *cmds) {
  int status;
  pid_t pid = fork();
  if (pid == 0) {

    if (input_fd != 0) {
      dup2(input_fd, 0);
      close(input_fd);
    }

    if (output_fd != 1) {
      dup2(output_fd, 1);
      close(output_fd);
    }
    return find_path_and_exe(cmds->commands[0], cmds->commands, m_environ);

  } else {
    waitpid(-1, &status, 0);
  }

  return pid;
}

int execute_piped_commands(int num_pipes, piped_commands *cmds) {
  int i = 0;
  int input_fd = 0; /* stdin */
  int fds[2];
  int status;
  pid_t pid;

  while (i < num_pipes) {
    if (pipe(fds) != 0)
      return 1;

    process_start(input_fd, fds[1], cmds + i);
    close(fds[1]);

    input_fd = fds[0];
    i++;
  }

  /* last command */
  pid = fork();
  if (pid == 0) {

    dup2(input_fd, 0);
    close(input_fd);

    if (find_path_and_exe(cmds[i].commands[0], cmds[i].commands, m_environ)) {
      exit(1);
    }

  } else {
    waitpid(-1, &status, 0);
  }

  return 0;
}

char *getenv(const char *arg) {
  int i;
  for (i = 0; m_environ[i] !=0 ; i++) {
    if (strncmp(m_environ[i], "PATH=", 5) == 0) { 
      return m_environ[i];
    } 
  }
 return NULL;
}

int setenv(char *path_variable, char *value, int overwrite) {
  
  //eg: setenv("PATH", path_value, 1); 
  //overwrite variable is not used now.

  int i;
  int var_len = strlen(path_variable);
  int value_len = strlen(value);

  for (i = 0; m_environ[i] !=0 ; i++) {

    if (strncmp(m_environ[i], "PATH=", 5) == 0) { 
      /*
       * first free the value. Then allocate for new value
       */
      free(m_environ[i]);
      m_environ[i]= malloc(value_len + var_len + 2);//include size of "=" as well.

      /*
       * copy the complete value in 3 steps. eg: PATH=/usr/bin
       */
      strncpy(m_environ[i], path_variable, var_len);	
      strncpy(m_environ[i] + var_len, "=", 1);	
      strcpy(m_environ[i] + var_len + 1, value);

      return 0;
    }
  }
  return 1;
}

int main(int argc, char *argv[], char *envp[]) {

  m_environ = envp + argc + 1;
  if (argc > 1) {

    /* Case 1 : Non-interactive mode */
    read_from_file(argc, argv);

  } else {

    /* Case 2 : Interactive mode */
    print_prompt();
    read_from_stdin();
  }

  return 0;
}

