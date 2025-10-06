#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define MAX_FILES 1024

// ---------------- Permissions Printer ----------------
void print_permissions(mode_t mode) {
    char perms[11] = "----------";

    if (S_ISDIR(mode)) perms[0] = 'd';
    else if (S_ISLNK(mode)) perms[0] = 'l';
    else if (S_ISCHR(mode)) perms[0] = 'c';
    else if (S_ISBLK(mode)) perms[0] = 'b';
    else if (S_ISFIFO(mode)) perms[0] = 'p';
    else if (S_ISSOCK(mode)) perms[0] = 's';

    if (mode & S_IRUSR) perms[1] = 'r';
    if (mode & S_IWUSR) perms[2] = 'w';
    if (mode & S_IXUSR) perms[3] = 'x';
    if (mode & S_IRGRP) perms[4] = 'r';
    if (mode & S_IWGRP) perms[5] = 'w';
    if (mode & S_IXGRP) perms[6] = 'x';
    if (mode & S_IROTH) perms[7] = 'r';
    if (mode & S_IWOTH) perms[8] = 'w';
    if (mode & S_IXOTH) perms[9] = 'x';

    printf("%s", perms);
}

// ---------------- Sorting Helper for qsort ----------------
int cmp_names(const void *a, const void *b) {
    const char *na = *(const char **)a;
    const char *nb = *(const char **)b;
    return strcmp(na, nb);
}

// ---------------- Read and Sort Filenames ----------------
int read_directory(const char *path, char ***names_out) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return 0;
    }

    struct dirent *entry;
    char **names = malloc(MAX_FILES * sizeof(char *));
    int count = 0;

    while ((entry = readdir(dir)) != NULL && count < MAX_FILES) {
        if (entry->d_name[0] == '.') continue; // skip hidden
        names[count++] = strdup(entry->d_name);
    }
    closedir(dir);

    qsort(names, count, sizeof(char *), cmp_names); // alphabetical sort
    *names_out = names;
    return count;
}

// ---------------- Long Listing ----------------
void print_long(const char *path, char **names, int count) {
    struct stat st;
    long total = 0;
    char fullpath[1024];

    // calculate total
    for (int i = 0; i < count; i++) {
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, names[i]);
        if (lstat(fullpath, &st) == 0)
            total += st.st_blocks;
    }
    printf("total %ld\n", total / 2);

    for (int i = 0; i < count; i++) {
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, names[i]);
        if (lstat(fullpath, &st) < 0) continue;

        print_permissions(st.st_mode);
        printf(" %2ld", (long)st.st_nlink);

        struct passwd *pw = getpwuid(st.st_uid);
        struct group *gr = getgrgid(st.st_gid);
        printf(" %-8s %-8s", pw ? pw->pw_name : "?", gr ? gr->gr_name : "?");

        printf(" %8ld", (long)st.st_size);

        char timebuf[64];
        strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", localtime(&st.st_mtime));
        printf(" %s %s\n", timebuf, names[i]);
    }
}

// ---------------- Default Down-Then-Across Display ----------------
void print_down_across(char **names, int count) {
    struct winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    int width = ws.ws_col ? ws.ws_col : 80;

    int maxlen = 0;
    for (int i = 0; i < count; i++) {
        int len = strlen(names[i]);
        if (len > maxlen) maxlen = len;
    }
    int spacing = 2;
    int cols = width / (maxlen + spacing);
    if (cols < 1) cols = 1;
    int rows = (count + cols - 1) / cols;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int idx = c * rows + r;
            if (idx < count)
                printf("%-*s", maxlen + spacing, names[idx]);
        }
        printf("\n");
    }
}

// ---------------- Horizontal Display (-x) ----------------
void print_horizontal(char **names, int count) {
    struct winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    int width = ws.ws_col ? ws.ws_col : 80;

    int maxlen = 0;
    for (int i = 0; i < count; i++) {
        int len = strlen(names[i]);
        if (len > maxlen) maxlen = len;
    }
    int spacing = 2;
    int colwidth = maxlen + spacing;
    int curr = 0;

    for (int i = 0; i < count; i++) {
        int next = curr + colwidth;
        if (next > width) {
            printf("\n");
            curr = 0;
        }
        printf("%-*s", colwidth, names[i]);
        curr += colwidth;
    }
    printf("\n");
}

// ---------------- Main ----------------
int main(int argc, char *argv[]) {
    int opt_long = 0, opt_horizontal = 0;
    const char *path = ".";

    // Argument parsing
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0)
            opt_long = 1;
        else if (strcmp(argv[i], "-x") == 0)
            opt_horizontal = 1;
        else
            path = argv[i];
    }

    char **names;
    int count = read_directory(path, &names);
    if (count == 0) return 0;

    if (opt_long)
        print_long(path, names, count);
    else if (opt_horizontal)
        print_horizontal(names, count);
    else
        print_down_across(names, count);

    for (int i = 0; i < count; i++)
        free(names[i]);
    free(names);

    return 0;
}
