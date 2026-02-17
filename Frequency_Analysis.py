from itertools import zip_longest
files = ["sp26-hw1-sql-q1_high_matches.txt", "sp26-hw1-sql-q2_high_matches.txt", "sp26-hw1-sql-q3_high_matches.txt", "sp26-hw1-sql-q4_high_matches.txt", "sp26-hw1-sql-q5_high_matches.txt", "sp26-hw1-sql-q6_high_matches.txt", "sp26-hw1-sql-q7_high_matches.txt", "sp26-hw1-sql-q8_high_matches.txt", "sp26-hw1-sql-q9_high_matches.txt", "sp26-hw1-sql-q10_high_matches.txt", "sp26-hw1-sql-q11_high_matches.txt", "sp26-hw1-sql-q12_high_matches.txt", "sp26-hw1-sql-q13_high_matches.txt", "sp26-hw1-sql-q14_high_matches.txt", "sp26-hw1-sql-q15_high_matches.txt"]
open("pairs_out.txt", "w").close()
open("freq_out.txt", "w").close()
for fname in files:
    pairlines1 = ""
    pairlines2 = ""
    pairlines3 = ""
    freqlines1 = ""
    freqlines2 = ""

    with open(fname) as f:
        lines = [line.strip() for line in f if line.strip()]

    i = lines.index("Flagged Response Pairs") + 1
    j = lines.index("NetID Frequency Map")

    for line in lines[i:j]:
        k, v = line.split(":")
        a, b = k.split("-")
        pairlines1 += a + "\t"
        pairlines2 += b + "\t"
        pairlines3 += v + "\t"

    with open("pairs_out.txt", "a") as out:
        out.write(pairlines1+"\n")
        out.write(pairlines2+"\n")
        out.write(pairlines3+"\n")

table1 = []

for line in open("pairs_out.txt", "r"):
    table1.append(line.split("\t"))

print(len(table1))