# REPORT.md
## Feature-2: ls-v1.1.0 – Complete Long Listing Format

### Report Questions and Answers

---

### **Q1. What is the crucial difference between `stat()` and `lstat()`? In the context of the `ls` command, when is it more appropriate to use `lstat()`?**

**Answer:**
The main difference between `stat()` and `lstat()` is how they handle symbolic links.

- **`stat()`** follows a symbolic link and returns information about the **target file** that the link points to.  
- **`lstat()`** does **not** follow the symbolic link. Instead, it returns information about the **link itself** (for example, its permissions, owner, and where it points).

In the context of the `ls` command, it is more appropriate to use **`lstat()`** because when we list files with the `-l` option, we need to show symbolic links as links — displaying both their metadata and their target (e.g., `linkname -> targetfile`).  
If we used `stat()`, the program would show information about the target file instead of the link itself, which would be incorrect for `ls -l`.

---

### **Q2. The `st_mode` field in `struct stat` is an integer that contains both the file type (e.g., regular file, directory) and the permission bits. Explain how you can use bitwise operators (like `&`) and predefined macros (like `S_IFDIR` or `S_IRUSR`) to extract this information.**

**Answer:**
The `st_mode` field uses **bit flags** to store information about both the **file type** and the **permissions**.  
We can use **bitwise AND (`&`)** with predefined macros to test which bits are set.

#### 1. Extracting File Type
The upper bits of `st_mode` represent the file type.  
You can check them using the mask `S_IFMT` together with macros like:
```c
if ((st_mode & S_IFMT) == S_IFDIR)
    printf("This is a directory.\n");

if ((st_mode & S_IFMT) == S_IFREG)
    printf("This is a regular file.\n");

