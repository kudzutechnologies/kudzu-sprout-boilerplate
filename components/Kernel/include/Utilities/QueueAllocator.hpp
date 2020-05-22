#ifndef KUDZUKERNEL_QUEUEALLOCATOR_HPP
#define KUDZUKERNEL_QUEUEALLOCATOR_HPP
#include <cstdint>
#include <cstring>
#include <cstddef>

#include "Errors.hpp"
#include "Sherlock.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

/**
 * The size of the queue
 */
typedef uint16_t queue_chunk_size_t;

struct QueueChunkPtr {
  queue_chunk_size_t    size;
  void *                data;

  // We intentionally put a void* field before the variable structure member
  // in order to align it's contents with the default system alignment. If we
  // don't use it, meta[] will not be aligned!
  uint8_t               meta[];
};

template <typename META>
struct QueueChunk {
  queue_chunk_size_t    size;
  void *                data;
  META                  meta;
};

/**
 * @brief      Template-agnostic, base API for interfacing with a queue when
 *             it's implementation is not known to the consumer.
 */
struct QueueAllocatorInterface {
public:

  /**
   * @brief      Remove all entries in the queue
   */
  virtual void clear() = 0;

  /**
   * @brief      Lock queue and allocate an item, passing additional optional
   *             meta-data to the newly created item.
   *
   * @param[in]  size  The size of the item
   * @param[in]  meta  The metadata to append on the item (size depends on implementation)
   *
   * @return     Returns the new buffer pointer or NULL if allocation failed
   */
  virtual void * lockAllocPtr(const queue_chunk_size_t size, const void * meta = NULL) = 0;

  /**
   * @brief      Lock queue and pop an item from the queue
   *
   * @return     Returns a type-agnostic chunk pointer or NULL if queue is empty
   */
  virtual QueueChunkPtr * lockPopPtr() = 0;

  /**
   * @brief      Peek the upcoming item of the queue without popping it
   *
   * @param[in]  dest  Where to store the information of the peeked item
   */
  virtual void peekPtr(QueueChunkPtr * dest) = 0;

  /**
   * @brief      Unlocks a previously locked pool
   */
  virtual void unlock() = 0;

};

/**
 * @brief      Allocates arbitrary memory regions in a pre-reserved memory slab,
 *             in a first-in-first-out continuous manner (queue). This particular
 *             feature of the allocator enables dynamic memory compacting that
 *             maximizes the slab usage. Optional meta-data can accompany any
 *              push operation (eg. for it's tag or mime-type)
 *
 * @tparam     SIZE         The static size of the queue allocator
 * @tparam     META         The meta-data data type that accompany every chunk
 * @tparam     CHUNKSIZE_T  The type of the chunk size field
 */
template <queue_chunk_size_t SIZE, typename META, typename CHUNKSIZE_T = queue_chunk_size_t>
class QueueAllocator: public QueueAllocatorInterface {
public:

  struct ChunkHdr_t {
    ChunkHdr_t *   next;
    CHUNKSIZE_T    size;
    META           meta;
  };

public:

  typedef QueueChunk<META> Chunk;

  /**
   * The maximum data structure that can fit in the queue at some point
   */
  static const size_t maxSize = SIZE - sizeof(ChunkHdr_t);

  QueueAllocator(): QueueAllocatorInterface(), head(NULL), tail(NULL), end(&memory[0]) {
    memset(memory, 0, SIZE);
    mutex = xSemaphoreCreateBinary();
    if (mutex == NULL) PANIC(E_OUT_OF_MEMORY);
    xSemaphoreGive(mutex);
  }

  /**
   * @brief      Remove all entries in the queue
   */
  virtual void clear() {
    head = NULL;
    tail = NULL;
    end = &memory[0];
    memset(memory, 0, SIZE);
  }

  /**
   * @brief      The type-agnostic interface to lockAlloc<META>
   *
   * @param[in]  size  The size of the chunk to allocate
   * @param[in]  meta  The metadata to associate to the new item
   *
   * @return     Returns the new data pointer or NULL if the buffer is full
   */
  virtual void * lockAllocPtr(const CHUNKSIZE_T size, const void * meta = NULL) {
    if (meta == NULL) {
      return lockAlloc(size, META());
    } else {
      return lockAlloc(size, *((META*)meta));
    }
  }

  /**
   * @brief      Allocate one chunk in the queue and gain exclusive lock on it's
   *             contents. This prevents other processes from triggering a gap
   *             compaction and misaligning the returned pointer. Remember to
   *             call .unlock() when done.
   *
   * @param[in]  size  The size of the chunk to allocate
   * @param[in]  meta  The metadata to associate to the new item
   *
   * @return     Returns the new data pointer or NULL if the buffer is full
   */
  void * lockAlloc(const CHUNKSIZE_T size, const META meta) {
    xSemaphoreTake(mutex, portMAX_DELAY);
    void * ptr = alloc(size, meta);
    if (ptr == NULL) {
      xSemaphoreGive(mutex);
      return NULL;
    }
    return ptr;
  }

  /**
   * @brief      Unguarded allocation of a chunk.
   *
   * @param[in]  size  The size
   *
   * @return     Returns the new data pointer or NULL if the buffer is full
   */
  void * alloc(const CHUNKSIZE_T size, const META meta) {
    if (size == 0) return NULL;

    ChunkHdr_t * chunk = chunkAlloc(size, meta);
    if (chunk == NULL) return NULL;

    // printf("  put <- %ld\n", REL(chunk));
    return (void*)((uint8_t*)chunk + sizeof(ChunkHdr_t));
  }

  /**
   * @brief      Peek the upcoming item of the queue without popping it
   *
   * @param[in]  dest  Where to store the information of the peeked item
   */
  virtual void peekPtr(QueueChunkPtr * c) {
    // This ensures that the assumption we are doing later is going to be successful
    // with the compiler bing used.
    static_assert(
      offsetof(struct QueueChunk<META>, meta) == offsetof(struct QueueChunkPtr, meta),
      "Misaligned QueueChunkPtr and QueueChunk<META> fields. "
      "That's probably a compiler or a configuration issue"
    );

    // Cast QueueChunk<META> to QueueChunkPtr, that is structured in a way
    // to let the contents of the .meta variable of QueueChunk<META> fit on the
    // dynamically-sized member .meta of the QueueChunkPtr
    peek((Chunk*)c);
  }

  /**
   * @brief      Peek the upcoming item of the queue without popping it
   *
   * @param[in]  dest  Where to store the information of the peeked item
   */
  void peek(Chunk * dest) {
    if (dest == NULL) return;
    dest->size = 0;
    dest->data = NULL;

    // Check if queue is empty
    if (tail == NULL) return;

    // Get the next item details from head
    dest->meta = head->meta;
    dest->size = head->size;
    dest->data = (void*)((uint8_t*)head + sizeof(ChunkHdr_t));
  }

  /**
   * @brief      The type-agnostic interface to lockPop<META>
   *
   * @return     Returns a pointer to the data or NULL if empty
   */
  virtual QueueChunkPtr * lockPopPtr() {
    Chunk * c = lockPop();
    if (c == NULL) return NULL;

    // This ensures that the assumption we are doing later is going to be successful
    // with the compiler bing used.
    static_assert(
      offsetof(struct QueueChunk<META>, meta) == offsetof(struct QueueChunkPtr, meta),
      "Misaligned QueueChunkPtr and QueueChunk<META> fields. "
      "That's probably a compiler or a configuration issue"
    );

    // Cast QueueChunk<META> to QueueChunkPtr, that is structured in a way
    // to let the contents of the .meta variable of QueueChunk<META> fit on the
    // dynamically-sized member .meta of the QueueChunkPtr
    return (QueueChunkPtr*)c;
  }

  /**
   * @brief      Pops one chunk from the memory queue and gain exclusive lock
   *             on the allocator. This prevents other processes from altering
   *             the released memory region. Remember to call .unlock() when
   *             done.
   *
   * @return     Returns a pointer to the data or NULL if empty
   */
  Chunk * lockPop() {

    // When using this method, the contents of th chunk are guarded with
    // the lock, so it's safe to return the same static chunk instance.
    static Chunk c;

    xSemaphoreTake(mutex, portMAX_DELAY);
    ChunkHdr_t * chunk = chunkPop();
    if (chunk == NULL) {
      xSemaphoreGive(mutex);
      return NULL;
    }

    c.meta = chunk->meta;
    c.size = chunk->size;
    c.data = (void*)((uint8_t*)chunk + sizeof(ChunkHdr_t));

    return &c;
  }

  /**
   * @brief      Locks the queue (with lockAlloc or lockPop)
   */
  void lock() {
    xSemaphoreTake(mutex, portMAX_DELAY);
  }

  /**
   * @brief      Unlocks a previously locked queue (with lockAlloc or lockPop)
   */
  virtual void unlock() {
    xSemaphoreGive(mutex);
  }

  /**
   * @brief      Returns the number of bytes available for allocation
   *
   * @return     The number of bytes
   */
  size_t available() {
    uint8_t *bhead = (head == NULL) ? &memory[0] : (uint8_t*)head;
    uint8_t *btail = (tail == NULL) ? &memory[0] : ((uint8_t*)tail + chunkSize(tail));

    if (head == NULL || head <= tail) {
      size_t start_gap = bhead - &memory[0];
      size_t end_gap = &memory[SIZE] - btail;

      if (start_gap + end_gap <= sizeof(ChunkHdr_t)) {
        return 0;
      }

      return start_gap + end_gap - sizeof(ChunkHdr_t);

    } else {
      size_t interim_gap = bhead - btail;
      size_t end_gap = &memory[SIZE] - end;

      if (interim_gap + end_gap <= sizeof(ChunkHdr_t)) {
        return 0;
      }

      return interim_gap + end_gap - sizeof(ChunkHdr_t);
    }
  }

  /**
   * @brief      Checks if the queue is empty
   *
   * @return     Returns `true` if there are no items in the queue
   */
  bool empty() {
    return head == NULL;
  }

private:

  inline intptr_t REL(void * p) {
    if (p == NULL) return 0xFFFFFFFF;
    return ((uint8_t*)p - &memory[0]);
  }


  /**
   * @brief      Adjust the given size so it's aligned with the CPU arch.
   *             This ensures that the pointers given to the user are correctly
   *             aligned with the CPU demands.
   *
   * @param[in]  size  The desired size
   *
   * @return     The alignment-corrected size
   */
  inline CHUNKSIZE_T alignedSize(CHUNKSIZE_T size) {
    return (size + sizeof(uint32_t) - 1) & ~(sizeof(uint32_t)-1);;
  }

  /**
   * @brief      Calculate the real size of the chunk, including the header and
   *             the aligned body size.
   *
   * @param      ptr   A pointer to the chunk header
   *
   * @return     The real chunk size
   */
  inline CHUNKSIZE_T chunkSize(ChunkHdr_t * ptr) {
    if (ptr == NULL) return 0;
    return sizeof(ChunkHdr_t) + alignedSize(ptr->size);
  }

  /**
   * @brief      Re-compute the end of the data
   */
  void recomputeEnd() {
    // printf("  {end-adj} from=%ld, to=", REL(end));
    end = &memory[0];
    if (head != NULL) {
      ChunkHdr_t * tmp = head;
      while (tmp != NULL) {
        if ((uint8_t*)tmp > end) end = (uint8_t*)tmp;
        tmp = tmp->next;
      }
      end += chunkSize((ChunkHdr_t *)end);
    }
    // printf("%ld\n", REL(end));
  }

  /**
   * @brief      Pops the first available chunk from the queue and reclaims it's
   *             space. No compacting operation takes pace here.
   *
   * @return     Returns the pointer to the removed chunk
   */
  ChunkHdr_t * chunkPop() {
    // printf("[pop] head=%ld, tail=%ld, end=%ld\n", REL(head), REL(tail), REL(end));
    if (tail == NULL) return NULL;

    // Pop next item
    ChunkHdr_t * item = head;
    head = head->next;

    // If this was the last item, reset
    if (head == NULL) {
      tail = NULL;
      end = &memory[0];
      // printf("  get -> %ld\n", REL(item));
      return item;
    }

    // printf("  {adj} head=%ld, tail=%ld\n", REL(head), REL(tail));
    // printf("  get -> %ld\n", REL(item));

    // If this item is adjusting the ending, re-compute it
    uint8_t * bend = (uint8_t*)item + chunkSize(item);
    if (bend == end) {
      recomputeEnd();
    }

    return item;
  }

  /**
   * @brief      Allocate a new chunk
   *
   * @param[in]  size  The size to allocate excluding the including header.
   *
   * @return     Returns the pointer to a newly initialized chunk.
   */
  ChunkHdr_t * chunkAlloc(const CHUNKSIZE_T size, const META meta) {
    ChunkHdr_t *ptr = NULL;
    CHUNKSIZE_T real_size = alignedSize(size) + sizeof(ChunkHdr_t);
    uint8_t *bhead = (head == NULL) ? &memory[0] : (uint8_t*)head;
    uint8_t *btail = (tail == NULL) ? &memory[0] : ((uint8_t*)tail + chunkSize(tail));

    // printf("[alloc] size=%d (%d), head=%ld, tail=%ld, btail=%ld, end=%ld\n", real_size, size, REL(head), REL(tail), REL(btail), REL(end));

    if (head == NULL || head <= tail) {
      size_t start_gap = bhead - &memory[0];
      size_t end_gap = &memory[SIZE] - btail;
      uint8_t *bptr;

      // printf("  {H<T} : start_gap=%zu, end_gap=%zu\n", start_gap, end_gap);

      // Case 0 : Not enough space
      if ((start_gap + end_gap) < real_size) {
        return NULL;
      }

      // Case 1 : Not enough data for end gap, but could compact either gaps
      if (end_gap < real_size) {
        if (start_gap < end_gap) {
          collapseStartGap();
        } else {
          collapseEndGap();
        }

        // Re-compute adjusted bounds
        bhead = (uint8_t*)head;
        btail = (uint8_t*)tail + ((tail == NULL) ? 0 : chunkSize(tail));
      }

      // Find the location where to allocate our pointer
      if (head == NULL) {
        bptr = &memory[0];
      } else {
        if (btail + real_size > &memory[SIZE]) {
          // Wrap overflown data
          bptr = &memory[0];
        } else {
          bptr = btail;
        }
      }

      // printf("  {H<T} : init_at=%ld\n", REL(bptr));

      // Initialize and place the node in the tree
      ptr = initAt(bptr, size, meta);
      if (head == NULL) {
        head = tail = ptr;
      } else {
        tail->next = ptr;
        tail = ptr;
      }
    }

    //
    else {
      size_t interim_gap = bhead - btail;
      size_t end_gap = &memory[SIZE] - end;

      // printf("  {T<H} : interim_gap=%zu, end_gap=%zu\n", interim_gap, end_gap);

      // Case 0 : Not enough space
      if ((interim_gap + end_gap) < real_size) {
        return NULL;
      }

      // Case 1 : Not enough data for interim gap, but could compact end gap
      if (interim_gap < real_size) {
        expandInterimGap();

        // Re-compute adjusted bounds
        bhead = (uint8_t*)head;
        btail = (uint8_t*)tail + ((tail == NULL) ? 0 : chunkSize(tail));
      }

      // Initialize and place the node in the tree
      ptr = initAt(btail, size, meta);
      if (head == NULL) {
        // NOTE(icharala): We should never reach this case
        head = tail = ptr;
      } else {
        tail->next = ptr;
        tail = ptr;
      }
    }

    return ptr;
  }

  /**
   * @brief      Initialize a chunk structure at the given pointer
   *
   * @param      ofs   The pointer in memory where to initialize a new chunk
   * @param[in]  size  The size of the chunk
   * @param[in]  meta  The meta-data of the chunk
   *
   * @return     The newly initialized chunk structure
   */
  ChunkHdr_t * initAt(uint8_t * ofs, const CHUNKSIZE_T size, const META meta) {
    ChunkHdr_t * ptr = (ChunkHdr_t *)ofs;
    memset(ptr, 0, sizeof(ChunkHdr_t));
    ptr->size = size;
    ptr->meta = meta;

    // printf("[init] size=%d (%d)\n", sizeof(ChunkHdr_t) + alignedSize(size), size);

    // Adjust end of data in the memory
    uint8_t * ptrEnd = ofs + sizeof(ChunkHdr_t) + alignedSize(size);
    if (end == NULL || ptrEnd > end) {
      end = ptrEnd;
    }

    return ptr;
  }

  /**
   * @brief      Helper function to shift the contents of the memory of the given
   *             region that also adjusts the linked list pointers.
   *
   * @param      vstart  The starting address
   * @param      vend    The ending address
   * @param[in]  shift   The number of bytes to shift
   */
  void shiftRegion(void* vstart, void* vend, int32_t shift) {
    uint8_t *start = (uint8_t*)vstart, *end = (uint8_t*)vend;
    // printf("  > shift from=%ld, to=%ld, ofs=%d\n", REL(start), REL(end), shift);

    // First adjust the pointer locations
    ChunkHdr_t * n = head;
    while ((n != NULL) && (n->next != NULL)) {
      uint8_t * bnext = (uint8_t*)n->next;
      ChunkHdr_t * u = n;
      n = n->next;

      // If it is pointing to a shifted region, adjust the pointer
      if ((bnext >= start) && (bnext <= end)) {
        bnext += shift;
        // printf("  > map=%ld to %ld\n", REL(u->next), REL(bnext));
        u->next = (ChunkHdr_t*)bnext;
      }
    }

    // Then adjust the data space
    memmove(start + shift, start, end - start);

    // Adjust individual pointers
    bool adjusted = false;
    if ( (((uint8_t*)head) >= start) && (((uint8_t*)head) <= end) ) {
      // printf("  > head=%ld to", REL(head));
      head = (ChunkHdr_t*)(((uint8_t*)head) + shift);
      // printf(" %ld\n", REL(head));
      adjusted = true;
    }
    if ( (((uint8_t*)tail) >= start) && (((uint8_t*)tail) <= end) ) {
      // printf("  > tail=%ld to", REL(tail));
      tail = (ChunkHdr_t*)(((uint8_t*)tail) + shift);
      // printf(" %ld\n", REL(tail));
      adjusted = true;
    }

    // Recompute upper bounds
    recomputeEnd();
  }

  /**
   * @brief      Move the T-H region from H to 0
   *             This function should be used when H < T and the start (left)
   *             gap must be collapsed. This will maximize the end (right) gap.
   *             This function will also adjust all the `next` pointers.
   */
  void collapseStartGap() {
    // printf("[collapse:start]\n");

    int32_t offset = (uint8_t*)head - &memory[0];
    shiftRegion(head, (uint8_t*)tail + chunkSize(tail), -offset);
  }

  /**
   * @brief      Move the T-H region from T to (END - (T-H))
   *             This function should be used when H < T and the end (right)
   *             gap must be collapsed. This will maximize the start (left) gap.
   *             The immediate action after calling this function is typically
   *             to overflow the tail pointer to the beginning of the buffer.
   *             This function will also adjust all the `next` pointers.
   */
  void collapseEndGap() {
    // printf("[collapse:end]\n");

    int32_t offset = &memory[SIZE] - (uint8_t*)tail - chunkSize(tail);
    shiftRegion(head, (uint8_t*)tail + chunkSize(tail), offset);
  }

  /**
   * @brief      Move the E-H region from H to (END - (E-H))
   *             This function should be used when T < H and the interim gap
   *             (between tail and head) has to be maximized, consuming the
   *             end (right) gap.
   */
  void expandInterimGap() {
    // printf("[expand:interim]\n");

    int32_t offset = &memory[SIZE] - end;
    shiftRegion(head, end, offset);
  }


private:

  uint8_t             memory[SIZE];
  ChunkHdr_t          *head, *tail;
  uint8_t             *end;
  SemaphoreHandle_t   mutex;

};

#endif
