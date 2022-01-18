import sys

sys.stdout.buffer.write(bytearray.fromhex(sys.stdin.read()))
