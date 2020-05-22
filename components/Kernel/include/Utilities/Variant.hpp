#ifndef KUDZUKERNEL_VARIANT_H
#define KUDZUKERNEL_VARIANT_H
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <array>

/**
 * @brief      The primitive type of variant carried by a `Variant` variant
 * @details    The variants of this enum is used to represent the type of the
 *             internal variant
 */
enum VariantType_t : uint8_t {
  V_NONE = 0,
  V_BOOL,
  V_UINT8,
  V_UINT16,
  V_UINT32,
  V_UINT64,
  V_INT8,
  V_INT16,
  V_INT32,
  V_INT64,
  V_FLOAT32,
  V_FLOAT64,
  V_BYTES,
  V_CSTRING,
};

/**
 * @brief      A variant variant structure
 * @details    This structure can be blindly assigned any integer, string or
 *             byte structure and it will automatically cast it to any desired
 *             type supported.
 */
struct Variant {
public:

  /**
   * Default constructor
   */
  Variant();

  /**
   * Default destructor
   */
  ~Variant();

  /**
   * Define a copy constructor that carries a variant object
   */
  Variant(const Variant &v);

  /**
   * Implicitly define a copy constructor for all of the set() functions
   */
  template <typename T> Variant(const T& v) {
    this->set(v);
  }

  /**
   * Implicitly define an assign operator overload for all of the set() functions
   */
  template <typename T>
  Variant& operator=(const T& v) {
    this->set(v);
    return *this;
  }

  /**
   * Variant to constant comparator
   */
  template <typename T>
  bool operator==(const T& v) const {
    return v == this->get<T>();
  }

  /**
   * Variant to string comparator
   */
  bool operator==(const char * v) const;

  /**
   * Variant to variant comparator
   */
  bool operator==(const Variant& v) const;

  /**
   * Release dynamic memory and reset state
   */
  void reset();

  /**
   * Return a data pointer
   */
  const void* getPtr() const;

  /**
   * Return the size of the data structure
   */
  size_t getSize();

  /////// Variant Getters ///////

  template <typename T>
  operator T() const {
    return get<T>();
  }

  operator const char*() const;

  /////// Variant Getters ///////

  /**
   * @brief      Explicit variant stringifier
   * @details    This function returns the stored variant as a NULL-terminated string.
   *             if the carried variant is not a string already, it will be stringified
   *             into a temporary string buffer.
   *
   * @return     Returns the variant as a constant c-string expression
   */
  const char * get() const;

  template <typename T>
  const T get() const {
    double v;

    switch (dtype) {
    case V_BOOL:
      return (T)data.b;

    case V_UINT8:
      return (T)data.u8;
    case V_UINT16:
      return (T)data.u16;
    case V_UINT32:
      return (T)data.u32;
    case V_UINT64:
      return (T)data.u64;

    case V_INT8:
      return (T)data.i8;
    case V_INT16:
      return (T)data.i16;
    case V_INT32:
      return (T)data.i32;
    case V_INT64:
      return (T)data.i64;

    case V_FLOAT32:
      return (T)data.f32;
    case V_FLOAT64:
      return (T)data.f64;

    case V_CSTRING:
      v = atof((char*)this->getPtr());
      return (T)v;

    default:
      return (T)0;
    }
  }

  // Numerical Setters

  void set(const bool& v);
  void set(const uint8_t& v);
  void set(const uint16_t& v);
  void set(const uint32_t& v);
  void set(const uint64_t& v);
  void set(const int8_t& v);
  void set(const int16_t& v);
  void set(const int32_t& v);
  void set(const int64_t& v);
  void set(const float& v);
  void set(const double& v);

  // Buffer setters

  void set(const char* v);
  void set(const void * ptr, size_t len);

  /**
   * @brief      Implicit buffer setter
   *
   * @param[in]  v     The buffer to assign
   * @tparam     size  The size of the buffer, as resolved by the compiler
   */
  template <size_t size>
  void set(const uint8_t(&v)[size]) {
    set((void*)&v, size);
  }

  template <typename T, size_t size>
  void set(const std::array<T, size> &v) {
    set((void*)&v, size);
  }

  /**
   * Checks if the variant is E_NONE
   */
  bool isEmpty() const;

  /**
   * Checks if the variant is numeric
   */
  bool isNumeric() const;

private:

  union {
    bool              b;
    uint8_t           u8;
    uint16_t          u16;
    uint32_t          u32;
    uint64_t          u64;
    int8_t            i8;
    int16_t           i16;
    int32_t           i32;
    int64_t           i64;
    float             f32;
    double            f64;
    char              c12[8]; // 8 + 4 in the refcount
    void*             ptr;
  } data;

  // Alignment padding (c12 is allowed to overflow on top of this)
  uint32_t            padding;

  uint32_t            dsize : 24;
  VariantType_t       dtype : 8;
};

#endif
