#ifndef BMP_H_
#define BMP_H_

// This is a generated file! Please edit source .ksy file and use
// kaitai-struct-compiler to rebuild

#include <kaitai/kaitaistream.h>
#include <kaitai/kaitaistruct.h>

#include <cstdint>
#include <vector>

namespace veles {
namespace kaitai {
namespace bmp {

class bmp_t : public kaitai::kstruct {
 public:
  class file_header_t;
  class dib_header_t;
  class bitmap_core_header_t;
  class bitmap_info_header_t;

  explicit bmp_t(kaitai::kstream* p_io, kaitai::kstruct* p_parent = nullptr,
                 bmp_t* p_root = nullptr);
  veles::dbif::ObjectHandle veles_obj;
  ~bmp_t();

  class file_header_t : public kaitai::kstruct {
   public:
    explicit file_header_t(kaitai::kstream* p_io, bmp_t* p_parent = nullptr,
                           bmp_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~file_header_t();

   private:
    std::vector<uint8_t> m_file_type;
    uint32_t m_file_size;
    uint16_t m_reserved1;
    uint16_t m_reserved2;
    int32_t m_bitmap_ofs;
    bmp_t* m__root;
    bmp_t* m__parent;

   public:
    std::vector<uint8_t> file_type() const { return m_file_type; }
    uint32_t file_size() const { return m_file_size; }
    uint16_t reserved1() const { return m_reserved1; }
    uint16_t reserved2() const { return m_reserved2; }
    int32_t bitmap_ofs() const { return m_bitmap_ofs; }
    bmp_t* _root() const { return m__root; }
    bmp_t* _parent() const { return m__parent; }
  };

  class dib_header_t : public kaitai::kstruct {
   public:
    explicit dib_header_t(kaitai::kstream* p_io, bmp_t* p_parent = nullptr,
                          bmp_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~dib_header_t();

   private:
    int32_t m_dib_header_size;
    bitmap_core_header_t* m_bitmap_core_header;
    bool n_bitmap_core_header;

   public:
    bool _is_null_bitmap_core_header() {
      bitmap_core_header();
      return n_bitmap_core_header;
    };

   private:
    bitmap_info_header_t* m_bitmap_info_header;
    bool n_bitmap_info_header;

   public:
    bool _is_null_bitmap_info_header() {
      bitmap_info_header();
      return n_bitmap_info_header;
    };

   private:
    bitmap_core_header_t* m_bitmap_v5_header;
    bool n_bitmap_v5_header;

   public:
    bool _is_null_bitmap_v5_header() {
      bitmap_v5_header();
      return n_bitmap_v5_header;
    };

   private:
    std::vector<uint8_t> m_dib_header_body;
    bool n_dib_header_body;

   public:
    bool _is_null_dib_header_body() {
      dib_header_body();
      return n_dib_header_body;
    };

   private:
    bmp_t* m__root;
    bmp_t* m__parent;
    std::vector<uint8_t> m__skip_me_bitmap_core_header;
    kaitai::kstream* m__io__skip_me_bitmap_core_header;
    std::vector<uint8_t> m__skip_me_bitmap_info_header;
    kaitai::kstream* m__io__skip_me_bitmap_info_header;
    std::vector<uint8_t> m__skip_me_bitmap_v5_header;
    kaitai::kstream* m__io__skip_me_bitmap_v5_header;

   public:
    int32_t dib_header_size() const { return m_dib_header_size; }
    bitmap_core_header_t* bitmap_core_header() const {
      return m_bitmap_core_header;
    }
    bitmap_info_header_t* bitmap_info_header() const {
      return m_bitmap_info_header;
    }
    bitmap_core_header_t* bitmap_v5_header() const {
      return m_bitmap_v5_header;
    }
    std::vector<uint8_t> dib_header_body() const { return m_dib_header_body; }
    bmp_t* _root() const { return m__root; }
    bmp_t* _parent() const { return m__parent; }
    std::vector<uint8_t> _skip_me_bitmap_core_header() const {
      return m__skip_me_bitmap_core_header;
    }
    kaitai::kstream* _io__skip_me_bitmap_core_header() const {
      return m__io__skip_me_bitmap_core_header;
    }
    std::vector<uint8_t> _skip_me_bitmap_info_header() const {
      return m__skip_me_bitmap_info_header;
    }
    kaitai::kstream* _io__skip_me_bitmap_info_header() const {
      return m__io__skip_me_bitmap_info_header;
    }
    std::vector<uint8_t> _skip_me_bitmap_v5_header() const {
      return m__skip_me_bitmap_v5_header;
    }
    kaitai::kstream* _io__skip_me_bitmap_v5_header() const {
      return m__io__skip_me_bitmap_v5_header;
    }
  };

  class bitmap_core_header_t : public kaitai::kstruct {
   public:
    explicit bitmap_core_header_t(kaitai::kstream* p_io,
                                  bmp_t::dib_header_t* p_parent = nullptr,
                                  bmp_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~bitmap_core_header_t();

   private:
    uint16_t m_image_width;
    uint16_t m_image_height;
    uint16_t m_num_planes;
    uint16_t m_bits_per_pixel;
    bmp_t* m__root;
    bmp_t::dib_header_t* m__parent;

   public:
    uint16_t image_width() const { return m_image_width; }
    uint16_t image_height() const { return m_image_height; }
    uint16_t num_planes() const { return m_num_planes; }
    uint16_t bits_per_pixel() const { return m_bits_per_pixel; }
    bmp_t* _root() const { return m__root; }
    bmp_t::dib_header_t* _parent() const { return m__parent; }
  };

  class bitmap_info_header_t : public kaitai::kstruct {
   public:
    explicit bitmap_info_header_t(kaitai::kstream* p_io,
                                  bmp_t::dib_header_t* p_parent = nullptr,
                                  bmp_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~bitmap_info_header_t();

   private:
    uint32_t m_image_width;
    uint32_t m_image_height;
    uint16_t m_num_planes;
    uint16_t m_bits_per_pixel;
    uint32_t m_compression;
    uint32_t m_size_image;
    uint32_t m_x_px_per_m;
    uint32_t m_y_px_per_m;
    uint32_t m_num_colors_used;
    uint32_t m_num_colors_important;
    bmp_t* m__root;
    bmp_t::dib_header_t* m__parent;

   public:
    uint32_t image_width() const { return m_image_width; }
    uint32_t image_height() const { return m_image_height; }
    uint16_t num_planes() const { return m_num_planes; }
    uint16_t bits_per_pixel() const { return m_bits_per_pixel; }
    uint32_t compression() const { return m_compression; }
    uint32_t size_image() const { return m_size_image; }
    uint32_t x_px_per_m() const { return m_x_px_per_m; }
    uint32_t y_px_per_m() const { return m_y_px_per_m; }
    uint32_t num_colors_used() const { return m_num_colors_used; }
    uint32_t num_colors_important() const { return m_num_colors_important; }
    bmp_t* _root() const { return m__root; }
    bmp_t::dib_header_t* _parent() const { return m__parent; }
  };

 private:
  bool f_image;
  std::vector<uint8_t> m_image;

 public:
  std::vector<uint8_t> image();

 private:
  file_header_t* m_file_header;
  dib_header_t* m_dib_header;
  bmp_t* m__root;
  kaitai::kstruct* m__parent;

 public:
  file_header_t* file_header() const { return m_file_header; }
  dib_header_t* dib_header() const { return m_dib_header; }
  bmp_t* _root() const { return m__root; }
  kaitai::kstruct* _parent() const { return m__parent; }
};

}  // namespace bmp
}  // namespace kaitai
}  // namespace veles
#endif  // BMP_H_
