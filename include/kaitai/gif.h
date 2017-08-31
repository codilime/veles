#ifndef GIF_H_
#define GIF_H_

// This is a generated file! Please edit source .ksy file and use
// kaitai-struct-compiler to rebuild

#include <kaitai/kaitaistream.h>
#include <kaitai/kaitaistruct.h>

#include <cstdint>
#include <vector>

namespace veles {
namespace kaitai {
namespace gif {

class gif_t : public kaitai::kstruct {
 public:
  class global_color_table_t;
  class image_data_t;
  class color_table_entry_t;
  class logical_screen_descriptor_t;
  class local_image_descriptor_t;
  class block_t;
  class header_t;
  class ext_graphic_control_t;
  class subblock_t;
  class ext_application_t;
  class subblocks_t;
  class extension_t;

  enum block_type_t {
    BLOCK_TYPE_EXTENSION = 33,
    BLOCK_TYPE_LOCAL_IMAGE_DESCRIPTOR = 44,
    BLOCK_TYPE_END_OF_FILE = 59
  };

  enum extension_label_t {
    EXTENSION_LABEL_GRAPHIC_CONTROL = 249,
    EXTENSION_LABEL_COMMENT = 254,
    EXTENSION_LABEL_APPLICATION = 255
  };

  explicit gif_t(kaitai::kstream* p_io, kaitai::kstruct* p_parent = nullptr,
                 gif_t* p_root = nullptr);
  veles::dbif::ObjectHandle veles_obj;
  ~gif_t();

  class global_color_table_t : public kaitai::kstruct {
   public:
    explicit global_color_table_t(kaitai::kstream* p_io,
                                  gif_t* p_parent = nullptr,
                                  gif_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~global_color_table_t();

   private:
    std::vector<color_table_entry_t*>* m_entries;
    gif_t* m__root;
    gif_t* m__parent;

   public:
    std::vector<color_table_entry_t*>* entries() const { return m_entries; }
    gif_t* _root() const { return m__root; }
    gif_t* _parent() const { return m__parent; }
  };

  class image_data_t : public kaitai::kstruct {
   public:
    explicit image_data_t(kaitai::kstream* p_io,
                          gif_t::local_image_descriptor_t* p_parent = nullptr,
                          gif_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~image_data_t();

   private:
    uint8_t m_lzw_min_code_size;
    subblocks_t* m_subblocks;
    gif_t* m__root;
    gif_t::local_image_descriptor_t* m__parent;

   public:
    uint8_t lzw_min_code_size() const { return m_lzw_min_code_size; }
    subblocks_t* subblocks() const { return m_subblocks; }
    gif_t* _root() const { return m__root; }
    gif_t::local_image_descriptor_t* _parent() const { return m__parent; }
  };

  class color_table_entry_t : public kaitai::kstruct {
   public:
    explicit color_table_entry_t(
        kaitai::kstream* p_io, gif_t::global_color_table_t* p_parent = nullptr,
        gif_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~color_table_entry_t();

   private:
    uint8_t m_red;
    uint8_t m_green;
    uint8_t m_blue;
    gif_t* m__root;
    gif_t::global_color_table_t* m__parent;

   public:
    uint8_t red() const { return m_red; }
    uint8_t green() const { return m_green; }
    uint8_t blue() const { return m_blue; }
    gif_t* _root() const { return m__root; }
    gif_t::global_color_table_t* _parent() const { return m__parent; }
  };

  class logical_screen_descriptor_t : public kaitai::kstruct {
   public:
    explicit logical_screen_descriptor_t(kaitai::kstream* p_io,
                                         gif_t* p_parent = nullptr,
                                         gif_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~logical_screen_descriptor_t();

   private:
    bool f_has_color_table;
    bool m_has_color_table;

   public:
    bool has_color_table();

   private:
    bool f_color_table_size;
    int32_t m_color_table_size;

   public:
    int32_t color_table_size();

   private:
    uint16_t m_screen_width;
    uint16_t m_screen_height;
    uint8_t m_flags;
    uint8_t m_bg_color_index;
    uint8_t m_pixel_aspect_ratio;
    gif_t* m__root;
    gif_t* m__parent;

   public:
    uint16_t screen_width() const { return m_screen_width; }
    uint16_t screen_height() const { return m_screen_height; }
    uint8_t flags() const { return m_flags; }
    uint8_t bg_color_index() const { return m_bg_color_index; }
    uint8_t pixel_aspect_ratio() const { return m_pixel_aspect_ratio; }
    gif_t* _root() const { return m__root; }
    gif_t* _parent() const { return m__parent; }
  };

  class local_image_descriptor_t : public kaitai::kstruct {
   public:
    explicit local_image_descriptor_t(kaitai::kstream* p_io,
                                      gif_t::block_t* p_parent = nullptr,
                                      gif_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~local_image_descriptor_t();

   private:
    bool f_has_color_table;
    bool m_has_color_table;

   public:
    bool has_color_table();

   private:
    bool f_has_interlace;
    bool m_has_interlace;

   public:
    bool has_interlace();

   private:
    bool f_has_sorted_color_table;
    bool m_has_sorted_color_table;

   public:
    bool has_sorted_color_table();

   private:
    bool f_color_table_size;
    int32_t m_color_table_size;

   public:
    int32_t color_table_size();

   private:
    uint16_t m_left;
    uint16_t m_top;
    uint16_t m_width;
    uint16_t m_height;
    uint8_t m_flags;
    image_data_t* m_image_data;
    gif_t* m__root;
    gif_t::block_t* m__parent;

   public:
    uint16_t left() const { return m_left; }
    uint16_t top() const { return m_top; }
    uint16_t width() const { return m_width; }
    uint16_t height() const { return m_height; }
    uint8_t flags() const { return m_flags; }
    image_data_t* image_data() const { return m_image_data; }
    gif_t* _root() const { return m__root; }
    gif_t::block_t* _parent() const { return m__parent; }
  };

  class block_t : public kaitai::kstruct {
   public:
    explicit block_t(kaitai::kstream* p_io, gif_t* p_parent = nullptr,
                     gif_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~block_t();

   private:
    block_type_t m_block_type;
    kaitai::kstruct* m_body;
    gif_t* m__root;
    gif_t* m__parent;

   public:
    block_type_t block_type() const { return m_block_type; }
    kaitai::kstruct* body() const { return m_body; }
    gif_t* _root() const { return m__root; }
    gif_t* _parent() const { return m__parent; }
  };

  class header_t : public kaitai::kstruct {
   public:
    explicit header_t(kaitai::kstream* p_io, gif_t* p_parent = nullptr,
                      gif_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~header_t();

   private:
    std::vector<uint8_t> m_magic;
    std::vector<uint8_t> m_version;
    gif_t* m__root;
    gif_t* m__parent;

   public:
    std::vector<uint8_t> magic() const { return m_magic; }
    std::vector<uint8_t> version() const { return m_version; }
    gif_t* _root() const { return m__root; }
    gif_t* _parent() const { return m__parent; }
  };

  class ext_graphic_control_t : public kaitai::kstruct {
   public:
    explicit ext_graphic_control_t(kaitai::kstream* p_io,
                                   gif_t::extension_t* p_parent = nullptr,
                                   gif_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~ext_graphic_control_t();

   private:
    bool f_transparent_color_flag;
    bool m_transparent_color_flag;

   public:
    bool transparent_color_flag();

   private:
    bool f_user_input_flag;
    bool m_user_input_flag;

   public:
    bool user_input_flag();

   private:
    std::vector<uint8_t> m_block_size;
    uint8_t m_flags;
    uint16_t m_delay_time;
    uint8_t m_transparent_idx;
    std::vector<uint8_t> m_terminator;
    gif_t* m__root;
    gif_t::extension_t* m__parent;

   public:
    std::vector<uint8_t> block_size() const { return m_block_size; }
    uint8_t flags() const { return m_flags; }
    uint16_t delay_time() const { return m_delay_time; }
    uint8_t transparent_idx() const { return m_transparent_idx; }
    std::vector<uint8_t> terminator() const { return m_terminator; }
    gif_t* _root() const { return m__root; }
    gif_t::extension_t* _parent() const { return m__parent; }
  };

  class subblock_t : public kaitai::kstruct {
   public:
    explicit subblock_t(kaitai::kstream* p_io,
                        kaitai::kstruct* p_parent = nullptr,
                        gif_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~subblock_t();

   private:
    uint8_t m_num_bytes;
    std::vector<uint8_t> m_bytes;
    gif_t* m__root;
    kaitai::kstruct* m__parent;

   public:
    uint8_t num_bytes() const { return m_num_bytes; }
    std::vector<uint8_t> bytes() const { return m_bytes; }
    gif_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
  };

  class ext_application_t : public kaitai::kstruct {
   public:
    explicit ext_application_t(kaitai::kstream* p_io,
                               gif_t::extension_t* p_parent = nullptr,
                               gif_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~ext_application_t();

   private:
    subblock_t* m_application_id;
    std::vector<subblock_t*>* m_subblocks;
    gif_t* m__root;
    gif_t::extension_t* m__parent;

   public:
    subblock_t* application_id() const { return m_application_id; }
    std::vector<subblock_t*>* subblocks() const { return m_subblocks; }
    gif_t* _root() const { return m__root; }
    gif_t::extension_t* _parent() const { return m__parent; }
  };

  class subblocks_t : public kaitai::kstruct {
   public:
    explicit subblocks_t(kaitai::kstream* p_io,
                         kaitai::kstruct* p_parent = nullptr,
                         gif_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~subblocks_t();

   private:
    std::vector<subblock_t*>* m_entries;
    gif_t* m__root;
    kaitai::kstruct* m__parent;

   public:
    std::vector<subblock_t*>* entries() const { return m_entries; }
    gif_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
  };

  class extension_t : public kaitai::kstruct {
   public:
    explicit extension_t(kaitai::kstream* p_io,
                         gif_t::block_t* p_parent = nullptr,
                         gif_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~extension_t();

   private:
    extension_label_t m_label;
    kaitai::kstruct* m_body;
    gif_t* m__root;
    gif_t::block_t* m__parent;

   public:
    extension_label_t label() const { return m_label; }
    kaitai::kstruct* body() const { return m_body; }
    gif_t* _root() const { return m__root; }
    gif_t::block_t* _parent() const { return m__parent; }
  };

 private:
  header_t* m_header;
  logical_screen_descriptor_t* m_logical_screen_descriptor;
  global_color_table_t* m_global_color_table;
  bool n_global_color_table;

 public:
  bool _is_null_global_color_table() {
    global_color_table();
    return n_global_color_table;
  };

 private:
  std::vector<block_t*>* m_blocks;
  gif_t* m__root;
  kaitai::kstruct* m__parent;
  std::vector<uint8_t> m__skip_me_global_color_table;
  kaitai::kstream* m__io__skip_me_global_color_table;

 public:
  header_t* header() const { return m_header; }
  logical_screen_descriptor_t* logical_screen_descriptor() const {
    return m_logical_screen_descriptor;
  }
  global_color_table_t* global_color_table() const {
    return m_global_color_table;
  }
  std::vector<block_t*>* blocks() const { return m_blocks; }
  gif_t* _root() const { return m__root; }
  kaitai::kstruct* _parent() const { return m__parent; }
  std::vector<uint8_t> _skip_me_global_color_table() const {
    return m__skip_me_global_color_table;
  }
  kaitai::kstream* _io__skip_me_global_color_table() const {
    return m__io__skip_me_global_color_table;
  }
};

}  // namespace gif
}  // namespace kaitai
}  // namespace veles
#endif  // GIF_H_
