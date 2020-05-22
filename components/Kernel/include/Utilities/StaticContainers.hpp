#ifndef KUDZUKERNEL_STATICCONTAINERS_H
#define KUDZUKERNEL_STATICCONTAINERS_H
#include <array>
#include <type_traits>
#include <utility>
#include <stdexcept>

template <typename T, std::size_t N>
class StaticStorage {
public:
  typedef T ValueType;
  typedef T* iterator;
  typedef const T* const_iterator;
  StaticStorage(): m_size(0) { };

  bool  empty() const  { return this->m_size == 0; }
  bool  full()  const  { return this->m_size == N; }
  size_t size() const  { return this->m_size; }

  void clear() {
    while (this->m_size > 0) {
      reinterpret_cast<T*>(&data[--this->m_size])->~T();
    }
  }

  iterator begin() {
    return reinterpret_cast<iterator>(&data[0]);
  }
  iterator end() {
    return reinterpret_cast<iterator>(&data[m_size]);
  }

  const_iterator cbegin() const {
    return reinterpret_cast<const_iterator>(&data[0]);
  }
  const_iterator cend() const {
    return reinterpret_cast<const_iterator>(&data[m_size]);
  }

  T& operator [](int idx) {
    return reinterpret_cast<T&>(data[idx]);
  }

  T* c_arr() {
    return reinterpret_cast<T*>(this->data.data());
  }

protected:

  // The type for properly aligned storage type
  typedef
    typename std::aligned_storage<
      sizeof(ValueType),
      std::alignment_of<ValueType>::value
    >::type StorageType;

  // The type of the static array of values
  typedef std::array<StorageType, N> ArrayType;

  // The actual value storage
  ArrayType     data;
  std::size_t   m_size;
};

/**
 * A static vector is a fixed-size structure of pre-allocated data
 * that you can use like std::queue
 */
template <typename T, std::size_t N>
class StaticVector: public StaticStorage<T,N> {
public:
  StaticVector() : StaticStorage<T,N>() { };

  /**
   * Push back one item, calling the respective constructor with the
   * arguments given.
   */
  template<typename ...Args> void push_back(Args&&... args) {
    if ( this->m_size >= N )
      throw std::overflow_error{"Pushing on a full vector"};

    // construct value in memory of aligned storage
    // using inplace operator new
    new(&this->data[this->m_size]) T(std::forward<Args>(args)...);
    ++this->m_size;
  }

  /**
   * Pop an item from the queue
   */
  T pop_back() {
    if ( this->m_size == 0)
      throw std::underflow_error{"Popping from an empty vector"};
    --this->m_size;

    auto ret = *reinterpret_cast<T*>(&this->data[this->m_size]);
    reinterpret_cast<T*>(&this->data[this->m_size])->~T();
    return ret;
  }

  T& back() {
    return *reinterpret_cast<T*>(&this->data[this->m_size]);
  }

  T& front() {
    return *reinterpret_cast<T*>(&this->data[0]);
  }

};

/**
 * A static queue has a push_back and pop_front
 */
template <typename T, std::size_t N>
class StaticQueue: public StaticStorage<T,N> {
public:
  StaticQueue() : StaticStorage<T,N>(), m_head(0), m_tail(0) { };

  /**
   * Push back one item, calling the respective constructor with the
   * arguments given.
   */
  template<typename ...Args> void push_back(Args&&... args) {
    if ( this->m_size >= N )
      throw std::overflow_error{"Pushing on a full queue"};

    // construct value in memory of aligned storage
    // using inplace operator new
    new(&this->data[m_tail]) T(std::forward<Args>(args)...);

    // Advance tail
    m_tail = (m_tail + 1) % N;
    ++this->m_size;
  }

  /**
   * Pop an item from the queue
   */
  T pop_front() {
    if ( this->m_size == 0)
      throw std::underflow_error{"Popping from an empty queue"};
    --this->m_size;

    auto ret = *reinterpret_cast<T*>(&this->data[m_head]);
    reinterpret_cast<T*>(&this->data[m_head])->~T();

    // Advance head
    m_head = (m_head + 1) % N;
    return ret;
  }

  T& back() {
    return *reinterpret_cast<T*>(&this->data[m_head]);
  }

  T& front() {
    return *reinterpret_cast<T*>(&this->data[m_tail]);
  }

  void clear() {
    StaticStorage<T,N>::clear();
    m_head = 0;
    m_tail = 0;
  }

private:
  std::size_t   m_head, m_tail;
};

/**
 * A static map with
 */
template <typename K, typename V, std::size_t N>
class StaticMap: public StaticStorage< std::pair<K,V>, N > {
public:

  V& operator[]( const K& key ) {
    for (auto i=this->begin(); i!=this->end(); ++i) {
      if (std::get<0>(*i) == key) return std::get<1>(*i);
    }

    if ( this->m_size >= N )
      throw std::overflow_error{"Creating new item on full map"};

    auto newptr = new (&this->data[this->m_size]) std::pair<K,V>(key, V());
    ++this->m_size;

    return std::get<1>(*newptr);
  }

};

#endif
