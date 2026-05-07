/* Stubs for uzlib checksum APIs not shipped in lib/uzlib/src/.
 * tinflate.c references these unconditionally inside uzlib_uncompress_chksum,
 * but the project only ever calls uzlib_uncompress (no-checksum variant) at
 * runtime, so plain stubs are link-safe. */
#include <stdint.h>

unsigned int uzlib_adler32(const void *data, unsigned int length,
                           unsigned int prev_sum) {
  (void)data; (void)length;
  return prev_sum;
}

unsigned int uzlib_crc32(const void *data, unsigned int length,
                         unsigned int prev_crc) {
  (void)data; (void)length;
  return prev_crc;
}
