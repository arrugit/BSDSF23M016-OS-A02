# REPORT.md

---

## Feature-2: ls-v1.1.0 – Complete Long Listing Format

### Report Questions and Answers

#### Q1. What is the crucial difference between `stat()` and `lstat()`? In the context of the `ls` command, when is it more appropriate to use `lstat()`?

**Answer:**  
The main difference between `stat()` and `lstat()` is how they handle symbolic links:

- `stat()` follows a symbolic link and returns information about the target file that the link points to.  
- `lstat()` does not follow the symbolic link. Instead, it returns information about the link itself (for example, its permissions, owner, and where it points).

In the context of the `ls` command, it is more appropriate to use `lstat()` because when we list files with the `-l` option, we need to show symbolic links as links — displaying both their metadata and their target (e.g., `linkname -> targetfile`).  
If we used `stat()`, the program would show information about the target file instead of the link itself, which would be incorrect for `ls -l`.

---

#### Q2. The `st_mode` field in `struct stat` is an integer that contains both the file type (e.g., regular file, directory) and the permission bits. Explain how you can use bitwise operators (like `&`) and predefined macros (like `S_IFDIR` or `S_IRUSR`) to extract this information.

**Answer:**  
The `st_mode` field uses bit flags to store information about both the file type and the permissions.  
We can use bitwise AND (`&`) with predefined macros to test which bits are set.

---

##### Extracting File Type
The upper bits of `st_mode` represent the file type.  
You can check them using the mask `S_IFMT` together with macros like:

```c
if ((st_mode & S_IFMT) == S_IFDIR)
    printf("This is a directory.\n");

if ((st_mode & S_IFMT) == S_IFREG)
    printf("This is a regular file.\n");
Extracting Permissions
Permission bits can be tested directly:

c
Copy code
if (st_mode & S_IRUSR) printf("Readable by owner\n");
if (st_mode & S_IWUSR) printf("Writable by owner\n");
if (st_mode & S_IXUSR) printf("Executable by owner\n");


Feature-3: ls-v1.2.0 – Column Display (Down Then Across)
Report Questions and Answers
Q1. Explain the general logic for printing items in a "down then across" columnar format. Why is a simple single loop through the list of filenames insufficient for this task?
Answer:
In a "down then across" layout, filenames are arranged into multiple columns, filling top-to-bottom first before moving left-to-right.

Calculate the number of rows and columns based on terminal width and longest filename.

For each row, print items spaced into columns by skipping over num_rows entries.

Example:

Row 0 → files[0], files[num_rows], files[2*num_rows], ...

Row 1 → files[1], files[1+num_rows], ...

A simple sequential loop prints left-to-right only, which does not achieve the down-then-across format.

Q2. What is the purpose of the ioctl system call in this context? What would be the limitations of your program if you only used a fixed-width fallback (e.g., 80 columns) instead of detecting the terminal size?
Answer:
ioctl with TIOCGWINSZ detects the current terminal’s width in characters. This allows the program to adjust the number of columns dynamically.

If only a fixed width (like 80 columns) is used:

On wider terminals → wasted space.

On narrower terminals → wrapping or overlapping text.

Detecting actual terminal size ensures correct formatting on all screens.

Feature-4: ls-v1.3.0 – Horizontal Column Display (-x Option)
Report Questions and Answers
Q1. Compare the implementation complexity of the "down then across" (vertical) printing logic versus the "across" (horizontal) printing logic. Which one requires more pre-calculation and why?
Answer:

Down then across is more complex since it requires pre-calculating both rows and columns, and iterating by skipping with num_rows.

Across is simpler: it just tracks horizontal width and wraps when it exceeds terminal width.

Thus, vertical layout requires more pre-calculation.

Q2. Describe the strategy you used in your code to manage the different display modes (-l, -x, and default). How did your program decide which function to call for printing?
Answer:
I used a flag/enum variable to track the display mode:

If -l → call long listing function.

If -x → call horizontal display function.

If no option → call default vertical display function.

This ensures exactly one function is called depending on the mode.

Feature-5: ls-v1.4.0 – Alphabetical Sort
Report Questions and Answers
Q1. Why is it necessary to read all directory entries into memory before you can sort them? What are the potential drawbacks of this approach for directories containing millions of files?
Answer:
Sorting requires all filenames in memory at once to apply qsort or another sorting algorithm.

Drawbacks for very large directories:

Very high memory consumption.

Sorting overhead O(n log n).

Program may become slow or even fail if memory is insufficient.

Q2. Explain the purpose and signature of the comparison function required by qsort(). How does it work, and why must it take const void * arguments?
Answer:
qsort is a generic sorting function, so it works with void pointers.
The comparison function tells qsort how to order elements. Its signature is:

c
Copy code
int cmpfunc(const void *a, const void *b);
Inside, we cast back to the correct type (e.g., char **) and use strcmp() for strings.
const void * is required for type safety and generality, since qsort must work with any data type.

Feature-6: ls-v1.5.0 – Colorized Output
Report Questions and Answers
Q1. How do ANSI escape codes work to produce color in a standard Linux terminal? Show the specific code sequence for printing text in green.
Answer:
ANSI escape codes are special sequences interpreted by the terminal to change appearance. General format:

php-template
Copy code
\033[<attributes>;<color_code>m
Example for green text:

c
Copy code
printf("\033[0;32mHello\033[0m\n");
\033 → escape character.

[0;32m → green color.

[0m → reset.

Q2. To color an executable file, you need to check its permission bits. Explain which bits in the st_mode field you need to check to determine if a file is executable by the owner, group, or others.
Answer:
Executable bits in st_mode:

S_IXUSR → executable by owner.

S_IXGRP → executable by group.

S_IXOTH → executable by others.

Check like:

c
Copy code
if (st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
    // file is executable
}
Feature-7: ls-v1.6.0 – Recursive Listing (-R Option)
Report Questions and Answers
Q1. In a recursive function, what is a "base case"? In the context of your recursive ls, what is the base case that stops the recursion from continuing forever?
Answer:
A base case is the condition that stops recursion.
For ls -R, the base case is when:

Entry is not a directory, or

Entry is "." or ".." (to avoid looping).

Q2. Explain why it is essential to construct a full path (e.g., "parent_dir/subdir") before making a recursive call. What would happen if you simply called do_ls("subdir") from within the do_ls("parent_dir") function call?
Answer:
Full paths are required because opendir() and stat() need the correct location.

If only "subdir" is passed:

Program looks in the wrong working directory.

Leads to missing or incorrect results.

By using "parent_dir/subdir", recursion properly descends into nested directories.
