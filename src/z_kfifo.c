#include "z_tool.h"
#include "z_debug.h"
#include "z_kfifo.h"

// Initializes the kfifo structure with a buffer and its size.
void z_kfifo_init(struct z_kfifo_struct *p_fifo, void *p_buffer, uint32_t size) {
    if (!p_fifo) return;          // Exit if the fifo pointer is null.
    p_fifo->p_buffer = p_buffer;  // Set the buffer pointer.
    p_fifo->size = size;          // Set the size of the buffer.
    p_fifo->in = p_fifo->out = 0; // Initialize read and write pointers to zero.
}

// Allocates memory for the kfifo buffer and initializes the structure.
int z_kfifo_malloc(struct z_kfifo_struct *p_fifo, uint32_t size) {
    if (!p_fifo) return -1; // Return error if fifo pointer is null.
    uint8_t *p_buffer;

    // Round up size to the nearest power of two if it's not already.
    if (size & (size - 1)) {
        size = Z_TOOL_ROUNDUP_POW_OF_TWO(size);
    }

    // Allocate memory for the buffer.
    p_buffer = (uint8_t *)z_tool_malloc(size);
    if (!p_buffer) {
        z_kfifo_init(p_fifo, NULL, 0); // Reset fifo if allocation fails.
        return -1;                     // Return error if memory allocation fails.
    }

    // Initialize fifo with allocated buffer and size.
    z_kfifo_init(p_fifo, p_buffer, size);
    return 0; // Return success.
}

// Frees the memory allocated for the kfifo buffer.
void z_kfifo_free(struct z_kfifo_struct *p_fifo) {
    if (!p_fifo) return; // Exit if the fifo pointer is null.
    if (p_fifo->p_buffer) {
        z_tool_free(p_fifo->p_buffer); // Free the allocated buffer.
    }
    z_kfifo_init(p_fifo, NULL, 0); // Reset the fifo structure.
}

// Writes data into the kfifo buffer.
uint32_t z_kfifo_in(struct z_kfifo_struct *p_fifo, const void *p_from, uint32_t len) {
    if (!p_fifo || !p_from) return 0; // Return zero if fifo or source pointer is null.
    uint32_t l, off;

    // Limit the length to the available space in the buffer.
    len = Z_TOOL_MIN(p_fifo->size - (p_fifo->in - p_fifo->out), len);
    off = p_fifo->in & (p_fifo->size - 1); // Calculate offset for circular buffer.

    // Determine how much data can be written in the first segment.
    l = Z_TOOL_MIN(len, p_fifo->size - off);
    memcpy(p_fifo->p_buffer + off, p_from, l); // Write to the buffer.

    // Write remaining data if the buffer wraps around.
    memcpy(p_fifo->p_buffer, (char *)p_from + l, len - l);
    p_fifo->in += len; // Update the write pointer.
    return len;        // Return the number of bytes written.
}

// Reads data from the kfifo buffer.
uint32_t z_kfifo_out(struct z_kfifo_struct *p_fifo, void *p_to, uint32_t len) {
    if (!p_fifo || !p_to) return 0; // Return zero if fifo or destination pointer is null.
    uint32_t off, l;

    // Limit the length to the available data in the buffer.
    len = Z_TOOL_MIN(p_fifo->in - p_fifo->out, len);
    off = p_fifo->out & (p_fifo->size - 1); // Calculate offset for circular buffer.

    // Determine how much data can be read in the first segment.
    l = Z_TOOL_MIN(len, p_fifo->size - off);
    memcpy(p_to, p_fifo->p_buffer + off, l); // Read from the buffer.

    // Read remaining data if the buffer wraps around.
    memcpy((char *)p_to + l, p_fifo->p_buffer, len - l);
    p_fifo->out += len; // Update the read pointer.
    return len;         // Return the number of bytes read.
}

// Checks the number of bytes available to read from the kfifo.
uint32_t z_kfifo_out_check(struct z_kfifo_struct *p_fifo, void *p_to, uint32_t len) {
    if (!p_fifo || !p_to) return 0; // Return zero if fifo or destination pointer is null.
    uint32_t off, l;

    // Limit the length to the available data in the buffer.
    len = Z_TOOL_MIN(p_fifo->in - p_fifo->out, len);
    off = p_fifo->out & (p_fifo->size - 1); // Calculate offset for circular buffer.

    // Determine how much data can be read in the first segment.
    l = Z_TOOL_MIN(len, p_fifo->size - off);
    memcpy(p_to, p_fifo->p_buffer + off, l); // Read from the buffer.

    // Read remaining data if the buffer wraps around.
    memcpy((char *)p_to + l, p_fifo->p_buffer, len - l);
    return len; // Return the number of bytes read.
}

// Returns the amount of free space in the kfifo.
uint32_t z_kfifo_space(struct z_kfifo_struct *p_fifo) {
    if (!p_fifo) return 0;                            // Return zero if fifo pointer is null.
    return p_fifo->size - (p_fifo->in - p_fifo->out); // Calculate available space.
}

// Returns the amount of data currently stored in the kfifo.
uint32_t z_kfifo_data_len(struct z_kfifo_struct *p_fifo) {
    if (!p_fifo) return 0;           // Return zero if fifo pointer is null.
    return p_fifo->in - p_fifo->out; // Calculate the length of stored data.
}

// Tests various functionalities of the kfifo.
uint32_t z_kfifo_test(void) {
    // Define the buffer size and initialize test data.
    const uint32_t buffer_size = 16;
    uint8_t test_data_in[buffer_size];
    uint8_t test_data_out[buffer_size];

    for (uint32_t i = 0; i < buffer_size; i++) {
        test_data_in[i] = i; // Fill test data with sequential values.
    }

    // Define kfifo structure and zero-initialize it.
    struct z_kfifo_struct fifo;
    memset(&fifo, 0, sizeof(fifo));

    // Test memory allocation for the kfifo.
    if (z_kfifo_malloc(&fifo, buffer_size) != 0) {
        Z_RAW("Memory allocation failed\n");
        return -1; // Return error if memory allocation fails.
    }

    // Test writing data into the kfifo.
    uint32_t written = z_kfifo_in(&fifo, test_data_in, buffer_size);
    if (written != buffer_size) {
        Z_RAW("Data write failed, written: %u\n", written);
        z_kfifo_free(&fifo);
        return -1; // Return error if the write operation fails.
    }

    // Test reading data from the kfifo.
    uint32_t read = z_kfifo_out(&fifo, test_data_out, buffer_size);
    if (read != buffer_size) {
        Z_RAW("Data read failed, read: %u\n", read);
        z_kfifo_free(&fifo);
        return -1; // Return error if the read operation fails.
    }

    // Verify that the read data matches the written data.
    for (uint32_t i = 0; i < buffer_size; i++) {
        if (test_data_out[i] != test_data_in[i]) {
            Z_RAW("Data mismatch at index %u, expected: %u, got: %u\n", i, test_data_in[i], test_data_out[i]);
            z_kfifo_free(&fifo);
            return -1; // Return error if there is a data mismatch.
        }
    }

    // Test writing to a full kfifo.
    written = z_kfifo_in(&fifo, test_data_in, buffer_size);
    if (written != buffer_size) {
        Z_RAW("Buffer full test failed, written: %u\n", written);
        z_kfifo_free(&fifo);
        return -1; // Return error if writing to full buffer fails.
    }

    // Test reading from an empty kfifo.
    read = z_kfifo_out(&fifo, test_data_out, buffer_size);
    if (read != buffer_size) {
        Z_RAW("Buffer empty test failed, read: %u\n", read);
        z_kfifo_free(&fifo);
        return -1; // Return error if reading from empty buffer fails.
    }

    // Test freeing memory allocated for the kfifo.
    z_kfifo_free(&fifo);
    if (fifo.p_buffer != NULL || fifo.size != 0 || fifo.in != 0 || fifo.out != 0) {
        Z_RAW("Memory free failed\n");
        return -1; // Return error if memory was not freed correctly.
    }

    Z_RAW("All tests passed successfully\n");
    return 0; // Return success if all tests pass.
}
