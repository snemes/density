/*
 * Centaurean Density
 *
 * Copyright (c) 2018, Guillaume Voirin
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     1. Redistributions of source code must retain the above copyright notice, this
 *        list of conditions and the following disclaimer.
 *
 *     2. Redistributions in binary form must reproduce the above copyright notice,
 *        this list of conditions and the following disclaimer in the documentation
 *        and/or other materials provided with the distribution.
 *
 *     3. Neither the name of the copyright holder nor the names of its
 *        contributors may be used to endorse or promote products derived from
 *        this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * 14/02/18 14:37
 *
 * ---------------
 * Swift algorithm
 * ---------------
 *
 * Author(s)
 * Guillaume Voirin (https://github.com/gpnuma)
 *
 * Description
 * Hash based kernel derived from Chameleon, with very small memory footprint and quicker learning for small datasets
 */

#ifndef DENSITY_SWIFT_ENCODE_H
#define DENSITY_SWIFT_ENCODE_H

#include "../dictionary/swift_dictionary.h"
#include "../../algorithms.h"

DENSITY_WINDOWS_EXPORT void density_swift_encode_prepare_signature(uint8_t **DENSITY_RESTRICT_DECLARE, density_swift_signature **DENSITY_RESTRICT_DECLARE, density_swift_signature *DENSITY_RESTRICT_DECLARE);
DENSITY_WINDOWS_EXPORT const uint16_t density_swift_encode_4(const uint8_t **DENSITY_RESTRICT_DECLARE, uint8_t **DENSITY_RESTRICT_DECLARE, uint_fast8_t, density_swift_signature *DENSITY_RESTRICT_DECLARE, density_swift_dictionary *DENSITY_RESTRICT_DECLARE, uint32_t *DENSITY_RESTRICT_DECLARE);

#endif
