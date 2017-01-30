#!/usr/bin/env python
import struct

import six

import veles.veles_api as vapi
from veles.objects.chunk_data_item import *


class BmpParser(object):
    # TODO change a lot of chunks to items with values when they are supported
    # and add some comments to fields
    # parse more variations of bitmap
    def __init__(self, ip='127.0.0.1', port=3135):
        self.client = vapi.VelesClient(ip, port)

    def parse_files(self):
        files = self.client.list_files()
        for file in files:
            if file.name.endswith('.bmp'):
                self.parse_bmp(file)

    def parse_bmp(self, file):
        if file[:2] != b'BM':
            # unsupported file type
            return
        file_size = struct.unpack('<I', file[2:6])[0]
        dib_size = struct.unpack('<I', file[14:18])[0]

        bitmap = file.create_chunk('bitmap', size=file_size)

        bitmap_header = bitmap.create_chunk('Bitmap file header', size=14)
        bitmap_header.create_chunk('FileType', size=2)
        bitmap_header.create_chunk_item('FileSize', bit_width=32)
        bitmap_header.create_chunk(
            'Reserved1', size=2,
            comment='value depends on the application that creates the image')
        bitmap_header.create_chunk(
            'Reserved2', size=2,
            comment='value depends on the application that creates the image')
        pix_start =  bitmap_header.create_chunk_item('BitmapOffset', bit_width=32).values[0]

        dib_header = bitmap.create_chunk('DIB header', size=dib_size)
        dib_header.create_chunk_item('Size', bit_width=32)
        image_size = 0
        if dib_size == 12:
            width = dib_header.create_chunk_item('Width', bit_width=16).values[0]
            height = dib_header.create_chunk_item('Height', bit_width=16).values[0]
            dib_header.create_chunk_item('NumPlanes', bit_width=16)
            bits_per_pix = dib_header.create_chunk_item('BitsPerPixel', bit_width=16).values[0]

        else:
            width = abs(dib_header.create_chunk_item('Width', bit_width=32, sign_mode=FieldSignMode.SIGNED).values[0])
            height = abs(dib_header.create_chunk_item('Height', bit_width=32, sign_mode=FieldSignMode.SIGNED).values[0])
            dib_header.create_chunk_item('NumPlanes', bit_width=16)
            bits_per_pix = dib_header.create_chunk_item('BitsPerPixel', bit_width=16).values[0]
            if dib_size > 16:
                compression = dib_header.create_chunk_item('Compression', bit_width=32).values[0]
                isize_chunk = dib_header.create_chunk_item('ImageDataSize', bit_width=32).values[0]
                dib_header.create_chunk_item('XResolution', bit_width=32)
                dib_header.create_chunk_item('YResolution', bit_width=32)
                colors_used = dib_header.create_chunk_item('ColorsUsed', bit_width=32).values[0]
                dib_header.create_chunk_item('ColorsImportant', bit_width=32)
            if dib_size == 64:
                dib_header.create_chunk_item('Units', bit_width=16)
                dib_header.create_chunk_item('Reserved', bit_width=16)
                dib_header.create_chunk_item('Recording', bit_width=16)
                dib_header.create_chunk_item('Rendering', bit_width=16)
                dib_header.create_chunk_item('Size1', bit_width=32)
                dib_header.create_chunk_item('Size2', bit_width=32)
                dib_header.create_chunk_item('ColorEncoding', bit_width=32)
                dib_header.create_chunk_item('Identifier', bit_width=32)
            if dib_size >= 108:
                dib_header.create_chunk_item('RedMask', bit_width=32)
                dib_header.create_chunk_item('GreenMask', bit_width=32)
                dib_header.create_chunk_item('BlueMask', bit_width=32)
                dib_header.create_chunk_item('AlphaMask', bit_width=32)
                dib_header.create_chunk_item('CSType', bit_width=32)
                endpoints = dib_header.create_chunk('Endpoints', size=36)
                endpoints.create_chunk_item('ciexyzRed', bit_width=96)
                endpoints.create_chunk_item('ciexyzGreen', bit_width=96)
                endpoints.create_chunk_item('ciexyzBlue', bit_width=96)
                dib_header.create_chunk_item('GammaRed', bit_width=32)
                dib_header.create_chunk_item('GammaGreen', bit_width=32)
                dib_header.create_chunk_item('GammaBlue', bit_width=32)
                if dib_size == 124:
                    dib_header.create_chunk_item('Intent', bit_width=32)
                    dib_header.create_chunk_item('ProfileData', bit_width=32)
                    dib_header.create_chunk_item('ProfileSize', bit_width=32)
                    dib_header.create_chunk_item('Reserved', bit_width=32)

        if dib_size == 40:
            if compression == 3:
                bitmap.create_chunk_item('Extra bit masks', bit_width=96)
            if compression == 4:
                bitmap.create_chunk_item('Extra bit masks', bit_width=128)

        if bits_per_pix <= 8:
            color_size = 3 if dib_size == 12 else 4
            colors = (min(colors_used, 2 ** bits_per_pix)
                      if colors_used != 0 else 2 ** bits_per_pix)
            color_table = bitmap.create_chunk(
                'Color table', size=color_size*colors)
            for i in six.moves.range(colors):
                color = color_table.create_chunk(
                    'Color {}'.format(i), size=color_size)
                color.create_chunk_item('Red', bit_width=8)
                color.create_chunk_item('Green', bit_width=8)
                color.create_chunk_item('Blue', bit_width=8)
                color.create_chunk_item('Alpha', bit_width=8)

        if bitmap._cursor < pix_start:
            bitmap.create_chunk('Gap1', end=pix_start)

        line_size = (bits_per_pix*width+31)//32*4
        pix_size = line_size * height if image_size == 0 else image_size

        pixel_array = bitmap.create_chunk(
            'Pixel array', start=pix_start, size=pix_size)
        if bits_per_pix:
            for i in six.moves.range(height):
                pixel_array.create_chunk('Row {}'.format(i), size=line_size)

        if pix_start + pix_size < file_size:
            bitmap.create_chunk(
                'Gap2', start=pix_start + pix_size, end=file_size)


if __name__ == "__main__":
    parser = BmpParser()
    parser.parse_files()
