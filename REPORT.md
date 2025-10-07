# REPORT.md
## Feature-5: ls-v1.4.0 – Alphabetical Sort

### Report Questions and Answers

---

### **Q1. Why is it necessary to read all directory entries into memory before you can sort them? What are the potential drawbacks of this approach for directories containing millions of files?**

**Answer:**
Sorting requires all filenames in memory at once to apply qsort or another sorting algorithm.

Drawbacks for very large directories:

Very high memory consumption.

Sorting overhead O(n log n).

Program may become slow or even fail if memory is insufficient

---

### **Q2. Explain the purpose and signature of the comparison function required by qsort(). How does it work, and why must it take const void * arguments?.**

**Answer:**
qsort is a generic sorting function, so it works with void pointers.
The comparison function tells qsort how to order elements. Its signature is:
'''c
int cmpfunc(const void *a, const void *b);


Inside, we cast back to the correct type (e.g., char **) and use strcmp() for strings.
const void * is required for type safety and generality, since qsort must work with any data type.

