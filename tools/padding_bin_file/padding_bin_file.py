import sys

def padding_bin_file(file_name):
    with open(file_name, "ab") as f:
        current_size = f.tell()
        required_padding = 4096 - (current_size % 4096)
        f.write(bytes([0xff] * required_padding))
        num = current_size % 4096
        f.write(num.to_bytes(2, byteorder='big'))
        f.write(bytes([0xff] * (4096 - 2)))
        
        #append boot.sig len
        with open("boot.sig", "rb") as sig_file:
            sig_len = len(sig_file.read())
            f.write(sig_len.to_bytes(4, byteorder='big'))
            checksum = 0
            for i in range(sig_len):
                checksum += sig_file.read(i)
            f.write(checksum.to_bytes(4, byteorder='big'))
            # append boot.sig
            f.write(sig_file.read())
            
if __name__ == '__main__':
    if len(sys.argv) != 2:
        print("Usage: python padding_bin_file.py [file_name]")
        sys.exit()
    file_name = sys.argv[1]
    padding_bin_file(file_name)