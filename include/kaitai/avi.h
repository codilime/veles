#ifndef AVI_H_
#define AVI_H_

// This is a generated file! Please edit source .ksy file and use
// kaitai-struct-compiler to rebuild

#include <kaitai/kaitaistream.h>
#include <kaitai/kaitaistruct.h>

#include <cstdint>
#include <vector>

namespace veles {
namespace kaitai {
namespace avi {

class avi_t : public kaitai::kstruct {
 public:
  class list_body_t;
  class rect_t;
  class blocks_t;
  class avih_body_t;
  class block_t;
  class strh_body_t;
  class strf_body_t;

  enum chunk_type_t {
    CHUNK_TYPE_IDX1 = 829973609,
    CHUNK_TYPE_JUNK = 1263424842,
    CHUNK_TYPE_INFO = 1330007625,
    CHUNK_TYPE_ISFT = 1413894985,
    CHUNK_TYPE_LIST = 1414744396,
    CHUNK_TYPE_STRF = 1718776947,
    CHUNK_TYPE_AVIH = 1751742049,
    CHUNK_TYPE_STRH = 1752331379,
    CHUNK_TYPE_MOVI = 1769369453,
    CHUNK_TYPE_HDRL = 1819436136,
    CHUNK_TYPE_STRL = 1819440243
  };

  enum stream_type_t {
    STREAM_TYPE_MIDS = 1935960429,
    STREAM_TYPE_VIDS = 1935960438,
    STREAM_TYPE_AUDS = 1935963489,
    STREAM_TYPE_TXTS = 1937012852
  };

  enum handler_type_t {
    HANDLER_TYPE_MP3 = 85,
    HANDLER_TYPE_AC3 = 8192,
    HANDLER_TYPE_DTS = 8193,
    HANDLER_TYPE_CVID = 1684633187,
    HANDLER_TYPE_XVID = 1684633208
  };

  explicit avi_t(kaitai::kstream* p_io, kaitai::kstruct* p_parent = nullptr,
                 avi_t* p_root = nullptr);
  veles::dbif::ObjectHandle veles_obj;
  ~avi_t();

  class list_body_t : public kaitai::kstruct {
   public:
    explicit list_body_t(kaitai::kstream* p_io,
                         avi_t::block_t* p_parent = nullptr,
                         avi_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~list_body_t();

   private:
    chunk_type_t m_list_type;
    blocks_t* m_data;
    avi_t* m__root;
    avi_t::block_t* m__parent;

   public:
    chunk_type_t list_type() const { return m_list_type; }
    blocks_t* data() const { return m_data; }
    avi_t* _root() const { return m__root; }
    avi_t::block_t* _parent() const { return m__parent; }
  };

  class rect_t : public kaitai::kstruct {
   public:
    explicit rect_t(kaitai::kstream* p_io,
                    avi_t::strh_body_t* p_parent = nullptr,
                    avi_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~rect_t();

   private:
    int16_t m_left;
    int16_t m_top;
    int16_t m_right;
    int16_t m_bottom;
    avi_t* m__root;
    avi_t::strh_body_t* m__parent;

   public:
    int16_t left() const { return m_left; }
    int16_t top() const { return m_top; }
    int16_t right() const { return m_right; }
    int16_t bottom() const { return m_bottom; }
    avi_t* _root() const { return m__root; }
    avi_t::strh_body_t* _parent() const { return m__parent; }
  };

  class blocks_t : public kaitai::kstruct {
   public:
    explicit blocks_t(kaitai::kstream* p_io,
                      kaitai::kstruct* p_parent = nullptr,
                      avi_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~blocks_t();

   private:
    std::vector<block_t*>* m_entries;
    avi_t* m__root;
    kaitai::kstruct* m__parent;

   public:
    std::vector<block_t*>* entries() const { return m_entries; }
    avi_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
  };

  /**
   * Main header of an AVI file, defined as AVIMAINHEADER structure
   */

  class avih_body_t : public kaitai::kstruct {
   public:
    explicit avih_body_t(kaitai::kstream* p_io,
                         avi_t::block_t* p_parent = nullptr,
                         avi_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~avih_body_t();

   private:
    uint32_t m_micro_sec_per_frame;
    uint32_t m_max_bytes_per_sec;
    uint32_t m_padding_granularity;
    uint32_t m_flags;
    uint32_t m_total_frames;
    uint32_t m_initial_frames;
    uint32_t m_streams;
    uint32_t m_suggested_buffer_size;
    uint32_t m_width;
    uint32_t m_height;
    std::vector<uint8_t> m_reserved;
    avi_t* m__root;
    avi_t::block_t* m__parent;

   public:
    uint32_t micro_sec_per_frame() const { return m_micro_sec_per_frame; }
    uint32_t max_bytes_per_sec() const { return m_max_bytes_per_sec; }
    uint32_t padding_granularity() const { return m_padding_granularity; }
    uint32_t flags() const { return m_flags; }
    uint32_t total_frames() const { return m_total_frames; }
    uint32_t initial_frames() const { return m_initial_frames; }
    uint32_t streams() const { return m_streams; }
    uint32_t suggested_buffer_size() const { return m_suggested_buffer_size; }
    uint32_t width() const { return m_width; }
    uint32_t height() const { return m_height; }
    std::vector<uint8_t> reserved() const { return m_reserved; }
    avi_t* _root() const { return m__root; }
    avi_t::block_t* _parent() const { return m__parent; }
  };

  class block_t : public kaitai::kstruct {
   public:
    explicit block_t(kaitai::kstream* p_io, avi_t::blocks_t* p_parent = nullptr,
                     avi_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~block_t();

   private:
    chunk_type_t m_four_cc;
    uint32_t m_block_size;
    kaitai::kstruct* m_data;
    avi_t* m__root;
    avi_t::blocks_t* m__parent;
    std::vector<uint8_t> m__skip_me_data;
    kaitai::kstream* m__io__skip_me_data;

   public:
    chunk_type_t four_cc() const { return m_four_cc; }
    uint32_t block_size() const { return m_block_size; }
    kaitai::kstruct* data() const { return m_data; }
    avi_t* _root() const { return m__root; }
    avi_t::blocks_t* _parent() const { return m__parent; }
    std::vector<uint8_t> _skip_me_data() const { return m__skip_me_data; }
    kaitai::kstream* _io__skip_me_data() const { return m__io__skip_me_data; }
  };

  /**
   * Stream header (one header per stream), defined as AVISTREAMHEADER structure
   */

  class strh_body_t : public kaitai::kstruct {
   public:
    explicit strh_body_t(kaitai::kstream* p_io,
                         avi_t::block_t* p_parent = nullptr,
                         avi_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~strh_body_t();

   private:
    stream_type_t m_fcc_type;
    handler_type_t m_fcc_handler;
    uint32_t m_flags;
    uint16_t m_priority;
    uint16_t m_language;
    uint32_t m_initial_frames;
    uint32_t m_scale;
    uint32_t m_rate;
    uint32_t m_start;
    uint32_t m_length;
    uint32_t m_suggested_buffer_size;
    uint32_t m_quality;
    uint32_t m_sample_size;
    rect_t* m_frame;
    avi_t* m__root;
    avi_t::block_t* m__parent;

   public:
    /**
     * Type of the data contained in the stream
     */
    stream_type_t fcc_type() const { return m_fcc_type; }

    /**
     * Type of preferred data handler for the stream (specifies codec for audio
     * / video streams)
     */
    handler_type_t fcc_handler() const { return m_fcc_handler; }
    uint32_t flags() const { return m_flags; }
    uint16_t priority() const { return m_priority; }
    uint16_t language() const { return m_language; }
    uint32_t initial_frames() const { return m_initial_frames; }
    uint32_t scale() const { return m_scale; }
    uint32_t rate() const { return m_rate; }
    uint32_t start() const { return m_start; }
    uint32_t length() const { return m_length; }
    uint32_t suggested_buffer_size() const { return m_suggested_buffer_size; }
    uint32_t quality() const { return m_quality; }
    uint32_t sample_size() const { return m_sample_size; }
    rect_t* frame() const { return m_frame; }
    avi_t* _root() const { return m__root; }
    avi_t::block_t* _parent() const { return m__parent; }
  };

  /**
   * Stream format description
   */

  class strf_body_t : public kaitai::kstruct {
   public:
    explicit strf_body_t(kaitai::kstream* p_io,
                         kaitai::kstruct* p_parent = nullptr,
                         avi_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~strf_body_t();

   private:
    avi_t* m__root;
    kaitai::kstruct* m__parent;

   public:
    avi_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
  };

 private:
  std::vector<uint8_t> m_magic1;
  uint32_t m_file_size;
  std::vector<uint8_t> m_magic2;
  blocks_t* m_data;
  avi_t* m__root;
  kaitai::kstruct* m__parent;
  std::vector<uint8_t> m__skip_me_data;
  kaitai::kstream* m__io__skip_me_data;

 public:
  std::vector<uint8_t> magic1() const { return m_magic1; }
  uint32_t file_size() const { return m_file_size; }
  std::vector<uint8_t> magic2() const { return m_magic2; }
  blocks_t* data() const { return m_data; }
  avi_t* _root() const { return m__root; }
  kaitai::kstruct* _parent() const { return m__parent; }
  std::vector<uint8_t> _skip_me_data() const { return m__skip_me_data; }
  kaitai::kstream* _io__skip_me_data() const { return m__io__skip_me_data; }
};

}  // namespace avi
}  // namespace kaitai
}  // namespace veles
#endif  // AVI_H_
