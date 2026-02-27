from datetime import datetime
import csv
import os
import sys

args = sys.argv


if len(args) <= 1 :
    print('please input log file name')
    exit()

log_file_name = args[1]
csv_file_name = os.path.splitext(log_file_name)[0] + '.csv'


event_list = []
with open('event log table.csv') as csv_file:
    csv_reader = csv.reader(csv_file)
    for row in csv_reader :
        event_list.append(row)

with open(log_file_name, 'rb') as f:
    file_data = f.read()
file_size = len(file_data)

if os.path.exists(csv_file_name) :
    os.remove(csv_file_name)

f = open(csv_file_name, 'w')
f.write('event_id,event_name,value,time\n')

index = 0
while index < file_size :
    event_id = int.from_bytes(file_data[index:index + 2], byteorder='little')
    try :
        event_name = event_list[event_id][1]
    except IndexError :
        event_name = ''
    index = index + 2
    bit_field = int.from_bytes(file_data[index:index + 2], byteorder='little')
    data_type = bit_field >> 15
    checksum = (bit_field >> 11) & 0x0F
    length = bit_field & 0x7FF
    index = index + 2
    timestamp = int.from_bytes(file_data[index:index + 4], byteorder='little')
    timestamp_str = datetime.fromtimestamp(timestamp).strftime('%Y-%m-%d %H:%M:%S')
    index = index + 4
    if length > 0 and data_type == 1 :
        content = file_data[index:index + length * 4]
        index_of_zero = content.find(b'\x00')
        str_bytes = content[:index_of_zero]
        log_value = str_bytes.decode("utf-8")
        log_value = '"' + log_value + '"'
    elif length == 1 :
        log_value = int.from_bytes(file_data[index:index + length * 4], byteorder='little')
    else :
        log_value = ''
    index = index + length * 4
    log_line = f'{event_id},{event_name},{log_value},{timestamp_str}'
    print(log_line)
    f.write(log_line + '\n')

f.close()
