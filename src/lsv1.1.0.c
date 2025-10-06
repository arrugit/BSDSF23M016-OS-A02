#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <errno.h>

void print_permissions(mode_t mode);
void print_simple(const char *path);
void print_long_format(const char *path);

int main(int argc, char *argv[]) {
    int opt;
    int long_listing = 0;
    char *path = "."; // Default directory

    // ---------- Argument Parsing ----------
    while ((opt = getopt(argc, argv, "l")) != -1) {
        switch (opt) {
            case 'l':
                long_listing = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-l] [directory]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Remaining argument (if provided) = path
    if (optind < argc)
        path = argv[optind];

    // ---------- Execute Appropriate Listing ----------
    if (long_listing)
        print_long_format(path);
    else
        print_simple(path);

    return 0;
}

/*---------------------------------------------
 * Simple ls listing (v1.0.0 style)
 *--------------------------------------------*/
void print_simple(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        // skip hidden files
        if (entry->d_name[0] == '.')
            continue;
        printf("%s  ", entry->d_name);
    }
    printf("\n");
    closedir(dir);
}

/*---------------------------------------------
 * Print permissions in rwxrwxrwx format
 *--------------------------------------------*/
void print_permissions(mode_t mode) {
    char str[10];
    strcpy(str, "---------");

    // owner permissions
    if (mode & S_IRUSR) str[0] = 'r';
    if (mode & S_IWUSR) str[1] = 'w';
    if (mode & S_IXUSR) str[2] = 'x';

    // group permissions
    if (mode & S_IRGRP) str[3] = 'r';
    if (mode & S_IWGRP) str[4] = 'w';
    if (mode & S_IXGRP) str[5] = 'x';

    // others permissions
    if (mode & S_IROTH) str[6] = 'r';
    if (mode & S_IWOTH) str[7] = 'w';
    if (mode & S_IXOTH) str[8] = 'x';

    // special permissions
    if (mode & S_ISUID) str[2] = 's';
    if (mode & S_ISGID) str[5] = 's';
    if (mode & S_ISVTX) str[8] = 't';

    printf("%s", str);
}

/*---------------------------------------------
 * Long listing (-l) format
 *--------------------------------------------*/
void print_long_format(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    struct stat buf;
    char fullpath[1024];
    long total_blocks = 0;

    // ---------- First Pass: calculate total ----------
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.')
            continue;

        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        if (lstat(fullpath, &buf) == 0)
            total_blocks += buf.st_blocks;  // st_blocks in 512-byte units
    }

    printf("total %ld\n", total_blocks / 2); // convert to 1K blocks

    // ---------- Second Pass: print details ----------
    rewinddir(dir);

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.')
            continue;

        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
        if (lstat(fullpath, &buf) < 0) {
            perror("lstat");
            continue;
        }

        // 1. File type
        char type;
        if (S_ISREG(buf.st_mode)) type = '-';
        else if (S_ISDIR(buf.st_mode)) type = 'd';
        else if (S_ISLNK(buf.st_mode)) type = 'l';
        else if (S_ISCHR(buf.st_mode)) type = 'c';
        else if (S_ISBLK(buf.st_mode)) type = 'b';
        else if (S_ISFIFO(buf.st_mode)) type = 'p';
        else if (S_ISSOCK(buf.st_mode)) type = 's';
        else type = '?';

        // 2. Permissions
        printf("%c", type);
        print_permissions(buf.st_mode);

        // 3. Link count
        printf(" %2ld", (long)buf.st_nlink);

        // 4. User and Group
        struct passwd *pwd = getpwuid(buf.st_uid);
        struct group  *grp = getgrgid(buf.st_gid);
        printf(" %-8s %-8s",
               pwd ? pwd->pw_name : "unknown",
               grp ? grp->gr_name : "unknown");

        // 5. File size
        printf(" %8ld", (long)buf.st_size);

        // 6. Modification time
        char timebuf[64];
        strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", localtime(&buf.st_mtime));
        printf(" %s", timebuf);

        // 7. File name
        printf(" %s\n", entry->d_name);
    }

    closedir(dir);
}
