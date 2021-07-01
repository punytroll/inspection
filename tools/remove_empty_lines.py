import os
import sys

def process_file(file_path):
    with open(file_path, "r") as file:
        lines = file.readlines()
    with open(file_path, "w") as file:
        for line in lines:
            if len(line.strip()) > 0:
                file.write(line)

for (root, directories, files) in os.walk(sys.argv[1]):
    directories.sort()
    files.sort()
    for file in files:
        process_file(os.path.join(root, file))
