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
class BytesAndBits:
    def __init__(self, bytes, bits):
        self.bytes = bytes
        self.bits = bits
        self._normalize()
    
    def _normalize(self):
        while self.bits < 0:
            self.bits += 8
            self.bytes -= 1
        while self.bits >= 8:
            self.bits -= 8
            self.bytes += 1
        assert self.bytes >= 0
    
    def get_bits(self):
        return self.bytes * 8 + self.bits
    
    def __add__(self, other):
        return BytesAndBits(self.bytes + other.bytes, self.bits + other.bits)
    
    def __mul__(self, number):
        return BytesAndBits(self.bytes * number, self.bits * number)
    
    def __sub__(self, other):
        return BytesAndBits(self.bytes - other.bytes, self.bits - other.bits)
    
    def __str__(self):
        return f"{self.bytes}.{self.bits}"
    
    def __repr__(self):
        return f"BytesAndBits({self.bytes}, {self.bits})"


class Field:
    def __init__(self, value, begin, length):
        self.value = value
        self.begin = begin
        self.length = length
        self.end = self.begin + self.length

    def __repr__(self):
        return f"Field(value={self.value}, begin={self.begin}, length={self.length}, end={self.end})"


# reading of "basic" data types
def get_buffer_unsigned_integer_8bit_ended_by_length(data, position, length):
    assert length.bits == 0
    return Field(bitstruct.unpack_from("u8" * length.bytes, data, position.get_bits()), position, length)

def get_bool_8bit(data, position):
    return Field(bitstruct.unpack_from("u8", data, position.get_bits())[0] == 0x01, position, BytesAndBits(1, 0))

def get_string_ascii_ended_by_length(data, position, length):
    assert length.bits == 0
    return Field(bitstruct.unpack_from(f"r{length.get_bits()}", data, position.get_bits())[0].decode("ascii"), position, length)

def get_string_utf16_big_endian(data, position, length):
    assert length.bits == 0
    return Field(bitstruct.unpack_from(f"r{length.get_bits()}", data, position.get_bits())[0].decode("utf_16_be"), position, length)

def get_unsigned_integer_16bit_big_endian(data, position):
    return Field(bitstruct.unpack_from("u16", data, position.get_bits())[0], position, BytesAndBits(2, 0))

def get_unsigned_integer_32bit_big_endian(data, offset):
    return Field(bitstruct.unpack_from("u32", data, offset.get_bits())[0], offset, BytesAndBits(4, 0))

def get_unsigned_integer_64bit_big_endian(data, offset):
    return (bitstruct.unpack_from("u64", data, offset.get_bits())[0], BytesAndBits(8, 0))

def get_unsigned_integer_8bit(data, position):
    return Field(bitstruct.unpack_from("u8", data, position.get_bits())[0], position, BytesAndBits(1, 0))


# helper functions for interpreting data
# have to be independent of file-"global" fields
def get_offset_and_size_from_raw_address(raw_address, alignment_header_offset):
    return (BytesAndBits(raw_address & ~0x1f, 0) + alignment_header.end, BytesAndBits(1 << (raw_address & 0x1f), 0))


# objects that represent parts of the file's content
class Type:
    def __init__(self, type_name):
        self._type_name = type_name

    def __repr__(self):
        def _get_fields_string(dictionary):
            return ", ".join([f"{key}={value}" for key, value in dictionary.items() if key.startswith("_") is False])
        return f"{self._type_name}({_get_fields_string(self.__dict__)})"

class Blob(Type):
    def __init__(self):
        Type.__init__(self, "Blob")

class Block(Type):
    def __init__(self):
        Type.__init__(self, "Block")

class BuddyAllocatorHeader(Type):
    def __init__(self):
        Type.__init__(self, "BuddyAllocatorHeader")

class MasterBlock(Type):
    def __init__(self):
        Type.__init__(self, "MasterBlock")

class Record(Type):
    def __init__(self):
        Type.__init__(self, "Record")

# getter functions for file objects
# may access file-"global" fields
def get_blob(data, position):
    current_position = position
    result = Blob()
    result.length = get_unsigned_integer_32bit_big_endian(data, current_position)
    current_position = result.length.end
    result.field = get_buffer_unsigned_integer_8bit_ended_by_length(data, current_position, BytesAndBits(result.length.value, 0))
    current_position = result.field.end
    return Field(result, position, current_position - position)

def get_buddy_allocator_header(data, position):
    result = BuddyAllocatorHeader()
    result.magic = get_unsigned_integer_32bit_big_endian(data, position)
    result.ofs_bookkeeping_info_block = get_unsigned_integer_32bit_big_endian(data, result.magic.end)
    result.len_bookkeeping_info_block = get_unsigned_integer_32bit_big_endian(data, result.ofs_bookkeeping_info_block.end)
    result.copy_ofs_bookkeeping_info_block = get_unsigned_integer_32bit_big_endian(data, result.len_bookkeeping_info_block.end)
    result.unnamed4 = get_buffer_unsigned_integer_8bit_ended_by_length(data, result.copy_ofs_bookkeeping_info_block.end, BytesAndBits(16, 0))
    return Field(result, position, result.unnamed4.end - position)

def get_record(data, position):
    current_position = position
    result = Record()
    result.file_name_length = get_unsigned_integer_32bit_big_endian(data, current_position)
    current_position = result.file_name_length.end
    result.file_name = get_string_utf16_big_endian(data, current_position, BytesAndBits(result.file_name_length.value * 2, 0))
    current_position = result.file_name.end
    result.structure_type = get_string_ascii_ended_by_length(data, current_position, BytesAndBits(4, 0))
    current_position = result.structure_type.end
    result.data_type = get_string_ascii_ended_by_length(data, current_position, BytesAndBits(4, 0))
    current_position = result.data_type.end
    if result.data_type.value == "long":
        result.value = get_unsigned_integer_32bit_big_endian(data, current_position)
    elif result.data_type.value == "shor":
        ignored = get_unsigned_integer_16bit_big_endian(data, current_position)
        current_position = ignored.end
        result.value = get_unsigned_integer_16bit_big_endian(data, current_position)
    elif result.data_type.value == "bool":
        result.value = get_bool_8bit(data, current_position)
    elif result.data_type.value == "blob":
        result.value = get_blob(data, current_position)
    elif result.data_type.value == "type":
        result.value = get_string_ascii_ended_by_length(data, current_position, BytesAndBits(4, 0))
    elif result.data_type.value == "ustr":
        character_count = get_unsigned_integer_32bit_big_endian(data, current_position)
        current_position = character_count.end
        result.value = get_string_utf16_big_endian(data, current_position, BytesAndBits(2 * character_count.value, 0))
    elif result.data_type.value == "comp":
        result.value = get_unsigned_integer_64bit_big_endian(data, current_position)
    elif result.data_type.value == "dutc":
        result.value = get_unsigned_integer_64bit_big_endian(data, current_position)
    else:
        assert False, f"data type \"{result.data_type.value}\" not implemented."
    #~ if result.structure_type == "dscl":
        #~ assert result.data_type == "bool"
        #~ result.value = get_bool_8bit(data, current_position)
        #~ current_position += BytesAndBits(1, 0)
    #~ elif result.structure_type == "fwi0":
        #~ assert result.data_type == "blob"
        #~ result.value, length = get_blob(data, current_position)
        #~ assert result.value.length == 16
        #~ print(result.value.data)
        #~ current_position += length
    #~ else:
        #~ assert False, f"structure type \"{result.structure_type}\" not implemented."
    current_position = result.value.end
    return Field(result, position, current_position - position)

def get_block(data, block_id):
    raw_address = root_block.value.offsets.value.offsets[block_id]
    offset, size = get_offset_and_size_from_raw_address(raw_address.value, alignment_header.end)
    current_position = offset
    result = Block()
    result.mode = get_unsigned_integer_32bit_big_endian(data, current_position)
    current_position = result.mode.end
    result.counter = get_unsigned_integer_32bit_big_endian(data, current_position)
    current_position = result.counter.end
    result.records = list()
    if result.mode.value == 0:
        for record_index in range(result.counter.value):
            record = get_record(data, current_position)
            result.records.append(record)
            current_position = record.end
    return Field(result, offset, current_position - offset)

def get_root_block_offsets(data, position):
    current_position = position
    result = Type("Offsets")
    result.num_blocks = get_unsigned_integer_32bit_big_endian(data, current_position)
    current_position = result.num_blocks.end
    result.unnamed1 = get_unsigned_integer_32bit_big_endian(data, current_position)
    current_position = result.unnamed1.end
    result.offsets = list()
    for index in range(result.num_blocks.value):
        raw_address = get_unsigned_integer_32bit_big_endian(data, current_position)
        current_position = raw_address.end
        result.offsets.append(raw_address)
    rest = (((result.num_blocks.value // 256) + 1) * 256) - result.num_blocks.value
    result.rest = get_buffer_unsigned_integer_8bit_ended_by_length(data, current_position, BytesAndBits(4, 0) * rest)
    current_position = result.rest.end
    return Field(result, position, current_position - position)

def get_root_block_table_of_contents(data, position):
    current_position = position
    result = Type("TableOfContents")
    result.num_directories = get_unsigned_integer_32bit_big_endian(data, current_position)
    current_position = result.num_directories.end
    result.directories = dict()
    for index in range(result.num_directories.value):
        directory_entry = Type("DirectoryEntry")
        directory_entry.len_name = get_unsigned_integer_8bit(data, current_position)
        current_position = directory_entry.len_name.end
        directory_entry.name = get_string_ascii_ended_by_length(data, current_position, BytesAndBits(directory_entry.len_name.value, 0))
        current_position = directory_entry.name.end
        directory_entry.block_id = get_unsigned_integer_32bit_big_endian(data, current_position)
        current_position = directory_entry.block_id.end
        result.directories[directory_entry.name.value] = directory_entry
    return Field(result, position, current_position - position)

def get_root_block_free_lists(data, position):
    current_position = position
    result = Type("FreeLists")
    result.free_lists = list()
    for index in range(32):
        free_list = Type("FreeList")
        free_list.counter = get_unsigned_integer_32bit_big_endian(data, current_position)
        current_position = free_list.counter.end
        free_list.offsets = list()
        for offset_index in range(free_list.counter.value):
            offset = get_unsigned_integer_32bit_big_endian(data, current_position)
            current_position = offset.end
            free_list.offsets.append(offset)
        result.free_lists.append(free_list)
    return Field(result, position, current_position - position)

def get_root_block(data, position):
    current_position = position
    result = Type("RootBlock")
    result.offsets = get_root_block_offsets(data, current_position)
    current_position = result.offsets.end
    result.table_of_contents = get_root_block_table_of_contents(data, current_position)
    current_position = result.table_of_contents.end
    result.free_lists = get_root_block_free_lists(data, current_position)
    current_position = result.free_lists.end
    return Field(result, position, current_position - position)

# reading
alignment_header = get_unsigned_integer_32bit_big_endian(data, BytesAndBits(0, 0))
print(alignment_header)
buddy_allocator_header = get_buddy_allocator_header(data, alignment_header.end)
print(buddy_allocator_header)
root_block = get_root_block(data, alignment_header.end + BytesAndBits(buddy_allocator_header.value.ofs_bookkeeping_info_block.value, 0))
print(root_block)
# iterating the directories
for directory_name, directory in root_block.value.table_of_contents.value.directories.items():
    directory_master_block_raw_address = root_block.value.offsets.value.offsets[directory.block_id.value]
    directory_master_block_offset, directory_master_block_size = get_offset_and_size_from_raw_address(directory_master_block_raw_address.value, alignment_header.end)
    current_position = directory_master_block_offset
    directory_master_block = MasterBlock()
    directory_master_block.root_block_id = get_unsigned_integer_32bit_big_endian(data, current_position)
    current_position = directory_master_block.root_block_id.end
    directory_master_block.num_internal_nodes = get_unsigned_integer_32bit_big_endian(data, current_position)
    current_position = directory_master_block.num_internal_nodes.end
    directory_master_block.num_records = get_unsigned_integer_32bit_big_endian(data, current_position)
    current_position = directory_master_block.num_records.end
    directory_master_block.num_nodes = get_unsigned_integer_32bit_big_endian(data, current_position)
    current_position = directory_master_block.num_nodes.end
    directory_master_block.unnamed4 = get_unsigned_integer_32bit_big_endian(data, current_position)
    current_position = directory_master_block.unnamed4.end
    directory_master_block.root_block = get_block(data, directory_master_block.root_block_id.value)
    current_position = directory_master_block.root_block.end
    print(f"directory master block \"{directory_name}\" [{directory.block_id.value}]: {directory_master_block}")
