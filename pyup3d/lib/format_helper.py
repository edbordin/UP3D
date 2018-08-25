import struct

def float_from_hex(val):
    val_hex = format(val, 'x')
    return struct.unpack('!f', bytes.fromhex(val_hex))[0]
