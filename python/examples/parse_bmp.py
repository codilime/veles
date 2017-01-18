#!/usr/bin/env python
import struct

import veles.veles_api as vapi
from veles import objects


class BmpParser(object):
    # TODO change a lot of chunks to items with values when they are supported
    # and add some comments to fields
    # parse more variations of bitmap
    def __init__(self, ip='127.0.0.1', port=3135):
        self.client = vapi.VelesApi(ip, port)
        self.current_start = 0

    def parse_files(self):
        files = self.client.list_files()
        for file in files:
            if file.name.endswith('.bmp'):
                self.parse_bmp(file)

    def create_chunk(self, parent, name,
                     start=None, end=None, size=None, comment=None,
                     update_start=True):
        new_obj = objects.LocalObject()

        new_obj.name = name
        if start is None:
            start = self.current_start
        new_obj.chunk_start = start
        if end is None:
            end = new_obj.chunk_start + size
        new_obj.chunk_end = end
        if update_start:
            self.current_start = new_obj.chunk_end
        if comment:
            new_obj.comment = comment

        self.client.create_chunk(parent, new_obj)
        return new_obj

    def parse_bmp(self, file):
        self.current_start = 0
        self.client.get_blob_data(file)
        if file.data[:2] != 'BM':
            # unsupported file type
            return
        file_size = struct.unpack('<I', file.data[2:6])[0]
        dib_size = struct.unpack('<I', file.data[14:18])[0]
        pix_start = struct.unpack('<I', file.data[10:14])[0]

        bitmap = self.create_chunk(file, 'bitmap',
                                   size=file_size, update_start=False)

        bitmap_header = self.create_chunk(
            bitmap, 'Bitmap file header', end=14, update_start=False)
        self.create_chunk(bitmap_header, 'FileType', size=2)
        self.create_chunk(bitmap_header, 'FileSize', size=4)
        self.create_chunk(
            bitmap_header, 'Reserved1', size=2,
            comment='value depends on the application that creates the image')
        self.create_chunk(
            bitmap_header, 'Reserved2', size=2,
            comment='value depends on the application that creates the image')
        self.create_chunk(bitmap_header, 'BitmapOffset', size=4)

        dib_header = self.create_chunk(
            bitmap, 'DIB header', size=dib_size, update_start=False)
        self.create_chunk(dib_header, 'Size', size=4)
        image_size = 0
        if dib_size == 12:
            width = struct.unpack(
                '<H', file.data[self.current_start:self.current_start+2])[0]
            self.create_chunk(dib_header, 'Width', size=2)
            height = struct.unpack(
                '<H', file.data[self.current_start:self.current_start+2])[0]
            self.create_chunk(dib_header, 'Height', size=2)
            self.create_chunk(dib_header, 'NumPlanes', size=2)
            bits_per_pix = struct.unpack(
                '<H', file.data[self.current_start:self.current_start+2])[0]
            self.create_chunk(dib_header, 'BitsPerPixel', size=2)
        else:
            width = abs(struct.unpack(
                '<i', file.data[self.current_start:self.current_start+4])[0])
            self.create_chunk(dib_header, 'Width', size=4)
            height = abs(struct.unpack(
                '<i', file.data[self.current_start:self.current_start+4])[0])
            self.create_chunk(dib_header, 'Height', size=4)
            self.create_chunk(dib_header, 'NumPlanes', size=2)
            bits_per_pix = struct.unpack(
                '<H', file.data[self.current_start:self.current_start+2])[0]
            self.create_chunk(dib_header, 'BitsPerPixel', size=2)
            if dib_size > 16:
                compression = struct.unpack(
                    '<I',
                    file.data[self.current_start:self.current_start+4])[0]
                self.create_chunk(dib_header, 'Compression', size=4)
                image_size = struct.unpack(
                    '<I',
                    file.data[self.current_start:self.current_start+4])[0]
                self.create_chunk(dib_header, 'ImageDataSize', size=4)
                self.create_chunk(dib_header, 'XResolution', size=4)
                self.create_chunk(dib_header, 'YResolution', size=4)
                colors_used = struct.unpack(
                    '<I',
                    file.data[self.current_start:self.current_start+4])[0]
                self.create_chunk(dib_header, 'ColorsUsed', size=4)
                self.create_chunk(dib_header, 'ColorsImportant', size=4)
            if dib_size == 64:
                self.create_chunk(dib_header, 'Units', size=2)
                self.create_chunk(dib_header, 'Reserved', size=2)
                self.create_chunk(dib_header, 'Recording', size=2)
                self.create_chunk(dib_header, 'Rendering', size=2)
                self.create_chunk(dib_header, 'Size1', size=4)
                self.create_chunk(dib_header, 'Size2', size=4)
                self.create_chunk(dib_header, 'ColorEncoding', size=4)
                self.create_chunk(dib_header, 'Identifier', size=4)
            if dib_size >= 108:
                self.create_chunk(dib_header, 'RedMask', size=4)
                self.create_chunk(dib_header, 'GreenMask', size=4)
                self.create_chunk(dib_header, 'BlueMask', size=4)
                self.create_chunk(dib_header, 'AlphaMask', size=4)
                self.create_chunk(dib_header, 'CSType', size=4)
                endpoints = self.create_chunk(
                    dib_header, 'Endpoints', size=36, update_start=False)
                self.create_chunk(endpoints, 'ciexyzRed', size=12)
                self.create_chunk(endpoints, 'ciexyzGreen', size=12)
                self.create_chunk(endpoints, 'ciexyzBlue', size=12)
                self.create_chunk(dib_header, 'GammaRed', size=4)
                self.create_chunk(dib_header, 'GammaGreen', size=4)
                self.create_chunk(dib_header, 'GammaBlue', size=4)
                if dib_size == 124:
                    self.create_chunk(dib_header, 'Intent', size=4)
                    self.create_chunk(dib_header, 'ProfileData', size=4)
                    self.create_chunk(dib_header, 'ProfileSize', size=4)
                    self.create_chunk(dib_header, 'Reserved', size=4)

        if dib_size == 40:
            if compression == 3:
                self.create_chunk(bitmap, 'Extra bit masks', size=12)
            if compression == 4:
                self.create_chunk(bitmap, 'Extra bit masks', size=16)

        if bits_per_pix <= 8:
            color_size = 3 if dib_size == 12 else 4
            colors = (min(colors_used, 2 ** bits_per_pix)
                      if colors_used != 0 else 2 ** bits_per_pix)
            color_table = self.create_chunk(
                bitmap, 'Color table',
                size=color_size*colors, update_start=False)
            for i in xrange(colors):
                color = self.create_chunk(
                    color_table, 'Color {}'.format(i),
                    size=color_size, update_start=False)
                self.create_chunk(color, 'Red', size=1)
                self.create_chunk(color, 'Green', size=1)
                self.create_chunk(color, 'Blue', size=1)
                self.create_chunk(color, 'Alpha', size=1)

        if self.current_start < pix_start:
            self.create_chunk(
                bitmap, 'Gap1', end=pix_start)

        line_size = (bits_per_pix*width+31)/32*4
        pix_size = line_size * height if image_size == 0 else image_size

        pixel_array = self.create_chunk(
            bitmap, 'Pixel array', start=pix_start,
            size=pix_size, update_start=False)
        if bits_per_pix:
            for i in xrange(height):
                self.create_chunk(
                    pixel_array, 'Row {}'.format(i), size=line_size)

        if pix_start + pix_size < file_size:
            self.create_chunk(
                bitmap, 'Gap2', start=pix_start + pix_size, end=file_size)


if __name__ == "__main__":
    parser = BmpParser()
    parser.parse_files()
