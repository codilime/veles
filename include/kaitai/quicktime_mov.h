#ifndef QUICKTIME_MOV_H_
#define QUICKTIME_MOV_H_

// This is a generated file! Please edit source .ksy file and use
// kaitai-struct-compiler to rebuild

#include <kaitai/kaitaistream.h>
#include <kaitai/kaitaistruct.h>

#include <cstdint>
#include <vector>

namespace veles {
namespace kaitai {
namespace quicktime_mov {
class quicktime_mov_t;
class quicktime_mov_t;
class quicktime_mov_t;
class quicktime_mov_t;
class quicktime_mov_t;
class quicktime_mov_t;
class quicktime_mov_t;
class quicktime_mov_t;

class quicktime_mov_t : public kaitai::kstruct {
 public:
  class mvhd_body_t;
  class ftyp_body_t;
  class fixed32_t;
  class fixed16_t;
  class atom_t;
  class tkhd_body_t;

  enum atom_type_t {
    ATOM_TYPE_XTRA = 1484026465,
    ATOM_TYPE_DINF = 1684631142,
    ATOM_TYPE_DREF = 1685218662,
    ATOM_TYPE_EDTS = 1701082227,
    ATOM_TYPE_ELST = 1701606260,
    ATOM_TYPE_FREE = 1718773093,
    ATOM_TYPE_FTYP = 1718909296,
    ATOM_TYPE_HDLR = 1751411826,
    ATOM_TYPE_IODS = 1768907891,
    ATOM_TYPE_MDAT = 1835295092,
    ATOM_TYPE_MDHD = 1835296868,
    ATOM_TYPE_MDIA = 1835297121,
    ATOM_TYPE_META = 1835365473,
    ATOM_TYPE_MINF = 1835626086,
    ATOM_TYPE_MOOF = 1836019558,
    ATOM_TYPE_MOOV = 1836019574,
    ATOM_TYPE_MVHD = 1836476516,
    ATOM_TYPE_SMHD = 1936549988,
    ATOM_TYPE_STBL = 1937007212,
    ATOM_TYPE_STCO = 1937007471,
    ATOM_TYPE_STSC = 1937011555,
    ATOM_TYPE_STSD = 1937011556,
    ATOM_TYPE_STSZ = 1937011578,
    ATOM_TYPE_STTS = 1937011827,
    ATOM_TYPE_TKHD = 1953196132,
    ATOM_TYPE_TRAF = 1953653094,
    ATOM_TYPE_TRAK = 1953653099,
    ATOM_TYPE_TREF = 1953654118,
    ATOM_TYPE_UDTA = 1969517665,
    ATOM_TYPE_VMHD = 1986881636
  };

  enum brand_t {
    BRAND_3G2A = 862401121,
    BRAND_3GE6 = 862414134,
    BRAND_3GE9 = 862414137,
    BRAND_3GF9 = 862414393,
    BRAND_3GG6 = 862414646,
    BRAND_3GG9 = 862414649,
    BRAND_3GH9 = 862414905,
    BRAND_3GM9 = 862416185,
    BRAND_3GP4 = 862416948,
    BRAND_3GP5 = 862416949,
    BRAND_3GP6 = 862416950,
    BRAND_3GP7 = 862416951,
    BRAND_3GP8 = 862416952,
    BRAND_3GP9 = 862416953,
    BRAND_3GR6 = 862417462,
    BRAND_3GR9 = 862417465,
    BRAND_3GS6 = 862417718,
    BRAND_3GS9 = 862417721,
    BRAND_3GT9 = 862417977,
    BRAND_ARRI = 1095914057,
    BRAND_CAEP = 1128351056,
    BRAND_CDES = 1128555891,
    BRAND_J2P0 = 1244811312,
    BRAND_J2P1 = 1244811313,
    BRAND_LCAG = 1279476039,
    BRAND_M4A = 1295270176,
    BRAND_M4B = 1295270432,
    BRAND_M4P = 1295274016,
    BRAND_M4V = 1295275552,
    BRAND_MFSM = 1296454477,
    BRAND_MGSV = 1296520022,
    BRAND_MPPI = 1297109065,
    BRAND_MSNV = 1297305174,
    BRAND_ROSS = 1380930387,
    BRAND_SEAU = 1397047637,
    BRAND_SEBK = 1397047883,
    BRAND_XAVC = 1480676931,
    BRAND_AVC1 = 1635148593,
    BRAND_BBXM = 1650620525,
    BRAND_CAQV = 1667330422,
    BRAND_CCFF = 1667458662,
    BRAND_DA0A = 1684090977,
    BRAND_DA0B = 1684090978,
    BRAND_DA1A = 1684091233,
    BRAND_DA1B = 1684091234,
    BRAND_DA2A = 1684091489,
    BRAND_DA2B = 1684091490,
    BRAND_DA3A = 1684091745,
    BRAND_DA3B = 1684091746,
    BRAND_DASH = 1684108136,
    BRAND_DBY1 = 1684175153,
    BRAND_DMB1 = 1684890161,
    BRAND_DSMS = 1685286259,
    BRAND_DV1A = 1685467489,
    BRAND_DV1B = 1685467490,
    BRAND_DV2A = 1685467745,
    BRAND_DV2B = 1685467746,
    BRAND_DV3A = 1685468001,
    BRAND_DV3B = 1685468002,
    BRAND_DVR1 = 1685484081,
    BRAND_DVT1 = 1685484593,
    BRAND_DXO = 1685614368,
    BRAND_EMSG = 1701671783,
    BRAND_IFRM = 1768321645,
    BRAND_ISC2 = 1769169714,
    BRAND_ISO2 = 1769172786,
    BRAND_ISO3 = 1769172787,
    BRAND_ISO4 = 1769172788,
    BRAND_ISO5 = 1769172789,
    BRAND_ISO6 = 1769172790,
    BRAND_ISOM = 1769172845,
    BRAND_JP2 = 1785737760,
    BRAND_JPM = 1785752864,
    BRAND_JPSI = 1785754473,
    BRAND_JPX = 1785755680,
    BRAND_JPXB = 1785755746,
    BRAND_LMSG = 1819112295,
    BRAND_MJ2S = 1835676275,
    BRAND_MJP2 = 1835692082,
    BRAND_MP21 = 1836069425,
    BRAND_MP41 = 1836069937,
    BRAND_MP42 = 1836069938,
    BRAND_MP71 = 1836070705,
    BRAND_MSDH = 1836278888,
    BRAND_MSIX = 1836280184,
    BRAND_NIKO = 1852402543,
    BRAND_ODCF = 1868850022,
    BRAND_OPF2 = 1869637170,
    BRAND_OPX2 = 1869641778,
    BRAND_PANA = 1885433441,
    BRAND_PIFF = 1885955686,
    BRAND_PNVI = 1886287465,
    BRAND_QT = 1903435808,
    BRAND_RISX = 1919513464,
    BRAND_SDV = 1935963680,
    BRAND_SENV = 1936027254,
    BRAND_SIMS = 1936289139,
    BRAND_SISX = 1936290680,
    BRAND_SSSS = 1936946035,
    BRAND_UVVU = 1970697845
  };

  explicit quicktime_mov_t(kaitai::kstream* p_io,
                           kaitai::kstruct* p_parent = nullptr,
                           quicktime_mov_t* p_root = nullptr);
  veles::dbif::ObjectHandle veles_obj;
  ~quicktime_mov_t();

  class mvhd_body_t : public kaitai::kstruct {
   public:
    explicit mvhd_body_t(kaitai::kstream* p_io,
                         quicktime_mov_t::atom_t* p_parent = nullptr,
                         quicktime_mov_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~mvhd_body_t();

   private:
    uint8_t m_version;
    std::vector<uint8_t> m_flags;
    uint32_t m_creation_time;
    uint32_t m_modification_time;
    uint32_t m_time_scale;
    uint32_t m_duration;
    fixed32_t* m_preferred_rate;
    fixed16_t* m_preferred_volume;
    std::vector<uint8_t> m_reserved1;
    std::vector<uint8_t> m_matrix;
    uint32_t m_preview_time;
    uint32_t m_preview_duration;
    uint32_t m_poster_time;
    uint32_t m_selection_time;
    uint32_t m_selection_duration;
    uint32_t m_current_time;
    uint32_t m_next_track_id;
    quicktime_mov_t* m__root;
    quicktime_mov_t::atom_t* m__parent;

   public:
    /**
     * Version of this movie header atom
     */
    uint8_t version() const { return m_version; }
    std::vector<uint8_t> flags() const { return m_flags; }
    uint32_t creation_time() const { return m_creation_time; }
    uint32_t modification_time() const { return m_modification_time; }

    /**
     * A time value that indicates the time scale for this movie—that is, the
     number of time units that pass per second in its time coordinate system. A
     time coordinate system that measures time in sixtieths of a second, for
     example, has a time scale of 60.

     */
    uint32_t time_scale() const { return m_time_scale; }

    /**
     * A time value that indicates the duration of the movie in time scale
     units. Note that this property is derived from the movie’s tracks. The
     value of this field corresponds to the duration of the longest track in the
     movie.

     */
    uint32_t duration() const { return m_duration; }

    /**
     * The rate at which to play this movie. A value of 1.0 indicates normal
     * rate.
     */
    fixed32_t* preferred_rate() const { return m_preferred_rate; }

    /**
     * How loud to play this movie’s sound. A value of 1.0 indicates full
     * volume.
     */
    fixed16_t* preferred_volume() const { return m_preferred_volume; }
    std::vector<uint8_t> reserved1() const { return m_reserved1; }

    /**
     * A matrix shows how to map points from one coordinate space into another.
     */
    std::vector<uint8_t> matrix() const { return m_matrix; }

    /**
     * The time value in the movie at which the preview begins.
     */
    uint32_t preview_time() const { return m_preview_time; }

    /**
     * The duration of the movie preview in movie time scale units.
     */
    uint32_t preview_duration() const { return m_preview_duration; }

    /**
     * The time value of the time of the movie poster.
     */
    uint32_t poster_time() const { return m_poster_time; }

    /**
     * The time value for the start time of the current selection.
     */
    uint32_t selection_time() const { return m_selection_time; }

    /**
     * The duration of the current selection in movie time scale units.
     */
    uint32_t selection_duration() const { return m_selection_duration; }

    /**
     * The time value for current time position within the movie.
     */
    uint32_t current_time() const { return m_current_time; }

    /**
     * Indicates a value to use for the track ID number of the next track added
     * to this movie. Note that 0 is not a valid track ID value.
     */
    uint32_t next_track_id() const { return m_next_track_id; }
    quicktime_mov_t* _root() const { return m__root; }
    quicktime_mov_t::atom_t* _parent() const { return m__parent; }
  };

  class ftyp_body_t : public kaitai::kstruct {
   public:
    explicit ftyp_body_t(kaitai::kstream* p_io,
                         quicktime_mov_t::atom_t* p_parent = nullptr,
                         quicktime_mov_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~ftyp_body_t();

   private:
    brand_t m_major_brand;
    std::vector<uint8_t> m_minor_version;
    std::vector<brand_t>* m_compatible_brands;
    quicktime_mov_t* m__root;
    quicktime_mov_t::atom_t* m__parent;

   public:
    brand_t major_brand() const { return m_major_brand; }
    std::vector<uint8_t> minor_version() const { return m_minor_version; }
    std::vector<brand_t>* compatible_brands() const {
      return m_compatible_brands;
    }
    quicktime_mov_t* _root() const { return m__root; }
    quicktime_mov_t::atom_t* _parent() const { return m__parent; }
  };

  class fixed32_t : public kaitai::kstruct {
   public:
    explicit fixed32_t(kaitai::kstream* p_io,
                       kaitai::kstruct* p_parent = nullptr,
                       quicktime_mov_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~fixed32_t();

   private:
    int16_t m_int_part;
    uint16_t m_frac_part;
    quicktime_mov_t* m__root;
    kaitai::kstruct* m__parent;

   public:
    int16_t int_part() const { return m_int_part; }
    uint16_t frac_part() const { return m_frac_part; }
    quicktime_mov_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
  };

  class fixed16_t : public kaitai::kstruct {
   public:
    explicit fixed16_t(kaitai::kstream* p_io,
                       quicktime_mov_t::mvhd_body_t* p_parent = nullptr,
                       quicktime_mov_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~fixed16_t();

   private:
    int8_t m_int_part;
    uint8_t m_frac_part;
    quicktime_mov_t* m__root;
    quicktime_mov_t::mvhd_body_t* m__parent;

   public:
    int8_t int_part() const { return m_int_part; }
    uint8_t frac_part() const { return m_frac_part; }
    quicktime_mov_t* _root() const { return m__root; }
    quicktime_mov_t::mvhd_body_t* _parent() const { return m__parent; }
  };

  class atom_t : public kaitai::kstruct {
   public:
    explicit atom_t(kaitai::kstream* p_io, quicktime_mov_t* p_parent = nullptr,
                    quicktime_mov_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~atom_t();

   private:
    bool f_len;
    int32_t m_len;

   public:
    int32_t len();

   private:
    uint32_t m_len32;
    atom_type_t m_atom_type;
    uint64_t m_len64;
    bool n_len64;

   public:
    bool _is_null_len64() {
      len64();
      return n_len64;
    };

   private:
    kaitai::kstruct* m_body;
    quicktime_mov_t* m__root;
    quicktime_mov_t* m__parent;
    std::vector<uint8_t> m__skip_me_body;
    kaitai::kstream* m__io__skip_me_body;

   public:
    uint32_t len32() const { return m_len32; }
    atom_type_t atom_type() const { return m_atom_type; }
    uint64_t len64() const { return m_len64; }
    kaitai::kstruct* body() const { return m_body; }
    quicktime_mov_t* _root() const { return m__root; }
    quicktime_mov_t* _parent() const { return m__parent; }
    std::vector<uint8_t> _skip_me_body() const { return m__skip_me_body; }
    kaitai::kstream* _io__skip_me_body() const { return m__io__skip_me_body; }
  };

  class tkhd_body_t : public kaitai::kstruct {
   public:
    explicit tkhd_body_t(kaitai::kstream* p_io,
                         quicktime_mov_t::atom_t* p_parent = nullptr,
                         quicktime_mov_t* p_root = nullptr);
    veles::dbif::ObjectHandle veles_obj;
    ~tkhd_body_t();

   private:
    uint8_t m_version;
    std::vector<uint8_t> m_flags;
    uint32_t m_creation_time;
    uint32_t m_modification_time;
    uint32_t m_track_id;
    std::vector<uint8_t> m_reserved1;
    uint32_t m_duration;
    std::vector<uint8_t> m_reserved2;
    uint16_t m_layer;
    uint16_t m_alternative_group;
    uint16_t m_volume;
    uint16_t m_reserved3;
    std::vector<uint8_t> m_matrix;
    fixed32_t* m_width;
    fixed32_t* m_height;
    quicktime_mov_t* m__root;
    quicktime_mov_t::atom_t* m__parent;

   public:
    uint8_t version() const { return m_version; }
    std::vector<uint8_t> flags() const { return m_flags; }
    uint32_t creation_time() const { return m_creation_time; }
    uint32_t modification_time() const { return m_modification_time; }

    /**
     * Integer that uniquely identifies the track. The value 0 cannot be used.
     */
    uint32_t track_id() const { return m_track_id; }
    std::vector<uint8_t> reserved1() const { return m_reserved1; }
    uint32_t duration() const { return m_duration; }
    std::vector<uint8_t> reserved2() const { return m_reserved2; }
    uint16_t layer() const { return m_layer; }
    uint16_t alternative_group() const { return m_alternative_group; }
    uint16_t volume() const { return m_volume; }
    uint16_t reserved3() const { return m_reserved3; }
    std::vector<uint8_t> matrix() const { return m_matrix; }
    fixed32_t* width() const { return m_width; }
    fixed32_t* height() const { return m_height; }
    quicktime_mov_t* _root() const { return m__root; }
    quicktime_mov_t::atom_t* _parent() const { return m__parent; }
  };

 private:
  std::vector<atom_t*>* m_atoms;
  quicktime_mov_t* m__root;
  kaitai::kstruct* m__parent;

 public:
  std::vector<atom_t*>* atoms() const { return m_atoms; }
  quicktime_mov_t* _root() const { return m__root; }
  kaitai::kstruct* _parent() const { return m__parent; }
};

}  // namespace quicktime_mov
}  // namespace kaitai
}  // namespace veles
#endif  // QUICKTIME_MOV_H_
