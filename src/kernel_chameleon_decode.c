/*
 * Centaurean Density
 *
 * Copyright (c) 2013, Guillaume Voirin
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Centaurean nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * 24/10/13 12:28
 *
 * -------------------
 * Chameleon algorithm
 * -------------------
 *
 * Author(s)
 * Guillaume Voirin (https://github.com/gpnuma)
 *
 * Description
 * Hash based superfast kernel
 */

#include "kernel_chameleon_decode.h"
#include "memory_location.h"

DENSITY_FORCE_INLINE DENSITY_KERNEL_DECODE_STATE density_chameleon_decode_check_state(density_memory_location *restrict out, density_chameleon_decode_state *restrict state) {
    if (out->available_bytes < DENSITY_CHAMELEON_DECODE_MINIMUM_OUTPUT_LOOKAHEAD)
        return DENSITY_KERNEL_DECODE_STATE_STALL_ON_OUTPUT_BUFFER;

    switch (state->signaturesCount) {
        case DENSITY_CHAMELEON_PREFERRED_EFFICIENCY_CHECK_SIGNATURES:
            if (state->efficiencyChecked ^ 0x1) {
                state->efficiencyChecked = 1;
                return DENSITY_KERNEL_DECODE_STATE_INFO_EFFICIENCY_CHECK;
            }
            break;
        case DENSITY_CHAMELEON_PREFERRED_BLOCK_SIGNATURES:
            state->signaturesCount = 0;
            state->efficiencyChecked = 0;

            if (state->resetCycle)
                state->resetCycle--;
            else {
                density_byte resetDictionaryCycleShift = state->parameters.as_bytes[0];
                if (resetDictionaryCycleShift) {
                    density_chameleon_dictionary_reset(&state->dictionary);
                    state->resetCycle = (uint_fast64_t) (1 << resetDictionaryCycleShift) - 1;
                }
            }

            return DENSITY_KERNEL_DECODE_STATE_INFO_NEW_BLOCK;
        default:
            break;
    }
    return DENSITY_KERNEL_DECODE_STATE_READY;
}

DENSITY_FORCE_INLINE void density_chameleon_decode_read_signature(density_memory_location *restrict in, density_chameleon_decode_state *restrict state) {
    state->signature = DENSITY_LITTLE_ENDIAN_64(*(density_chameleon_signature *) (in->pointer));
    in->pointer += sizeof(density_chameleon_signature);
    state->shift = 0;
    state->signaturesCount++;
}

DENSITY_FORCE_INLINE void density_chameleon_decode_read_compressed_chunk(uint16_t *restrict chunk, density_memory_location *restrict in) {
    *chunk = *(uint16_t *) (in->pointer);
    in->pointer += sizeof(uint16_t);
}

DENSITY_FORCE_INLINE void density_chameleon_decode_read_uncompressed_chunk(uint32_t *restrict chunk, density_memory_location *restrict in) {
    *chunk = *(uint32_t *) (in->pointer);
    in->pointer += sizeof(uint32_t);
}

DENSITY_FORCE_INLINE void density_chameleon_decode_compressed_chunk(const uint16_t *restrict chunk, density_memory_location *restrict out, density_chameleon_decode_state *restrict state) {
    *(uint32_t *) (out->pointer) = (&state->dictionary.entries[DENSITY_LITTLE_ENDIAN_16(*chunk)])->as_uint32_t;
    out->pointer += sizeof(uint32_t);
}

DENSITY_FORCE_INLINE void density_chameleon_decode_uncompressed_chunk(const uint32_t *restrict chunk, density_memory_location *restrict out, density_chameleon_decode_state *restrict state) {
    uint32_t hash;
    DENSITY_CHAMELEON_HASH_ALGORITHM(hash, DENSITY_LITTLE_ENDIAN_32(*chunk));
    (&state->dictionary.entries[hash])->as_uint32_t = *chunk;
    *(uint32_t *) (out->pointer) = *chunk;
    out->pointer += sizeof(uint32_t);
}

DENSITY_FORCE_INLINE void density_chameleon_decode_kernel(density_memory_location *restrict in, density_memory_location *restrict out, const density_bool compressed, density_chameleon_decode_state *restrict state) {
    if (compressed) {
        uint16_t chunk;
        density_chameleon_decode_read_compressed_chunk(&chunk, in);
        density_chameleon_decode_compressed_chunk(&chunk, out, state);
    } else {
        uint32_t chunk;
        density_chameleon_decode_read_uncompressed_chunk(&chunk, in);
        density_chameleon_decode_uncompressed_chunk(&chunk, out, state);
    }
}

DENSITY_FORCE_INLINE const bool density_chameleon_decode_test_compressed(density_chameleon_decode_state *state) {
    return (density_bool const) ((state->signature >> state->shift) & DENSITY_CHAMELEON_SIGNATURE_FLAG_MAP);
}

DENSITY_FORCE_INLINE void density_chameleon_decode_process_data(density_memory_location *restrict in, density_memory_location *restrict out, density_chameleon_decode_state *restrict state) {
    while (state->shift ^ 64) {
        density_chameleon_decode_kernel(in, out, density_chameleon_decode_test_compressed(state), state);
        state->shift++;
    }
}

DENSITY_FORCE_INLINE DENSITY_KERNEL_DECODE_STATE density_chameleon_decode_init(density_chameleon_decode_state *restrict state, const density_main_header_parameters parameters, const uint_fast32_t endDataOverhead) {
    state->signaturesCount = 0;
    state->efficiencyChecked = 0;
    density_chameleon_dictionary_reset(&state->dictionary);

    state->parameters = parameters;
    density_byte resetDictionaryCycleShift = state->parameters.as_bytes[0];
    if (resetDictionaryCycleShift)
        state->resetCycle = (uint_fast64_t) (1 << resetDictionaryCycleShift) - 1;

    state->endDataOverhead = endDataOverhead;

    state->process = DENSITY_CHAMELEON_DECODE_PROCESS_PREPARE_NEW_BLOCK;

    return DENSITY_KERNEL_DECODE_STATE_READY;
}

DENSITY_FORCE_INLINE DENSITY_KERNEL_DECODE_STATE density_chameleon_decode_process(density_memory_teleport *restrict in, density_memory_location *restrict out, density_chameleon_decode_state *restrict state, const density_bool flush) {
    DENSITY_KERNEL_DECODE_STATE returnState;
    density_memory_location *readMemoryLocation;
    uint_fast64_t remaining;

    switch (state->process) {
        case DENSITY_CHAMELEON_DECODE_PROCESS_PREPARE_NEW_BLOCK:
        prepare_new_block:
            if ((returnState = density_chameleon_decode_check_state(out, state)))
                return returnState;
            state->process = DENSITY_CHAMELEON_DECODE_PROCESS_SIGNATURE;

        case DENSITY_CHAMELEON_DECODE_PROCESS_SIGNATURE:
            if (flush) {
                remaining = density_memory_teleport_available(in) - sizeof(density_block_footer) - sizeof(density_main_footer);
                if (remaining < DENSITY_CHAMELEON_ENCODE_PROCESS_UNIT_SIZE) {
                    if(remaining > out->available_bytes)
                        return DENSITY_KERNEL_DECODE_STATE_STALL_ON_OUTPUT_BUFFER;
                    density_memory_teleport_copy(in, out, remaining);
                    return DENSITY_KERNEL_DECODE_STATE_FINISHED;
                }
            }
            if (!(readMemoryLocation = density_memory_teleport_read(in, sizeof(density_chameleon_signature))))
                return DENSITY_KERNEL_DECODE_STATE_STALL_ON_INPUT_BUFFER;
            density_chameleon_decode_read_signature(readMemoryLocation, state);
            readMemoryLocation->available_bytes -= sizeof(density_chameleon_signature);
            state->bodyLength = (uint_fast32_t) (sizeof(uint32_t) * bitsizeof(density_chameleon_signature) - __builtin_popcountll(state->signature) * (sizeof(uint32_t) - sizeof(uint16_t)));
            state->process = DENSITY_CHAMELEON_DECODE_PROCESS_DECOMPRESS_BODY;

        case DENSITY_CHAMELEON_DECODE_PROCESS_DECOMPRESS_BODY:
            if (!(readMemoryLocation = density_memory_teleport_read(in, state->bodyLength)))
                return DENSITY_KERNEL_DECODE_STATE_STALL_ON_INPUT_BUFFER;
            density_chameleon_decode_process_data(readMemoryLocation, out, state);
            readMemoryLocation->available_bytes -= state->bodyLength;
            out->available_bytes -= bitsizeof(density_chameleon_signature) * sizeof(uint32_t);
            state->process = DENSITY_CHAMELEON_DECODE_PROCESS_PREPARE_NEW_BLOCK;
            goto prepare_new_block;
    }

    return DENSITY_KERNEL_DECODE_STATE_READY;
}

DENSITY_FORCE_INLINE DENSITY_KERNEL_DECODE_STATE density_chameleon_decode_finish(density_chameleon_decode_state *state) {
    return DENSITY_KERNEL_DECODE_STATE_READY;
}