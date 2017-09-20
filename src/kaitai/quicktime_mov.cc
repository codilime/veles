// This is a generated file! Please edit source .ksy file and use
// kaitai-struct-compiler to rebuild

#include "kaitai/quicktime_mov.h"

namespace veles {
namespace kaitai {
namespace quicktime_mov {

quicktime_mov_t::quicktime_mov_t(kaitai::kstream* p_io,
                                 kaitai::kstruct* p_parent,
                                 quicktime_mov_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = this;
  m__io->popName();
  veles_obj = m__io->startChunk("quicktime_mov");
  m_atoms = new std::vector<atom_t*>();
  while (!m__io->is_eof()) {
    m__io->pushName("atoms");
    m_atoms->push_back(new atom_t(m__io, this, m__root));
    m__io->popName();
  }
  m__io->endChunk();
}

quicktime_mov_t::~quicktime_mov_t() {
  for (std::vector<atom_t*>::iterator it = m_atoms->begin();
       it != m_atoms->end(); ++it) {
    delete *it;
  }
  delete m_atoms;
}

quicktime_mov_t::mvhd_body_t::mvhd_body_t(kaitai::kstream* p_io,
                                          quicktime_mov_t::atom_t* p_parent,
                                          quicktime_mov_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("mvhd_body");
  m__io->pushName("version");
  m_version = m__io->read_u1();
  m__io->popName();
  m__io->pushName("flags");
  m_flags = m__io->read_bytes(3);
  m__io->popName();
  m__io->pushName("creation_time");
  m_creation_time = m__io->read_u4be();
  m__io->popName();
  m__io->pushName("modification_time");
  m_modification_time = m__io->read_u4be();
  m__io->popName();
  m__io->pushName("time_scale");
  m_time_scale = m__io->read_u4be();
  m__io->popName();
  m__io->pushName("duration");
  m_duration = m__io->read_u4be();
  m__io->popName();
  m__io->pushName("preferred_rate");
  m_preferred_rate = new fixed32_t(m__io, this, m__root);
  m__io->popName();
  m__io->pushName("preferred_volume");
  m_preferred_volume = new fixed16_t(m__io, this, m__root);
  m__io->popName();
  m__io->pushName("reserved1");
  m_reserved1 = m__io->read_bytes(10);
  m__io->popName();
  m__io->pushName("matrix");
  m_matrix = m__io->read_bytes(36);
  m__io->popName();
  m__io->pushName("preview_time");
  m_preview_time = m__io->read_u4be();
  m__io->popName();
  m__io->pushName("preview_duration");
  m_preview_duration = m__io->read_u4be();
  m__io->popName();
  m__io->pushName("poster_time");
  m_poster_time = m__io->read_u4be();
  m__io->popName();
  m__io->pushName("selection_time");
  m_selection_time = m__io->read_u4be();
  m__io->popName();
  m__io->pushName("selection_duration");
  m_selection_duration = m__io->read_u4be();
  m__io->popName();
  m__io->pushName("current_time");
  m_current_time = m__io->read_u4be();
  m__io->popName();
  m__io->pushName("next_track_id");
  m_next_track_id = m__io->read_u4be();
  m__io->popName();
  m__io->endChunk();
}

quicktime_mov_t::mvhd_body_t::~mvhd_body_t() {
  delete m_preferred_rate;
  delete m_preferred_volume;
}

quicktime_mov_t::ftyp_body_t::ftyp_body_t(kaitai::kstream* p_io,
                                          quicktime_mov_t::atom_t* p_parent,
                                          quicktime_mov_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("ftyp_body");
  m__io->pushName("major_brand");
  m_major_brand = static_cast<quicktime_mov_t::brand_t>(m__io->read_u4be());
  m__io->popName();
  m__io->pushName("minor_version");
  m_minor_version = m__io->read_bytes(4);
  m__io->popName();
  m_compatible_brands = new std::vector<brand_t>();
  while (!m__io->is_eof()) {
    m__io->pushName("compatible_brands");
    m_compatible_brands->push_back(
        static_cast<quicktime_mov_t::brand_t>(m__io->read_u4be()));
    m__io->popName();
  }
  m__io->endChunk();
}

quicktime_mov_t::ftyp_body_t::~ftyp_body_t() { delete m_compatible_brands; }

quicktime_mov_t::fixed32_t::fixed32_t(kaitai::kstream* p_io,
                                      kaitai::kstruct* p_parent,
                                      quicktime_mov_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("fixed32");
  m__io->pushName("int_part");
  m_int_part = m__io->read_s2be();
  m__io->popName();
  m__io->pushName("frac_part");
  m_frac_part = m__io->read_u2be();
  m__io->popName();
  m__io->endChunk();
}

quicktime_mov_t::fixed32_t::~fixed32_t() {}

quicktime_mov_t::fixed16_t::fixed16_t(kaitai::kstream* p_io,
                                      quicktime_mov_t::mvhd_body_t* p_parent,
                                      quicktime_mov_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("fixed16");
  m__io->pushName("int_part");
  m_int_part = m__io->read_s1();
  m__io->popName();
  m__io->pushName("frac_part");
  m_frac_part = m__io->read_u1();
  m__io->popName();
  m__io->endChunk();
}

quicktime_mov_t::fixed16_t::~fixed16_t() {}

quicktime_mov_t::atom_t::atom_t(kaitai::kstream* p_io,
                                quicktime_mov_t* p_parent,
                                quicktime_mov_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("atom");
  f_len = false;
  m__io->pushName("len32");
  m_len32 = m__io->read_u4be();
  m__io->popName();
  m__io->pushName("atom_type");
  m_atom_type = static_cast<quicktime_mov_t::atom_type_t>(m__io->read_u4be());
  m__io->popName();
  n_len64 = true;
  if (len32() == 1) {
    n_len64 = false;
    m__io->pushName("len64");
    m_len64 = m__io->read_u8be();
    m__io->popName();
  }
  switch (atom_type()) {
    case ATOM_TYPE_STBL:
      m__io->pushName("_skip_me_body");
      m__skip_me_body = m__io->read_bytes(len());
      m__io->popName();
      m__io->pushName("m__skip_me_body" + 3);
      m__io__skip_me_body = new kaitai::kstream(
          m__io->blob(), m__io->pos() - m__skip_me_body.size(), veles_obj,
          m__io->pos(), m__io->error());
      m__io->popName();
      m__io->pushName("body");
      m_body = new quicktime_mov_t(m__io__skip_me_body);
      m__io->popName();
      break;
    case ATOM_TYPE_MOOF:
      m__io->pushName("_skip_me_body");
      m__skip_me_body = m__io->read_bytes(len());
      m__io->popName();
      m__io->pushName("m__skip_me_body" + 3);
      m__io__skip_me_body = new kaitai::kstream(
          m__io->blob(), m__io->pos() - m__skip_me_body.size(), veles_obj,
          m__io->pos(), m__io->error());
      m__io->popName();
      m__io->pushName("body");
      m_body = new quicktime_mov_t(m__io__skip_me_body);
      m__io->popName();
      break;
    case ATOM_TYPE_MVHD:
      m__io->pushName("_skip_me_body");
      m__skip_me_body = m__io->read_bytes(len());
      m__io->popName();
      m__io->pushName("m__skip_me_body" + 3);
      m__io__skip_me_body = new kaitai::kstream(
          m__io->blob(), m__io->pos() - m__skip_me_body.size(), veles_obj,
          m__io->pos(), m__io->error());
      m__io->popName();
      m__io->pushName("body");
      m_body = new mvhd_body_t(m__io__skip_me_body, this, m__root);
      m__io->popName();
      break;
    case ATOM_TYPE_MINF:
      m__io->pushName("_skip_me_body");
      m__skip_me_body = m__io->read_bytes(len());
      m__io->popName();
      m__io->pushName("m__skip_me_body" + 3);
      m__io__skip_me_body = new kaitai::kstream(
          m__io->blob(), m__io->pos() - m__skip_me_body.size(), veles_obj,
          m__io->pos(), m__io->error());
      m__io->popName();
      m__io->pushName("body");
      m_body = new quicktime_mov_t(m__io__skip_me_body);
      m__io->popName();
      break;
    case ATOM_TYPE_TRAK:
      m__io->pushName("_skip_me_body");
      m__skip_me_body = m__io->read_bytes(len());
      m__io->popName();
      m__io->pushName("m__skip_me_body" + 3);
      m__io__skip_me_body = new kaitai::kstream(
          m__io->blob(), m__io->pos() - m__skip_me_body.size(), veles_obj,
          m__io->pos(), m__io->error());
      m__io->popName();
      m__io->pushName("body");
      m_body = new quicktime_mov_t(m__io__skip_me_body);
      m__io->popName();
      break;
    case ATOM_TYPE_TRAF:
      m__io->pushName("_skip_me_body");
      m__skip_me_body = m__io->read_bytes(len());
      m__io->popName();
      m__io->pushName("m__skip_me_body" + 3);
      m__io__skip_me_body = new kaitai::kstream(
          m__io->blob(), m__io->pos() - m__skip_me_body.size(), veles_obj,
          m__io->pos(), m__io->error());
      m__io->popName();
      m__io->pushName("body");
      m_body = new quicktime_mov_t(m__io__skip_me_body);
      m__io->popName();
      break;
    case ATOM_TYPE_MDIA:
      m__io->pushName("_skip_me_body");
      m__skip_me_body = m__io->read_bytes(len());
      m__io->popName();
      m__io->pushName("m__skip_me_body" + 3);
      m__io__skip_me_body = new kaitai::kstream(
          m__io->blob(), m__io->pos() - m__skip_me_body.size(), veles_obj,
          m__io->pos(), m__io->error());
      m__io->popName();
      m__io->pushName("body");
      m_body = new quicktime_mov_t(m__io__skip_me_body);
      m__io->popName();
      break;
    case ATOM_TYPE_FTYP:
      m__io->pushName("_skip_me_body");
      m__skip_me_body = m__io->read_bytes(len());
      m__io->popName();
      m__io->pushName("m__skip_me_body" + 3);
      m__io__skip_me_body = new kaitai::kstream(
          m__io->blob(), m__io->pos() - m__skip_me_body.size(), veles_obj,
          m__io->pos(), m__io->error());
      m__io->popName();
      m__io->pushName("body");
      m_body = new ftyp_body_t(m__io__skip_me_body, this, m__root);
      m__io->popName();
      break;
    case ATOM_TYPE_MOOV:
      m__io->pushName("_skip_me_body");
      m__skip_me_body = m__io->read_bytes(len());
      m__io->popName();
      m__io->pushName("m__skip_me_body" + 3);
      m__io__skip_me_body = new kaitai::kstream(
          m__io->blob(), m__io->pos() - m__skip_me_body.size(), veles_obj,
          m__io->pos(), m__io->error());
      m__io->popName();
      m__io->pushName("body");
      m_body = new quicktime_mov_t(m__io__skip_me_body);
      m__io->popName();
      break;
    case ATOM_TYPE_TKHD:
      m__io->pushName("_skip_me_body");
      m__skip_me_body = m__io->read_bytes(len());
      m__io->popName();
      m__io->pushName("m__skip_me_body" + 3);
      m__io__skip_me_body = new kaitai::kstream(
          m__io->blob(), m__io->pos() - m__skip_me_body.size(), veles_obj,
          m__io->pos(), m__io->error());
      m__io->popName();
      m__io->pushName("body");
      m_body = new tkhd_body_t(m__io__skip_me_body, this, m__root);
      m__io->popName();
      break;
    case ATOM_TYPE_DINF:
      m__io->pushName("_skip_me_body");
      m__skip_me_body = m__io->read_bytes(len());
      m__io->popName();
      m__io->pushName("m__skip_me_body" + 3);
      m__io__skip_me_body = new kaitai::kstream(
          m__io->blob(), m__io->pos() - m__skip_me_body.size(), veles_obj,
          m__io->pos(), m__io->error());
      m__io->popName();
      m__io->pushName("body");
      m_body = new quicktime_mov_t(m__io__skip_me_body);
      m__io->popName();
      break;
    default:
      m__io->pushName("_skip_me_body");
      m__skip_me_body = m__io->read_bytes(len());
      m__io->popName();
      break;
  }
  m__io->endChunk();
}

quicktime_mov_t::atom_t::~atom_t() {}

int32_t quicktime_mov_t::atom_t::len() {
  if (f_len) {
    return m_len;
  }
  m__io->pushName("len");
  m__io->pushName("len");
  m_len = (len32() == 0)
              ? ((_io()->size() - 8))
              : ((len32() == 1) ? ((len64() - 16)) : ((len32() - 8)));
  m__io->popName();
  f_len = true;
  m__io->popName();
  return m_len;
}

quicktime_mov_t::tkhd_body_t::tkhd_body_t(kaitai::kstream* p_io,
                                          quicktime_mov_t::atom_t* p_parent,
                                          quicktime_mov_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("tkhd_body");
  m__io->pushName("version");
  m_version = m__io->read_u1();
  m__io->popName();
  m__io->pushName("flags");
  m_flags = m__io->read_bytes(3);
  m__io->popName();
  m__io->pushName("creation_time");
  m_creation_time = m__io->read_u4be();
  m__io->popName();
  m__io->pushName("modification_time");
  m_modification_time = m__io->read_u4be();
  m__io->popName();
  m__io->pushName("track_id");
  m_track_id = m__io->read_u4be();
  m__io->popName();
  m__io->pushName("reserved1");
  m_reserved1 = m__io->read_bytes(4);
  m__io->popName();
  m__io->pushName("duration");
  m_duration = m__io->read_u4be();
  m__io->popName();
  m__io->pushName("reserved2");
  m_reserved2 = m__io->read_bytes(8);
  m__io->popName();
  m__io->pushName("layer");
  m_layer = m__io->read_u2be();
  m__io->popName();
  m__io->pushName("alternative_group");
  m_alternative_group = m__io->read_u2be();
  m__io->popName();
  m__io->pushName("volume");
  m_volume = m__io->read_u2be();
  m__io->popName();
  m__io->pushName("reserved3");
  m_reserved3 = m__io->read_u2be();
  m__io->popName();
  m__io->pushName("matrix");
  m_matrix = m__io->read_bytes(36);
  m__io->popName();
  m__io->pushName("width");
  m_width = new fixed32_t(m__io, this, m__root);
  m__io->popName();
  m__io->pushName("height");
  m_height = new fixed32_t(m__io, this, m__root);
  m__io->popName();
  m__io->endChunk();
}

quicktime_mov_t::tkhd_body_t::~tkhd_body_t() {
  delete m_width;
  delete m_height;
}

}  // namespace quicktime_mov
}  // namespace kaitai
}  // namespace veles
