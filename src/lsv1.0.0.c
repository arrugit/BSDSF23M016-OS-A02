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
#include <errno.h>

int long_flag = 0;
int across_flag = 0;
int recursive_flag = 0;

void print_permissions(mode_t mode) {
    printf( (S_ISDIR(mode)) ? "d" : "-");
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

int compare_ascii(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

void display_long(const char *path, char **files, int count) {
    for (int i = 0; i < count; i++) {
        char fullpath[1024];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, files[i]);

        struct stat st;
        if (lstat(fullpath, &st) == -1) {
            perror("lstat");
            continue;
        }

        print_permissions(st.st_mode);
        printf(" %3ld ", (long)st.st_nlink);

        struct passwd *pw = getpwuid(st.st_uid);
        struct group  *gr = getgrgid(st.st_gid);
        printf("%-8s %-8s ", pw ? pw->pw_name : "?", gr ? gr->gr_name : "?");
        printf("%8ld ", (long)st.st_size);

        char timebuf[80];
        strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", localtime(&st.st_mtime));
        printf("%s ", timebuf);

        printf("%s\n", files[i]);
    }
}

void display_across(char **files, int count) {
    for (int i = 0; i < count; i++) {
        printf("%s  ", files[i]);
    }
    printf("\n");
}

void display_down(char **files, int count) {
    for (int i = 0; i < count; i++) {
        printf("%s\n", files[i]);
    }
}

void do_ls(const char *path) {
    DIR *dp;
    struct dirent *entry;

    dp = opendir(path);
    if (!dp) {
        perror(path);
        return;
    }

    printf("\n%s:\n", path);

    char **names = NULL;
    int count = 0;

    while ((entry = readdir(dp)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        names = realloc(names, sizeof(char*) * (count + 1));
        names[count] = strdup(entry->d_name);
        count++;
    }
    closedir(dp);

    qsort(names, count, sizeof(char*), compare_ascii);

    if (long_flag)
        display_long(path, names, count);
    else if (across_flag)
        display_across(names, count);
    else
        display_down(names, count);

    // Recursive logic if -R enabled
    if (recursive_flag) {
        for (int i = 0; i < count; i++) {
            char fullpath[1024];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", path, names[i]);

            struct stat st;
            if (lstat(fullpath, &st) == -1)
                continue;

            if (S_ISDIR(st.st_mode)) {
                do_ls(fullpath); // recursive call
            }
        }
    }

    for (int i = 0; i < count; i++)
        free(names[i]);
    free(names);
}

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "lxR")) != -1) {
        switch (opt) {
            case 'l': long_flag = 1; break;
            case 'x': across_flag = 1; break;
            case 'R': recursive_flag = 1; break;
            default:
                fprintf(stderr, "Usage: %s [-l] [-x] [-R] [directory]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    const char *path = (optind < argc) ? argv[optind] : ".";

    do_ls(path);
    return 0;
}
