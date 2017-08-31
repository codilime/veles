#ifndef MICROSOFT_PE_H_
#define MICROSOFT_PE_H_

// This is a generated file! Please edit source .ksy file and use
// kaitai-struct-compiler to rebuild

#include <kaitai/kaitaistream.h>
#include <kaitai/kaitaistruct.h>

#include <cstdint>
#include <vector>

namespace veles {
namespace kaitai {
namespace microsoft_pe {

class microsoft_pe_t : public kaitai::kstruct {
 public:
  class optional_header_windows_t;
  class optional_header_data_dirs_t;
  class data_dir_t;
  class optional_header_t;
  class section_t;
  class mz_placeholder_t;
  class optional_header_std_t;
  class coff_header_t;

  explicit microsoft_pe_t(kaitai::kstream* p_io,
                          kaitai::kstruct* p_parent = nullptr,
                          microsoft_pe_t* p_root = nullptr);
  veles::dbif::ObjectHandle veles_obj;
  ~microsoft_pe_t();

  class optional_header_windows_t : public kaitai::kstruct {
   public:
    enum subsystem_t {
      SUBSYSTEM_UNKNOWN = 0,
      SUBSYSTEM_NATIVE = 1,
      SUBSYSTEM_WINDOWS_GUI = 2,
      SUBSYSTEM_WINDOWS_CUI = 3,
      SUBSYSTEM_POSIX_CUI = 7,
      SUBSYSTEM_WINDOWS_CE_GUI = 9,
      SUBSYSTEM_EFI_APPLICATION = 10,
      SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER = 11,
      SUBSYSTEM_EFI_RUNTIME_DRIVER = 12,
      SUBSYSTEM_EFI_ROM = 13,
      SUBSYSTEM_XBOX = 14
    };

    explicit optional_header_windows_t(
        kaitai::kstream* p_io,
        microsoft_pe_t::optional_header_t* p_parent = nullptr,
        microsoft_pe_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~optional_header_windows_t();

   private:
    uint32_t m_image_base;
    bool n_image_base;

   public:
    bool _is_null_image_base() {
      image_base();
      return n_image_base;
    };

   private:
    uint64_t m_image_base2;
    bool n_image_base2;

   public:
    bool _is_null_image_base2() {
      image_base2();
      return n_image_base2;
    };

   private:
    uint32_t m_section_alignment;
    uint32_t m_file_alignment;
    uint16_t m_major_operating_system_version;
    uint16_t m_minor_operating_system_version;
    uint16_t m_major_image_version;
    uint16_t m_minor_image_version;
    uint16_t m_major_subsystem_version;
    uint16_t m_minor_subsystem_version;
    uint32_t m_win32_version_value;
    uint32_t m_size_of_image;
    uint32_t m_size_of_headers;
    uint32_t m_check_sum;
    subsystem_t m_subsystem;
    uint16_t m_dll_characteristics;
    uint32_t m_size_of_stack_reserve;
    bool n_size_of_stack_reserve;

   public:
    bool _is_null_size_of_stack_reserve() {
      size_of_stack_reserve();
      return n_size_of_stack_reserve;
    };

   private:
    uint64_t m_size_of_stack_reserve2;
    bool n_size_of_stack_reserve2;

   public:
    bool _is_null_size_of_stack_reserve2() {
      size_of_stack_reserve2();
      return n_size_of_stack_reserve2;
    };

   private:
    uint32_t m_size_of_stack_commit;
    bool n_size_of_stack_commit;

   public:
    bool _is_null_size_of_stack_commit() {
      size_of_stack_commit();
      return n_size_of_stack_commit;
    };

   private:
    uint64_t m_size_of_stack_commit2;
    bool n_size_of_stack_commit2;

   public:
    bool _is_null_size_of_stack_commit2() {
      size_of_stack_commit2();
      return n_size_of_stack_commit2;
    };

   private:
    uint32_t m_size_of_heap_reserve;
    bool n_size_of_heap_reserve;

   public:
    bool _is_null_size_of_heap_reserve() {
      size_of_heap_reserve();
      return n_size_of_heap_reserve;
    };

   private:
    uint64_t m_size_of_heap_reserve2;
    bool n_size_of_heap_reserve2;

   public:
    bool _is_null_size_of_heap_reserve2() {
      size_of_heap_reserve2();
      return n_size_of_heap_reserve2;
    };

   private:
    uint32_t m_size_of_heap_commit;
    bool n_size_of_heap_commit;

   public:
    bool _is_null_size_of_heap_commit() {
      size_of_heap_commit();
      return n_size_of_heap_commit;
    };

   private:
    uint64_t m_size_of_heap_commit2;
    bool n_size_of_heap_commit2;

   public:
    bool _is_null_size_of_heap_commit2() {
      size_of_heap_commit2();
      return n_size_of_heap_commit2;
    };

   private:
    uint32_t m_loader_flags;
    uint32_t m_number_of_rva_and_sizes;
    microsoft_pe_t* m__root;
    microsoft_pe_t::optional_header_t* m__parent;

   public:
    uint32_t image_base() const { return m_image_base; }
    uint64_t image_base2() const { return m_image_base2; }
    uint32_t section_alignment() const { return m_section_alignment; }
    uint32_t file_alignment() const { return m_file_alignment; }
    uint16_t major_operating_system_version() const {
      return m_major_operating_system_version;
    }
    uint16_t minor_operating_system_version() const {
      return m_minor_operating_system_version;
    }
    uint16_t major_image_version() const { return m_major_image_version; }
    uint16_t minor_image_version() const { return m_minor_image_version; }
    uint16_t major_subsystem_version() const {
      return m_major_subsystem_version;
    }
    uint16_t minor_subsystem_version() const {
      return m_minor_subsystem_version;
    }
    uint32_t win32_version_value() const { return m_win32_version_value; }
    uint32_t size_of_image() const { return m_size_of_image; }
    uint32_t size_of_headers() const { return m_size_of_headers; }
    uint32_t check_sum() const { return m_check_sum; }
    subsystem_t subsystem() const { return m_subsystem; }
    uint16_t dll_characteristics() const { return m_dll_characteristics; }
    uint32_t size_of_stack_reserve() const { return m_size_of_stack_reserve; }
    uint64_t size_of_stack_reserve2() const { return m_size_of_stack_reserve2; }
    uint32_t size_of_stack_commit() const { return m_size_of_stack_commit; }
    uint64_t size_of_stack_commit2() const { return m_size_of_stack_commit2; }
    uint32_t size_of_heap_reserve() const { return m_size_of_heap_reserve; }
    uint64_t size_of_heap_reserve2() const { return m_size_of_heap_reserve2; }
    uint32_t size_of_heap_commit() const { return m_size_of_heap_commit; }
    uint64_t size_of_heap_commit2() const { return m_size_of_heap_commit2; }
    uint32_t loader_flags() const { return m_loader_flags; }
    uint32_t number_of_rva_and_sizes() const {
      return m_number_of_rva_and_sizes;
    }
    microsoft_pe_t* _root() const { return m__root; }
    microsoft_pe_t::optional_header_t* _parent() const { return m__parent; }
  };

  class optional_header_data_dirs_t : public kaitai::kstruct {
   public:
    explicit optional_header_data_dirs_t(
        kaitai::kstream* p_io,
        microsoft_pe_t::optional_header_t* p_parent = nullptr,
        microsoft_pe_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~optional_header_data_dirs_t();

   private:
    data_dir_t* m_export_table;
    data_dir_t* m_import_table;
    data_dir_t* m_resource_table;
    data_dir_t* m_exception_table;
    data_dir_t* m_certificate_table;
    data_dir_t* m_base_relocation_table;
    data_dir_t* m_debug;
    data_dir_t* m_architecture;
    data_dir_t* m_global_ptr;
    data_dir_t* m_tls_table;
    data_dir_t* m_load_config_table;
    data_dir_t* m_bound_import;
    data_dir_t* m_iat;
    data_dir_t* m_delay_import_descriptor;
    data_dir_t* m_clr_runtime_header;
    microsoft_pe_t* m__root;
    microsoft_pe_t::optional_header_t* m__parent;

   public:
    data_dir_t* export_table() const { return m_export_table; }
    data_dir_t* import_table() const { return m_import_table; }
    data_dir_t* resource_table() const { return m_resource_table; }
    data_dir_t* exception_table() const { return m_exception_table; }
    data_dir_t* certificate_table() const { return m_certificate_table; }
    data_dir_t* base_relocation_table() const {
      return m_base_relocation_table;
    }
    data_dir_t* debug() const { return m_debug; }
    data_dir_t* architecture() const { return m_architecture; }
    data_dir_t* global_ptr() const { return m_global_ptr; }
    data_dir_t* tls_table() const { return m_tls_table; }
    data_dir_t* load_config_table() const { return m_load_config_table; }
    data_dir_t* bound_import() const { return m_bound_import; }
    data_dir_t* iat() const { return m_iat; }
    data_dir_t* delay_import_descriptor() const {
      return m_delay_import_descriptor;
    }
    data_dir_t* clr_runtime_header() const { return m_clr_runtime_header; }
    microsoft_pe_t* _root() const { return m__root; }
    microsoft_pe_t::optional_header_t* _parent() const { return m__parent; }
  };

  class data_dir_t : public kaitai::kstruct {
   public:
    explicit data_dir_t(
        kaitai::kstream* p_io,
        microsoft_pe_t::optional_header_data_dirs_t* p_parent = nullptr,
        microsoft_pe_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~data_dir_t();

   private:
    uint32_t m_virtual_address;
    uint32_t m_size;
    microsoft_pe_t* m__root;
    microsoft_pe_t::optional_header_data_dirs_t* m__parent;

   public:
    uint32_t virtual_address() const { return m_virtual_address; }
    uint32_t size() const { return m_size; }
    microsoft_pe_t* _root() const { return m__root; }
    microsoft_pe_t::optional_header_data_dirs_t* _parent() const {
      return m__parent;
    }
  };

  class optional_header_t : public kaitai::kstruct {
   public:
    explicit optional_header_t(kaitai::kstream* p_io,
                               microsoft_pe_t* p_parent = nullptr,
                               microsoft_pe_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~optional_header_t();

   private:
    optional_header_std_t* m_std;
    optional_header_windows_t* m_windows;
    optional_header_data_dirs_t* m_data_dirs;
    microsoft_pe_t* m__root;
    microsoft_pe_t* m__parent;

   public:
    optional_header_std_t* std() const { return m_std; }
    optional_header_windows_t* windows() const { return m_windows; }
    optional_header_data_dirs_t* data_dirs() const { return m_data_dirs; }
    microsoft_pe_t* _root() const { return m__root; }
    microsoft_pe_t* _parent() const { return m__parent; }
  };

  class section_t : public kaitai::kstruct {
   public:
    explicit section_t(kaitai::kstream* p_io,
                       microsoft_pe_t* p_parent = nullptr,
                       microsoft_pe_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~section_t();

   private:
    bool f_body;
    std::vector<uint8_t> m_body;

   public:
    std::vector<uint8_t> body();

   private:
    std::string m_name;
    uint32_t m_virtual_size;
    uint32_t m_virtual_address;
    uint32_t m_size_of_raw_data;
    uint32_t m_pointer_to_raw_data;
    uint32_t m_pointer_to_relocations;
    uint32_t m_pointer_to_linenumbers;
    uint16_t m_number_of_relocations;
    uint16_t m_number_of_linenumbers;
    uint32_t m_characteristics;
    microsoft_pe_t* m__root;
    microsoft_pe_t* m__parent;

   public:
    std::string name() const { return m_name; }
    uint32_t virtual_size() const { return m_virtual_size; }
    uint32_t virtual_address() const { return m_virtual_address; }
    uint32_t size_of_raw_data() const { return m_size_of_raw_data; }
    uint32_t pointer_to_raw_data() const { return m_pointer_to_raw_data; }
    uint32_t pointer_to_relocations() const { return m_pointer_to_relocations; }
    uint32_t pointer_to_linenumbers() const { return m_pointer_to_linenumbers; }
    uint16_t number_of_relocations() const { return m_number_of_relocations; }
    uint16_t number_of_linenumbers() const { return m_number_of_linenumbers; }
    uint32_t characteristics() const { return m_characteristics; }
    microsoft_pe_t* _root() const { return m__root; }
    microsoft_pe_t* _parent() const { return m__parent; }
  };

  class mz_placeholder_t : public kaitai::kstruct {
   public:
    explicit mz_placeholder_t(kaitai::kstream* p_io,
                              microsoft_pe_t* p_parent = nullptr,
                              microsoft_pe_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~mz_placeholder_t();

   private:
    std::vector<uint8_t> m_magic;
    std::vector<uint8_t> m_data1;
    uint32_t m_header_size;
    microsoft_pe_t* m__root;
    microsoft_pe_t* m__parent;

   public:
    std::vector<uint8_t> magic() const { return m_magic; }
    std::vector<uint8_t> data1() const { return m_data1; }
    uint32_t header_size() const { return m_header_size; }
    microsoft_pe_t* _root() const { return m__root; }
    microsoft_pe_t* _parent() const { return m__parent; }
  };

  class optional_header_std_t : public kaitai::kstruct {
   public:
    enum pe_formatx_t {
      PE_FORMATX_ROM_IMAGE = 263,
      PE_FORMATX_PE32 = 267,
      PE_FORMATX_PE32_PLUS = 523
    };

    explicit optional_header_std_t(
        kaitai::kstream* p_io,
        microsoft_pe_t::optional_header_t* p_parent = nullptr,
        microsoft_pe_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~optional_header_std_t();

   private:
    uint16_t m_format;
    uint8_t m_major_linker_version;
    uint8_t m_minor_linker_version;
    uint32_t m_size_of_code;
    uint32_t m_size_of_initialized_data;
    uint32_t m_size_of_uninitialized_data;
    uint32_t m_address_of_entry_point;
    uint32_t m_base_of_code;
    uint32_t m_base_of_data;
    bool n_base_of_data;

   public:
    bool _is_null_base_of_data() {
      base_of_data();
      return n_base_of_data;
    };

   private:
    microsoft_pe_t* m__root;
    microsoft_pe_t::optional_header_t* m__parent;

   public:
    uint16_t format() const { return m_format; }
    uint8_t major_linker_version() const { return m_major_linker_version; }
    uint8_t minor_linker_version() const { return m_minor_linker_version; }
    uint32_t size_of_code() const { return m_size_of_code; }
    uint32_t size_of_initialized_data() const {
      return m_size_of_initialized_data;
    }
    uint32_t size_of_uninitialized_data() const {
      return m_size_of_uninitialized_data;
    }
    uint32_t address_of_entry_point() const { return m_address_of_entry_point; }
    uint32_t base_of_code() const { return m_base_of_code; }
    uint32_t base_of_data() const { return m_base_of_data; }
    microsoft_pe_t* _root() const { return m__root; }
    microsoft_pe_t::optional_header_t* _parent() const { return m__parent; }
  };

  class coff_header_t : public kaitai::kstruct {
   public:
    enum machine_type_t {
      MACHINE_TYPE_UNKNOWN = 0,
      MACHINE_TYPE_I386 = 332,
      MACHINE_TYPE_R4000 = 358,
      MACHINE_TYPE_WCEMIPSV2 = 361,
      MACHINE_TYPE_SH3 = 418,
      MACHINE_TYPE_SH3DSP = 419,
      MACHINE_TYPE_SH4 = 422,
      MACHINE_TYPE_SH5 = 424,
      MACHINE_TYPE_ARM = 448,
      MACHINE_TYPE_THUMB = 450,
      MACHINE_TYPE_ARMNT = 452,
      MACHINE_TYPE_AM33 = 467,
      MACHINE_TYPE_POWERPC = 496,
      MACHINE_TYPE_POWERPCFP = 497,
      MACHINE_TYPE_IA64 = 512,
      MACHINE_TYPE_MIPS16 = 614,
      MACHINE_TYPE_MIPSFPU = 870,
      MACHINE_TYPE_MIPSFPU16 = 1126,
      MACHINE_TYPE_EBC = 3772,
      MACHINE_TYPE_RISCV32 = 20530,
      MACHINE_TYPE_RISCV64 = 20580,
      MACHINE_TYPE_RISCV128 = 20776,
      MACHINE_TYPE_AMD64 = 34404,
      MACHINE_TYPE_M32R = 36929
    };

    explicit coff_header_t(kaitai::kstream* p_io,
                           microsoft_pe_t* p_parent = nullptr,
                           microsoft_pe_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~coff_header_t();

   private:
    machine_type_t m_machine;
    uint16_t m_number_of_sections;
    uint32_t m_time_date_stamp;
    uint32_t m_pointer_to_symbol_table;
    uint32_t m_number_of_symbols;
    uint16_t m_size_of_optional_header;
    uint16_t m_characteristics;
    microsoft_pe_t* m__root;
    microsoft_pe_t* m__parent;

   public:
    machine_type_t machine() const { return m_machine; }
    uint16_t number_of_sections() const { return m_number_of_sections; }
    uint32_t time_date_stamp() const { return m_time_date_stamp; }
    uint32_t pointer_to_symbol_table() const {
      return m_pointer_to_symbol_table;
    }
    uint32_t number_of_symbols() const { return m_number_of_symbols; }
    uint16_t size_of_optional_header() const {
      return m_size_of_optional_header;
    }
    uint16_t characteristics() const { return m_characteristics; }
    microsoft_pe_t* _root() const { return m__root; }
    microsoft_pe_t* _parent() const { return m__parent; }
  };

 private:
  mz_placeholder_t* m_mz1;
  std::vector<uint8_t> m_mz2;
  std::vector<uint8_t> m_pe_signature;
  coff_header_t* m_coff_header;
  optional_header_t* m_optional_header;
  std::vector<section_t*>* m_sections;
  microsoft_pe_t* m__root;
  kaitai::kstruct* m__parent;
  std::vector<uint8_t> m__skip_me_optional_header;
  kaitai::kstream* m__io__skip_me_optional_header;

 public:
  mz_placeholder_t* mz1() const { return m_mz1; }
  std::vector<uint8_t> mz2() const { return m_mz2; }
  std::vector<uint8_t> pe_signature() const { return m_pe_signature; }
  coff_header_t* coff_header() const { return m_coff_header; }
  optional_header_t* optional_header() const { return m_optional_header; }
  std::vector<section_t*>* sections() const { return m_sections; }
  microsoft_pe_t* _root() const { return m__root; }
  kaitai::kstruct* _parent() const { return m__parent; }
  std::vector<uint8_t> _skip_me_optional_header() const {
    return m__skip_me_optional_header;
  }
  kaitai::kstream* _io__skip_me_optional_header() const {
    return m__io__skip_me_optional_header;
  }
};

}  // namespace microsoft_pe
}  // namespace kaitai
}  // namespace veles
#endif  // MICROSOFT_PE_H_
