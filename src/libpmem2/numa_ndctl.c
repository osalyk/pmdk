// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020-2023, Intel Corporation */

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
 * pmem2_source_numa_node -- gets the numa node on which a pmem file
 * is located from given source structure
 */
int
pmem2_source_numa_node(const struct pmem2_source *src, int *numa_node)
{
	char *env_config = os_getenv(USE_NDCTL_VAR);
	if (env_config != NULL && env_config[0] == '1') {
		PMEM2_ERR_CLR();

		LOG(3, "src %p numa_node %p", src, numa_node);

		struct ndctl_ctx *ctx;
		struct ndctl_region *region = NULL;

		if (src->type == PMEM2_SOURCE_ANON) {
			ERR("Anonymous sources are not bound to numa nodes.");
			return PMEM2_E_NOSUPP;
		}

		ASSERTeq(src->type, PMEM2_SOURCE_FD);

		LOG(3, "ndctl_new()");
		errno = ndctl_new(&ctx) * (-1);
		if (errno) {
			ERR("!ndctl_new");
			return PMEM2_E_ERRNO;
		}

		int ret = pmem2_region_namespace(ctx, src, &region, NULL);
		if (ret < 0) {
			LOG(1, "getting region failed");
			goto end;
		}

		if (region == NULL) {
			ERR("unknown region");
			ret = PMEM2_E_DAX_REGION_NOT_FOUND;
			goto end;
		}

		*numa_node = ndctl_region_get_numa_node(region);
		LOG(3, "src numa node %d", *numa_node);

	end:
		ndctl_unref(ctx);
		return ret;
	} else {
		SUPPRESS_UNUSED(src, numa_node);
		ERR("Cannot get numa node from source - ndctl is not available");

		return PMEM2_E_NOSUPP;
	}
}
