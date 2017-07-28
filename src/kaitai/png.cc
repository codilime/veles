// This is a generated file! Please edit source .ksy file and use
// kaitai-struct-compiler to rebuild

#include "kaitai/png.h"

namespace veles {
namespace kaitai {
namespace png {

png_t::png_t(kaitai::kstream* p_io, kaitai::kstruct* p_parent, png_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = this;
  m__io->popName();
  veles_obj = m__io->startChunk("png");
  m__io->pushName("m_magic" + 2);
  m_magic = m__io->ensure_fixed_contents(
      std::string("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8));
  m__io->popName();
  m__io->pushName("m_ihdr_len" + 2);
  m_ihdr_len = m__io->ensure_fixed_contents(std::string("\x00\x00\x00\x0D", 4));
  m__io->popName();
  m__io->pushName("m_ihdr_type" + 2);
  m_ihdr_type =
      m__io->ensure_fixed_contents(std::string("\x49\x48\x44\x52", 4));
  m__io->popName();
  m__io->pushName("ihdr");
  m_ihdr = new ihdr_chunk_t(m__io, this, m__root);
  m__io->popName();
  m__io->pushName("ihdr_crc");
  m_ihdr_crc = m__io->read_bytes(4);
  m__io->popName();
  m_chunks = new std::vector<chunk_t*>();
  while (!m__io->is_eof()) {
    m__io->pushName("chunks");
    m_chunks->push_back(new chunk_t(m__io, this, m__root));
    m__io->popName();
  }
  m__io->endChunk();
}

png_t::~png_t() {
  delete m_ihdr;
  for (std::vector<chunk_t*>::iterator it = m_chunks->begin();
       it != m_chunks->end(); ++it) {
    delete *it;
  }
  delete m_chunks;
}

png_t::rgb_t::rgb_t(kaitai::kstream* p_io, png_t::plte_chunk_t* p_parent,
                    png_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("rgb");
  m__io->pushName("r");
  m_r = m__io->read_u1();
  m__io->popName();
  m__io->pushName("g");
  m_g = m__io->read_u1();
  m__io->popName();
  m__io->pushName("b");
  m_b = m__io->read_u1();
  m__io->popName();
  m__io->endChunk();
}

png_t::rgb_t::~rgb_t() {}

png_t::chunk_t::chunk_t(kaitai::kstream* p_io, png_t* p_parent, png_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("chunk");
  m__io->pushName("len");
  m_len = m__io->read_u4be();
  m__io->popName();
  m__io->pushName("type");
  m_type = m__io->read_str_byte_limit(4, "UTF-8");
  m__io->popName();
  {
    std::string on = type();
    if (on == std::string("gAMA")) {
      m__io->pushName("_skip_me_body");
      m__skip_me_body = m__io->read_bytes(len());
      m__io->popName();
      m__io->pushName("m__skip_me_body" + 3);
      m__io__skip_me_body = new kaitai::kstream(
          m__io->blob(), m__io->pos() - m__skip_me_body.size(), veles_obj,
          m__io->pos(), m__io->error());
      m__io->popName();
      m__io->pushName("body");
      m_body = new gama_chunk_t(m__io__skip_me_body, this, m__root);
      m__io->popName();
    } else if (on == std::string("tIME")) {
      m__io->pushName("_skip_me_body");
      m__skip_me_body = m__io->read_bytes(len());
      m__io->popName();
      m__io->pushName("m__skip_me_body" + 3);
      m__io__skip_me_body = new kaitai::kstream(
          m__io->blob(), m__io->pos() - m__skip_me_body.size(), veles_obj,
          m__io->pos(), m__io->error());
      m__io->popName();
      m__io->pushName("body");
      m_body = new time_chunk_t(m__io__skip_me_body, this, m__root);
      m__io->popName();
    } else if (on == std::string("PLTE")) {
      m__io->pushName("_skip_me_body");
      m__skip_me_body = m__io->read_bytes(len());
      m__io->popName();
      m__io->pushName("m__skip_me_body" + 3);
      m__io__skip_me_body = new kaitai::kstream(
          m__io->blob(), m__io->pos() - m__skip_me_body.size(), veles_obj,
          m__io->pos(), m__io->error());
      m__io->popName();
      m__io->pushName("body");
      m_body = new plte_chunk_t(m__io__skip_me_body, this, m__root);
      m__io->popName();
    } else if (on == std::string("bKGD")) {
      m__io->pushName("_skip_me_body");
      m__skip_me_body = m__io->read_bytes(len());
      m__io->popName();
      m__io->pushName("m__skip_me_body" + 3);
      m__io__skip_me_body = new kaitai::kstream(
          m__io->blob(), m__io->pos() - m__skip_me_body.size(), veles_obj,
          m__io->pos(), m__io->error());
      m__io->popName();
      m__io->pushName("body");
      m_body = new bkgd_chunk_t(m__io__skip_me_body, this, m__root);
      m__io->popName();
    } else if (on == std::string("pHYs")) {
      m__io->pushName("_skip_me_body");
      m__skip_me_body = m__io->read_bytes(len());
      m__io->popName();
      m__io->pushName("m__skip_me_body" + 3);
      m__io__skip_me_body = new kaitai::kstream(
          m__io->blob(), m__io->pos() - m__skip_me_body.size(), veles_obj,
          m__io->pos(), m__io->error());
      m__io->popName();
      m__io->pushName("body");
      m_body = new phys_chunk_t(m__io__skip_me_body, this, m__root);
      m__io->popName();
    } else if (on == std::string("tEXt")) {
      m__io->pushName("_skip_me_body");
      m__skip_me_body = m__io->read_bytes(len());
      m__io->popName();
      m__io->pushName("m__skip_me_body" + 3);
      m__io__skip_me_body = new kaitai::kstream(
          m__io->blob(), m__io->pos() - m__skip_me_body.size(), veles_obj,
          m__io->pos(), m__io->error());
      m__io->popName();
      m__io->pushName("body");
      m_body = new text_chunk_t(m__io__skip_me_body, this, m__root);
      m__io->popName();
    } else if (on == std::string("cHRM")) {
      m__io->pushName("_skip_me_body");
      m__skip_me_body = m__io->read_bytes(len());
      m__io->popName();
      m__io->pushName("m__skip_me_body" + 3);
      m__io__skip_me_body = new kaitai::kstream(
          m__io->blob(), m__io->pos() - m__skip_me_body.size(), veles_obj,
          m__io->pos(), m__io->error());
      m__io->popName();
      m__io->pushName("body");
      m_body = new chrm_chunk_t(m__io__skip_me_body, this, m__root);
      m__io->popName();
    } else if (on == std::string("sRGB")) {
      m__io->pushName("_skip_me_body");
      m__skip_me_body = m__io->read_bytes(len());
      m__io->popName();
      m__io->pushName("m__skip_me_body" + 3);
      m__io__skip_me_body = new kaitai::kstream(
          m__io->blob(), m__io->pos() - m__skip_me_body.size(), veles_obj,
          m__io->pos(), m__io->error());
      m__io->popName();
      m__io->pushName("body");
      m_body = new srgb_chunk_t(m__io__skip_me_body, this, m__root);
      m__io->popName();
    } else {
      m__io->pushName("_skip_me_body");
      m__skip_me_body = m__io->read_bytes(len());
      m__io->popName();
    }
  }
  m__io->pushName("crc");
  m_crc = m__io->read_bytes(4);
  m__io->popName();
  m__io->endChunk();
}

png_t::chunk_t::~chunk_t() {}

png_t::bkgd_indexed_t::bkgd_indexed_t(kaitai::kstream* p_io,
                                      png_t::bkgd_chunk_t* p_parent,
                                      png_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("bkgd_indexed");
  m__io->pushName("palette_index");
  m_palette_index = m__io->read_u1();
  m__io->popName();
  m__io->endChunk();
}

png_t::bkgd_indexed_t::~bkgd_indexed_t() {}

png_t::point_t::point_t(kaitai::kstream* p_io, png_t::chrm_chunk_t* p_parent,
                        png_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("point");
  f_x = false;
  f_y = false;
  m__io->pushName("x_int");
  m_x_int = m__io->read_u4be();
  m__io->popName();
  m__io->pushName("y_int");
  m_y_int = m__io->read_u4be();
  m__io->popName();
  m__io->endChunk();
}

png_t::point_t::~point_t() {}

double png_t::point_t::x() {
  if (f_x) {
    return m_x;
  }
  m__io->pushName("x");
  m__io->pushName("x");
  m_x = (x_int() / 100000.0);
  m__io->popName();
  f_x = true;
  m__io->popName();
  return m_x;
}

double png_t::point_t::y() {
  if (f_y) {
    return m_y;
  }
  m__io->pushName("y");
  m__io->pushName("y");
  m_y = (y_int() / 100000.0);
  m__io->popName();
  f_y = true;
  m__io->popName();
  return m_y;
}

png_t::bkgd_greyscale_t::bkgd_greyscale_t(kaitai::kstream* p_io,
                                          png_t::bkgd_chunk_t* p_parent,
                                          png_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("bkgd_greyscale");
  m__io->pushName("value");
  m_value = m__io->read_u2be();
  m__io->popName();
  m__io->endChunk();
}

png_t::bkgd_greyscale_t::~bkgd_greyscale_t() {}

png_t::chrm_chunk_t::chrm_chunk_t(kaitai::kstream* p_io,
                                  png_t::chunk_t* p_parent, png_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("chrm_chunk");
  m__io->pushName("white_point");
  m_white_point = new point_t(m__io, this, m__root);
  m__io->popName();
  m__io->pushName("red");
  m_red = new point_t(m__io, this, m__root);
  m__io->popName();
  m__io->pushName("green");
  m_green = new point_t(m__io, this, m__root);
  m__io->popName();
  m__io->pushName("blue");
  m_blue = new point_t(m__io, this, m__root);
  m__io->popName();
  m__io->endChunk();
}

png_t::chrm_chunk_t::~chrm_chunk_t() {
  delete m_white_point;
  delete m_red;
  delete m_green;
  delete m_blue;
}

png_t::ihdr_chunk_t::ihdr_chunk_t(kaitai::kstream* p_io, png_t* p_parent,
                                  png_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("ihdr_chunk");
  m__io->pushName("width");
  m_width = m__io->read_u4be();
  m__io->popName();
  m__io->pushName("height");
  m_height = m__io->read_u4be();
  m__io->popName();
  m__io->pushName("bit_depth");
  m_bit_depth = m__io->read_u1();
  m__io->popName();
  m__io->pushName("color_type");
  m_color_type = static_cast<png_t::color_type_t>(m__io->read_u1());
  m__io->popName();
  m__io->pushName("compression_method");
  m_compression_method = m__io->read_u1();
  m__io->popName();
  m__io->pushName("filter_method");
  m_filter_method = m__io->read_u1();
  m__io->popName();
  m__io->pushName("interlace_method");
  m_interlace_method = m__io->read_u1();
  m__io->popName();
  m__io->endChunk();
}

png_t::ihdr_chunk_t::~ihdr_chunk_t() {}

png_t::plte_chunk_t::plte_chunk_t(kaitai::kstream* p_io,
                                  png_t::chunk_t* p_parent, png_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("plte_chunk");
  m_entries = new std::vector<rgb_t*>();
  while (!m__io->is_eof()) {
    m__io->pushName("entries");
    m_entries->push_back(new rgb_t(m__io, this, m__root));
    m__io->popName();
  }
  m__io->endChunk();
}

png_t::plte_chunk_t::~plte_chunk_t() {
  for (std::vector<rgb_t*>::iterator it = m_entries->begin();
       it != m_entries->end(); ++it) {
    delete *it;
  }
  delete m_entries;
}

png_t::srgb_chunk_t::srgb_chunk_t(kaitai::kstream* p_io,
                                  png_t::chunk_t* p_parent, png_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("srgb_chunk");
  m__io->pushName("render_intent");
  m_render_intent =
      static_cast<png_t::srgb_chunk_t::intent_t>(m__io->read_u1());
  m__io->popName();
  m__io->endChunk();
}

png_t::srgb_chunk_t::~srgb_chunk_t() {}

png_t::bkgd_truecolor_t::bkgd_truecolor_t(kaitai::kstream* p_io,
                                          png_t::bkgd_chunk_t* p_parent,
                                          png_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("bkgd_truecolor");
  m__io->pushName("red");
  m_red = m__io->read_u2be();
  m__io->popName();
  m__io->pushName("green");
  m_green = m__io->read_u2be();
  m__io->popName();
  m__io->pushName("blue");
  m_blue = m__io->read_u2be();
  m__io->popName();
  m__io->endChunk();
}

png_t::bkgd_truecolor_t::~bkgd_truecolor_t() {}

png_t::gama_chunk_t::gama_chunk_t(kaitai::kstream* p_io,
                                  png_t::chunk_t* p_parent, png_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("gama_chunk");
  f_gamma_ratio = false;
  m__io->pushName("gamma_int");
  m_gamma_int = m__io->read_u4be();
  m__io->popName();
  m__io->endChunk();
}

png_t::gama_chunk_t::~gama_chunk_t() {}

double png_t::gama_chunk_t::gamma_ratio() {
  if (f_gamma_ratio) {
    return m_gamma_ratio;
  }
  m__io->pushName("gamma_ratio");
  m__io->pushName("gamma_ratio");
  m_gamma_ratio = (100000.0 / gamma_int());
  m__io->popName();
  f_gamma_ratio = true;
  m__io->popName();
  return m_gamma_ratio;
}

png_t::bkgd_chunk_t::bkgd_chunk_t(kaitai::kstream* p_io,
                                  png_t::chunk_t* p_parent, png_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("bkgd_chunk");
  switch (_root()->ihdr()->color_type()) {
    case COLOR_TYPE_GREYSCALE_ALPHA:
      m__io->pushName("bkgd");
      m_bkgd = new bkgd_greyscale_t(m__io, this, m__root);
      m__io->popName();
      break;
    case COLOR_TYPE_INDEXED:
      m__io->pushName("bkgd");
      m_bkgd = new bkgd_indexed_t(m__io, this, m__root);
      m__io->popName();
      break;
    case COLOR_TYPE_GREYSCALE:
      m__io->pushName("bkgd");
      m_bkgd = new bkgd_greyscale_t(m__io, this, m__root);
      m__io->popName();
      break;
    case COLOR_TYPE_TRUECOLOR_ALPHA:
      m__io->pushName("bkgd");
      m_bkgd = new bkgd_truecolor_t(m__io, this, m__root);
      m__io->popName();
      break;
    case COLOR_TYPE_TRUECOLOR:
      m__io->pushName("bkgd");
      m_bkgd = new bkgd_truecolor_t(m__io, this, m__root);
      m__io->popName();
      break;
    default:
      break;
  }
  m__io->endChunk();
}

png_t::bkgd_chunk_t::~bkgd_chunk_t() {}

png_t::phys_chunk_t::phys_chunk_t(kaitai::kstream* p_io,
                                  png_t::chunk_t* p_parent, png_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("phys_chunk");
  m__io->pushName("pixels_per_unit_x");
  m_pixels_per_unit_x = m__io->read_u4be();
  m__io->popName();
  m__io->pushName("pixels_per_unit_y");
  m_pixels_per_unit_y = m__io->read_u4be();
  m__io->popName();
  m__io->pushName("unit");
  m_unit = static_cast<png_t::phys_unit_t>(m__io->read_u1());
  m__io->popName();
  m__io->endChunk();
}

png_t::phys_chunk_t::~phys_chunk_t() {}

png_t::text_chunk_t::text_chunk_t(kaitai::kstream* p_io,
                                  png_t::chunk_t* p_parent, png_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("text_chunk");
  m__io->pushName("keyword");
  m_keyword = m__io->read_strz("iso8859-1", 0, false, true, true);
  m__io->popName();
  m__io->pushName("text");
  m_text = m__io->read_str_eos("iso8859-1");
  m__io->popName();
  m__io->endChunk();
}

png_t::text_chunk_t::~text_chunk_t() {}

png_t::time_chunk_t::time_chunk_t(kaitai::kstream* p_io,
                                  png_t::chunk_t* p_parent, png_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("time_chunk");
  m__io->pushName("year");
  m_year = m__io->read_u2be();
  m__io->popName();
  m__io->pushName("month");
  m_month = m__io->read_u1();
  m__io->popName();
  m__io->pushName("day");
  m_day = m__io->read_u1();
  m__io->popName();
  m__io->pushName("hour");
  m_hour = m__io->read_u1();
  m__io->popName();
  m__io->pushName("minute");
  m_minute = m__io->read_u1();
  m__io->popName();
  m__io->pushName("second");
  m_second = m__io->read_u1();
  m__io->popName();
  m__io->endChunk();
}

png_t::time_chunk_t::~time_chunk_t() {}

}  // namespace png
}  // namespace kaitai
}  // namespace veles
