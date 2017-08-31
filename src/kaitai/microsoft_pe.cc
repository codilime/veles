// This is a generated file! Please edit source .ksy file and use
// kaitai-struct-compiler to rebuild

#include "kaitai/microsoft_pe.h"

namespace veles {
namespace kaitai {
namespace microsoft_pe {

microsoft_pe_t::microsoft_pe_t(kaitai::kstream* p_io, kaitai::kstruct* p_parent,
                               microsoft_pe_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = this;
  m__io->popName();
  veles_obj = m__io->startChunk("microsoft_pe");
  m__io->pushName("mz1");
  m_mz1 = new mz_placeholder_t(m__io, this, m__root);
  m__io->popName();
  m__io->pushName("mz2");
  m_mz2 = m__io->read_bytes((mz1()->header_size() - 64));
  m__io->popName();
  m__io->pushName("m_pe_signature" + 2);
  m_pe_signature =
      m__io->ensure_fixed_contents(std::string("\x50\x45\x00\x00", 4));
  m__io->popName();
  m__io->pushName("coff_header");
  m_coff_header = new coff_header_t(m__io, this, m__root);
  m__io->popName();
  m__io->pushName("_skip_me_optional_header");
  m__skip_me_optional_header =
      m__io->read_bytes(coff_header()->size_of_optional_header());
  m__io->popName();
  m__io->pushName("m__skip_me_optional_header" + 3);
  m__io__skip_me_optional_header = new kaitai::kstream(
      m__io->blob(), m__io->pos() - m__skip_me_optional_header.size(),
      veles_obj, m__io->pos(), m__io->error());
  m__io->popName();
  m__io->pushName("optional_header");
  m_optional_header =
      new optional_header_t(m__io__skip_me_optional_header, this, m__root);
  m__io->popName();
  int l_sections = coff_header()->number_of_sections();
  m_sections = new std::vector<section_t*>();
  m_sections->reserve(l_sections);
  for (int i = 0; i < l_sections; i++) {
    m__io->pushName("sections");
    m_sections->push_back(new section_t(m__io, this, m__root));
    m__io->popName();
  }
  m__io->endChunk();
}

microsoft_pe_t::~microsoft_pe_t() {
  delete m_mz1;
  delete m_coff_header;
  delete m__io__skip_me_optional_header;
  delete m_optional_header;
  for (std::vector<section_t*>::iterator it = m_sections->begin();
       it != m_sections->end(); ++it) {
    delete *it;
  }
  delete m_sections;
}

microsoft_pe_t::optional_header_windows_t::optional_header_windows_t(
    kaitai::kstream* p_io, microsoft_pe_t::optional_header_t* p_parent,
    microsoft_pe_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("optional_header_windows");
  n_image_base = true;
  if (_parent()->std()->format() == 267) {
    n_image_base = false;
    m__io->pushName("image_base");
    m_image_base = m__io->read_u4le();
    m__io->popName();
  }
  n_image_base2 = true;
  if (_parent()->std()->format() == 523) {
    n_image_base2 = false;
    m__io->pushName("image_base2");
    m_image_base2 = m__io->read_u8le();
    m__io->popName();
  }
  m__io->pushName("section_alignment");
  m_section_alignment = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("file_alignment");
  m_file_alignment = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("major_operating_system_version");
  m_major_operating_system_version = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("minor_operating_system_version");
  m_minor_operating_system_version = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("major_image_version");
  m_major_image_version = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("minor_image_version");
  m_minor_image_version = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("major_subsystem_version");
  m_major_subsystem_version = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("minor_subsystem_version");
  m_minor_subsystem_version = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("win32_version_value");
  m_win32_version_value = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("size_of_image");
  m_size_of_image = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("size_of_headers");
  m_size_of_headers = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("check_sum");
  m_check_sum = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("subsystem");
  m_subsystem =
      static_cast<microsoft_pe_t::optional_header_windows_t::subsystem_t>(
          m__io->read_u2le());
  m__io->popName();
  m__io->pushName("dll_characteristics");
  m_dll_characteristics = m__io->read_u2le();
  m__io->popName();
  n_size_of_stack_reserve = true;
  if (_parent()->std()->format() == 267) {
    n_size_of_stack_reserve = false;
    m__io->pushName("size_of_stack_reserve");
    m_size_of_stack_reserve = m__io->read_u4le();
    m__io->popName();
  }
  n_size_of_stack_reserve2 = true;
  if (_parent()->std()->format() == 523) {
    n_size_of_stack_reserve2 = false;
    m__io->pushName("size_of_stack_reserve2");
    m_size_of_stack_reserve2 = m__io->read_u8le();
    m__io->popName();
  }
  n_size_of_stack_commit = true;
  if (_parent()->std()->format() == 267) {
    n_size_of_stack_commit = false;
    m__io->pushName("size_of_stack_commit");
    m_size_of_stack_commit = m__io->read_u4le();
    m__io->popName();
  }
  n_size_of_stack_commit2 = true;
  if (_parent()->std()->format() == 523) {
    n_size_of_stack_commit2 = false;
    m__io->pushName("size_of_stack_commit2");
    m_size_of_stack_commit2 = m__io->read_u8le();
    m__io->popName();
  }
  n_size_of_heap_reserve = true;
  if (_parent()->std()->format() == 267) {
    n_size_of_heap_reserve = false;
    m__io->pushName("size_of_heap_reserve");
    m_size_of_heap_reserve = m__io->read_u4le();
    m__io->popName();
  }
  n_size_of_heap_reserve2 = true;
  if (_parent()->std()->format() == 523) {
    n_size_of_heap_reserve2 = false;
    m__io->pushName("size_of_heap_reserve2");
    m_size_of_heap_reserve2 = m__io->read_u8le();
    m__io->popName();
  }
  n_size_of_heap_commit = true;
  if (_parent()->std()->format() == 267) {
    n_size_of_heap_commit = false;
    m__io->pushName("size_of_heap_commit");
    m_size_of_heap_commit = m__io->read_u4le();
    m__io->popName();
  }
  n_size_of_heap_commit2 = true;
  if (_parent()->std()->format() == 523) {
    n_size_of_heap_commit2 = false;
    m__io->pushName("size_of_heap_commit2");
    m_size_of_heap_commit2 = m__io->read_u8le();
    m__io->popName();
  }
  m__io->pushName("loader_flags");
  m_loader_flags = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("number_of_rva_and_sizes");
  m_number_of_rva_and_sizes = m__io->read_u4le();
  m__io->popName();
  m__io->endChunk();
}

microsoft_pe_t::optional_header_windows_t::~optional_header_windows_t() {}

microsoft_pe_t::optional_header_data_dirs_t::optional_header_data_dirs_t(
    kaitai::kstream* p_io, microsoft_pe_t::optional_header_t* p_parent,
    microsoft_pe_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("optional_header_data_dirs");
  m__io->pushName("export_table");
  m_export_table = new data_dir_t(m__io, this, m__root);
  m__io->popName();
  m__io->pushName("import_table");
  m_import_table = new data_dir_t(m__io, this, m__root);
  m__io->popName();
  m__io->pushName("resource_table");
  m_resource_table = new data_dir_t(m__io, this, m__root);
  m__io->popName();
  m__io->pushName("exception_table");
  m_exception_table = new data_dir_t(m__io, this, m__root);
  m__io->popName();
  m__io->pushName("certificate_table");
  m_certificate_table = new data_dir_t(m__io, this, m__root);
  m__io->popName();
  m__io->pushName("base_relocation_table");
  m_base_relocation_table = new data_dir_t(m__io, this, m__root);
  m__io->popName();
  m__io->pushName("debug");
  m_debug = new data_dir_t(m__io, this, m__root);
  m__io->popName();
  m__io->pushName("architecture");
  m_architecture = new data_dir_t(m__io, this, m__root);
  m__io->popName();
  m__io->pushName("global_ptr");
  m_global_ptr = new data_dir_t(m__io, this, m__root);
  m__io->popName();
  m__io->pushName("tls_table");
  m_tls_table = new data_dir_t(m__io, this, m__root);
  m__io->popName();
  m__io->pushName("load_config_table");
  m_load_config_table = new data_dir_t(m__io, this, m__root);
  m__io->popName();
  m__io->pushName("bound_import");
  m_bound_import = new data_dir_t(m__io, this, m__root);
  m__io->popName();
  m__io->pushName("iat");
  m_iat = new data_dir_t(m__io, this, m__root);
  m__io->popName();
  m__io->pushName("delay_import_descriptor");
  m_delay_import_descriptor = new data_dir_t(m__io, this, m__root);
  m__io->popName();
  m__io->pushName("clr_runtime_header");
  m_clr_runtime_header = new data_dir_t(m__io, this, m__root);
  m__io->popName();
  m__io->endChunk();
}

microsoft_pe_t::optional_header_data_dirs_t::~optional_header_data_dirs_t() {
  delete m_export_table;
  delete m_import_table;
  delete m_resource_table;
  delete m_exception_table;
  delete m_certificate_table;
  delete m_base_relocation_table;
  delete m_debug;
  delete m_architecture;
  delete m_global_ptr;
  delete m_tls_table;
  delete m_load_config_table;
  delete m_bound_import;
  delete m_iat;
  delete m_delay_import_descriptor;
  delete m_clr_runtime_header;
}

microsoft_pe_t::data_dir_t::data_dir_t(
    kaitai::kstream* p_io,
    microsoft_pe_t::optional_header_data_dirs_t* p_parent,
    microsoft_pe_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("data_dir");
  m__io->pushName("virtual_address");
  m_virtual_address = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("size");
  m_size = m__io->read_u4le();
  m__io->popName();
  m__io->endChunk();
}

microsoft_pe_t::data_dir_t::~data_dir_t() {}

microsoft_pe_t::optional_header_t::optional_header_t(kaitai::kstream* p_io,
                                                     microsoft_pe_t* p_parent,
                                                     microsoft_pe_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("optional_header");
  m__io->pushName("std");
  m_std = new optional_header_std_t(m__io, this, m__root);
  m__io->popName();
  m__io->pushName("windows");
  m_windows = new optional_header_windows_t(m__io, this, m__root);
  m__io->popName();
  m__io->pushName("data_dirs");
  m_data_dirs = new optional_header_data_dirs_t(m__io, this, m__root);
  m__io->popName();
  m__io->endChunk();
}

microsoft_pe_t::optional_header_t::~optional_header_t() {
  delete m_std;
  delete m_windows;
  delete m_data_dirs;
}

microsoft_pe_t::section_t::section_t(kaitai::kstream* p_io,
                                     microsoft_pe_t* p_parent,
                                     microsoft_pe_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("section");
  f_body = false;
  m__io->pushName("name");
  m_name = m__io->read_str_byte_limit(8, "UTF-8");
  m__io->popName();
  m__io->pushName("virtual_size");
  m_virtual_size = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("virtual_address");
  m_virtual_address = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("size_of_raw_data");
  m_size_of_raw_data = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("pointer_to_raw_data");
  m_pointer_to_raw_data = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("pointer_to_relocations");
  m_pointer_to_relocations = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("pointer_to_linenumbers");
  m_pointer_to_linenumbers = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("number_of_relocations");
  m_number_of_relocations = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("number_of_linenumbers");
  m_number_of_linenumbers = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("characteristics");
  m_characteristics = m__io->read_u4le();
  m__io->popName();
  m__io->endChunk();
}

microsoft_pe_t::section_t::~section_t() {}

std::vector<uint8_t> microsoft_pe_t::section_t::body() {
  if (f_body) {
    return m_body;
  }
  m__io->pushName("body");
  auto saved_io = m__io;
  auto saved_veles_obj = veles_obj;
  m__io = new kaitai::kstream(saved_io->blob(), pointer_to_raw_data(),
                              veles_obj, 0, saved_io->error());
  veles_obj = m__io->startChunk(saved_io->currentName());
  m__io->pushName("body");
  m_body = m__io->read_bytes(size_of_raw_data());
  m__io->popName();
  m__io->endChunk();
  delete m__io;
  veles_obj = saved_veles_obj;
  m__io = saved_io;
  f_body = true;
  m__io->popName();
  return m_body;
}

microsoft_pe_t::mz_placeholder_t::mz_placeholder_t(kaitai::kstream* p_io,
                                                   microsoft_pe_t* p_parent,
                                                   microsoft_pe_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("mz_placeholder");
  m__io->pushName("m_magic" + 2);
  m_magic = m__io->ensure_fixed_contents(std::string("\x4D\x5A", 2));
  m__io->popName();
  m__io->pushName("data1");
  m_data1 = m__io->read_bytes(58);
  m__io->popName();
  m__io->pushName("header_size");
  m_header_size = m__io->read_u4le();
  m__io->popName();
  m__io->endChunk();
}

microsoft_pe_t::mz_placeholder_t::~mz_placeholder_t() {}

microsoft_pe_t::optional_header_std_t::optional_header_std_t(
    kaitai::kstream* p_io, microsoft_pe_t::optional_header_t* p_parent,
    microsoft_pe_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("optional_header_std");
  m__io->pushName("format");
  m_format = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("major_linker_version");
  m_major_linker_version = m__io->read_u1();
  m__io->popName();
  m__io->pushName("minor_linker_version");
  m_minor_linker_version = m__io->read_u1();
  m__io->popName();
  m__io->pushName("size_of_code");
  m_size_of_code = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("size_of_initialized_data");
  m_size_of_initialized_data = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("size_of_uninitialized_data");
  m_size_of_uninitialized_data = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("address_of_entry_point");
  m_address_of_entry_point = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("base_of_code");
  m_base_of_code = m__io->read_u4le();
  m__io->popName();
  n_base_of_data = true;
  if (format() == 267) {
    n_base_of_data = false;
    m__io->pushName("base_of_data");
    m_base_of_data = m__io->read_u4le();
    m__io->popName();
  }
  m__io->endChunk();
}

microsoft_pe_t::optional_header_std_t::~optional_header_std_t() {}

microsoft_pe_t::coff_header_t::coff_header_t(kaitai::kstream* p_io,
                                             microsoft_pe_t* p_parent,
                                             microsoft_pe_t* p_root)
    : kaitai::kstruct(p_io) {
  m__io->pushName("_parent");
  m__parent = p_parent;
  m__io->popName();
  m__io->pushName("_root");
  m__root = p_root;
  m__io->popName();
  veles_obj = m__io->startChunk("coff_header");
  m__io->pushName("machine");
  m_machine = static_cast<microsoft_pe_t::coff_header_t::machine_type_t>(
      m__io->read_u2le());
  m__io->popName();
  m__io->pushName("number_of_sections");
  m_number_of_sections = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("time_date_stamp");
  m_time_date_stamp = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("pointer_to_symbol_table");
  m_pointer_to_symbol_table = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("number_of_symbols");
  m_number_of_symbols = m__io->read_u4le();
  m__io->popName();
  m__io->pushName("size_of_optional_header");
  m_size_of_optional_header = m__io->read_u2le();
  m__io->popName();
  m__io->pushName("characteristics");
  m_characteristics = m__io->read_u2le();
  m__io->popName();
  m__io->endChunk();
}

microsoft_pe_t::coff_header_t::~coff_header_t() {}

}  // namespace microsoft_pe
}  // namespace kaitai
}  // namespace veles
