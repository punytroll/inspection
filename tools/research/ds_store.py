# This program is NOT meant to be a good working example of reading a .DS_Store file! It is
# intended to explore what it means to have a *flat* description of a file and what happens
# when you follow it.

# resources:
# - https://0day.work/parsing-the-ds_store-file-format/
# - https://formats.kaitai.io/ds_store/index.html

import bitstruct
import os
import sys

# preliminaries
if len(sys.argv) <= 1:
    print("Please give a .DS_Store file as a command line parameter.")
    sys.exit(1)
file_path = sys.argv[1]
if os.path.exists(file_path) is False:
    print(f"The file \"{file_path}\" does not exist.")
    sys.exit(1)
file = open(file_path, "rb")
data = file.read()
file.close()

# Helper class "BytesAndBits" to store lengths and offsets in the data buffer.
class BytesAndBits(object):
    def __init__(self, bytes, bits):
        self.bytes = bytes
        self.bits = bits
        self._normalize()
    
    def _normalize(self):
        assert(self.bytes >= 0)
        assert(self.bits >= 0)
        while self.bits >= 8:
            self.bits -= 8
            self.bytes += 1
    
    def get_bits(self):
        return self.bytes * 8 + self.bits
    
    def __add__(self, other):
        return BytesAndBits(self.bytes + other.bytes, self.bits + other.bits)
    
    def __mul__(self, number):
        return BytesAndBits(self.bytes * number, self.bits * number)
    
    def __str__(self):
        return f"{self.bytes}.{self.bits}"

# reading of "basic" data types
def get_string_ascii_ended_by_length(data, offset, length):
    assert(length.bits == 0)
    return bitstruct.unpack_from(f"r{length.get_bits()}", data, offset.get_bits())[0].decode("ascii")

def get_buffer_unsigned_integer_8bit(data, offset, length):
    assert(length.bits == 0)
    return bitstruct.unpack_from("u8" * length.bytes, data, offset.get_bits())

def get_unsigned_integer_32bit_big_endian(data, offset):
    return bitstruct.unpack_from("u32", data, offset.get_bits())[0]

def get_unsigned_integer_8bit(data, offset):
    return bitstruct.unpack_from("u8", data, offset.get_bits())[0]

# reading
alignment_header_start = BytesAndBits(0, 0)
alignment_header = get_unsigned_integer_32bit_big_endian(data, alignment_header_start)
alignment_header_end = alignment_header_start + BytesAndBits(4, 0)
buddy_allocator_header_magic = get_unsigned_integer_32bit_big_endian(data, BytesAndBits(4, 0))
buddy_allocator_header_ofs_bookkeeping_info_block = get_unsigned_integer_32bit_big_endian(data, BytesAndBits(8, 0))
buddy_allocator_header_len_bookkeeping_info_block = get_unsigned_integer_32bit_big_endian(data, BytesAndBits(12, 0))
buddy_allocator_header_copy_ofs_bookkeeping_info_block = get_unsigned_integer_32bit_big_endian(data, BytesAndBits(16, 0))
buddy_allocator_header_unnamed4 = get_buffer_unsigned_integer_8bit(data, BytesAndBits(20, 0), BytesAndBits(16, 0))
root_block_offsets_start = alignment_header_end + BytesAndBits(buddy_allocator_header_ofs_bookkeeping_info_block, 0)
root_block_offsets_end = root_block_offsets_start + BytesAndBits(buddy_allocator_header_len_bookkeeping_info_block, 0)
root_block_offsets_num_blocks = get_unsigned_integer_32bit_big_endian(data, root_block_offsets_start)
root_block_offsets_unnamed1 = get_unsigned_integer_32bit_big_endian(data, root_block_offsets_start + BytesAndBits(4, 0))
root_block_offsets_offsets_start = root_block_offsets_start + BytesAndBits(8, 0)
root_block_offsets_offsets = list()
for offset_index in range(root_block_offsets_num_blocks):
    root_block_offsets_offsets.append(get_unsigned_integer_32bit_big_endian(data, root_block_offsets_offsets_start + BytesAndBits(4, 0) * offset_index))
rest = (((root_block_offsets_num_blocks // 256) + 1) * 256) - root_block_offsets_num_blocks
root_block_offsets_rest = get_buffer_unsigned_integer_8bit(data, root_block_offsets_offsets_start + BytesAndBits(4, 0) * root_block_offsets_num_blocks, BytesAndBits(4, 0) * rest)
root_block_table_of_content_start = root_block_offsets_offsets_start + BytesAndBits(4, 0) * (root_block_offsets_num_blocks + rest)
root_block_table_of_content_num_directories = get_unsigned_integer_32bit_big_endian(data, root_block_table_of_content_start)
root_block_table_of_content_directories = dict()
current_position = root_block_table_of_content_start + BytesAndBits(4, 0)
for directory_index in range(root_block_table_of_content_num_directories):
    directory_entry_len_name = get_unsigned_integer_8bit(data, current_position)
    directory_entry_name = get_string_ascii_ended_by_length(data, current_position + BytesAndBits(1, 0), BytesAndBits(directory_entry_len_name, 0))
    directory_entry_block_id = get_unsigned_integer_32bit_big_endian(data, current_position + BytesAndBits(1 + directory_entry_len_name, 0))
    root_block_table_of_content_directories[directory_entry_name] = directory_entry_block_id
    current_position += BytesAndBits(1 + directory_entry_len_name + 4, 0)
root_block_free_list = list()
for free_list_bucket_index in range(32):
    free_list_bucket_counter = get_unsigned_integer_32bit_big_endian(data, current_position)
    current_position += BytesAndBits(4, 0)
    free_list_bucket_offsets = list()
    for free_list_bucket_offsets_index in range(free_list_bucket_counter):
        free_list_bucket_offsets_offset = get_unsigned_integer_32bit_big_endian(data, current_position)
        current_position += BytesAndBits(4, 0)
        free_list_bucket_offsets.append(free_list_bucket_offsets_offset)
    root_block_free_list.append(free_list_bucket_offsets)
for directory_name, directory_block_id in root_block_table_of_content_directories.items():
    raw_address = root_block_offsets_offsets[directory_block_id]
    offset = BytesAndBits(raw_address & ~0x1f, 0) + alignment_header_end
    size = BytesAndBits(1 << (raw_address & 0x1f), 0)
    print(offset, size)
