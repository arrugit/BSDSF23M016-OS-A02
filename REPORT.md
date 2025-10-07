# REPORT.md
## Feature-4: ls-v1.3.0 – Horizontal Column Display (-x Option)

### Report Questions and Answers

---

### **Q1. Compare the implementation complexity of the "down then across" (vertical) printing logic versus the "across" (horizontal) printing logic. Which one requires more pre-calculation and why?**

**Answer:**
Down then across is more complex since it requires pre-calculating both rows and columns, and iterating by skipping with num_rows.

Across is simpler: it just tracks horizontal width and wraps when it exceeds terminal width.

Thus, vertical layout requires more pre-calculation.

---

### **Q2. Describe the strategy you used in your code to manage the different display modes (-l, -x, and default). How did your program decide which function to call for printing?**

**Answer:**
I used a flag/enum variable to track the display mode:

If -l → call long listing function.

If -x → call horizontal display function.

If no option → call default vertical display function.

This ensures exactly one function is called depending on the mode.
