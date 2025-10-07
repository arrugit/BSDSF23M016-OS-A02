# REPORT.md
## Feature-6: ls-v1.5.0 – Colorized Output

### Report Questions and Answers

---

### **Q1. How do ANSI escape codes work to produce color in a standard Linux terminal? Show the specific code sequence for printing text in green.**

**Answer:**
ANSI escape codes are special sequences interpreted by the terminal to change appearance. General format:

\033[<attributes>;<color_code>m


Example for green text:

printf("\033[0;32mHello\033[0m\n");


\033 → escape character.

[0;32m → green color.

[0m → reset.

---

### **Q2. To color an executable file, you need to check its permission bits. Explain which bits in the st_mode field you need to check to determine if a file is executable by the owner, group, or others..**

**Answer:**
Executable bits in st_mode:

S_IXUSR → executable by owner.

S_IXGRP → executable by group.

S_IXOTH → executable by others.

Check like:


```c

if (st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
    // file is executable
}

