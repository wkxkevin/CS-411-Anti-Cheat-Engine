# CS-411-Anti-Cheat-Engine

This program analyzes student question responses for suspicious activity and potential academic integrity issues. It performs two main tasks:

1. **Identify suspicious response times** based on predefined thresholds.  
2. **Compute similarity (match) scores** between students’ submitted files to detect unusually high matches.

---

## **Features**

- Reads student question data from a CSV file.  
- Flags students with unusually fast or minimal attempts.  
- Loads student submission files from directories and calculates pairwise match scores.  
- Uses **multithreading** to speed up alignment computations.  
- Outputs:
  - `weird_times.txt` → suspicious completion times.  
  - `<question>_high_matches.txt` → high similarity response pairs and student frequency of being flagged.

---

## **Requirements**

- C++17 compatible compiler (`g++` recommended)  
- POSIX threads (`pthread`)  
- `csv.hpp` header for CSV reading (included in repository)  
- File system support (`<filesystem>`)

---

## **Building**

Compile using the provided `Makefile`:

```bash
make
```

This produces the executable:

```bash
./bin/exec
```

Clean the build with:

```bash
make clean
```

---

## **Usage**

```bash
./bin/exec <instance_questions_file.csv> <student_submissions_folder>
```

- `<instance_questions_file.csv>` → CSV containing student question data.  
- `<student_submissions_folder>` → folder containing student submission directories.
Note: Ensure compressed file is unzipped

Example:

```bash
./bin/exec student_data.csv submissions/
```

---

## **Output Files**

1. **`weird_times.txt`** – lists students with suspicious response times.  
2. **`<question>_high_matches.txt`** – for each question:  
   - Pairs of students with high match scores  
   - Frequency of students being flagged  

---

## **Configuration**

- **Match scoring** constants:

```cpp
const size_t ALPHA = 1;  // match score
const size_t BETA = 0;   // mismatch score
const size_t GAMMA = 0;  // gap score
const size_t NUM_THREADS = 4; // number of threads
const double THRES_MULT = 3;  // standard deviation multiplier for threshold
```

- Modify these values to tune sensitivity for detecting high match scores or suspicious times.

---

## **Notes**

- The program assumes **student submission directories** are named with the student's netID.  
- File naming for responses should follow the pattern used in the code to correctly extract the question name.  
- Large datasets may benefit from increasing `NUM_THREADS` to speed up processing.
