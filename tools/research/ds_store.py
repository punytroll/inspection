# This program is NOT meant to be a good working example of reading a .DS_Store file! It is
# intended to explore what it means to have a *flat* description of a file and what happens
# when you follow it.

# resources:
# - https://0day.work/parsing-the-ds_store-file-format/
# - https://formats.kaitai.io/ds_store/index.html

import bitstruct
from library import BytesAndBits, get_data

data = get_data()
data_length = BytesAndBits(len(data), 0)

class Field:
    def __init__(self, value = None, value_begin = None, value_length = None):
        self.value = value
        self.value_begin = value_begin
        self.value_length = value_length
        self.name = None
        self.__fields = list()
        self.__fields_begin = None
        self.__fields_end = None
        self.__parent = None
    
    @property
    def end(self):
        result = None
        if self.value is not None and self.value_end is not None:
            result = self.value_end
        if len(self.__fields) > 0 and self.__fields_end is not None and (result is None or self.__fields_end > result):
            result = self.__fields_end
        return result
    
    @property
    def value_end(self):
        if self.value_begin is not None and self.value_length is not None:
            return self.value_begin + self.value_length
        else:
            return None
    
    @property
    def fields_begin(self):
        return self.__fields_begin
    
    @property
    def fields_end(self):
        return self.__fields_end
    
    @property
    def fields_length(self):
        if self.__fields_begin is not None and self.__fields_end is not None:
            return self.__fields_end - self.__fields_begin
        else:
            return None
    
    @property
    def parent(self):
        return self.__parent
    
    def add_field(self, name, field):
        assert(field.__parent is None)
        field.name = name
        self.__fields.append(field)
        field.__parent = self
        if field.value_begin is not None and (self.__fields_begin is None or self.__fields_begin > field.value_begin):
            self.__fields_begin = field.value_begin
        if field.__fields_begin is not None and (self.__fields_begin is None or self.__fields_begin > field.__fields_begin):
            self.__fields_begin = field.__fields_begin
        if field.value_end is not None and (self.__fields_end is None or self.__fields_end < field.value_end):
            self.__fields_end = field.value_end
        if field.__fields_end is not None and (self.__fields_end is None or self.__fields_end < field.__fields_end):
            self.__fields_end = field.__fields_end
    
    def __repr__(self):
        properties = list()
        if self.name is not None:
            properties.append(f"name={self.name}")
        if self.value is not None:
            properties.append(f"value={self.value}")
        if self.value_begin is not None:
            properties.append(f"value_begin={self.value_begin}")
        if self.__fields_begin is not None:
            properties.append(f"fields_begin={self.__fields_begin}")
        if self.value_length is not None:
            properties.append(f"value_length={self.value_length}")
        if self.fields_length is not None:
            properties.append(f"fields_length={self.fields_length}")
        if self.value_end is not None:
            properties.append(f"value_end={self.value_end}")
        if self.__fields_end is not None:
            properties.append(f"fields_end={self.__fields_end}")
        if len(self.__fields) > 0:
            properties.append(f"""fields=[{", ".join(map(repr, self.__fields))}]""")
        return f"""Field({", ".join(properties)})"""
    
    def get_path(self):
        result = list()
        current = self
        while current is not None:
            if current.name is not None:
                result.insert(0, current.name)
            else:
                if current.__parent is not None:
                    result.insert(0, "[" + str(current.__parent.__fields.index(current)) + "]")
                else:
                    result.insert(0, "[0]")
            current = current.__parent
        return result
    
    def __getitem__(self, field_accessor):
        if isinstance(field_accessor, str) == True:
            for field in self.__fields:
                if field.name == field_accessor:
                    return field
        elif isinstance(field_accessor, int) == True:
            return self.__fields[field_accessor]
    
    def __iter__(self):
        for field in self.__fields:
            yield field

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
def get_offset_and_size_from_raw_address(raw_address):
    return (BytesAndBits(raw_address & ~0x1f, 0), BytesAndBits(1 << (raw_address & 0x1f), 0))


# getter functions for file objects
# may access file-"global" fields
def get_raw_offset_and_size(data, position):
    raw_offset_and_address = get_unsigned_integer_32bit_big_endian(data, position)
    result = Field()
    result.add_field("RawOffset", Field(BytesAndBits(raw_offset_and_address.value & ~0x1f, 0), position, BytesAndBits(0, 27)))
    result.add_field("Size", Field(BytesAndBits(1 << (raw_offset_and_address.value & 0x1f), 0), position + BytesAndBits(0, 27), BytesAndBits(0, 5)))
    return result
    
def get_blob(data, position):
    result = Field()
    length = get_unsigned_integer_32bit_big_endian(data, position)
    result.add_field("Length", length)
    field = get_buffer_unsigned_integer_8bit_ended_by_length(data, length.value_end, BytesAndBits(length.value, 0))
    result.add_field("Data", field)
    return result

def get_buddy_allocator_header(data, position):
    result = Field()
    magic = get_unsigned_integer_32bit_big_endian(data, position)
    result.add_field("Magic", magic)
    ofs_bookkeeping_info_block = get_unsigned_integer_32bit_big_endian(data, magic.value_end)
    result.add_field("OffsetBookkeepingInfoBlock", ofs_bookkeeping_info_block)
    len_bookkeeping_info_block = get_unsigned_integer_32bit_big_endian(data, ofs_bookkeeping_info_block.value_end)
    result.add_field("LengthBookkeepingInfoBlock", len_bookkeeping_info_block)
    copy_ofs_bookkeeping_info_block = get_unsigned_integer_32bit_big_endian(data, len_bookkeeping_info_block.value_end)
    result.add_field("CopyOffsetBookkeepingInfoBlock", copy_ofs_bookkeeping_info_block)
    unnamed4 = get_buffer_unsigned_integer_8bit_ended_by_length(data, copy_ofs_bookkeeping_info_block.value_end, BytesAndBits(16, 0))
    result.add_field("Unnamed4", unnamed4)
    return result

def get_record(data, position):
    result = Field()
    file_name_length = get_unsigned_integer_32bit_big_endian(data, position)
    result.add_field("FileNameLength", file_name_length)
    file_name = get_string_utf16_big_endian(data, file_name_length.value_end, BytesAndBits(file_name_length.value * 2, 0))
    result.add_field("FileName", file_name)
    structure_type = get_string_ascii_ended_by_length(data, file_name.value_end, BytesAndBits(4, 0))
    result.add_field("StructureType", structure_type)
    data_type = get_string_ascii_ended_by_length(data, structure_type.value_end, BytesAndBits(4, 0))
    result.add_field("DataType", data_type)
    value = None
    if data_type.value == "long":
        value = get_unsigned_integer_32bit_big_endian(data, data_type.value_end)
        result.add_field("Value", value)
    elif data_type.value == "shor":
        ignored = get_unsigned_integer_16bit_big_endian(data, data_type.value_end)
        result.add_field("Ignored", ignored)
        value = get_unsigned_integer_16bit_big_endian(data, ignored.value_end)
        result.add_field("Value", value)
    elif data_type.value == "bool":
        value = get_bool_8bit(data, data_type.value_end)
        result.add_field("Value", value)
    elif data_type.value == "blob":
        value = get_blob(data, data_type.value_end)
        result.add_field("Value", value)
    elif data_type.value == "type":
        value = get_string_ascii_ended_by_length(data, data_type.value_end, BytesAndBits(4, 0))
        result.add_field("Value", value)
    elif data_type.value == "ustr":
        character_count = get_unsigned_integer_32bit_big_endian(data, data_type.value_end)
        result.add_field("CharacterCount", character_count)
        value = get_string_utf16_big_endian(data, character_count.value_end, BytesAndBits(2 * character_count.value, 0))
        result.add_field("Value", value)
    elif data_type.value == "comp":
        value = get_unsigned_integer_64bit_big_endian(data, data_type.value_end)
        result.add_field("Value", value)
    elif data_type.value == "dutc":
        value = get_unsigned_integer_64bit_big_endian(data, data_type.value_end)
        result.add_field("Value", value)
    else:
        assert False, f"data type \"{data_type.value}\" not implemented."
    return result

def get_block(data, position):
    result = Field()
    mode = get_unsigned_integer_32bit_big_endian(data, position)
    result.add_field("Mode", mode)
    counter = get_unsigned_integer_32bit_big_endian(data, mode.end)
    result.add_field("Counter", counter)
    records = Field()
    position = counter.end
    if mode.value == 0:
        for record_index in range(counter.value):
            record = get_record(data, position)
            records.add_field(None, record)
            position = record.end
    else:
        child_node_block_ids = Field()
        for record_index in range(counter.value):
            child_node_block_id = get_unsigned_integer_32bit_big_endian(data, position)
            child_node_block_ids.add_field(None, child_node_block_id)
            record = get_record(data, child_node_block_id.end)
            records.add_field(None, record)
            position = record.end
        result.add_field("ChildNodeBlockIDs", child_node_block_ids)
    result.add_field("Records", records)
    return result

def get_root_block_offsets(data, position):
    result = Field()
    num_blocks = get_unsigned_integer_32bit_big_endian(data, position)
    result.add_field("NumBlocks", num_blocks)
    unnamed1 = get_unsigned_integer_32bit_big_endian(data, num_blocks.end)
    result.add_field("Unnamed1", unnamed1)
    position = unnamed1.end
    offsets = Field()
    for index in range(num_blocks.value):
        raw_offset_and_size = get_raw_offset_and_size(data, position)
        position = raw_offset_and_size.end
        offsets.add_field(None, raw_offset_and_size)
    result.add_field("Offsets", offsets)
    rest = (((num_blocks.value // 256) + 1) * 256) - num_blocks.value
    rest = get_buffer_unsigned_integer_8bit_ended_by_length(data, position, BytesAndBits(4, 0) * rest)
    result.add_field("Rest", rest)
    return result

def get_root_block_table_of_contents(data, position):
    result = Field()
    num_directories = get_unsigned_integer_32bit_big_endian(data, position)
    result.add_field("NumDirectories", num_directories)
    position = num_directories.end
    directories = Field()
    for index in range(num_directories.value):
        directory_entry = Field()
        len_name = get_unsigned_integer_8bit(data, position)
        directory_entry.add_field("LenName", len_name)
        name = get_string_ascii_ended_by_length(data, len_name.end, BytesAndBits(len_name.value, 0))
        directory_entry.add_field("Name", name)
        root_block_id = get_unsigned_integer_32bit_big_endian(data, name.end)
        directory_entry.add_field("RootBlockID", root_block_id)
        position = root_block_id.end
        directories.add_field(None, directory_entry)
    result.add_field("Directories", directories)
    return result

def get_root_block_free_lists(data, position):
    result = Field()
    for index in range(32):
        free_list = Field()
        counter = get_unsigned_integer_32bit_big_endian(data, position)
        free_list.add_field("Counter", counter)
        position = counter.value_end
        offsets = Field()
        for offset_index in range(counter.value):
            offset = get_unsigned_integer_32bit_big_endian(data, position)
            offsets.add_field(None, offset)
            position = offset.value_end
        free_list.add_field("Offsets", offsets)
        result.add_field(None, free_list)
    return result

def get_root_block(data, position):
    result = Field()
    offsets = get_root_block_offsets(data, position)
    result.add_field("Offsets", offsets)
    table_of_contents = get_root_block_table_of_contents(data, offsets.end)
    result.add_field("TableOfContents", table_of_contents)
    free_lists = get_root_block_free_lists(data, table_of_contents.end)
    result.add_field("FreeLists", free_lists)
    return result

# reading
file = Field()
file.name = "DSStoreFile"
alignment_header = get_unsigned_integer_32bit_big_endian(data, BytesAndBits(0, 0))
file.add_field("AlignmentHeader", alignment_header)
buddy_allocator_header = get_buddy_allocator_header(data, alignment_header.value_end)
file.add_field("BuddyAllocator", buddy_allocator_header)
root_block = get_root_block(data, alignment_header.end + BytesAndBits(buddy_allocator_header["OffsetBookkeepingInfoBlock"].value, 0))
file.add_field("RootBlock", root_block)
directories = Field()
# iterating the directories
for directory in root_block["TableOfContents"]["Directories"]:
    directory_master_block_offset = root_block["Offsets"]["Offsets"][directory["RootBlockID"].value]["RawOffset"].value + alignment_header.end
    directory_master_block = Field()
    root_block_id = get_unsigned_integer_32bit_big_endian(data, directory_master_block_offset)
    directory_master_block.add_field("RootNodeBlockID", root_block_id)
    num_internal_nodes = get_unsigned_integer_32bit_big_endian(data, root_block_id.end)
    directory_master_block.add_field("NumInternalNodes", num_internal_nodes)
    num_records = get_unsigned_integer_32bit_big_endian(data, num_internal_nodes.end)
    directory_master_block.add_field("NumRecords", num_records)
    num_nodes = get_unsigned_integer_32bit_big_endian(data, num_records.end)
    directory_master_block.add_field("NumNodes", num_nodes)
    unnamed4 = get_unsigned_integer_32bit_big_endian(data, num_nodes.end)
    directory_master_block.add_field("Unnamed4", unnamed4)
    inner_nodes = Field()
    directory_master_block.add_field("InnerNodes", inner_nodes)
    node_block_ids = [directory_master_block["RootNodeBlockID"].value]
    while len(node_block_ids) > 0:
        node_block_id = node_block_ids.pop(0)
        node_block_offset = root_block["Offsets"]["Offsets"][node_block_id]["RawOffset"].value + alignment_header.end
        node_block = get_block(data, node_block_offset)
        if node_block_id == directory_master_block["RootNodeBlockID"].value:
            directory_master_block.add_field("RootNodeBlock", node_block)
        else:
            inner_nodes.add_field(None, node_block)
        if node_block["Mode"].value != 0:
            child_node_block_ids = [node_block["Mode"].value] + [child_node_block_id.value for child_node_block_id in node_block["ChildNodeBlockIDs"]]
            node_block_ids += child_node_block_ids
    directories.add_field(None, directory_master_block)
file.add_field("Directories", directories)

def iterate(field):
    yield field
    for sub_field in field:
        yield from iterate(sub_field)

def out(field):
    if field.value is not None:
        print("Value", end = "")
    else:
        print("Container", end = "")
    print(": \"" + "/".join(field.get_path()) + "\"  ->  ", end = "")
    properties = list()
    if field.value is not None:
        properties.append("value=" + str(field.value))
    if field.value_begin is not None:
        properties.append("value_begin=" + str(field.value_begin))
    if field.value_length is not None:
        properties.append("value_length=" + str(field.value_length))
    if field.value_end is not None:
        properties.append("value_end=" + str(field.value_end))
    print(", ".join(properties))

def is_value(field):
    return field.value is not None

begin = BytesAndBits(0, 0)
for field in sorted(filter(is_value, iterate(file)), key = lambda field: field.value_begin):
    if field.value_begin > begin:
        print("Gap: begin=" + str(begin) + ", length=" + str(field.value_begin - begin) + ", end=" + str(field.value_begin))
    begin = field.value_end
    out(field)
if data_length > begin:
    print("Gap: begin=" + str(begin) + ", length=" + str(data_length - begin) + ", end=" + str(data_length))
