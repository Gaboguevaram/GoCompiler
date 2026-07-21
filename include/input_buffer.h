#ifndef INPUT_BUFFER_H
#define INPUT_BUFFER_H

/*
 * Input buffer: the classic two-block (twin) buffer with sentinels.
 * A single physical buffer of 2*BLOCK_SIZE+2 bytes is
 * split into two logical blocks, A and B, each closed by a sentinel byte.
 * Two pointers walk the buffer: `start` marks the beginning of the current
 * lexeme and `forward` is the lookahead cursor. When `forward` crosses a
 * sentinel, the *other* block is refilled from the file, which keeps the
 * number of read() system calls low.
 */

#define BLOCK_SIZE 16        /* size of each logical block (N) */
#define EOF_SENTINEL '\377'  /* in-band end-of-input marker; equals (char)EOF */

/**
 * Open the input file and prime the first block.
 * @param path path to the source file to scan.
 */
void input_open(const char *path);

/**
 * @return the next character from the buffer (EOF_SENTINEL at end of input).
 */
char read_char(void);

/**
 * Step the forward pointer back by one character (handles block wrap-around).
 */
void unread_char(void);

/**
 * Materialise the current lexeme (from `start` up to `forward`) as a freshly
 * allocated, NUL-terminated string. Caller owns the returned memory.
 * @return the lexeme; never NULL.
 */
char *get_lexeme(void);

/**
 * Move `start` up to `forward`, discarding the characters in between.
 */
void sync_lexeme_start(void);

/**
 * Close the input file and release the buffer.
 */
void input_close(void);

#endif /* INPUT_BUFFER_H */
