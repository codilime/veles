// This is a generated file! Please edit source .ksy file and use
// kaitai-struct-compiler to rebuild

#include "kaitai/avi.h"

namespace veles {
namespace kaitai {
namespace avi {

avi_t::avi_t(kaitai::kstream* p_io, kaitai::kstruct* p_parent,
             avi_t* /*p_root*/)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = this;
  m__io->popName();
  veles_obj = m__io->startChunk("avi");
  m__io->pushName("m_magic1" + 2);
  m_magic1 = m__io->ensure_fixed_contents(std::string("\x52\x49\x46\x46", 4));
  m__io->popName();
  m__io->pushName("file_size");
  m_file_size = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("m_magic2" + 2);
  m_magic2 = m__io->ensure_fixed_contents(std::string("\x41\x56\x49\x20", 4));
  m__io->popName();
  m__io->pushName("_skip_me_data");
  m__skip_me_data = m__io->read_bytes((file_size() - 4));
  m__io->popName();
  m__io->pushName("m__skip_me_data" + 3);
  m__io__skip_me_data =
      new kaitai::kstream(m__io->blob(), m__io->pos() - m__skip_me_data.size(),
                          veles_obj, m__io->pos(), m__io->error());
  m__io->popName();
  m__io->pushName("data");
  m_data = new blocks_t(m__io__skip_me_data, this, m__root);
  m__io->popName();
  m__io->endChunk();
}

avi_t::~avi_t() {
  delete m__io__skip_me_data;
  delete m_data;
}

avi_t::list_body_t::list_body_t(kaitai::kstream* p_io, avi_t::block_t* p_parent,
                                avi_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("list_body");
  m__io->pushName("list_type");
  m_list_type = static_cast<avi_t::chunk_type_t>(m__io->read_u4le());
  m__io->popName();
  m__io->pushName("data");
  m_data = new blocks_t(m__io, this, m__root);
  m__io->popName();
  m__io->endChunk();
}

avi_t::list_body_t::~list_body_t() { delete m_data; }

avi_t::rect_t::rect_t(kaitai::kstream* p_io, avi_t::strh_body_t* p_parent,
                      avi_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("rect");
  m__io->pushName("left");
  m_left = m__io->read_s2le();
  m__io->popName();
  m__io->pushName("top");
  m_top = m__io->read_s2le();
  m__io->popName();
  m__io->pushName("right");
  m_right = m__io->read_s2le();
  m__io->popName();
  m__io->pushName("bottom");
  m_bottom = m__io->read_s2le();
  m__io->popName();
  m__io->endChunk();
}

avi_t::rect_t::~rect_t() {}

avi_t::blocks_t::blocks_t(kaitai::kstream* p_io, kaitai::kstruct* p_parent,
                          avi_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("blocks");
  m_entries = new std::vector<block_t*>();
  while (!m__io->is_eof()) {
    m__io->pushName("entries");
    m_entries->push_back(new block_t(m__io, this, m__root));
    m__io->popName();
  }
  m__io->endChunk();
}

avi_t::blocks_t::~blocks_t() {
  for (std::vector<block_t*>::iterator it = m_entries->begin();
       it != m_entries->end(); ++it) {
    delete *it;
  }
  delete m_entries;
}

avi_t::avih_body_t::avih_body_t(kaitai::kstream* p_io, avi_t::block_t* p_parent,
                                avi_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("avih_body");
  m__io->pushName("micro_sec_per_frame");
  m_micro_sec_per_frame = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("max_bytes_per_sec");
  m_max_bytes_per_sec = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("padding_granularity");
  m_padding_granularity = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("flags");
  m_flags = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("total_frames");
  m_total_frames = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("initial_frames");
  m_initial_frames = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("streams");
  m_streams = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("suggested_buffer_size");
  m_suggested_buffer_size = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("width");
  m_width = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("height");
  m_height = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("reserved");
  m_reserved = m__io->read_bytes(16);
  m__io->popName();
  m__io->endChunk();
}

avi_t::avih_body_t::~avih_body_t() {}

avi_t::block_t::block_t(kaitai::kstream* p_io, avi_t::blocks_t* p_parent,
                        avi_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("block");
  m__io->pushName("four_cc");
  m_four_cc = static_cast<avi_t::chunk_type_t>(m__io->read_u4le());
  m__io->popName();
  m__io->pushName("block_size");
  m_block_size = m__io->read_u4le();
  m__io->popName();
  switch (four_cc()) {
    case CHUNK_TYPE_LIST:
      m__io->pushName("_skip_me_data");
      m__skip_me_data = m__io->read_bytes(block_size());
      m__io->popName();
      m__io->pushName("m__skip_me_data" + 3);
      m__io__skip_me_data = new kaitai::kstream(
          m__io->blob(), m__io->pos() - m__skip_me_data.size(), veles_obj,
          m__io->pos(), m__io->error());
      m__io->popName();
      m__io->pushName("data");
      m_data = new list_body_t(m__io__skip_me_data, this, m__root);
      m__io->popName();
      break;
    case CHUNK_TYPE_AVIH:
      m__io->pushName("_skip_me_data");
      m__skip_me_data = m__io->read_bytes(block_size());
      m__io->popName();
      m__io->pushName("m__skip_me_data" + 3);
      m__io__skip_me_data = new kaitai::kstream(
          m__io->blob(), m__io->pos() - m__skip_me_data.size(), veles_obj,
          m__io->pos(), m__io->error());
      m__io->popName();
      m__io->pushName("data");
      m_data = new avih_body_t(m__io__skip_me_data, this, m__root);
      m__io->popName();
      break;
    case CHUNK_TYPE_STRH:
      m__io->pushName("_skip_me_data");
      m__skip_me_data = m__io->read_bytes(block_size());
      m__io->popName();
      m__io->pushName("m__skip_me_data" + 3);
      m__io__skip_me_data = new kaitai::kstream(
          m__io->blob(), m__io->pos() - m__skip_me_data.size(), veles_obj,
          m__io->pos(), m__io->error());
      m__io->popName();
      m__io->pushName("data");
      m_data = new strh_body_t(m__io__skip_me_data, this, m__root);
      m__io->popName();
      break;
    default:
      m__io->pushName("_skip_me_data");
      m__skip_me_data = m__io->read_bytes(block_size());
      m__io->popName();
      break;
  }
  m__io->endChunk();
}

avi_t::block_t::~block_t() {}

avi_t::strh_body_t::strh_body_t(kaitai::kstream* p_io, avi_t::block_t* p_parent,
                                avi_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("strh_body");
  m__io->pushName("fcc_type");
  m_fcc_type = static_cast<avi_t::stream_type_t>(m__io->read_u4le());
  m__io->popName();
  m__io->pushName("fcc_handler");
  m_fcc_handler = static_cast<avi_t::handler_type_t>(m__io->read_u4le());
  m__io->popName();
  m__io->pushName("flags");
  m_flags = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("priority");
  m_priority = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("language");
  m_language = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("initial_frames");
  m_initial_frames = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("scale");
  m_scale = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("rate");
  m_rate = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("start");
  m_start = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("length");
  m_length = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("suggested_buffer_size");
  m_suggested_buffer_size = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("quality");
  m_quality = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("sample_size");
  m_sample_size = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("frame");
  m_frame = new rect_t(m__io, this, m__root);
  m__io->popName();
  m__io->endChunk();
}

avi_t::strh_body_t::~strh_body_t() { delete m_frame; }

avi_t::strf_body_t::strf_body_t(kaitai::kstream* p_io,
                                kaitai::kstruct* p_parent, avi_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("strf_body");
  m__io->endChunk();
}

avi_t::strf_body_t::~strf_body_t() {}

}  // namespace avi
}  // namespace kaitai
}  // namespace veles
