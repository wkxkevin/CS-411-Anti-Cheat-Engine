import os
import shutil
import sys


def restructure(quiz_dir):
    for filename in os.listdir(quiz_dir):
        full_path = os.path.join(quiz_dir, filename)
        if not os.path.isfile(full_path):
            continue

        # Student email is everything before the first _
        first_underscore = filename.find("_")
        if first_underscore == -1:
            print(f"[SKIP] Can't parse: {filename}")
            continue

        student = filename[:first_underscore]  # e.g. aaronx2@illinois.edu
        student_dir = os.path.join(quiz_dir, student)
        os.makedirs(student_dir, exist_ok=True)

        dest = os.path.join(student_dir, filename)
        shutil.move(full_path, dest)
        print(f"[MOVED] {filename} -> {student_dir}/")


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python restructure.py <quiz_folder>")
        sys.exit(1)
    restructure(sys.argv[1])
