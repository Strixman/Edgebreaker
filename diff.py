import sys

def find_first_difference(file1, file2):
    # Read both files completely
    with open(file1, 'r', encoding='utf-8') as f1:
        text1 = f1.read()
    with open(file2, 'r', encoding='utf-8') as f2:
        text2 = f2.read()

    # Determine the length to iterate (up to the length of the shortest text)
    min_len = min(len(text1), len(text2))
    diff_index = None

    # Find first index where the two texts differ
    for i in range(min_len):
        if text1[i] != text2[i]:
            diff_index = i
            break

    # If no difference found in common length but texts are of different size
    if diff_index is None:
        if len(text1) != len(text2):
            diff_index = min_len  # first difference: one text ended
        else:
            return None, None, None  # Files are identical

    # Get 10 characters of context before and after the difference
    start = max(diff_index - 30, 0)
    # We want 10 characters after the diff_index, including the different character itself.
    # Thus, we end at diff_index + 10 + 1 to include the character at diff_index.
    end1 = min(diff_index + 31, len(text1))
    end2 = min(diff_index + 31, len(text2))

    context1 = text1[start:end1]
    context2 = text2[start:end2]

    return diff_index, context1, context2

def main():
    if len(sys.argv) != 3:
        print("Usage: python script.py file1 file2")
        sys.exit(1)

    file1 = sys.argv[1]
    file2 = sys.argv[2]
    
    diff_index, context1, context2 = find_first_difference(file1, file2)
    
    if diff_index is None:
        print("The files are identical.")
    else:
        print(f"First difference at index {diff_index}:")
        print(f"File 1 context: '{context1}'")
        print(f"File 2 context: '{context2}'")

if __name__ == "__main__":
    main()