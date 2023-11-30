import sys

def padding_bin_file(file_name):
    with open(file_name, "ab") as f:
        current_size = f.tell()
        required_padding = 4096 - (current_size % 4096)
        if required_padding > 1: 
            num = current_size % 4096
            f.write(num.to_bytes(2, byteorder='big'))
            f.write(bytes([0xff] * (required_padding - 2)))
        else:
            f.write(bytes([0xff] * required_padding))
            required_padding = 4 - (current_size % 4)
            f.write(bytes(current_size % 4096))
            f.write(bytes([0xff] * 2))
            

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python padding_bin_file.py [file_name]")
        sys.exit()
    file_name = sys.argv[1]
    padding_bin_file(file_name)