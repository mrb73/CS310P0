#include <stdio.h>  // needed for size_t
#include <sys/mman.h> // needed for mmap
#include <assert.h> // needed for asserts
#include "dmm.h"

/* You can improve the below metadata structure using the concepts from Bryant
 * and OHallaron book (chapter 9).
 */

typedef struct metadata {
  /* size_t is the return type of the sizeof operator. Since the size of an
   * object depends on the architecture and its implementation, size_t is used
   * to represent the maximum size of any object in the particular
   * implementation. size contains the size of the data object or the number of
   * free bytes
   */
  size_t size;
  struct metadata* next;
  struct metadata* prev; 
} metadata_t;

/* freelist maintains all the blocks which are not in use; freelist is kept
 * sorted to improve coalescing efficiency 
 */

static metadata_t* freelist = NULL;

void* dmalloc(size_t numbytes) {
  /* initialize through sbrk call first time */
  if(freelist == NULL) { 			
    if(!dmalloc_init())
      return NULL;
  }

  assert(numbytes > 0);

  /* your code here */

  int blocksize = numbytes + METADATA_T_ALIGNED;
  metadata_t *freelist_iter = freelist;
  while(freelist_iter->size < blocksize && freelist_iter->next != NULL){
    freelist_iter = freelist_iter->next;
  }
  assert(freelist_iter->size >= blocksize);
  metadata_t *new_block = freelist_iter + (freelist_iter->size - blocksize);
  new_block->size = blocksize;
  new_block->next = NULL;
  new_block->prev = NULL;
  freelist_iter->size -= blocksize;
	
  return (void*) new_block + METADATA_T_ALIGNED;
}

void dfree(void* ptr) {
  /* your code here */
  if(freelist == NULL) {      
    if(!dmalloc_init())
      return NULL;
  }
  metadata_t *freelist_iter = freelist;
  metadata_t *new_block = ptr;
  while(freelist_iter < ptr && freelist_iter->next != NULL){
    freelist_iter = freelist_iter->next;
  }
  if(freelist_iter < ptr){
    freelist_iter->next = new_block;
    return (void*) freelist;
  }
  freelist_iter->prev->next = new_block;
  new_block->next = freelist_iter;
  new_block->prev = freelist_iter->prev;
  freelist_iter->prev = new_block;

  metadata_t *coalescing = freelist;
  while(coalescing->next != NULL){
    if(coalescing + coalescing->size == coalescing->next){
      coalescing->size += coalescing->next->size;
      coalescing->next = coalescing->next->next;
      coalescing->next->prev = coalescing;
    }
    else{
      coalescing = coalescing->next;
    }
  }
  
}

bool dmalloc_init() {

  /* Two choices: 
   * 1. Append prologue and epilogue blocks to the start and the
   * end of the freelist 
   *
   * 2. Initialize freelist pointers to NULL
   *
   * Note: We provide the code for 2. Using 1 will help you to tackle the 
   * corner cases succinctly.
   */

  size_t max_bytes = ALIGN(MAX_HEAP_SIZE);
  /* returns heap_region, which is initialized to freelist */
  freelist = (metadata_t*) mmap(NULL, max_bytes, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  /* Q: Why casting is used? i.e., why (void*)-1? */
  if (freelist == (void *)-1)
    return false;
  freelist->next = NULL;
  freelist->prev = NULL;
  freelist->size = max_bytes-METADATA_T_ALIGNED;
  return true;
}

/* for debugging; can be turned off through -NDEBUG flag*/
void print_freelist() {
  metadata_t *freelist_head = freelist;
  while(freelist_head != NULL) {
    DEBUG("\tFreelist Size:%zd, Head:%p, Prev:%p, Next:%p\t",
	  freelist_head->size,
	  freelist_head,
	  freelist_head->prev,
	  freelist_head->next);
    freelist_head = freelist_head->next;
  }
  DEBUG("\n");
}
