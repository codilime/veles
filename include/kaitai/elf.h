#ifndef ELF_H_
#define ELF_H_

// This is a generated file! Please edit source .ksy file and use
// kaitai-struct-compiler to rebuild

#include <kaitai/kaitaistream.h>
#include <kaitai/kaitaistruct.h>

#include <cstdint>
#include <vector>

namespace veles {
namespace kaitai {
namespace elf {

class elf_t : public kaitai::kstruct {
 public:
  class file_header_t;
  class program_header_t;
  class section_header_t;
  class strings_t;

  enum endian_t { ENDIAN_LE = 1, ENDIAN_BE = 2 };

  enum sh_type_t {
    SH_TYPE_NULL_TYPE = 0,
    SH_TYPE_PROGBITS = 1,
    SH_TYPE_SYMTAB = 2,
    SH_TYPE_STRTAB = 3,
    SH_TYPE_RELA = 4,
    SH_TYPE_HASH = 5,
    SH_TYPE_DYNAMIC = 6,
    SH_TYPE_NOTE = 7,
    SH_TYPE_NOBITS = 8,
    SH_TYPE_REL = 9,
    SH_TYPE_SHLIB = 10,
    SH_TYPE_DYNSYM = 11,
    SH_TYPE_INIT_ARRAY = 14,
    SH_TYPE_FINI_ARRAY = 15,
    SH_TYPE_PREINIT_ARRAY = 16,
    SH_TYPE_GROUP = 17,
    SH_TYPE_SYMTAB_SHNDX = 18,
    SH_TYPE_SUNW_CAPCHAIN = 1879048175,
    SH_TYPE_SUNW_CAPINFO = 1879048176,
    SH_TYPE_SUNW_SYMSORT = 1879048177,
    SH_TYPE_SUNW_TLSSORT = 1879048178,
    SH_TYPE_SUNW_LDYNSYM = 1879048179,
    SH_TYPE_SUNW_DOF = 1879048180,
    SH_TYPE_SUNW_CAP = 1879048181,
    SH_TYPE_SUNW_SIGNATURE = 1879048182,
    SH_TYPE_SUNW_ANNOTATE = 1879048183,
    SH_TYPE_SUNW_DEBUGSTR = 1879048184,
    SH_TYPE_SUNW_DEBUG = 1879048185,
    SH_TYPE_SUNW_MOVE = 1879048186,
    SH_TYPE_SUNW_COMDAT = 1879048187,
    SH_TYPE_SUNW_SYMINFO = 1879048188,
    SH_TYPE_SUNW_VERDEF = 1879048189,
    SH_TYPE_SUNW_VERNEED = 1879048190,
    SH_TYPE_SUNW_VERSYM = 1879048191,
    SH_TYPE_SPARC_GOTDATA = 1879048192,
    SH_TYPE_AMD64_UNWIND = 1879048193
  };

  enum os_abi_t {
    OS_ABI_SYSTEM_V = 0,
    OS_ABI_HP_UX = 1,
    OS_ABI_NETBSD = 2,
    OS_ABI_GNU = 3,
    OS_ABI_SOLARIS = 6,
    OS_ABI_AIX = 7,
    OS_ABI_IRIX = 8,
    OS_ABI_FREEBSD = 9,
    OS_ABI_TRU64 = 10,
    OS_ABI_MODESTO = 11,
    OS_ABI_OPENBSD = 12,
    OS_ABI_OPENVMS = 13,
    OS_ABI_NSK = 14,
    OS_ABI_AROS = 15,
    OS_ABI_FENIXOS = 16,
    OS_ABI_CLOUDABI = 17,
    OS_ABI_OPENVOS = 18
  };

  enum machine_t {
    MACHINE_NOT_SET = 0,
    MACHINE_SPARC = 2,
    MACHINE_X86 = 3,
    MACHINE_MIPS = 8,
    MACHINE_POWERPC = 20,
    MACHINE_ARM = 40,
    MACHINE_SUPERH = 42,
    MACHINE_IA_64 = 50,
    MACHINE_X86_64 = 62,
    MACHINE_AARCH64 = 183
  };

  enum bits_t { BITS_B32 = 1, BITS_B64 = 2 };

  enum ph_type_t {
    PH_TYPE_NULL_TYPE = 0,
    PH_TYPE_LOAD = 1,
    PH_TYPE_DYNAMIC = 2,
    PH_TYPE_INTERP = 3,
    PH_TYPE_NOTE = 4,
    PH_TYPE_SHLIB = 5,
    PH_TYPE_PHDR = 6,
    PH_TYPE_TLS = 7,
    PH_TYPE_GNU_EH_FRAME = 1685382480,
    PH_TYPE_GNU_STACK = 1685382481,
    PH_TYPE_HIOS = 1879048191
  };

  enum obj_type_t {
    OBJ_TYPE_RELOCATABLE = 1,
    OBJ_TYPE_EXECUTABLE = 2,
    OBJ_TYPE_SHARED = 3,
    OBJ_TYPE_CORE = 4
  };

  explicit elf_t(kaitai::kstream* p_io, kaitai::kstruct* p_parent = nullptr,
                 elf_t* p_root = nullptr);
  veles::dbif::ObjectHandle veles_obj;
  ~elf_t();

  class file_header_t : public kaitai::kstruct {
   public:
    explicit file_header_t(kaitai::kstream* p_io, elf_t* p_parent = nullptr,
                           elf_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~file_header_t();

   private:
    std::vector<uint8_t> m_magic;
    bits_t m_bits;
    endian_t m_endian;
    uint8_t m_ei_version;
    os_abi_t m_abi;
    uint8_t m_abi_version;
    std::vector<uint8_t> m_pad;
    obj_type_t m_e_type;
    machine_t m_machine;
    uint32_t m_e_version;
    uint64_t m_entry_point;
    uint64_t m_program_header_offset;
    uint64_t m_section_header_offset;
    std::vector<uint8_t> m_flags;
    uint16_t m_e_ehsize;
    uint16_t m_program_header_entry_size;
    uint16_t m_qty_program_header;
    uint16_t m_section_header_entry_size;
    uint16_t m_qty_section_header;
    uint16_t m_section_names_idx;
    elf_t* m__root;
    elf_t* m__parent;

   public:
    /**
     * File identification, must be 0x7f + "ELF".
     */
    std::vector<uint8_t> magic() const { return m_magic; }

    /**
     * File class: designates target machine word size (32 or 64 bits). The size
     of many integer fields in this format will depend on this setting.

     */
    bits_t bits() const { return m_bits; }

    /**
     * Endianness used for all integers.
     */
    endian_t endian() const { return m_endian; }

    /**
     * ELF header version.
     */
    uint8_t ei_version() const { return m_ei_version; }

    /**
     * Specifies which OS- and ABI-related extensions will be used in this ELF
     file.

     */
    os_abi_t abi() const { return m_abi; }

    /**
     * Version of ABI targeted by this ELF file. Interpretation depends on `abi`
     attribute.

     */
    uint8_t abi_version() const { return m_abi_version; }
    std::vector<uint8_t> pad() const { return m_pad; }
    obj_type_t e_type() const { return m_e_type; }
    machine_t machine() const { return m_machine; }
    uint32_t e_version() const { return m_e_version; }
    uint64_t entry_point() const { return m_entry_point; }
    uint64_t program_header_offset() const { return m_program_header_offset; }
    uint64_t section_header_offset() const { return m_section_header_offset; }
    std::vector<uint8_t> flags() const { return m_flags; }
    uint16_t e_ehsize() const { return m_e_ehsize; }
    uint16_t program_header_entry_size() const {
      return m_program_header_entry_size;
    }
    uint16_t qty_program_header() const { return m_qty_program_header; }
    uint16_t section_header_entry_size() const {
      return m_section_header_entry_size;
    }
    uint16_t qty_section_header() const { return m_qty_section_header; }
    uint16_t section_names_idx() const { return m_section_names_idx; }
    elf_t* _root() const { return m__root; }
    elf_t* _parent() const { return m__parent; }
  };

  class program_header_t : public kaitai::kstruct {
   public:
    explicit program_header_t(kaitai::kstream* p_io, elf_t* p_parent = nullptr,
                              elf_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~program_header_t();

   private:
    ph_type_t m_type;
    uint32_t m_flags64;
    bool n_flags64;

   public:
    bool _is_null_flags64() {
      flags64();
      return n_flags64;
    };

   private:
    uint64_t m_offset;
    uint64_t m_vaddr;
    uint64_t m_paddr;
    uint64_t m_filesz;
    uint64_t m_memsz;
    uint32_t m_flags32;
    bool n_flags32;

   public:
    bool _is_null_flags32() {
      flags32();
      return n_flags32;
    };

   private:
    uint64_t m_align;
    elf_t* m__root;
    elf_t* m__parent;

   public:
    ph_type_t type() const { return m_type; }
    uint32_t flags64() const { return m_flags64; }
    uint64_t offset() const { return m_offset; }
    uint64_t vaddr() const { return m_vaddr; }
    uint64_t paddr() const { return m_paddr; }
    uint64_t filesz() const { return m_filesz; }
    uint64_t memsz() const { return m_memsz; }
    uint32_t flags32() const { return m_flags32; }
    uint64_t align() const { return m_align; }
    elf_t* _root() const { return m__root; }
    elf_t* _parent() const { return m__parent; }
  };

  class section_header_t : public kaitai::kstruct {
   public:
    explicit section_header_t(kaitai::kstream* p_io, elf_t* p_parent = nullptr,
                              elf_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~section_header_t();

   private:
    bool f_body;
    std::vector<uint8_t> m_body;

   public:
    std::vector<uint8_t> body();

   private:
    bool f_name;
    std::string m_name;

   public:
    std::string name();

   private:
    uint32_t m_name_offset;
    sh_type_t m_type;
    uint64_t m_flags;
    uint64_t m_addr;
    uint64_t m_offset;
    uint64_t m_size;
    uint32_t m_linked_section_idx;
    std::vector<uint8_t> m_info;
    uint64_t m_align;
    uint64_t m_entry_size;
    elf_t* m__root;
    elf_t* m__parent;

   public:
    uint32_t name_offset() const { return m_name_offset; }
    sh_type_t type() const { return m_type; }
    uint64_t flags() const { return m_flags; }
    uint64_t addr() const { return m_addr; }
    uint64_t offset() const { return m_offset; }
    uint64_t size() const { return m_size; }
    uint32_t linked_section_idx() const { return m_linked_section_idx; }
    std::vector<uint8_t> info() const { return m_info; }
    uint64_t align() const { return m_align; }
    uint64_t entry_size() const { return m_entry_size; }
    elf_t* _root() const { return m__root; }
    elf_t* _parent() const { return m__parent; }
  };

  class strings_t : public kaitai::kstruct {
   public:
    explicit strings_t(kaitai::kstream* p_io, elf_t* p_parent = nullptr,
                       elf_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~strings_t();

   private:
    std::vector<std::string>* m_entries;
    elf_t* m__root;
    elf_t* m__parent;

   public:
    std::vector<std::string>* entries() const { return m_entries; }
    elf_t* _root() const { return m__root; }
    elf_t* _parent() const { return m__parent; }
  };

 private:
  bool f_program_headers;
  std::vector<program_header_t*>* m_program_headers;

 public:
  std::vector<program_header_t*>* program_headers();

 private:
  bool f_section_headers;
  std::vector<section_header_t*>* m_section_headers;

 public:
  std::vector<section_header_t*>* section_headers();

 private:
  bool f_strings;
  strings_t* m_strings;

 public:
  strings_t* strings();

 private:
  file_header_t* m_file_header;
  elf_t* m__root;
  kaitai::kstruct* m__parent;
  std::vector<std::vector<uint8_t>>* m__skip_me_program_headers;
  kaitai::kstream* m__io__skip_me_program_headers;
  std::vector<std::vector<uint8_t>>* m__skip_me_section_headers;
  kaitai::kstream* m__io__skip_me_section_headers;
  std::vector<uint8_t> m__skip_me_strings;
  kaitai::kstream* m__io__skip_me_strings;

 public:
  file_header_t* file_header() const { return m_file_header; }
  elf_t* _root() const { return m__root; }
  kaitai::kstruct* _parent() const { return m__parent; }
  std::vector<std::vector<uint8_t>>* _skip_me_program_headers() const {
    return m__skip_me_program_headers;
  }
  kaitai::kstream* _io__skip_me_program_headers() const {
    return m__io__skip_me_program_headers;
  }
  std::vector<std::vector<uint8_t>>* _skip_me_section_headers() const {
    return m__skip_me_section_headers;
  }
  kaitai::kstream* _io__skip_me_section_headers() const {
    return m__io__skip_me_section_headers;
  }
  std::vector<uint8_t> _skip_me_strings() const { return m__skip_me_strings; }
  kaitai::kstream* _io__skip_me_strings() const {
    return m__io__skip_me_strings;
  }
};

}  // namespace elf
}  // namespace kaitai
}  // namespace veles
#endif  // ELF_H_
