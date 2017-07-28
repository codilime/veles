// This is a generated file! Please edit source .ksy file and use
// kaitai-struct-compiler to rebuild

#include "kaitai/zip.h"

namespace veles {
namespace kaitai {
namespace zip {

zip_t::zip_t(kaitai::kstream* p_io, kaitai::kstruct* p_parent, zip_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = this;
  m__io->popName();
  veles_obj = m__io->startChunk("zip");
  m_sections = new std::vector<pk_section_t*>();
  while (!m__io->is_eof()) {
    m__io->pushName("sections");
    m_sections->push_back(new pk_section_t(m__io, this, m__root));
    m__io->popName();
  }
  m__io->endChunk();
}

zip_t::~zip_t() {
  for (std::vector<pk_section_t*>::iterator it = m_sections->begin();
       it != m_sections->end(); ++it) {
    delete *it;
  }
  delete m_sections;
}

zip_t::local_file_t::local_file_t(kaitai::kstream* p_io,
                                  zip_t::pk_section_t* p_parent, zip_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("local_file");
  m__io->pushName("header");
  m_header = new local_file_header_t(m__io, this, m__root);
  m__io->popName();
  m__io->pushName("body");
  m_body = m__io->read_bytes(header()->compressed_size());
  m__io->popName();
  m__io->endChunk();
}

zip_t::local_file_t::~local_file_t() { delete m_header; }

zip_t::central_dir_entry_t::central_dir_entry_t(kaitai::kstream* p_io,
                                                zip_t::pk_section_t* p_parent,
                                                zip_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("central_dir_entry");
  m__io->pushName("version_made_by");
  m_version_made_by = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("version_needed_to_extract");
  m_version_needed_to_extract = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("flags");
  m_flags = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("compression_method");
  m_compression_method = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("last_mod_file_time");
  m_last_mod_file_time = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("last_mod_file_date");
  m_last_mod_file_date = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("crc32");
  m_crc32 = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("compressed_size");
  m_compressed_size = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("uncompressed_size");
  m_uncompressed_size = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("file_name_len");
  m_file_name_len = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("extra_len");
  m_extra_len = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("comment_len");
  m_comment_len = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("disk_number_start");
  m_disk_number_start = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("int_file_attr");
  m_int_file_attr = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("ext_file_attr");
  m_ext_file_attr = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("local_header_offset");
  m_local_header_offset = m__io->read_s4le();
  m__io->popName();
  m__io->pushName("file_name");
  m_file_name = m__io->read_str_byte_limit(file_name_len(), "UTF-8");
  m__io->popName();
  m__io->pushName("extra");
  m_extra = m__io->read_bytes(extra_len());
  m__io->popName();
  m__io->pushName("comment");
  m_comment = m__io->read_str_byte_limit(comment_len(), "UTF-8");
  m__io->popName();
  m__io->endChunk();
}

zip_t::central_dir_entry_t::~central_dir_entry_t() {}

zip_t::pk_section_t::pk_section_t(kaitai::kstream* p_io, zip_t* p_parent,
                                  zip_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("pk_section");
  m__io->pushName("m_magic" + 2);
  m_magic = m__io->ensure_fixed_contents(std::string("\x50\x4B", 2));
  m__io->popName();
  m__io->pushName("section_type");
  m_section_type = m__io->read_u2le();
  m__io->popName();
  switch (section_type()) {
    case 513:
      m__io->pushName("body");
      m_body = new central_dir_entry_t(m__io, this, m__root);
      m__io->popName();
      break;
    case 1027:
      m__io->pushName("body");
      m_body = new local_file_t(m__io, this, m__root);
      m__io->popName();
      break;
    case 1541:
      m__io->pushName("body");
      m_body = new end_of_central_dir_t(m__io, this, m__root);
      m__io->popName();
      break;
    default:
      break;
  }
  m__io->endChunk();
}

zip_t::pk_section_t::~pk_section_t() {}

zip_t::local_file_header_t::local_file_header_t(kaitai::kstream* p_io,
                                                zip_t::local_file_t* p_parent,
                                                zip_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("local_file_header");
  m__io->pushName("version");
  m_version = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("flags");
  m_flags = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("compression");
  m_compression = static_cast<zip_t::compression_t>(m__io->read_u2le());
  m__io->popName();
  m__io->pushName("file_mod_time");
  m_file_mod_time = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("file_mod_date");
  m_file_mod_date = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("crc32");
  m_crc32 = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("compressed_size");
  m_compressed_size = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("uncompressed_size");
  m_uncompressed_size = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("file_name_len");
  m_file_name_len = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("extra_len");
  m_extra_len = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("file_name");
  m_file_name = m__io->read_str_byte_limit(file_name_len(), "UTF-8");
  m__io->popName();
  m__io->pushName("extra");
  m_extra = m__io->read_bytes(extra_len());
  m__io->popName();
  m__io->endChunk();
}

zip_t::local_file_header_t::~local_file_header_t() {}

zip_t::end_of_central_dir_t::end_of_central_dir_t(kaitai::kstream* p_io,
                                                  zip_t::pk_section_t* p_parent,
                                                  zip_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("end_of_central_dir");
  m__io->pushName("disk_of_end_of_central_dir");
  m_disk_of_end_of_central_dir = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("disk_of_central_dir");
  m_disk_of_central_dir = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("qty_central_dir_entries_on_disk");
  m_qty_central_dir_entries_on_disk = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("qty_central_dir_entries_total");
  m_qty_central_dir_entries_total = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("central_dir_size");
  m_central_dir_size = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("central_dir_offset");
  m_central_dir_offset = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("comment_len");
  m_comment_len = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("comment");
  m_comment = m__io->read_str_byte_limit(comment_len(), "UTF-8");
  m__io->popName();
  m__io->endChunk();
}

zip_t::end_of_central_dir_t::~end_of_central_dir_t() {}

}  // namespace zip
}  // namespace kaitai
}  // namespace veles
