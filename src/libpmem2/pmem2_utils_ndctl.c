// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

#include <errno.h>
#include <ndctl/libndctl.h>

#include "libpmem2.h"
#include "out.h"
#include "pmem2_utils.h"
#include "region_namespace_ndctl.h"
#include "source.h"

// Set USE_NDCTL environment variable to '1' to use functions with ndctl
#define USE_NDCTL_VAR "USE_NDCTL"

/*
 * pmem2_device_dax_alignment -- checks the alignment of a given
 * dax device from given source
 */
int
pmem2_device_dax_alignment(const struct pmem2_source *src, size_t *alignment)
{
	char *env_config = os_getenv(USE_NDCTL_VAR);
	if (env_config != NULL && env_config[0] == '1') {
		int ret = 0;
		size_t size = 0;
		struct ndctl_ctx *ctx;
		struct ndctl_namespace *ndns;

		errno = ndctl_new(&ctx) * (-1);
		if (errno) {
			ERR("!ndctl_new");
			return PMEM2_E_ERRNO;
		}

		ret = pmem2_region_namespace(ctx, src, NULL, &ndns);
		if (ret) {
			LOG(1, "getting region and namespace failed");
			goto end;
		}

		struct ndctl_dax *dax = ndctl_namespace_get_dax(ndns);

		if (dax)
			size = ndctl_dax_get_align(dax);
		else
			ret = PMEM2_E_INVALID_ALIGNMENT_FORMAT;

	end:
		ndctl_unref(ctx);

		*alignment = size;
		LOG(4, "device alignment %zu", *alignment);

		return ret;
	} else {
		ERR("Cannot read Device Dax alignment - ndctl is not available");

		return PMEM2_E_NOSUPP;
	}
}

/*
 * pmem2_device_dax_size -- checks the size of a given
 * dax device from given source structure
 */
int
pmem2_device_dax_size(const struct pmem2_source *src, size_t *size)
{
	char *env_config = os_getenv(USE_NDCTL_VAR);
	if (env_config != NULL && env_config[0] == '1') {
		int ret = 0;
		struct ndctl_ctx *ctx;
		struct ndctl_namespace *ndns;

		errno = ndctl_new(&ctx) * (-1);
		if (errno) {
			ERR("!ndctl_new");
			return PMEM2_E_ERRNO;
		}

		ret = pmem2_region_namespace(ctx, src, NULL, &ndns);
		if (ret) {
			LOG(1, "getting region and namespace failed");
			goto end;
		}

		struct ndctl_dax *dax = ndctl_namespace_get_dax(ndns);

		if (dax) {
			*size = ndctl_dax_get_size(dax);
		} else {
			ret = PMEM2_E_DAX_REGION_NOT_FOUND;
			ERR("Issue while reading Device Dax size - cannot "
				"find dax region");
		}

	end:
		ndctl_unref(ctx);
		LOG(4, "device size %zu", *size);

		return ret;
	} else {
		ERR("Cannot read Device Dax size - ndctl is not available");

		return PMEM2_E_NOSUPP;
	}
}
