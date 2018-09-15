#include <stdio.h>  // needed for size_t
#include <sys/mman.h> // needed for mmap
#include <assert.h> // needed for asserts
#include <limits.h>
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
  metadata_t *max = freelist_iter;
  int val = INT_MAX;
  printf("Blocksize: %i\n", blocksize);
  while(freelist_iter->next != NULL){
    if(freelist_iter->size > blocksize && freelist_iter->size < val){
      val = freelist_iter->size;
      max = freelist_iter;
    }
    freelist_iter = freelist_iter->next;
    printf("%p\n", freelist_iter);
  }
  assert(max->size >= blocksize);
  //freelist_iter=max;
  metadata_t *new_block = (void*)max + (max->size + METADATA_T_ALIGNED - blocksize);
  new_block->size = numbytes;
  new_block->next = NULL;
  new_block->prev = NULL;
  freelist_iter->size -= blocksize;
  //if(freelist_iter->next != NULL){
    //freelist_iter->next = (void*)freelist_iter + blocksize;
    //freelist_iter->next->prev = freelist_iter;
  //}
	
  return (void*) new_block + METADATA_T_ALIGNED;
}

void dfree(void* ptr) {
  /* your code here */
  ptr -= METADATA_T_ALIGNED;
  metadata_t *freelist_iter = freelist;
  metadata_t *new_block = (metadata_t*) ptr;
  while(freelist_iter < (metadata_t*)ptr && freelist_iter->next != NULL){
        printf("%p\n", freelist_iter->prev);
    printf("%p\n", freelist_iter);
    printf("%p\n", freelist_iter->next);
    freelist_iter = freelist_iter->next;
        printf("%p\n", freelist_iter->prev);
    printf("%p\n", freelist_iter);
    printf("%p\n", freelist_iter->next);
  }
      printf("%p\n", freelist_iter->prev);
    printf("%p\n", freelist_iter);
    printf("%p\n", freelist_iter->next);
  if(freelist_iter < (metadata_t*)ptr){
    printf("%s\n", "here");
    freelist_iter->next = new_block;
    new_block->prev = freelist_iter;
  }
  else{
    new_block->next = freelist_iter;
    if(freelist_iter->prev != NULL){
      //printf("%p\n", freelist_iter->prev->next);
      metadata_t *prev_node = freelist_iter->prev;
      prev_node->next = new_block;
      new_block->prev = prev_node;
    }
    freelist_iter->prev = new_block;
  }

  metadata_t *coalescing = freelist;
  while(coalescing->next != NULL){
    if((void*)coalescing + coalescing->size + METADATA_T_ALIGNED == coalescing->next){
      coalescing->size += coalescing->next->size + METADATA_T_ALIGNED;
      coalescing->next = coalescing->next->next;
      if(coalescing->next != NULL){
        coalescing->next->prev = coalescing;
      }
    }
    else{
      coalescing = (metadata_t*)coalescing->next;
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
