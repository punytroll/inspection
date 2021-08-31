# This program is NOT meant to be a good working example of reading a .DS_Store file! It is
# intended to explore what it means to have a *flat* description of a file and what happens
# when you follow it.

# resources: https://0day.work/parsing-the-ds_store-file-format/

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

import bitstruct

# reading of "basic" data types
def get_buffer_unsigned_integer_8bit(data, offset, length):
    assert(length.bits == 0)
    return bitstruct.unpack_from(f"r{length.get_bits()}", data, offset.get_bits())[0]

def get_unsigned_integer_32bit_big_endian(data, offset):
    return bitstruct.unpack_from("u32", data, offset.get_bits())[0]

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
