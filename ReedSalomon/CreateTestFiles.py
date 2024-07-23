import os
import numpy as np
import shutil

def generate_random_binary_file(file_path, size):
    """Generates a binary file filled with random bytes."""
    with open(file_path, 'wb') as f:
        f.write(os.urandom(size))
    print(f"Generated random binary file: {file_path} ({size} bytes)")

def add_errors_to_file(src_file, dst_file, burst_params, random_params):
    """Adds burst and random errors to a binary file."""
    # Copy the source file to the destination file
    shutil.copyfile(src_file, dst_file)

    # Load the file into memory
    with open(dst_file, 'rb') as f:
        data = bytearray(f.read())

    file_size = len(data)
    num_burst_errors, burst_mean, burst_std = burst_params
    num_random_errors = random_params

    # Generate burst errors
    burst_lengths = np.random.normal(burst_mean, burst_std, num_burst_errors).astype(int)
    burst_positions = np.random.randint(0, file_size, num_burst_errors)
    
    for burst_len, pos in zip(burst_lengths, burst_positions):
        burst_len = max(1, min(file_size - pos, burst_len))  # Ensure burst length is within bounds
        for i in range(burst_len):
            data[pos + i] ^= np.random.randint(0, 256)  # Flip bits randomly within the burst

    # Generate random errors
    for pos in range(num_random_errors):
        data[np.random.randint(0, file_size)] ^= np.random.randint(0, 256)  # Flip bits randomly

    # Write the modified data back to the file
    with open(dst_file, 'wb') as f:
        f.write(data)

    print(f"Added errors to binary file: {dst_file}")

if __name__ == "__main__":
    # Parameters
    original_file = "original.bin"
    corrupted_file = "corrupted.bin"
    file_size = 1020  # 1 kB

    burst_error_params = (0, 100, 20)  # (number of bursts, mean length, std deviation of burst length)
    random_error_params = (60)  # (number of random errors)

    # Generate the original random binary file
    generate_random_binary_file(original_file, file_size)

    # Create a corrupted version of the file with errors
    add_errors_to_file(original_file, corrupted_file, burst_error_params, random_error_params)
