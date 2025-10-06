#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <sys/ioctl.h>  

// ANSI color codes
#define RESET       "\033[0m"
#define BLUE        "\033[0;34m"
#define GREEN       "\033[0;32m"
#define RED         "\033[0;31m"
#define PINK        "\033[0;35m"
#define REVERSE     "\033[7m"

// display modes
#define DEFAULT_MODE 0
#define LONG_MODE    1
#define HORIZONTAL_MODE 2

// function to print permissions
void print_permissions(mode_t mode) {
    printf( (S_ISDIR(mode)) ? "d" :
            (S_ISLNK(mode)) ? "l" :
            (S_ISCHR(mode)) ? "c" :
            (S_ISBLK(mode)) ? "b" :
            (S_ISSOCK(mode))? "s" :
            (S_ISFIFO(mode))? "p" : "-");

    printf( (mode & S_IRUSR) ? "r" : "-");
    printf( (mode & S_IWUSR) ? "w" : "-");
    printf( (mode & S_IXUSR) ? "x" : "-");
    printf( (mode & S_IRGRP) ? "r" : "-");
    printf( (mode & S_IWGRP) ? "w" : "-");
    printf( (mode & S_IXGRP) ? "x" : "-");
    printf( (mode & S_IROTH) ? "r" : "-");
    printf( (mode & S_IWOTH) ? "w" : "-");
    printf( (mode & S_IXOTH) ? "x" : "-");
}

// color printing based on file type
void print_colored_name(const char *name, const char *path) {
    struct stat st;
    char fullpath[1024];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", path, name);
    lstat(fullpath, &st);

    const char *color = RESET;

    if (S_ISDIR(st.st_mode))
        color = BLUE;
    else if (S_ISLNK(st.st_mode))
        color = PINK;
    else if (S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode) || S_ISSOCK(st.st_mode))
        color = REVERSE;
    else if (st.st_mode & S_IXUSR)
        color = GREEN;
    else if (strstr(name, ".tar") || strstr(name, ".gz") || strstr(name, ".zip"))
        color = RED;

    printf("%s%s%s", color, name, RESET);
}

// comparison function for qsort (ASCII-based)
int compare_names(const void *a, const void *b) {
    const char *fa = *(const char **)a;
    const char *fb = *(const char **)b;
    return strcmp(fa, fb);  // case-sensitive ASCII order
}

// read directory and return filenames
char **read_dir(const char *path, int *count) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        exit(1);
    }

    struct dirent *entry;
    char **names = NULL;
    *count = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue; // skip hidden
        names = realloc(names, sizeof(char*) * (*count + 1));
        names[*count] = strdup(entry->d_name);
        (*count)++;
    }

    closedir(dir);

    qsort(names, *count, sizeof(char*), compare_names); // sort alphabetically (ASCII)
    return names;
}

// display in columns (vertical)
void display_default(char **names, int count, const char *path) {
    for (int i = 0; i < count; i++) {
        print_colored_name(names[i], path);
        printf("\n");
    }
}

// display horizontally (-x)
void display_horizontal(char **names, int count, const char *path) {
    struct winsize w;
    int term_width = 80;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0)
        term_width = w.ws_col;

    int maxlen = 0;
    for (int i = 0; i < count; i++) {
        int len = strlen(names[i]);
        if (len > maxlen) maxlen = len;
    }

    int cols = term_width / (maxlen + 2);
    if (cols < 1) cols = 1;

    for (int i = 0; i < count; i++) {
        print_colored_name(names[i], path);
        int len = strlen(names[i]);
        int padding = maxlen - len + 2;
        for (int j = 0; j < padding; j++) printf(" ");
        if ((i + 1) % cols == 0) printf("\n");
    }
    printf("\n");
}

// display long listing (-l)
void display_long(char **names, int count, const char *path) {
    struct stat st;
    for (int i = 0; i < count; i++) {
        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, names[i]);
        if (lstat(fullpath, &st) == -1) continue;

        print_permissions(st.st_mode);
        printf(" %2ld", st.st_nlink);

        struct passwd *pw = getpwuid(st.st_uid);
        struct group *gr = getgrgid(st.st_gid);
        printf(" %-8s %-8s %8ld ", 
            pw ? pw->pw_name : "?", 
            gr ? gr->gr_name : "?", 
            st.st_size);

        char timebuf[64];
        strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", localtime(&st.st_mtime));
        printf("%s ", timebuf);

        print_colored_name(names[i], path);
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    int opt;
    int mode = DEFAULT_MODE;
    const char *path = ".";

    while ((opt = getopt(argc, argv, "lx")) != -1) {
        switch (opt) {
            case 'l': mode = LONG_MODE; break;
            case 'x': mode = HORIZONTAL_MODE; break;
            default:  fprintf(stderr, "Usage: %s [-l | -x] [dir]\n", argv[0]); exit(1);
        }
    }

    if (optind < argc) path = argv[optind];

    int count;
    char **names = read_dir(path, &count);

    if (mode == LONG_MODE)
        display_long(names, count, path);
    else if (mode == HORIZONTAL_MODE)
        display_horizontal(names, count, path);
    else
        display_default(names, count, path);

    for (int i = 0; i < count; i++)
        free(names[i]);
    free(names);

    return 0;
}
