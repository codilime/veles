#ifndef PNG_H_
#define PNG_H_

// This is a generated file! Please edit source .ksy file and use
// kaitai-struct-compiler to rebuild

#include <kaitai/kaitaistream.h>
#include <kaitai/kaitaistruct.h>

#include <cstdint>
#include <vector>

namespace veles {
namespace kaitai {
namespace png {

class png_t : public kaitai::kstruct {
 public:
  class rgb_t;
  class chunk_t;
  class bkgd_indexed_t;
  class point_t;
  class bkgd_greyscale_t;
  class chrm_chunk_t;
  class ihdr_chunk_t;
  class plte_chunk_t;
  class srgb_chunk_t;
  class bkgd_truecolor_t;
  class gama_chunk_t;
  class bkgd_chunk_t;
  class phys_chunk_t;
  class text_chunk_t;
  class time_chunk_t;

  enum color_type_t {
    COLOR_TYPE_GREYSCALE = 0,
    COLOR_TYPE_TRUECOLOR = 2,
    COLOR_TYPE_INDEXED = 3,
    COLOR_TYPE_GREYSCALE_ALPHA = 4,
    COLOR_TYPE_TRUECOLOR_ALPHA = 6
  };

  enum phys_unit_t { PHYS_UNIT_UNKNOWN = 0, PHYS_UNIT_METER = 1 };

  explicit png_t(kaitai::kstream* p_io, kaitai::kstruct* p_parent = nullptr,
                 png_t* p_root = nullptr);
  veles::dbif::ObjectHandle veles_obj;
  ~png_t();

  class rgb_t : public kaitai::kstruct {
   public:
    explicit rgb_t(kaitai::kstream* p_io,
                   png_t::plte_chunk_t* p_parent = nullptr,
                   png_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~rgb_t();

   private:
    uint8_t m_r;
    uint8_t m_g;
    uint8_t m_b;
    png_t* m__root;
    png_t::plte_chunk_t* m__parent;

   public:
    uint8_t r() const { return m_r; }
    uint8_t g() const { return m_g; }
    uint8_t b() const { return m_b; }
    png_t* _root() const { return m__root; }
    png_t::plte_chunk_t* _parent() const { return m__parent; }
  };

  class chunk_t : public kaitai::kstruct {
   public:
    explicit chunk_t(kaitai::kstream* p_io, png_t* p_parent = nullptr,
                     png_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~chunk_t();

   private:
    uint32_t m_len;
    std::string m_type;
    kaitai::kstruct* m_body;
    std::vector<uint8_t> m_crc;
    png_t* m__root;
    png_t* m__parent;
    std::vector<uint8_t> m__skip_me_body;
    kaitai::kstream* m__io__skip_me_body;

   public:
    uint32_t len() const { return m_len; }
    std::string type() const { return m_type; }
    kaitai::kstruct* body() const { return m_body; }
    std::vector<uint8_t> crc() const { return m_crc; }
    png_t* _root() const { return m__root; }
    png_t* _parent() const { return m__parent; }
    std::vector<uint8_t> _skip_me_body() const { return m__skip_me_body; }
    kaitai::kstream* _io__skip_me_body() const { return m__io__skip_me_body; }
  };

  class bkgd_indexed_t : public kaitai::kstruct {
   public:
    explicit bkgd_indexed_t(kaitai::kstream* p_io,
                            png_t::bkgd_chunk_t* p_parent = nullptr,
                            png_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~bkgd_indexed_t();

   private:
    uint8_t m_palette_index;
    png_t* m__root;
    png_t::bkgd_chunk_t* m__parent;

   public:
    uint8_t palette_index() const { return m_palette_index; }
    png_t* _root() const { return m__root; }
    png_t::bkgd_chunk_t* _parent() const { return m__parent; }
  };

  class point_t : public kaitai::kstruct {
   public:
    explicit point_t(kaitai::kstream* p_io,
                     png_t::chrm_chunk_t* p_parent = nullptr,
                     png_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~point_t();

   private:
    bool f_x;
    double m_x;

   public:
    double x();

   private:
    bool f_y;
    double m_y;

   public:
    double y();

   private:
    uint32_t m_x_int;
    uint32_t m_y_int;
    png_t* m__root;
    png_t::chrm_chunk_t* m__parent;

   public:
    uint32_t x_int() const { return m_x_int; }
    uint32_t y_int() const { return m_y_int; }
    png_t* _root() const { return m__root; }
    png_t::chrm_chunk_t* _parent() const { return m__parent; }
  };

  class bkgd_greyscale_t : public kaitai::kstruct {
   public:
    explicit bkgd_greyscale_t(kaitai::kstream* p_io,
                              png_t::bkgd_chunk_t* p_parent = nullptr,
                              png_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~bkgd_greyscale_t();

   private:
    uint16_t m_value;
    png_t* m__root;
    png_t::bkgd_chunk_t* m__parent;

   public:
    uint16_t value() const { return m_value; }
    png_t* _root() const { return m__root; }
    png_t::bkgd_chunk_t* _parent() const { return m__parent; }
  };

  class chrm_chunk_t : public kaitai::kstruct {
   public:
    explicit chrm_chunk_t(kaitai::kstream* p_io,
                          png_t::chunk_t* p_parent = nullptr,
                          png_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~chrm_chunk_t();

   private:
    point_t* m_white_point;
    point_t* m_red;
    point_t* m_green;
    point_t* m_blue;
    png_t* m__root;
    png_t::chunk_t* m__parent;

   public:
    point_t* white_point() const { return m_white_point; }
    point_t* red() const { return m_red; }
    point_t* green() const { return m_green; }
    point_t* blue() const { return m_blue; }
    png_t* _root() const { return m__root; }
    png_t::chunk_t* _parent() const { return m__parent; }
  };

  class ihdr_chunk_t : public kaitai::kstruct {
   public:
    explicit ihdr_chunk_t(kaitai::kstream* p_io, png_t* p_parent = nullptr,
                          png_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~ihdr_chunk_t();

   private:
    uint32_t m_width;
    uint32_t m_height;
    uint8_t m_bit_depth;
    color_type_t m_color_type;
    uint8_t m_compression_method;
    uint8_t m_filter_method;
    uint8_t m_interlace_method;
    png_t* m__root;
    png_t* m__parent;

   public:
    uint32_t width() const { return m_width; }
    uint32_t height() const { return m_height; }
    uint8_t bit_depth() const { return m_bit_depth; }
    color_type_t color_type() const { return m_color_type; }
    uint8_t compression_method() const { return m_compression_method; }
    uint8_t filter_method() const { return m_filter_method; }
    uint8_t interlace_method() const { return m_interlace_method; }
    png_t* _root() const { return m__root; }
    png_t* _parent() const { return m__parent; }
  };

  class plte_chunk_t : public kaitai::kstruct {
   public:
    explicit plte_chunk_t(kaitai::kstream* p_io,
                          png_t::chunk_t* p_parent = nullptr,
                          png_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~plte_chunk_t();

   private:
    std::vector<rgb_t*>* m_entries;
    png_t* m__root;
    png_t::chunk_t* m__parent;

   public:
    std::vector<rgb_t*>* entries() const { return m_entries; }
    png_t* _root() const { return m__root; }
    png_t::chunk_t* _parent() const { return m__parent; }
  };

  class srgb_chunk_t : public kaitai::kstruct {
   public:
    enum intent_t {
      INTENT_PERCEPTUAL = 0,
      INTENT_RELATIVE_COLORIMETRIC = 1,
      INTENT_SATURATION = 2,
      INTENT_ABSOLUTE_COLORIMETRIC = 3
    };

    explicit srgb_chunk_t(kaitai::kstream* p_io,
                          png_t::chunk_t* p_parent = nullptr,
                          png_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~srgb_chunk_t();

   private:
    intent_t m_render_intent;
    png_t* m__root;
    png_t::chunk_t* m__parent;

   public:
    intent_t render_intent() const { return m_render_intent; }
    png_t* _root() const { return m__root; }
    png_t::chunk_t* _parent() const { return m__parent; }
  };

  class bkgd_truecolor_t : public kaitai::kstruct {
   public:
    explicit bkgd_truecolor_t(kaitai::kstream* p_io,
                              png_t::bkgd_chunk_t* p_parent = nullptr,
                              png_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~bkgd_truecolor_t();

   private:
    uint16_t m_red;
    uint16_t m_green;
    uint16_t m_blue;
    png_t* m__root;
    png_t::bkgd_chunk_t* m__parent;

   public:
    uint16_t red() const { return m_red; }
    uint16_t green() const { return m_green; }
    uint16_t blue() const { return m_blue; }
    png_t* _root() const { return m__root; }
    png_t::bkgd_chunk_t* _parent() const { return m__parent; }
  };

  class gama_chunk_t : public kaitai::kstruct {
   public:
    explicit gama_chunk_t(kaitai::kstream* p_io,
                          png_t::chunk_t* p_parent = nullptr,
                          png_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~gama_chunk_t();

   private:
    bool f_gamma_ratio;
    double m_gamma_ratio;

   public:
    double gamma_ratio();

   private:
    uint32_t m_gamma_int;
    png_t* m__root;
    png_t::chunk_t* m__parent;

   public:
    uint32_t gamma_int() const { return m_gamma_int; }
    png_t* _root() const { return m__root; }
    png_t::chunk_t* _parent() const { return m__parent; }
  };

  class bkgd_chunk_t : public kaitai::kstruct {
   public:
    explicit bkgd_chunk_t(kaitai::kstream* p_io,
                          png_t::chunk_t* p_parent = nullptr,
                          png_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~bkgd_chunk_t();

   private:
    kaitai::kstruct* m_bkgd;
    png_t* m__root;
    png_t::chunk_t* m__parent;

   public:
    kaitai::kstruct* bkgd() const { return m_bkgd; }
    png_t* _root() const { return m__root; }
    png_t::chunk_t* _parent() const { return m__parent; }
  };

  class phys_chunk_t : public kaitai::kstruct {
   public:
    explicit phys_chunk_t(kaitai::kstream* p_io,
                          png_t::chunk_t* p_parent = nullptr,
                          png_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~phys_chunk_t();

   private:
    uint32_t m_pixels_per_unit_x;
    uint32_t m_pixels_per_unit_y;
    phys_unit_t m_unit;
    png_t* m__root;
    png_t::chunk_t* m__parent;

   public:
    uint32_t pixels_per_unit_x() const { return m_pixels_per_unit_x; }
    uint32_t pixels_per_unit_y() const { return m_pixels_per_unit_y; }
    phys_unit_t unit() const { return m_unit; }
    png_t* _root() const { return m__root; }
    png_t::chunk_t* _parent() const { return m__parent; }
  };

  class text_chunk_t : public kaitai::kstruct {
   public:
    explicit text_chunk_t(kaitai::kstream* p_io,
                          png_t::chunk_t* p_parent = nullptr,
                          png_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~text_chunk_t();

   private:
    std::string m_keyword;
    std::string m_text;
    png_t* m__root;
    png_t::chunk_t* m__parent;

   public:
    std::string keyword() const { return m_keyword; }
    std::string text() const { return m_text; }
    png_t* _root() const { return m__root; }
    png_t::chunk_t* _parent() const { return m__parent; }
  };

  class time_chunk_t : public kaitai::kstruct {
   public:
    explicit time_chunk_t(kaitai::kstream* p_io,
                          png_t::chunk_t* p_parent = nullptr,
                          png_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~time_chunk_t();

   private:
    uint16_t m_year;
    uint8_t m_month;
    uint8_t m_day;
    uint8_t m_hour;
    uint8_t m_minute;
    uint8_t m_second;
    png_t* m__root;
    png_t::chunk_t* m__parent;

   public:
    uint16_t year() const { return m_year; }
    uint8_t month() const { return m_month; }
    uint8_t day() const { return m_day; }
    uint8_t hour() const { return m_hour; }
    uint8_t minute() const { return m_minute; }
    uint8_t second() const { return m_second; }
    png_t* _root() const { return m__root; }
    png_t::chunk_t* _parent() const { return m__parent; }
  };

 private:
  std::vector<uint8_t> m_magic;
  std::vector<uint8_t> m_ihdr_len;
  std::vector<uint8_t> m_ihdr_type;
  ihdr_chunk_t* m_ihdr;
  std::vector<uint8_t> m_ihdr_crc;
  std::vector<chunk_t*>* m_chunks;
  png_t* m__root;
  kaitai::kstruct* m__parent;

 public:
  std::vector<uint8_t> magic() const { return m_magic; }
  std::vector<uint8_t> ihdr_len() const { return m_ihdr_len; }
  std::vector<uint8_t> ihdr_type() const { return m_ihdr_type; }
  ihdr_chunk_t* ihdr() const { return m_ihdr; }
  std::vector<uint8_t> ihdr_crc() const { return m_ihdr_crc; }
  std::vector<chunk_t*>* chunks() const { return m_chunks; }
  png_t* _root() const { return m__root; }
  kaitai::kstruct* _parent() const { return m__parent; }
};

}  // namespace png
}  // namespace kaitai
}  // namespace veles
#endif  // PNG_H_
