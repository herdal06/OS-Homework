// prompt için renkler

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

// Built-in komutlar için fonksiyon başlıkları
int lsh_cd(char **);
int lsh_help(char **);
int lsh_exit(char **);
int lsh_showpid(char **);

char **lsh_split_line(char *);
int lsh_launch(char **);
char *lsh_read_line(void);
void lsh_loop(void);
void printPrompt();