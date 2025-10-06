#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

// ---------- Function to print permissions ----------
void print_permissions(mode_t mode) {
    char perms[11] = "----------";

    if (S_ISDIR(mode)) perms[0] = 'd';
    else if (S_ISLNK(mode)) perms[0] = 'l';

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

// ---------- Function to print in long listing (-l) ----------
void print_long_format(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    struct stat info;
    char fullpath[1024];
    long total_blocks = 0;

    // First pass: count total blocks
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
        if (lstat(fullpath, &info) == 0)
            total_blocks += info.st_blocks;
    }

    printf("total %ld\n", total_blocks / 2);

    rewinddir(dir); // go back to start

    // Second pass: print file details
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        if (lstat(fullpath, &info) == -1) continue;

        print_permissions(info.st_mode);
        printf(" %2ld", (long)info.st_nlink);

        struct passwd *pw = getpwuid(info.st_uid);
        struct group *gr = getgrgid(info.st_gid);
        printf(" %-8s %-8s", pw ? pw->pw_name : "?", gr ? gr->gr_name : "?");

        printf(" %8ld", (long)info.st_size);

        char timebuf[64];
        strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", localtime(&info.st_mtime));
        printf(" %s %s\n", timebuf, entry->d_name);
    }

    closedir(dir);
}

// ---------- Default (down then across) ----------
void print_default(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        printf("%s\n", entry->d_name);
    }

    closedir(dir);
}

// ---------- Horizontal across (-x) ----------
void print_horizontal(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    int count = 0;
    char *names[512];

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        names[count++] = strdup(entry->d_name);
    }

    for (int i = 0; i < count; i++) {
        printf("%-15s", names[i]);
        if ((i + 1) % 5 == 0) printf("\n");  // 5 columns per line
        free(names[i]);
    }
    printf("\n");

    closedir(dir);
}

// ---------- Main ----------
int main(int argc, char *argv[]) {
    const char *path = ".";
    int mode = 0; // 0 = default, 1 = -l, 2 = -x

    if (argc > 1) {
        if (strcmp(argv[1], "-l") == 0) mode = 1;
        else if (strcmp(argv[1], "-x") == 0) mode = 2;
        else path = argv[1];
    }

    switch (mode) {
        case 1: print_long_format(path); break;
        case 2: print_horizontal(path); break;
        default: print_default(path); break;
    }

    return 0;
}
