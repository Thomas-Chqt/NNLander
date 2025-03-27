#!/usr/bin/env python3

import os
import re

# File extensions to process
TARGET_EXTENSIONS = (
    '.js', '.css', '.html', '.example', ".md", ".mmd", ".py", ".yaml", ".yml", ".sh", ".txt", ".h", ".cpp")

def strip_trailing_whitespace(file_path):
    """Remove trailing whitespaces from each line in a file."""
    try:
        with open(file_path, 'r', encoding='utf-8') as file:
            lines = file.readlines()

        # Check if any line has trailing whitespace
        has_trailing_whitespace = any(line.rstrip() != line.rstrip('\n') for line in lines)

        if has_trailing_whitespace:
            # Strip trailing whitespace from each line
            stripped_lines = [line.rstrip() + '\n' if line.endswith('\n') else line.rstrip() for line in lines]

            # Write the changes back to the file
            with open(file_path, 'w', encoding='utf-8') as file:
                file.writelines(stripped_lines)

            print(f"Stripped trailing whitespace from: {file_path}")
        else:
            print(f"No trailing whitespace in: {file_path}")
    except Exception as e:
        print(f"Error processing {file_path}: {e}")

def main():
    """Main function to traverse directories and process files."""
    # Start from the current directory
    root_dir = os.getcwd()

    file_count = 0
    processed_count = 0

    print(f"Scanning directory: {root_dir}")
    print(f"Looking for files with extensions: {', '.join(TARGET_EXTENSIONS)}")

    # Walk through all directories and files
    for dirpath, _, filenames in os.walk(root_dir):
        # Skip if it's node_modules
        if "node_modules" in dirpath:
            continue

        for filename in filenames:
            if filename.endswith(TARGET_EXTENSIONS):
                file_count += 1
                file_path = os.path.join(dirpath, filename)
                strip_trailing_whitespace(file_path)
                processed_count += 1

    print(f"\nSummary:")
    print(f"Found {file_count} applicable files")
    print(f"Processed {processed_count} files")

if __name__ == "__main__":
    main()
