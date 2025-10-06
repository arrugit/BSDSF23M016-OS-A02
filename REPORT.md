# REPORT.md

## Feature 2: ls-v1.1.0 – Complete Long Listing Format

### Question 1  
**Explain the general logic for printing items in a "down then across" columnar format. Why is a simple single loop through the list of filenames insufficient for this task?**

In a **“down then across”** columnar layout, filenames are printed top-to-bottom in each column and then left-to-right across columns (the same visual style used by the Linux `ls` command).

**General logic**
1. Determine the **maximum filename length** and the **terminal width**.
2. Compute the **number of columns** that fit:
   ```c
   colwidth = maxlen + space;
   cols = width / colwidth;
   if (cols < 1) cols = 1;
Compute the number of rows required to place all items:

rows = (count + cols - 1) / cols;


Print using two nested loops:

Outer loop iterates rows r = 0 .. rows-1.

Inner loop iterates columns c = 0 .. cols-1.

Use the index calculation:

index = c * rows + r;


to pick the correct filename from the linear list.

Why a single linear loop is insufficient

A single loop (for (i = 0; i < count; i++)) prints items in a single dimension (one row or one column) and cannot place items into a grid that balances vertically-first ordering.

The grid requires computing positions across two dimensions (row and column), so the index for each printed cell must be computed from both row and column indices — which necessitates nested loops and the index = c * rows + r formula.

The nested approach ensures neat alignment, balanced columns, and fills the terminal width properly.

### Question 2

**What is the purpose of the ioctl system call in this context? What would be the limitations of your program if you only used a fixed-width fallback (e.g., 80 columns) instead of detecting the terminal size?**

Purpose of ioctl

ioctl(STDOUT_FILENO, TIOCGWINSZ, &w); queries the terminal for its current size and fills a struct winsize (commonly w.ws_col for width).

The program uses w.ws_col to calculate how many columns of filenames will fit on the screen, allowing dynamic layout that adapts to the user’s terminal.

Limitations of using a fixed-width fallback (e.g., 80 columns)

Not adaptive: Output would not scale to larger or smaller terminal windows.

Wasted space on wide terminals: Wide terminals would show fewer columns than possible, leaving unused horizontal space.

Wrapping or misalignment on narrow terminals: On small terminals lines may wrap unexpectedly, breaking the column alignment and readability.

Poor user experience: The display would be less professional and less like the real ls, which adapts to terminal size.

Conclusion: Using ioctl makes the listing responsive and correctly aligned for the actual terminal size; a static fallback is simpler but leads to suboptimal, sometimes broken, output.
