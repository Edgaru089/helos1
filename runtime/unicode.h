#pragma once

#include <stddef.h>
#include <stdint.h>


// Constants

#define HELOS_UNICODE_TYPE (uint32_t) // We use uint32_t to hold a Unicode codepoint.

#define HELOS_UNICODE_ERROR          ((uint32_t)0xfffd)   // The "error" char or "Unicode replacement character".
#define HELOS_UNICODE_MAX            ((uint32_t)0x10ffff) // Maximum valid Unicode codepoint.
#define HELOS_UNICODE_SELF_REPRESENT (0x80)               // Chars below SELF_REPRESENT represents themselves in a single byte.

#define HELOS_UTF8_MAX_BYTES          4    // Maximum number of bytes encoding one unicode point in UTF-8.
#define HELOS_UTF8_CONTINUATION_MIN   0xc0 // Minimum of the continuation range in which bytes are not the first byte of a sequence.
#define HELOS_UTF8_CONTINUATION_MAX   0xbf // Maximum of the continuation range.
#define HELOS_UTF8_OVERLONG_LEADER_1  0xc0 // First leader byte forming an overlong sequence (encoding <=0xff in 2 bytes).
#define HELOS_UTF8_OVERLONG_LEADER_2  0xc1 // Second leader byte forming an overlong sequence.
#define HELOS_UTF8_INVALID_LEADER_MIN 0xf5 // Minimum of the tailing range in which leader bytes form sequences more than 4 bytes long.
#define HELOS_UTF8_INVALID_LEADER_MAX 0xff // Maximum of the tailing range in which leader bytes are invalid.


#ifdef __cplusplus
extern "C" {
#endif


// utf8_Decode advances the UTF-8 sequence by one character,
// returning the number of bytes advanced.
//
// The codepoint pointer, if not NULL, is set to the decoded value.
// If the Unicode sequence is invalid, the replacement char is returned.
size_t utf8_Decode(const char *utf8, size_t length, uint32_t *codepoint);

// utf8_EncodeLength returns the number of bytes required to encode
// the given codepoint in UTF-8 (ranging from 1 to 4).
//
// returns 0 if the codepoint is invalid.
size_t utf8_EncodeLength(uint32_t codepoint);

// utf8_Encode encodes a new character into the UTF-8 buffer,
// if utf8 is not NULL, returning the number of bytes written (max 4).
//
// The buffer must have enough space.
size_t utf8_Encode(char *utf8, uint32_t codepoint);


#ifdef __cplusplus
}
#endif
