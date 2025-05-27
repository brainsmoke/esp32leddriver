#!/usr/bin/env python3

#
# create STM Extended DFU (DeFUse) files from an elf file
#
#
#
# Usage: python3 elf_to_dfuse.py <vendor>:<product> infile.elf outfile.dfu

import sys, subprocess

#
# Elf 'parsing'
#

READELF = [ 'arm-none-eabi-readelf', '-lW', '--' ] 

def exec_get_output_on_success(argv):
    ps = subprocess.Popen(argv, stdout=subprocess.PIPE)
    data = ps.stdout.read()
    ret = ps.wait()
    if ret != 0:
        raise Exception("cannot get the output of readelf")
    return data

def hex_int(b):
    if not b.startswith(b'0x'):
        raise ValueError("not a hex value")
    return int(b[2:], 16)

def get_elf_program_segments(filename):
    readelf_data = exec_get_output_on_success(READELF + [ filename ] )
    segments = []
    for line in readelf_data.split(b'\n'):
        if line.startswith(b'  LOAD '):
            cols = [ x for x in line.split(b' ') if x != b'' ]
            segment = {
                'offset'   : hex_int(cols[1]),
                'virt_addr': hex_int(cols[2]),
                'phys_addr': hex_int(cols[3]),
                'file_size': hex_int(cols[4]),
                'mem_size' : hex_int(cols[5]),
            }
            segments.append ( segment )
    return segments

def merge_chunks(chunks):
    last_addr = 0
    last_data = b''
    new_chunks = []
    for addr, data in chunks:
        if addr == last_addr + len(last_data):
            last_data += data
        else:
            if len(last_data) > 0:
                new_chunks.append( (last_addr, last_data) )
            last_addr, last_data = addr, data
    if len(last_data) > 0:
        new_chunks.append( (last_addr, last_data) )

    return new_chunks


def get_chunks(filename):
    elf_data = open( filename, 'rb' ).read()
    chunks = [ ]

    for segment in get_elf_program_segments(filename):
        offset, addr, size = segment['offset'], segment['phys_addr'], segment['file_size']
        data = elf_data[offset:offset+size]
        chunks.append ( ( addr, data ) )

    return merge_chunks(chunks)

#
# DFU / DeFUse
#

import struct, binascii

DEFUSE_PREFIX_SIZE = 11
DEFUSE_SUFFIX_SIZE = 16
ALT_SETTING=0

def defuse_suffix(body_crc, id_vendor, id_product, version):
    suffix = struct.pack("<HHHH", version, id_product, id_vendor, 0x011A) + b'UFD' + \
             struct.pack("<B", DEFUSE_SUFFIX_SIZE)
    crc32 = binascii.crc32(suffix, body_crc)
    suffix = suffix + struct.pack("<I", 0xffffffff^crc32)
    return suffix

def defuse_prefix(size, n_targets):
    return b'DfuSe\x01'+struct.pack("<IB", size, n_targets)

def defuse_target_prefix(images_size, n_elements, alt_setting=ALT_SETTING, name=None):
    if name == None:
        named = 0
        name = b''
    else:
        named = 1

    return b'Target'+struct.pack("<BI255sII", alt_setting, named, name, images_size, n_elements)

def defuse_image(addr, data):
    return struct.pack("<II", addr, len(data)) + data

def defuse_get_binary( chunks, id_vendor, id_product, version=0xffff ):
    images = [ defuse_image(addr, data) for addr, data in get_chunks(file_in) ]
    images_packed = b''.join(images)
    target_pre = defuse_target_prefix(images_size = len(images_packed), n_elements = len(images))
    total_file_size = DEFUSE_PREFIX_SIZE + len(target_pre) + len(images_packed) + DEFUSE_SUFFIX_SIZE
    prefix = defuse_prefix(total_file_size, 1)
    body = prefix + target_pre + images_packed
    body_crc = binascii.crc32(body)
    suffix = defuse_suffix(body_crc, id_vendor, id_product, version)
    return body+suffix

usb_id, file_in, file_out = sys.argv[1:4]
id_vendor, id_product = ( int(x, 16) for x in usb_id.split(':') )

chunks = get_chunks(file_in) # (addr, binary_data) pairs
defuse_file = defuse_get_binary(chunks, id_vendor, id_product)

with open(file_out, "wb") as f:
    f.write(defuse_file)


