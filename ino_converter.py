import os

def combine_source_files(directory, output_file):
    extensions = ('.ino', '.cpp', '.h')
    with open(output_file, 'w', encoding='utf-8') as outfile:
        for root, _, files in os.walk(directory):
            for file in files:
                if file.endswith(extensions):
                    file_path = os.path.join(root, file)
                    outfile.write(f"\n\n// === FILE: {file} ===\n\n")
                    with open(file_path, 'r', encoding='utf-8') as infile:
                        outfile.write(infile.read())
                        outfile.write('\n')

if __name__ == "__main__":
    source_directory = "path/to/your/source/folder"  # replace with your source folder path
    output_text_file = "combined_code.txt"
    combine_source_files(source_directory, output_text_file)
