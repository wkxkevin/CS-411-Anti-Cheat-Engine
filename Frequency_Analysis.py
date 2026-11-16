from itertools import zip_longest

files = [
    "fa26-hw3-q4-ra-v2",
    "fa26-hw3-q4-ra-v3",
    "fa26-hw3-q5-ra-v1",
    "fa26-hw3-q5-ra-v2",
    "fa26-hw3-q5-ra-v3",
    "sp25-hw3-q6-block-based-join",
    "fa25-hw3-q6-block-based-join-v2",
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
