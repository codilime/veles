#!/usr/bin/env python
import struct

import six

import veles.veles_api as vapi


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
        pix_start = struct.unpack('<I', file[10:14])[0]

        bitmap = file.create_chunk('bitmap', size=file_size)

        bitmap_header = bitmap.create_chunk('Bitmap file header', size=14)
        bitmap_header.create_chunk('FileType', size=2)
        bitmap_header.create_chunk('FileSize', size=4)
        bitmap_header.create_chunk(
            'Reserved1', size=2,
            comment='value depends on the application that creates the image')
        bitmap_header.create_chunk(
            'Reserved2', size=2,
            comment='value depends on the application that creates the image')
        bitmap_header.create_chunk('BitmapOffset', size=4)

        dib_header = bitmap.create_chunk('DIB header', size=dib_size)
        dib_header.create_chunk('Size', size=4)
        image_size = 0
        if dib_size == 12:
            width_chunk = dib_header.create_chunk('Width', size=2)
            width = struct.unpack('<H', width_chunk[0:2])[0]
            height_chunk = dib_header.create_chunk('Height', size=2)
            height = struct.unpack('<H', height_chunk[0:2])[0]
            dib_header.create_chunk('NumPlanes', size=2)
            bpp = dib_header.create_chunk('BitsPerPixel', size=2)
            bits_per_pix = struct.unpack('<H', bpp[0:2])[0]

        else:
            width_chunk = dib_header.create_chunk('Width', size=4)
            width = abs(struct.unpack('<i', width_chunk[0:4])[0])
            height_chunk = dib_header.create_chunk('Height', size=4)
            height = abs(struct.unpack('<i', height_chunk[0:4])[0])
            dib_header.create_chunk('NumPlanes', size=2)
            bpp = dib_header.create_chunk('BitsPerPixel', size=2)
            bits_per_pix = struct.unpack('<H', bpp[0:2])[0]
            if dib_size > 16:
                compr_chunk = dib_header.create_chunk('Compression', size=4)
                compression = struct.unpack('<I', compr_chunk[0:4])[0]
                isize_chunk = dib_header.create_chunk('ImageDataSize', size=4)
                image_size = struct.unpack('<I', isize_chunk[0:4])[0]
                dib_header.create_chunk('XResolution', size=4)
                dib_header.create_chunk('YResolution', size=4)
                col_used_chunk = dib_header.create_chunk('ColorsUsed', size=4)
                colors_used = struct.unpack('<I', col_used_chunk[0:4])[0]
                dib_header.create_chunk('ColorsImportant', size=4)
            if dib_size == 64:
                dib_header.create_chunk('Units', size=2)
                dib_header.create_chunk('Reserved', size=2)
                dib_header.create_chunk('Recording', size=2)
                dib_header.create_chunk('Rendering', size=2)
                dib_header.create_chunk('Size1', size=4)
                dib_header.create_chunk('Size2', size=4)
                dib_header.create_chunk('ColorEncoding', size=4)
                dib_header.create_chunk('Identifier', size=4)
            if dib_size >= 108:
                dib_header.create_chunk('RedMask', size=4)
                dib_header.create_chunk('GreenMask', size=4)
                dib_header.create_chunk('BlueMask', size=4)
                dib_header.create_chunk('AlphaMask', size=4)
                dib_header.create_chunk('CSType', size=4)
                endpoints = dib_header.create_chunk('Endpoints', size=36)
                endpoints.create_chunk('ciexyzRed', size=12)
                endpoints.create_chunk('ciexyzGreen', size=12)
                endpoints.create_chunk('ciexyzBlue', size=12)
                dib_header.create_chunk('GammaRed', size=4)
                dib_header.create_chunk('GammaGreen', size=4)
                dib_header.create_chunk('GammaBlue', size=4)
                if dib_size == 124:
                    dib_header.create_chunk('Intent', size=4)
                    dib_header.create_chunk('ProfileData', size=4)
                    dib_header.create_chunk('ProfileSize', size=4)
                    dib_header.create_chunk('Reserved', size=4)

        if dib_size == 40:
            if compression == 3:
                bitmap.create_chunk('Extra bit masks', size=12)
            if compression == 4:
                bitmap.create_chunk('Extra bit masks', size=16)

        if bits_per_pix <= 8:
            color_size = 3 if dib_size == 12 else 4
            colors = (min(colors_used, 2 ** bits_per_pix)
                      if colors_used != 0 else 2 ** bits_per_pix)
            color_table = bitmap.create_chunk(
                'Color table', size=color_size*colors)
            for i in six.moves.range(colors):
                color = color_table.create_chunk(
                    'Color {}'.format(i), size=color_size)
                color.create_chunk('Red', size=1)
                color.create_chunk('Green', size=1)
                color.create_chunk('Blue', size=1)
                color.create_chunk('Alpha', size=1)

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
