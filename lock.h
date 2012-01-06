/************
 *
 * Lock module provides tools for modules to share common resources (e.g. the SPI interface)
 *
 * NOTE: the lock routines are not interrupt-safe by default, they're meant to be used from within an event loop
 *
 */
 
/**
 * Opaque type to hold the state of a lock
 */
typedef const void* lock_t;


/**
 * Attempts to acquire given lock. 
 *
 * @param lock Pointer to the lock to attempt to acquire
 * @param a unique value representing the code attempting to acquire the lock. An
 *        easy way to have a guaranteed unique value is to use a pointed to a function
 *        or data structure in PROGMEM
 * @return True value if the lock was acquired successfully or has already been acquired by this module
 *         False if the lock is being held by another module
 *
 */
static inline uint8_t acquire_lock(lock_t *lock, const void *value) {
  if(!*lock) {
      return !!(*lock = value);
  } else {
      return *lock == value;
  }
}

/**
 * Checks to see if the module defined by lock_val holds the current lock
 *
 */
static inline uint8_t has_lock(lock_t *lock, const void *lock_val) {
    return *lock == lock_val;
}

/**
 * Unlocks the given lock
 *
 */
static inline void release_lock(lock_t *lock) {
    *lock = 0;
}
