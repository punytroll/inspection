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
    
    def __gt__(self, other):
        return self.bytes > other.bytes or (self.bytes == other.bytes and self.bits > other.bits)
    
    def __str__(self):
        return f"{self.bytes}.{self.bits}"
    
    def __repr__(self):
        return f"BytesAndBits({self.bytes}, {self.bits})"

def get_data():
    import os
    import sys
    
    if len(sys.argv) <= 1:
        print("Please give a .DS_Store file as a command line parameter.")
        sys.exit(1)
    file_path = sys.argv[1]
    if os.path.exists(file_path) is False:
        print(f"The file \"{file_path}\" does not exist.")
        sys.exit(1)
    with open(file_path, "rb") as file:
        data = file.read()
    return data
