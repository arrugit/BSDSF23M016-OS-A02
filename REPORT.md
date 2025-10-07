# REPORT.md
## Feature-7: ls-v1.6.0 – Recursive Listing (-R Option)

### Report Questions and Answers

---

### **Q1. In a recursive function, what is a "base case"? In the context of your recursive ls, what is the base case that stops the recursion from continuing forever?**

**Answer:**
A base case is the condition that stops recursion.
For ls -R, the base case is when:

Entry is not a directory, or

Entry is "." or ".." (to avoid looping).

---

### **Explain why it is essential to construct a full path (e.g., "parent_dir/subdir") before making a recursive call. What would happen if you simply called do_ls("subdir") from within the do_ls("parent_dir") function call?**

**Answer:**
Full paths are required because opendir() and stat() need the correct location.

If only "subdir" is passed:

Program looks in the wrong working directory.

Leads to missing or incorrect results.

By using "parent_dir/subdir", recursion properly descends into nested directories.
