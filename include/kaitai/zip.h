#ifndef ZIP_H_
#define ZIP_H_

// This is a generated file! Please edit source .ksy file and use
// kaitai-struct-compiler to rebuild

#include <kaitai/kaitaistream.h>
#include <kaitai/kaitaistruct.h>

#include <cstdint>
#include <vector>

namespace veles {
namespace kaitai {
namespace zip {

class zip_t : public kaitai::kstruct {
 public:
  class local_file_t;
  class central_dir_entry_t;
  class pk_section_t;
  class local_file_header_t;
  class end_of_central_dir_t;

  enum compression_t {
    COMPRESSION_NONE = 0,
    COMPRESSION_SHRUNK = 1,
    COMPRESSION_REDUCED_1 = 2,
    COMPRESSION_REDUCED_2 = 3,
    COMPRESSION_REDUCED_3 = 4,
    COMPRESSION_REDUCED_4 = 5,
    COMPRESSION_IMPLODED = 6,
    COMPRESSION_DEFLATED = 8,
    COMPRESSION_ENHANCED_DEFLATED = 9,
    COMPRESSION_PKWARE_DCL_IMPLODED = 10,
    COMPRESSION_BZIP2 = 12,
    COMPRESSION_LZMA = 14,
    COMPRESSION_IBM_TERSE = 18,
    COMPRESSION_IBM_LZ77_Z = 19,
    COMPRESSION_PPMD = 98
  };

  explicit zip_t(kaitai::kstream* p_io, kaitai::kstruct* p_parent = nullptr,
                 zip_t* p_root = nullptr);
  veles::dbif::ObjectHandle veles_obj;
  ~zip_t();

  class local_file_t : public kaitai::kstruct {
   public:
    explicit local_file_t(kaitai::kstream* p_io,
                          zip_t::pk_section_t* p_parent = nullptr,
                          zip_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~local_file_t();

   private:
    local_file_header_t* m_header;
    std::vector<uint8_t> m_body;
    zip_t* m__root;
    zip_t::pk_section_t* m__parent;

   public:
    local_file_header_t* header() const { return m_header; }
    std::vector<uint8_t> body() const { return m_body; }
    zip_t* _root() const { return m__root; }
    zip_t::pk_section_t* _parent() const { return m__parent; }
  };

  class central_dir_entry_t : public kaitai::kstruct {
   public:
    explicit central_dir_entry_t(kaitai::kstream* p_io,
                                 zip_t::pk_section_t* p_parent = nullptr,
                                 zip_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~central_dir_entry_t();

   private:
    uint16_t m_version_made_by;
    uint16_t m_version_needed_to_extract;
    uint16_t m_flags;
    uint16_t m_compression_method;
    uint16_t m_last_mod_file_time;
    uint16_t m_last_mod_file_date;
    uint32_t m_crc32;
    uint32_t m_compressed_size;
    uint32_t m_uncompressed_size;
    uint16_t m_file_name_len;
    uint16_t m_extra_len;
    uint16_t m_comment_len;
    uint16_t m_disk_number_start;
    uint16_t m_int_file_attr;
    uint32_t m_ext_file_attr;
    int32_t m_local_header_offset;
    std::string m_file_name;
    std::vector<uint8_t> m_extra;
    std::string m_comment;
    zip_t* m__root;
    zip_t::pk_section_t* m__parent;

   public:
    uint16_t version_made_by() const { return m_version_made_by; }
    uint16_t version_needed_to_extract() const {
      return m_version_needed_to_extract;
    }
    uint16_t flags() const { return m_flags; }
    uint16_t compression_method() const { return m_compression_method; }
    uint16_t last_mod_file_time() const { return m_last_mod_file_time; }
    uint16_t last_mod_file_date() const { return m_last_mod_file_date; }
    uint32_t crc32() const { return m_crc32; }
    uint32_t compressed_size() const { return m_compressed_size; }
    uint32_t uncompressed_size() const { return m_uncompressed_size; }
    uint16_t file_name_len() const { return m_file_name_len; }
    uint16_t extra_len() const { return m_extra_len; }
    uint16_t comment_len() const { return m_comment_len; }
    uint16_t disk_number_start() const { return m_disk_number_start; }
    uint16_t int_file_attr() const { return m_int_file_attr; }
    uint32_t ext_file_attr() const { return m_ext_file_attr; }
    int32_t local_header_offset() const { return m_local_header_offset; }
    std::string file_name() const { return m_file_name; }
    std::vector<uint8_t> extra() const { return m_extra; }
    std::string comment() const { return m_comment; }
    zip_t* _root() const { return m__root; }
    zip_t::pk_section_t* _parent() const { return m__parent; }
  };

  class pk_section_t : public kaitai::kstruct {
   public:
    explicit pk_section_t(kaitai::kstream* p_io, zip_t* p_parent = nullptr,
                          zip_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~pk_section_t();

   private:
    std::vector<uint8_t> m_magic;
    uint16_t m_section_type;
    kaitai::kstruct* m_body;
    zip_t* m__root;
    zip_t* m__parent;

   public:
    std::vector<uint8_t> magic() const { return m_magic; }
    uint16_t section_type() const { return m_section_type; }
    kaitai::kstruct* body() const { return m_body; }
    zip_t* _root() const { return m__root; }
    zip_t* _parent() const { return m__parent; }
  };

  class local_file_header_t : public kaitai::kstruct {
   public:
    explicit local_file_header_t(kaitai::kstream* p_io,
                                 zip_t::local_file_t* p_parent = nullptr,
                                 zip_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~local_file_header_t();

   private:
    uint16_t m_version;
    uint16_t m_flags;
    compression_t m_compression;
    uint16_t m_file_mod_time;
    uint16_t m_file_mod_date;
    uint32_t m_crc32;
    uint32_t m_compressed_size;
    uint32_t m_uncompressed_size;
    uint16_t m_file_name_len;
    uint16_t m_extra_len;
    std::string m_file_name;
    std::vector<uint8_t> m_extra;
    zip_t* m__root;
    zip_t::local_file_t* m__parent;

   public:
    uint16_t version() const { return m_version; }
    uint16_t flags() const { return m_flags; }
    compression_t compression() const { return m_compression; }
    uint16_t file_mod_time() const { return m_file_mod_time; }
    uint16_t file_mod_date() const { return m_file_mod_date; }
    uint32_t crc32() const { return m_crc32; }
    uint32_t compressed_size() const { return m_compressed_size; }
    uint32_t uncompressed_size() const { return m_uncompressed_size; }
    uint16_t file_name_len() const { return m_file_name_len; }
    uint16_t extra_len() const { return m_extra_len; }
    std::string file_name() const { return m_file_name; }
    std::vector<uint8_t> extra() const { return m_extra; }
    zip_t* _root() const { return m__root; }
    zip_t::local_file_t* _parent() const { return m__parent; }
  };

  class end_of_central_dir_t : public kaitai::kstruct {
   public:
    explicit end_of_central_dir_t(kaitai::kstream* p_io,
                                  zip_t::pk_section_t* p_parent = nullptr,
                                  zip_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~end_of_central_dir_t();

   private:
    uint16_t m_disk_of_end_of_central_dir;
    uint16_t m_disk_of_central_dir;
    uint16_t m_qty_central_dir_entries_on_disk;
    uint16_t m_qty_central_dir_entries_total;
    uint32_t m_central_dir_size;
    uint32_t m_central_dir_offset;
    uint16_t m_comment_len;
    std::string m_comment;
    zip_t* m__root;
    zip_t::pk_section_t* m__parent;

   public:
    uint16_t disk_of_end_of_central_dir() const {
      return m_disk_of_end_of_central_dir;
    }
    uint16_t disk_of_central_dir() const { return m_disk_of_central_dir; }
    uint16_t qty_central_dir_entries_on_disk() const {
      return m_qty_central_dir_entries_on_disk;
    }
    uint16_t qty_central_dir_entries_total() const {
      return m_qty_central_dir_entries_total;
    }
    uint32_t central_dir_size() const { return m_central_dir_size; }
    uint32_t central_dir_offset() const { return m_central_dir_offset; }
    uint16_t comment_len() const { return m_comment_len; }
    std::string comment() const { return m_comment; }
    zip_t* _root() const { return m__root; }
    zip_t::pk_section_t* _parent() const { return m__parent; }
  };

 private:
  std::vector<pk_section_t*>* m_sections;
  zip_t* m__root;
  kaitai::kstruct* m__parent;

 public:
  std::vector<pk_section_t*>* sections() const { return m_sections; }
  zip_t* _root() const { return m__root; }
  kaitai::kstruct* _parent() const { return m__parent; }
};

}  // namespace zip
}  // namespace kaitai
}  // namespace veles
#endif  // ZIP_H_
