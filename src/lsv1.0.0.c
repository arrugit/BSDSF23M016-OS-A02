#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <errno.h>
#include <getopt.h>

/* ---------- Function Prototypes ---------- */
void print_permissions(mode_t mode);
void list_simple(const char *path);
void list_long(const char *path);

/* ---------- Main Function ---------- */
int main(int argc, char *argv[]) {
    int opt;
    int long_listing = 0;
    char *path = ".";

    // Argument parsing
    while ((opt = getopt(argc, argv, "l")) != -1) {
        if (opt == 'l')
            long_listing = 1;
        else {
            fprintf(stderr, "Usage: %s [-l] [directory]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (optind < argc)
        path = argv[optind];

    // Execute mode
    if (long_listing)
        list_long(path);
    else
        list_simple(path);

    return 0;
}

/* ---------- Print Permissions in rwxrwxrwx Format ---------- */
void print_permissions(mode_t mode) {
    char perms[11];
    perms[0] = (S_ISDIR(mode)) ? 'd' :
               (S_ISLNK(mode)) ? 'l' :
               (S_ISCHR(mode)) ? 'c' :
               (S_ISBLK(mode)) ? 'b' :
               (S_ISFIFO(mode)) ? 'p' :
               (S_ISSOCK(mode)) ? 's' : '-';

    perms[1] = (mode & S_IRUSR) ? 'r' : '-';
    perms[2] = (mode & S_IWUSR) ? 'w' : '-';
    perms[3] = (mode & S_IXUSR) ? 'x' : '-';
    perms[4] = (mode & S_IRGRP) ? 'r' : '-';
    perms[5] = (mode & S_IWGRP) ? 'w' : '-';
    perms[6] = (mode & S_IXGRP) ? 'x' : '-';
    perms[7] = (mode & S_IROTH) ? 'r' : '-';
    perms[8] = (mode & S_IWOTH) ? 'w' : '-';
    perms[9] = (mode & S_IXOTH) ? 'x' : '-';
    perms[10] = '\0';

    printf("%s", perms);
}

/* ---------- Long Listing (-l Option) ---------- */
void list_long(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    struct stat info;
    char fullpath[1024];
    long total_blocks = 0;

    // First pass: total block count
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
        if (lstat(fullpath, &info) == 0)
            total_blocks += info.st_blocks;
    }

    printf("total %ld\n", total_blocks / 2);

    rewinddir(dir);

    // Second pass: detailed listing
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
        if (lstat(fullpath, &info) < 0) continue;

        // Permissions + Type
        print_permissions(info.st_mode);

        // Link count
        printf(" %2ld", (long)info.st_nlink);

        // Owner and group
        struct passwd *pw = getpwuid(info.st_uid);
        struct group  *gr = getgrgid(info.st_gid);
        printf(" %-8s %-8s",
               pw ? pw->pw_name : "unknown",
               gr ? gr->gr_name : "unknown");

        // Size
        printf(" %8ld", (long)info.st_size);

        // Time
        char timebuf[64];
        strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", localtime(&info.st_mtime));
        printf(" %s", timebuf);

        // File name
        printf(" %s\n", entry->d_name);
    }

    closedir(dir);
}

/* ---------- Simple Listing (Multi-Column Display) ---------- */
void list_simple(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    char *names[1000];
    int count = 0, maxlen = 0;

    // Collect non-hidden file names
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        names[count] = strdup(entry->d_name);
        int len = strlen(entry->d_name);
        if (len > maxlen) maxlen = len;
        count++;
    }
    closedir(dir);

    if (count == 0) return;

    // Get terminal width
    struct winsize w;
    int width = 80;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0)
        width = w.ws_col;

    int space = 2;
    int colwidth = maxlen + space;
    int cols = width / colwidth;
    if (cols < 1) cols = 1;
    int rows = (count + cols - 1) / cols;

    // Print in columns (down then across)
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int i = c * rows + r;
            if (i < count)
                printf("%-*s", colwidth, names[i]);
        }
        printf("\n");
    }

    for (int i = 0; i < count; i++)
        free(names[i]);
}
