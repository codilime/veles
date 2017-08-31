// This is a generated file! Please edit source .ksy file and use
// kaitai-struct-compiler to rebuild

#include "kaitai/bmp.h"

namespace veles {
namespace kaitai {
namespace bmp {

bmp_t::bmp_t(kaitai::kstream* p_io, kaitai::kstruct* p_parent,
             bmp_t* /*p_root*/)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = this;
  m__io->popName();
  veles_obj = m__io->startChunk("bmp");
  f_image = false;
  m__io->pushName("file_header");
  m_file_header = new file_header_t(m__io, this, m__root);
  m__io->popName();
  m__io->pushName("dib_header");
  m_dib_header = new dib_header_t(m__io, this, m__root);
  m__io->popName();
  m__io->endChunk();
}

bmp_t::~bmp_t() {
  delete m_file_header;
  delete m_dib_header;
}

bmp_t::file_header_t::file_header_t(kaitai::kstream* p_io, bmp_t* p_parent,
                                    bmp_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("file_header");
  m__io->pushName("file_type");
  m_file_type = m__io->read_bytes(2);
  m__io->popName();
  m__io->pushName("file_size");
  m_file_size = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("reserved1");
  m_reserved1 = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("reserved2");
  m_reserved2 = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("bitmap_ofs");
  m_bitmap_ofs = m__io->read_s4le();
  m__io->popName();
  m__io->endChunk();
}

bmp_t::file_header_t::~file_header_t() {}

bmp_t::dib_header_t::dib_header_t(kaitai::kstream* p_io, bmp_t* p_parent,
                                  bmp_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("dib_header");
  m__io->pushName("dib_header_size");
  m_dib_header_size = m__io->read_s4le();
  m__io->popName();
  n_bitmap_core_header = true;
  if (dib_header_size() == 12) {
    n_bitmap_core_header = false;
    m__io->pushName("_skip_me_bitmap_core_header");
    m__skip_me_bitmap_core_header = m__io->read_bytes((dib_header_size() - 4));
    m__io->popName();
    m__io->pushName("m__skip_me_bitmap_core_header" + 3);
    m__io__skip_me_bitmap_core_header = new kaitai::kstream(
        m__io->blob(), m__io->pos() - m__skip_me_bitmap_core_header.size(),
        veles_obj, m__io->pos(), m__io->error());
    m__io->popName();
    m__io->pushName("bitmap_core_header");
    m_bitmap_core_header = new bitmap_core_header_t(
        m__io__skip_me_bitmap_core_header, this, m__root);
    m__io->popName();
  }
  n_bitmap_info_header = true;
  if (dib_header_size() == 40) {
    n_bitmap_info_header = false;
    m__io->pushName("_skip_me_bitmap_info_header");
    m__skip_me_bitmap_info_header = m__io->read_bytes((dib_header_size() - 4));
    m__io->popName();
    m__io->pushName("m__skip_me_bitmap_info_header" + 3);
    m__io__skip_me_bitmap_info_header = new kaitai::kstream(
        m__io->blob(), m__io->pos() - m__skip_me_bitmap_info_header.size(),
        veles_obj, m__io->pos(), m__io->error());
    m__io->popName();
    m__io->pushName("bitmap_info_header");
    m_bitmap_info_header = new bitmap_info_header_t(
        m__io__skip_me_bitmap_info_header, this, m__root);
    m__io->popName();
  }
  n_bitmap_v5_header = true;
  if (dib_header_size() == 124) {
    n_bitmap_v5_header = false;
    m__io->pushName("_skip_me_bitmap_v5_header");
    m__skip_me_bitmap_v5_header = m__io->read_bytes((dib_header_size() - 4));
    m__io->popName();
    m__io->pushName("m__skip_me_bitmap_v5_header" + 3);
    m__io__skip_me_bitmap_v5_header = new kaitai::kstream(
        m__io->blob(), m__io->pos() - m__skip_me_bitmap_v5_header.size(),
        veles_obj, m__io->pos(), m__io->error());
    m__io->popName();
    m__io->pushName("bitmap_v5_header");
    m_bitmap_v5_header = new bitmap_core_header_t(
        m__io__skip_me_bitmap_v5_header, this, m__root);
    m__io->popName();
  }
  n_dib_header_body = true;
  if (dib_header_size() != 12 && dib_header_size() != 40 &&
      dib_header_size() != 124) {
    n_dib_header_body = false;
    m__io->pushName("dib_header_body");
    m_dib_header_body = m__io->read_bytes((dib_header_size() - 4));
    m__io->popName();
  }
  m__io->endChunk();
}

bmp_t::dib_header_t::~dib_header_t() {
  if (!n_bitmap_core_header) {
    delete m__io__skip_me_bitmap_core_header;
    delete m_bitmap_core_header;
  }
  if (!n_bitmap_info_header) {
    delete m__io__skip_me_bitmap_info_header;
    delete m_bitmap_info_header;
  }
  if (!n_bitmap_v5_header) {
    delete m__io__skip_me_bitmap_v5_header;
    delete m_bitmap_v5_header;
  }
}

bmp_t::bitmap_core_header_t::bitmap_core_header_t(kaitai::kstream* p_io,
                                                  bmp_t::dib_header_t* p_parent,
                                                  bmp_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("bitmap_core_header");
  m__io->pushName("image_width");
  m_image_width = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("image_height");
  m_image_height = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("num_planes");
  m_num_planes = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("bits_per_pixel");
  m_bits_per_pixel = m__io->read_u2le();
  m__io->popName();
  m__io->endChunk();
}

bmp_t::bitmap_core_header_t::~bitmap_core_header_t() {}

bmp_t::bitmap_info_header_t::bitmap_info_header_t(kaitai::kstream* p_io,
                                                  bmp_t::dib_header_t* p_parent,
                                                  bmp_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("bitmap_info_header");
  m__io->pushName("image_width");
  m_image_width = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("image_height");
  m_image_height = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("num_planes");
  m_num_planes = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("bits_per_pixel");
  m_bits_per_pixel = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("compression");
  m_compression = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("size_image");
  m_size_image = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("x_px_per_m");
  m_x_px_per_m = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("y_px_per_m");
  m_y_px_per_m = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("num_colors_used");
  m_num_colors_used = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("num_colors_important");
  m_num_colors_important = m__io->read_u4le();
  m__io->popName();
  m__io->endChunk();
}

bmp_t::bitmap_info_header_t::~bitmap_info_header_t() {}

std::vector<uint8_t> bmp_t::image() {
  if (f_image) {
    return m_image;
  }
  m__io->pushName("image");
  auto saved_io = m__io;
  auto saved_veles_obj = veles_obj;
  m__io = new kaitai::kstream(saved_io->blob(), file_header()->bitmap_ofs(),
                              veles_obj, 0, saved_io->error());
  veles_obj = m__io->startChunk(saved_io->currentName());
  m__io->pushName("image");
  m_image = m__io->read_bytes_full();
  m__io->popName();
  m__io->endChunk();
  delete m__io;
  veles_obj = saved_veles_obj;
  m__io = saved_io;
  f_image = true;
  m__io->popName();
  return m_image;
}

}  // namespace bmp
}  // namespace kaitai
}  // namespace veles
