from itertools import zip_longest

files = [
    "sp25-hw1-sql-q1",
    "fa25-hw1-sql-q2",
    "sp25-hw1-sql-q3",
    "sp25-hw1-sql-q4",
    "fa25-hw1-sql-q5",
    "fa25-hw1-sql-q6",
    "sp25-hw1-sql-q7",
    "fa25-hw1-sql-q8",
    "sp25-hw1-sql-q9",
    "fa25-hw1-sql-q10",
    "sp25-hw1-sql-q11",
    "fa25-hw1-sql-q12",
    "fa25-hw1-sql-q13",
    "sp25-hw1-sql-q14",
    "sp25-hw1-sql-q15",
]

open("pairs_out.txt", "w").close()
for fname in files:
    pairlines1 = ""
    pairlines2 = ""
    pairlines3 = ""

    with open(fname + "_high_matches.txt") as f:
        lines = [line.strip() for line in f if line.strip()]

    i = lines.index("Flagged Response Pairs") + 1
    j = lines.index("NetID Frequency Map")

    for line in lines[i:j]:
        k, v = line.split(":")
        a, b = k.split("-")
        pairlines1 += a + "\t"
        pairlines2 += b + "\t"
        pairlines3 += v + "\t"

    # uncomment pairlines1,2 and 4 when pasting to official flagged submission google sheets
    with open("pairs_out.txt", "a") as out:
        out.write(pairlines1 + "\n")
        out.write(pairlines2 + "\n")
        out.write(pairlines3 + "\n")
        # out.write("\n")

table1 = []

for line in open("pairs_out.txt", "r"):
    table1.append(line.split("\t"))

print(len(table1))
