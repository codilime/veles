// This is a generated file! Please edit source .ksy file and use
// kaitai-struct-compiler to rebuild

#include "kaitai/gif.h"

namespace veles {
namespace kaitai {
namespace gif {

gif_t::gif_t(kaitai::kstream* p_io, kaitai::kstruct* p_parent, gif_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = this;
  m__io->popName();
  veles_obj = m__io->startChunk("gif");
  m__io->pushName("header");
  m_header = new header_t(m__io, this, m__root);
  m__io->popName();
  m__io->pushName("logical_screen_descriptor");
  m_logical_screen_descriptor =
      new logical_screen_descriptor_t(m__io, this, m__root);
  m__io->popName();
  n_global_color_table = true;
  if (logical_screen_descriptor()->has_color_table()) {
    n_global_color_table = false;
    m__io->pushName("_skip_me_global_color_table");
    m__skip_me_global_color_table =
        m__io->read_bytes(logical_screen_descriptor()->color_table_size() * 3);
    m__io->popName();
    m__io->pushName("m__skip_me_global_color_table" + 3);
    m__io__skip_me_global_color_table = new kaitai::kstream(
        m__io->blob(), m__io->pos() - m__skip_me_global_color_table.size(),
        veles_obj, m__io->pos(), m__io->error());
    m__io->popName();
    m__io->pushName("global_color_table");
    m_global_color_table = new global_color_table_t(
        m__io__skip_me_global_color_table, this, m__root);
    m__io->popName();
  }
  m_blocks = new std::vector<block_t*>();
  while (!m__io->is_eof()) {
    m__io->pushName("blocks");
    m_blocks->push_back(new block_t(m__io, this, m__root));
    m__io->popName();
  }
  m__io->endChunk();
}

gif_t::~gif_t() {
  delete m_header;
  delete m_logical_screen_descriptor;
  if (!n_global_color_table) {
    delete m__io__skip_me_global_color_table;
    delete m_global_color_table;
  }
  for (std::vector<block_t*>::iterator it = m_blocks->begin();
       it != m_blocks->end(); ++it) {
    delete *it;
  }
  delete m_blocks;
}

gif_t::global_color_table_t::global_color_table_t(kaitai::kstream* p_io,
                                                  gif_t* p_parent,
                                                  gif_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("global_color_table");
  m_entries = new std::vector<color_table_entry_t*>();
  while (!m__io->is_eof()) {
    m__io->pushName("entries");
    m_entries->push_back(new color_table_entry_t(m__io, this, m__root));
    m__io->popName();
  }
  m__io->endChunk();
}

gif_t::global_color_table_t::~global_color_table_t() {
  for (std::vector<color_table_entry_t*>::iterator it = m_entries->begin();
       it != m_entries->end(); ++it) {
    delete *it;
  }
  delete m_entries;
}

gif_t::image_data_t::image_data_t(kaitai::kstream* p_io,
                                  gif_t::local_image_descriptor_t* p_parent,
                                  gif_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("image_data");
  m__io->pushName("lzw_min_code_size");
  m_lzw_min_code_size = m__io->read_u1();
  m__io->popName();
  m__io->pushName("subblocks");
  m_subblocks = new subblocks_t(m__io, this, m__root);
  m__io->popName();
  m__io->endChunk();
}

gif_t::image_data_t::~image_data_t() { delete m_subblocks; }

gif_t::color_table_entry_t::color_table_entry_t(
    kaitai::kstream* p_io, gif_t::global_color_table_t* p_parent, gif_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("color_table_entry");
  m__io->pushName("red");
  m_red = m__io->read_u1();
  m__io->popName();
  m__io->pushName("green");
  m_green = m__io->read_u1();
  m__io->popName();
  m__io->pushName("blue");
  m_blue = m__io->read_u1();
  m__io->popName();
  m__io->endChunk();
}

gif_t::color_table_entry_t::~color_table_entry_t() {}

gif_t::logical_screen_descriptor_t::logical_screen_descriptor_t(
    kaitai::kstream* p_io, gif_t* p_parent, gif_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("logical_screen_descriptor");
  f_has_color_table = false;
  f_color_table_size = false;
  m__io->pushName("screen_width");
  m_screen_width = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("screen_height");
  m_screen_height = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("flags");
  m_flags = m__io->read_u1();
  m__io->popName();
  m__io->pushName("bg_color_index");
  m_bg_color_index = m__io->read_u1();
  m__io->popName();
  m__io->pushName("pixel_aspect_ratio");
  m_pixel_aspect_ratio = m__io->read_u1();
  m__io->popName();
  m__io->endChunk();
}

gif_t::logical_screen_descriptor_t::~logical_screen_descriptor_t() {}

bool gif_t::logical_screen_descriptor_t::has_color_table() {
  if (f_has_color_table) {
    return m_has_color_table;
  }
  m__io->pushName("has_color_table");
  m__io->pushName("has_color_table");
  m_has_color_table = (flags() & 128) != 0;
  m__io->popName();
  f_has_color_table = true;
  m__io->popName();
  return m_has_color_table;
}

int32_t gif_t::logical_screen_descriptor_t::color_table_size() {
  if (f_color_table_size) {
    return m_color_table_size;
  }
  m__io->pushName("color_table_size");
  m__io->pushName("color_table_size");
  m_color_table_size = (2 << (flags() & 7));
  m__io->popName();
  f_color_table_size = true;
  m__io->popName();
  return m_color_table_size;
}

gif_t::local_image_descriptor_t::local_image_descriptor_t(
    kaitai::kstream* p_io, gif_t::block_t* p_parent, gif_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("local_image_descriptor");
  f_has_color_table = false;
  f_has_interlace = false;
  f_has_sorted_color_table = false;
  f_color_table_size = false;
  m__io->pushName("left");
  m_left = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("top");
  m_top = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("width");
  m_width = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("height");
  m_height = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("flags");
  m_flags = m__io->read_u1();
  m__io->popName();
  m__io->pushName("image_data");
  m_image_data = new image_data_t(m__io, this, m__root);
  m__io->popName();
  m__io->endChunk();
}

gif_t::local_image_descriptor_t::~local_image_descriptor_t() {
  delete m_image_data;
}

bool gif_t::local_image_descriptor_t::has_color_table() {
  if (f_has_color_table) {
    return m_has_color_table;
  }
  m__io->pushName("has_color_table");
  m__io->pushName("has_color_table");
  m_has_color_table = (flags() & 128) != 0;
  m__io->popName();
  f_has_color_table = true;
  m__io->popName();
  return m_has_color_table;
}

bool gif_t::local_image_descriptor_t::has_interlace() {
  if (f_has_interlace) {
    return m_has_interlace;
  }
  m__io->pushName("has_interlace");
  m__io->pushName("has_interlace");
  m_has_interlace = (flags() & 64) != 0;
  m__io->popName();
  f_has_interlace = true;
  m__io->popName();
  return m_has_interlace;
}

bool gif_t::local_image_descriptor_t::has_sorted_color_table() {
  if (f_has_sorted_color_table) {
    return m_has_sorted_color_table;
  }
  m__io->pushName("has_sorted_color_table");
  m__io->pushName("has_sorted_color_table");
  m_has_sorted_color_table = (flags() & 32) != 0;
  m__io->popName();
  f_has_sorted_color_table = true;
  m__io->popName();
  return m_has_sorted_color_table;
}

int32_t gif_t::local_image_descriptor_t::color_table_size() {
  if (f_color_table_size) {
    return m_color_table_size;
  }
  m__io->pushName("color_table_size");
  m__io->pushName("color_table_size");
  m_color_table_size = (2 << (flags() & 7));
  m__io->popName();
  f_color_table_size = true;
  m__io->popName();
  return m_color_table_size;
}

gif_t::block_t::block_t(kaitai::kstream* p_io, gif_t* p_parent, gif_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("block");
  m__io->pushName("block_type");
  m_block_type = static_cast<gif_t::block_type_t>(m__io->read_u1());
  m__io->popName();
  switch (block_type()) {
    case BLOCK_TYPE_EXTENSION:
      m__io->pushName("body");
      m_body = new extension_t(m__io, this, m__root);
      m__io->popName();
      break;
    case BLOCK_TYPE_LOCAL_IMAGE_DESCRIPTOR:
      m__io->pushName("body");
      m_body = new local_image_descriptor_t(m__io, this, m__root);
      m__io->popName();
      break;
    default:
      break;
  }
  m__io->endChunk();
}

gif_t::block_t::~block_t() {}

gif_t::header_t::header_t(kaitai::kstream* p_io, gif_t* p_parent, gif_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("header");
  m__io->pushName("m_magic" + 2);
  m_magic = m__io->ensure_fixed_contents(std::string("\x47\x49\x46", 3));
  m__io->popName();
  m__io->pushName("version");
  m_version = m__io->read_bytes(3);
  m__io->popName();
  m__io->endChunk();
}

gif_t::header_t::~header_t() {}

gif_t::ext_graphic_control_t::ext_graphic_control_t(
    kaitai::kstream* p_io, gif_t::extension_t* p_parent, gif_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("ext_graphic_control");
  f_transparent_color_flag = false;
  f_user_input_flag = false;
  m__io->pushName("m_block_size" + 2);
  m_block_size = m__io->ensure_fixed_contents(std::string("\x04", 1));
  m__io->popName();
  m__io->pushName("flags");
  m_flags = m__io->read_u1();
  m__io->popName();
  m__io->pushName("delay_time");
  m_delay_time = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("transparent_idx");
  m_transparent_idx = m__io->read_u1();
  m__io->popName();
  m__io->pushName("m_terminator" + 2);
  m_terminator = m__io->ensure_fixed_contents(std::string("\x00", 1));
  m__io->popName();
  m__io->endChunk();
}

gif_t::ext_graphic_control_t::~ext_graphic_control_t() {}

bool gif_t::ext_graphic_control_t::transparent_color_flag() {
  if (f_transparent_color_flag) {
    return m_transparent_color_flag;
  }
  m__io->pushName("transparent_color_flag");
  m__io->pushName("transparent_color_flag");
  m_transparent_color_flag = (flags() & 1) != 0;
  m__io->popName();
  f_transparent_color_flag = true;
  m__io->popName();
  return m_transparent_color_flag;
}

bool gif_t::ext_graphic_control_t::user_input_flag() {
  if (f_user_input_flag) {
    return m_user_input_flag;
  }
  m__io->pushName("user_input_flag");
  m__io->pushName("user_input_flag");
  m_user_input_flag = (flags() & 2) != 0;
  m__io->popName();
  f_user_input_flag = true;
  m__io->popName();
  return m_user_input_flag;
}

gif_t::subblock_t::subblock_t(kaitai::kstream* p_io, kaitai::kstruct* p_parent,
                              gif_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("subblock");
  m__io->pushName("num_bytes");
  m_num_bytes = m__io->read_u1();
  m__io->popName();
  m__io->pushName("bytes");
  m_bytes = m__io->read_bytes(num_bytes());
  m__io->popName();
  m__io->endChunk();
}

gif_t::subblock_t::~subblock_t() {}

gif_t::ext_application_t::ext_application_t(kaitai::kstream* p_io,
                                            gif_t::extension_t* p_parent,
                                            gif_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("ext_application");
  m__io->pushName("application_id");
  m_application_id = new subblock_t(m__io, this, m__root);
  m__io->popName();
  m_subblocks = new std::vector<subblock_t*>();
  {
    subblock_t* _;
    do {
      _ = new subblock_t(m__io, this, m__root);
      m__io->pushName("subblocks");
      m_subblocks->push_back(_);
      m__io->popName();
    } while (!(_->num_bytes() == 0));
  }
  m__io->endChunk();
}

gif_t::ext_application_t::~ext_application_t() {
  delete m_application_id;
  for (std::vector<subblock_t*>::iterator it = m_subblocks->begin();
       it != m_subblocks->end(); ++it) {
    delete *it;
  }
  delete m_subblocks;
}

gif_t::subblocks_t::subblocks_t(kaitai::kstream* p_io,
                                kaitai::kstruct* p_parent, gif_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("subblocks");
  m_entries = new std::vector<subblock_t*>();
  {
    subblock_t* _;
    do {
      _ = new subblock_t(m__io, this, m__root);
      m__io->pushName("entries");
      m_entries->push_back(_);
      m__io->popName();
    } while (!(_->num_bytes() == 0));
  }
  m__io->endChunk();
}

gif_t::subblocks_t::~subblocks_t() {
  for (std::vector<subblock_t*>::iterator it = m_entries->begin();
       it != m_entries->end(); ++it) {
    delete *it;
  }
  delete m_entries;
}

gif_t::extension_t::extension_t(kaitai::kstream* p_io, gif_t::block_t* p_parent,
                                gif_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("extension");
  m__io->pushName("label");
  m_label = static_cast<gif_t::extension_label_t>(m__io->read_u1());
  m__io->popName();
  switch (label()) {
    case EXTENSION_LABEL_APPLICATION:
      m__io->pushName("body");
      m_body = new ext_application_t(m__io, this, m__root);
      m__io->popName();
      break;
    case EXTENSION_LABEL_COMMENT:
      m__io->pushName("body");
      m_body = new subblocks_t(m__io, this, m__root);
      m__io->popName();
      break;
    case EXTENSION_LABEL_GRAPHIC_CONTROL:
      m__io->pushName("body");
      m_body = new ext_graphic_control_t(m__io, this, m__root);
      m__io->popName();
      break;
    default:
      m__io->pushName("body");
      m_body = new subblocks_t(m__io, this, m__root);
      m__io->popName();
      break;
  }
  m__io->endChunk();
}

gif_t::extension_t::~extension_t() {}

}  // namespace gif
}  // namespace kaitai
}  // namespace veles
