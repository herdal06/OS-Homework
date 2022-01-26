#include <stdio.h> // printf, fprintf
#include <stdlib.h> // malloc, exit, execvp
#include <string.h> // strcmp, strtok
#include <sys/wait.h> // waitpid
#include <unistd.h> // fork, chdir, exec, pid_t
#include "shell.h"

static char* currentDirectory;
// built-in komutlar için fonksiyon başlıkları
int lsh_cd(char **args);
int lsh_showpid(char **args);
int lsh_exit(char **args);


void PromptBas() {
	char hostn[1204] = "";
	gethostname(hostn, sizeof(hostn));
  // getcwd ile geçerli çalışma dizinini aldık.
	printf(YEL "%s@%s:"RESET RED "%s > " RESET, getenv("LOGNAME"), hostn, getcwd(currentDirectory, 1024));
}

// built-in komutları içeren dizi
char *builtin_str[] = {"cd", "showpid", "exit"};

int (*builtin_func[]) (char **) = {
  &lsh_cd,
  &lsh_showpid,
  &lsh_exit
};
// built-in komut sayısı
int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

//Built-in fonksiyonlar

int lsh_cd(char **args) {
  // args[0] -> cd
  if (args[1] == NULL) {
    fprintf(stderr, "lsh: Beklenmeyen argüman \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("lsh");
    }
  }
  return 1;
}

int lsh_showpid(char **args) {
  int i;
	int status;
	for (i=0;i<5;i++) { // 5 tane yavru process'in pid'sini yazdıracak olan döngü
		int pid=fork();
		if(pid==0) { // yavru process
			printf("%d\n",getpid()); // yavru process'in pid'si yazdırılıyor.
			exit(0);
		}
		else if(pid>0){ // ebeveyn process
			// waitpid
			pid = waitpid(pid, &status, 0); // yavru process'in işini beklemek için
		}
		else { // pid<0

		}
  }
  return 1; // programın sonlanmaması için
}

// exit komutuyla shell'e exit yazdırılıp uygulama sonlandırılıyor.
int lsh_exit(char **args) {
	int status;
	while (!waitpid(-1,&status,WNOHANG)){} // WNOHANG-> zombie process oluşup oluşmadığını kontrol etmek için
  printf("exit\n"); 
	exit(0); // programın sonlanması için.
}

// programı başlatmaya yarayan metod
int lsh_launch(char **args) {
  pid_t pid;
  int status;
  // kabuğu başlatmak için forkladık.
  pid = fork();
  if (pid == 0) { // Yavru process
    if (execvp(args[0], args) == -1) { 
      fprintf(stderr, "Hata : Komut icra edilemiyor.\n");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {// Forklanamadi.
    perror("lsh");
  } else { // fork düzgün çalıştıysa
    // Ebeveyn process
    do {
      waitpid(pid, &status, WUNTRACED); // Ana shell bekletiliyor. Yavru process'in görevini tamamlaması bekleniyor.
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1; // uygulamanın devam edebilmesi için 1 dönmeli.
}

int lsh_execute(char **args)
{
  int i; // döngü değişkeni

  if (args[0] == NULL) { // boş komut girildiyse
    return 1; // uygulamadan çıkmaz
  }
  // tanımladığımız built-in komut sayısı kadar dönecek olan döngü
  for (i = 0; i < lsh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) { // shell'den gelen komut ile oluşturduğumuz dizi eşleşiyor mu kontrolü
      return (*builtin_func[i])(args);
    }
  }
  // shell'in terminate edilmesi durumunda 0 dönecek ve process sonlanacak. 1 dönmesi halinde shell çalışmaya devam edecek.
  return lsh_launch(args);
}

// stdin input satırını stdin'den oku.

#define LSH_RL_BUFSIZE 80
char *lsh_read_line(void)
{
  int bufsize = LSH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "lsh: bellek tahsis edilemedi...\n");
    exit(EXIT_FAILURE);
  }
  while (1) {
    // karakteri oku
    c = getchar();

    if (c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;
  }
}
#define LSH_TOK_BUFSIZE 80
#define LSH_TOK_DELIM " \t\r\n\a"
// satırı token'lara bölen metod
char **lsh_split_line(char *line)
{
  int bufsize = LSH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "lsh: bellek tahsis edilemedi...\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, LSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += LSH_TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
		free(tokens_backup);
        fprintf(stderr, "lsh: bellek tahsis edilemedi...\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, LSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}
 
int main(int argc, char **argv)
{
  // lsh_loop();
  // inputu alır ve çalıştırır
  char *line;
  char **args;
  int status;
  // Kullanıcı programı bitirene kadar sürecek olan sonsuz döngü
  do {
    PromptBas();
    line = lsh_read_line();
    args = lsh_split_line(line);
    status = lsh_execute(args);

    // bellekte çöp oluşmaması için belleğe iade ediliyor.
    free(line);
    free(args);
  } while (status);

  return EXIT_SUCCESS;
}
