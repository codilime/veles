// This is a generated file! Please edit source .ksy file and use
// kaitai-struct-compiler to rebuild

#include "kaitai/elf.h"

namespace veles {
namespace kaitai {
namespace elf {

elf_t::elf_t(kaitai::kstream* p_io, kaitai::kstruct* p_parent, elf_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = this;
  m__io->popName();
  veles_obj = m__io->startChunk("elf");
  f_program_headers = false;
  f_section_headers = false;
  f_strings = false;
  m__io->pushName("file_header");
  m_file_header = new file_header_t(m__io, this, m__root);
  m__io->popName();
  m__io->endChunk();
}

elf_t::~elf_t() {
  delete m_file_header;
  delete m__skip_me_program_headers;
  for (std::vector<program_header_t*>::iterator it = m_program_headers->begin();
       it != m_program_headers->end(); ++it) {
    delete *it;
  }
  delete m_program_headers;
  delete m__skip_me_section_headers;
  for (std::vector<section_header_t*>::iterator it = m_section_headers->begin();
       it != m_section_headers->end(); ++it) {
    delete *it;
  }
  delete m_section_headers;
  if (f_strings) {
    delete m__io__skip_me_strings;
    delete m_strings;
  }
}

elf_t::file_header_t::file_header_t(kaitai::kstream* p_io, elf_t* p_parent,
                                    elf_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("file_header");
  m__io->pushName("m_magic" + 2);
  m_magic = m__io->ensure_fixed_contents(std::string("\x7F\x45\x4C\x46", 4));
  m__io->popName();
  m__io->pushName("bits");
  m_bits = static_cast<elf_t::bits_t>(m__io->read_u1());
  m__io->popName();
  m__io->pushName("endian");
  m_endian = static_cast<elf_t::endian_t>(m__io->read_u1());
  m__io->popName();
  m__io->pushName("ei_version");
  m_ei_version = m__io->read_u1();
  m__io->popName();
  m__io->pushName("abi");
  m_abi = static_cast<elf_t::os_abi_t>(m__io->read_u1());
  m__io->popName();
  m__io->pushName("abi_version");
  m_abi_version = m__io->read_u1();
  m__io->popName();
  m__io->pushName("pad");
  m_pad = m__io->read_bytes(7);
  m__io->popName();
  m__io->pushName("e_type");
  m_e_type = static_cast<elf_t::obj_type_t>(m__io->read_u2le());
  m__io->popName();
  m__io->pushName("machine");
  m_machine = static_cast<elf_t::machine_t>(m__io->read_u2le());
  m__io->popName();
  m__io->pushName("e_version");
  m_e_version = m__io->read_u4le();
  m__io->popName();
  switch (bits()) {
    case BITS_B32:
      m__io->pushName("entry_point");
      m_entry_point = m__io->read_u4le();
      m__io->popName();
      break;
    case BITS_B64:
      m__io->pushName("entry_point");
      m_entry_point = m__io->read_u8le();
      m__io->popName();
      break;
    default:
      break;
  }
  switch (bits()) {
    case BITS_B32:
      m__io->pushName("program_header_offset");
      m_program_header_offset = m__io->read_u4le();
      m__io->popName();
      break;
    case BITS_B64:
      m__io->pushName("program_header_offset");
      m_program_header_offset = m__io->read_u8le();
      m__io->popName();
      break;
    default:
      break;
  }
  switch (bits()) {
    case BITS_B32:
      m__io->pushName("section_header_offset");
      m_section_header_offset = m__io->read_u4le();
      m__io->popName();
      break;
    case BITS_B64:
      m__io->pushName("section_header_offset");
      m_section_header_offset = m__io->read_u8le();
      m__io->popName();
      break;
    default:
      break;
  }
  m__io->pushName("flags");
  m_flags = m__io->read_bytes(4);
  m__io->popName();
  m__io->pushName("e_ehsize");
  m_e_ehsize = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("program_header_entry_size");
  m_program_header_entry_size = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("qty_program_header");
  m_qty_program_header = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("section_header_entry_size");
  m_section_header_entry_size = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("qty_section_header");
  m_qty_section_header = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("section_names_idx");
  m_section_names_idx = m__io->read_u2le();
  m__io->popName();
  m__io->endChunk();
}

elf_t::file_header_t::~file_header_t() {}

elf_t::program_header_t::program_header_t(kaitai::kstream* p_io,
                                          elf_t* p_parent, elf_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("program_header");
  m__io->pushName("type");
  m_type = static_cast<elf_t::ph_type_t>(m__io->read_u4le());
  m__io->popName();
  n_flags64 = true;
  if (_root()->file_header()->bits() == BITS_B64) {
    n_flags64 = false;
    m__io->pushName("flags64");
    m_flags64 = m__io->read_u4le();
    m__io->popName();
  }
  switch (_root()->file_header()->bits()) {
    case BITS_B32:
      m__io->pushName("offset");
      m_offset = m__io->read_u4le();
      m__io->popName();
      break;
    case BITS_B64:
      m__io->pushName("offset");
      m_offset = m__io->read_u8le();
      m__io->popName();
      break;
    default:
      break;
  }
  switch (_root()->file_header()->bits()) {
    case BITS_B32:
      m__io->pushName("vaddr");
      m_vaddr = m__io->read_u4le();
      m__io->popName();
      break;
    case BITS_B64:
      m__io->pushName("vaddr");
      m_vaddr = m__io->read_u8le();
      m__io->popName();
      break;
    default:
      break;
  }
  switch (_root()->file_header()->bits()) {
    case BITS_B32:
      m__io->pushName("paddr");
      m_paddr = m__io->read_u4le();
      m__io->popName();
      break;
    case BITS_B64:
      m__io->pushName("paddr");
      m_paddr = m__io->read_u8le();
      m__io->popName();
      break;
    default:
      break;
  }
  switch (_root()->file_header()->bits()) {
    case BITS_B32:
      m__io->pushName("filesz");
      m_filesz = m__io->read_u4le();
      m__io->popName();
      break;
    case BITS_B64:
      m__io->pushName("filesz");
      m_filesz = m__io->read_u8le();
      m__io->popName();
      break;
    default:
      break;
  }
  switch (_root()->file_header()->bits()) {
    case BITS_B32:
      m__io->pushName("memsz");
      m_memsz = m__io->read_u4le();
      m__io->popName();
      break;
    case BITS_B64:
      m__io->pushName("memsz");
      m_memsz = m__io->read_u8le();
      m__io->popName();
      break;
    default:
      break;
  }
  n_flags32 = true;
  if (_root()->file_header()->bits() == BITS_B32) {
    n_flags32 = false;
    m__io->pushName("flags32");
    m_flags32 = m__io->read_u4le();
    m__io->popName();
  }
  switch (_root()->file_header()->bits()) {
    case BITS_B32:
      m__io->pushName("align");
      m_align = m__io->read_u4le();
      m__io->popName();
      break;
    case BITS_B64:
      m__io->pushName("align");
      m_align = m__io->read_u8le();
      m__io->popName();
      break;
    default:
      break;
  }
  m__io->endChunk();
}

elf_t::program_header_t::~program_header_t() {}

elf_t::section_header_t::section_header_t(kaitai::kstream* p_io,
                                          elf_t* p_parent, elf_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("section_header");
  f_body = false;
  f_name = false;
  m__io->pushName("name_offset");
  m_name_offset = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("type");
  m_type = static_cast<elf_t::sh_type_t>(m__io->read_u4le());
  m__io->popName();
  switch (_root()->file_header()->bits()) {
    case BITS_B32:
      m__io->pushName("flags");
      m_flags = m__io->read_u4le();
      m__io->popName();
      break;
    case BITS_B64:
      m__io->pushName("flags");
      m_flags = m__io->read_u8le();
      m__io->popName();
      break;
    default:
      break;
  }
  switch (_root()->file_header()->bits()) {
    case BITS_B32:
      m__io->pushName("addr");
      m_addr = m__io->read_u4le();
      m__io->popName();
      break;
    case BITS_B64:
      m__io->pushName("addr");
      m_addr = m__io->read_u8le();
      m__io->popName();
      break;
    default:
      break;
  }
  switch (_root()->file_header()->bits()) {
    case BITS_B32:
      m__io->pushName("offset");
      m_offset = m__io->read_u4le();
      m__io->popName();
      break;
    case BITS_B64:
      m__io->pushName("offset");
      m_offset = m__io->read_u8le();
      m__io->popName();
      break;
    default:
      break;
  }
  switch (_root()->file_header()->bits()) {
    case BITS_B32:
      m__io->pushName("size");
      m_size = m__io->read_u4le();
      m__io->popName();
      break;
    case BITS_B64:
      m__io->pushName("size");
      m_size = m__io->read_u8le();
      m__io->popName();
      break;
    default:
      break;
  }
  m__io->pushName("linked_section_idx");
  m_linked_section_idx = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("info");
  m_info = m__io->read_bytes(4);
  m__io->popName();
  switch (_root()->file_header()->bits()) {
    case BITS_B32:
      m__io->pushName("align");
      m_align = m__io->read_u4le();
      m__io->popName();
      break;
    case BITS_B64:
      m__io->pushName("align");
      m_align = m__io->read_u8le();
      m__io->popName();
      break;
    default:
      break;
  }
  switch (_root()->file_header()->bits()) {
    case BITS_B32:
      m__io->pushName("entry_size");
      m_entry_size = m__io->read_u4le();
      m__io->popName();
      break;
    case BITS_B64:
      m__io->pushName("entry_size");
      m_entry_size = m__io->read_u8le();
      m__io->popName();
      break;
    default:
      break;
  }
  m__io->endChunk();
}

elf_t::section_header_t::~section_header_t() {}

std::vector<uint8_t> elf_t::section_header_t::body() {
  if (f_body) {
    return m_body;
  }
  m__io->pushName("body");
  kaitai::kstream* io = _root()->_io();
  auto saved_io = io;
  auto saved_veles_obj = veles_obj;
  io = new kaitai::kstream(saved_io->blob(), offset(), veles_obj, 0,
                           saved_io->error());
  veles_obj = io->startChunk(saved_io->currentName());
  m__io->pushName("body");
  m_body = io->read_bytes(size());
  m__io->popName();
  io->endChunk();
  delete io;
  veles_obj = saved_veles_obj;
  f_body = true;
  m__io->popName();
  return m_body;
}

std::string elf_t::section_header_t::name() {
  if (f_name) {
    return m_name;
  }
  m__io->pushName("name");
  kaitai::kstream* io = _root()->strings()->_io();
  auto saved_io = io;
  auto saved_veles_obj = veles_obj;
  io = new kaitai::kstream(saved_io->blob(), name_offset(), veles_obj, 0,
                           saved_io->error());
  veles_obj = io->startChunk(saved_io->currentName());
  m__io->pushName("name");
  m_name = io->read_strz("ASCII", 0, false, true, true);
  m__io->popName();
  io->endChunk();
  delete io;
  veles_obj = saved_veles_obj;
  f_name = true;
  m__io->popName();
  return m_name;
}

elf_t::strings_t::strings_t(kaitai::kstream* p_io, elf_t* p_parent,
                            elf_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("strings");
  m_entries = new std::vector<std::string>();
  while (!m__io->is_eof()) {
    m__io->pushName("entries");
    m_entries->push_back(m__io->read_strz("ASCII", 0, false, true, true));
    m__io->popName();
  }
  m__io->endChunk();
}

elf_t::strings_t::~strings_t() { delete m_entries; }

std::vector<elf_t::program_header_t*>* elf_t::program_headers() {
  if (f_program_headers) {
    return m_program_headers;
  }
  m__io->pushName("program_headers");
  auto saved_io = m__io;
  auto saved_veles_obj = veles_obj;
  m__io = new kaitai::kstream(saved_io->blob(),
                              file_header()->program_header_offset(), veles_obj,
                              0, saved_io->error());
  veles_obj = m__io->startChunk(saved_io->currentName());
  int l_program_headers = file_header()->qty_program_header();
  m__skip_me_program_headers = new std::vector<std::vector<uint8_t>>();
  m__skip_me_program_headers->reserve(l_program_headers);
  m_program_headers = new std::vector<program_header_t*>();
  m_program_headers->reserve(l_program_headers);
  for (int i = 0; i < l_program_headers; i++) {
    m__io->pushName("_skip_me_program_headers");
    m__skip_me_program_headers->push_back(
        m__io->read_bytes(file_header()->program_header_entry_size()));
    m__io->popName();
    m__io->pushName(
        "m__skip_me_program_headers->at(m__skip_me_program_headers->size() - "
        "1)" +
        3);
    m__io__skip_me_program_headers =
        new kaitai::kstream(m__io->blob(),
                            m__io->pos() -
                                m__skip_me_program_headers
                                    ->at(m__skip_me_program_headers->size() - 1)
                                    .size(),
                            veles_obj, m__io->pos(), m__io->error());
    m__io->popName();
    m__io->pushName("program_headers");
    m_program_headers->push_back(
        new program_header_t(m__io__skip_me_program_headers, this, m__root));
    m__io->popName();
  }
  m__io->endChunk();
  delete m__io;
  veles_obj = saved_veles_obj;
  m__io = saved_io;
  f_program_headers = true;
  m__io->popName();
  return m_program_headers;
}

std::vector<elf_t::section_header_t*>* elf_t::section_headers() {
  if (f_section_headers) {
    return m_section_headers;
  }
  m__io->pushName("section_headers");
  auto saved_io = m__io;
  auto saved_veles_obj = veles_obj;
  m__io = new kaitai::kstream(saved_io->blob(),
                              file_header()->section_header_offset(), veles_obj,
                              0, saved_io->error());
  veles_obj = m__io->startChunk(saved_io->currentName());
  int l_section_headers = file_header()->qty_section_header();
  m__skip_me_section_headers = new std::vector<std::vector<uint8_t>>();
  m__skip_me_section_headers->reserve(l_section_headers);
  m_section_headers = new std::vector<section_header_t*>();
  m_section_headers->reserve(l_section_headers);
  for (int i = 0; i < l_section_headers; i++) {
    m__io->pushName("_skip_me_section_headers");
    m__skip_me_section_headers->push_back(
        m__io->read_bytes(file_header()->section_header_entry_size()));
    m__io->popName();
    m__io->pushName(
        "m__skip_me_section_headers->at(m__skip_me_section_headers->size() - "
        "1)" +
        3);
    m__io__skip_me_section_headers =
        new kaitai::kstream(m__io->blob(),
                            m__io->pos() -
                                m__skip_me_section_headers
                                    ->at(m__skip_me_section_headers->size() - 1)
                                    .size(),
                            veles_obj, m__io->pos(), m__io->error());
    m__io->popName();
    m__io->pushName("section_headers");
    m_section_headers->push_back(
        new section_header_t(m__io__skip_me_section_headers, this, m__root));
    m__io->popName();
  }
  m__io->endChunk();
  delete m__io;
  veles_obj = saved_veles_obj;
  m__io = saved_io;
  f_section_headers = true;
  m__io->popName();
  return m_section_headers;
}

elf_t::strings_t* elf_t::strings() {
  if (f_strings) {
    return m_strings;
  }
  m__io->pushName("strings");
  auto saved_io = m__io;
  auto saved_veles_obj = veles_obj;
  m__io = new kaitai::kstream(
      saved_io->blob(),
      section_headers()->at(file_header()->section_names_idx())->offset(),
      veles_obj, 0, saved_io->error());
  veles_obj = m__io->startChunk(saved_io->currentName());
  m__io->pushName("_skip_me_strings");
  m__skip_me_strings = m__io->read_bytes(
      section_headers()->at(file_header()->section_names_idx())->size());
  m__io->popName();
  m__io->pushName("m__skip_me_strings" + 3);
  m__io__skip_me_strings = new kaitai::kstream(
      m__io->blob(), m__io->pos() - m__skip_me_strings.size(), veles_obj,
      m__io->pos(), m__io->error());
  m__io->popName();
  m__io->pushName("strings");
  m_strings = new strings_t(m__io__skip_me_strings, this, m__root);
  m__io->popName();
  m__io->endChunk();
  delete m__io;
  veles_obj = saved_veles_obj;
  m__io = saved_io;
  f_strings = true;
  m__io->popName();
  return m_strings;
}

}  // namespace elf
}  // namespace kaitai
}  // namespace veles
