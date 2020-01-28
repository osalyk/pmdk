/*
 * Copyright 2019-2020, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * pmem2.c -- pmem2 entry points for libpmem2
 */

#include "libpmem2.h"

int
pmem2_config_set_sharing(struct pmem2_config *cfg, unsigned type)
{
	return PMEM2_E_NOSUPP;
}

int
pmem2_config_set_protection(struct pmem2_config *cfg, unsigned flag)
{
	return PMEM2_E_NOSUPP;
}

int
pmem2_config_use_anonymous_mapping(struct pmem2_config *cfg, unsigned on)
{
	return PMEM2_E_NOSUPP;
}

int
pmem2_config_set_address(struct pmem2_config *cfg, unsigned type, void *addr)
{
	return PMEM2_E_NOSUPP;
}

#ifndef _WIN32
int
pmem2_get_device_id(const struct pmem2_config *cfg, char *id, size_t *len)
{
	return PMEM2_E_NOSUPP;
}
#else
int
pmem2_get_device_idW(const struct pmem2_config *cfg, wchar_t *id, size_t *len)
{
	return PMEM2_E_NOSUPP;
}

int
pmem2_get_device_idU(const struct pmem2_config *cfg, char *id, size_t *len)
{
	return PMEM2_E_NOSUPP;
}
#endif

int
pmem2_get_device_usc(const struct pmem2_config *cfg, uint64_t *usc)
{
	return PMEM2_E_NOSUPP;
}

int
pmem2_badblock_iterator_new(const struct pmem2_config *cfg,
		struct pmem2_badblock_iterator **pbb)
{
	return PMEM2_E_NOSUPP;
}

int
pmem2_badblock_next(struct pmem2_badblock_iterator *pbb,
		struct pmem2_badblock *bb)
{
	return PMEM2_E_NOSUPP;
}

void pmem2_badblock_iterator_delete(
		struct pmem2_badblock_iterator **pbb)
{
}

int
pmem2_badblock_clear(const struct pmem2_config *cfg,
		const struct pmem2_badblock *bb)
{
	return PMEM2_E_NOSUPP;
}
