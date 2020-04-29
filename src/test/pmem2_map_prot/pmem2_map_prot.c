// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2020, Intel Corporation */

/*
 * pmem2_map_prot.c -- pmem2_map_prot unittests
 */

#include <stdbool.h>
#include <signal.h>
#include <setjmp.h>

#include "config.h"
#include "source.h"
#include "map.h"
#include "out.h"
#include "pmem2.h"
#include "unittest.h"
#include "ut_pmem2.h"
#include "ut_fh.h"

/*
 * prepare_config -- fill pmem2_config
 */
static void
prepare_config(struct pmem2_config *cfg, struct pmem2_source **src,
	struct FHandle **fh, const char *file, size_t length, size_t offset,
	int access)
{
#ifdef _WIN32
	*fh = UT_FH_OPEN(FH_HANDLE, file, access);
#else
	*fh = UT_FH_OPEN(FH_FD, file, access);
#endif

	pmem2_config_init(cfg);
	cfg->offset = offset;
	cfg->length = length;
	cfg->requested_max_granularity = PMEM2_GRANULARITY_PAGE;

	PMEM2_SOURCE_FROM_FH(src, *fh);
}

/*
 * unmap_map -- unmap the mapping according to pmem2_map struct
 */
static void
unmap_map(struct pmem2_map *map)
{
#ifdef _WIN32
	UT_ASSERTne(UnmapViewOfFile(map->addr), 0);
#else
	UT_ASSERTeq(munmap(map->addr, map->reserved_length), 0);
#endif
	UT_ASSERTeq(pmem2_unregister_mapping(map), 0);
}

static const char *word1 = "Persistent or nonpersistent: that is the question.";

static ut_jmp_buf_t Jmp;

/*
 * signal_handler -- called on SIGSEGV
 */
static void
signal_handler(int sig)
{
	ut_siglongjmp(Jmp);
}

/*
 * test_rw_mode_rw_prot -- test R/W protection
 * pmem2_map() - should success
 * memcpy - should success
 */
static int
test_rw_mode_rw_prot(const struct test_case *tc,
		int argc, char *argv[])
{
	if (argc < 1)
		UT_FATAL("usage: test_map_prot <file>");

	char *file = argv[0];
	struct FHandle *fh;
	struct pmem2_config cfg;
	struct pmem2_source *src;

	/* read/write */
	prepare_config(&cfg, &src, &fh, file, 0, 0, FH_RDWR);
	pmem2_config_set_protection(&cfg, PMEM2_PROT_READ | PMEM2_PROT_WRITE);

	struct pmem2_map *map;
	int ret = pmem2_map(&cfg, src, &map);
	UT_ASSERTeq(ret, 0);

	pmem2_memcpy_fn memcpy_fn = pmem2_get_memcpy_fn(map);
	void *addr_map = pmem2_map_get_address(map);
	memcpy_fn(addr_map, word1, strlen(word1), 0);
	UT_ASSERTeq(memcmp(addr_map, word1, strlen(word1)), 0);

	unmap_map(map);
	FREE(map);
	UT_FH_CLOSE(fh);
	PMEM2_SOURCE_DELETE(&src);
	return 1;
}

/*
 * test_r_mode_rw_prot -- test R/W protection
 * pmem2_map() - should fail
 */
static int
test_r_mode_rw_prot(const struct test_case *tc,
	int argc, char *argv[])
{
	if (argc < 1)
		UT_FATAL("usage: test_map_prot <file>");

	char *file = argv[0];
	struct FHandle *fh;
	struct pmem2_config cfg;
	struct pmem2_source *src;

	/* read/write on file opened in read-only mode - should fail */
	prepare_config(&cfg, &src, &fh, file, 0, 0, FH_READ);
	pmem2_config_set_protection(&cfg, PMEM2_PROT_READ | PMEM2_PROT_WRITE);

	struct pmem2_map *map;
	int ret = pmem2_map(&cfg, src, &map);
	UT_PMEM2_EXPECT_RETURN(ret, -EACCES);

	UT_FH_CLOSE(fh);
	PMEM2_SOURCE_DELETE(&src);
	return 1;
}

/*
 * test_rw_mode_r_prot -- test R/W protection
 * pmem2_map() - should success
 * memcpy - should fail
 */
static int
test_rw_mode_r_prot(const struct test_case *tc,
		int argc, char *argv[])
{
	if (argc < 1)
		UT_FATAL("usage: test_map_prot <file>");

	/* arrange to catch SEGV */
	struct sigaction v;
	sigemptyset(&v.sa_mask);
	v.sa_flags = 0;
	v.sa_handler = signal_handler;
	SIGACTION(SIGSEGV, &v, NULL);

	char *file = argv[0];
	struct FHandle *fh;
	struct pmem2_config cfg;
	struct pmem2_source *src;

	/* read-only */
	prepare_config(&cfg, &src, &fh, file, 0, 0, FH_RDWR);
	pmem2_config_set_protection(&cfg, PMEM2_PROT_READ);

	struct pmem2_map *map;
	int ret = pmem2_map(&cfg, src, &map);
	UT_ASSERTeq(ret, 0);

	pmem2_memcpy_fn memcpy_fn = pmem2_get_memcpy_fn(map);
	void *addr_map = pmem2_map_get_address(map);
	if (!ut_sigsetjmp(Jmp)) {
		/* same memcpy from above should now fail */
		memcpy_fn(addr_map, word1, strlen(word1), 0);
	} else {
		UT_OUT("memcpy successful");
	}

	unmap_map(map);
	FREE(map);
	UT_FH_CLOSE(fh);
	PMEM2_SOURCE_DELETE(&src);
	return 1;
}

/*
 * test_r_mode_r_prot -- test R/W protection
 * pmem2_map() - should success
 * memcpy - should fail
 */
static int
test_r_mode_r_prot(const struct test_case *tc,
		int argc, char *argv[])
{
	if (argc < 1)
		UT_FATAL("usage: test_map_prot <file>");

	struct sigaction v;
	sigemptyset(&v.sa_mask);
	v.sa_flags = 0;
	v.sa_handler = signal_handler;
	SIGACTION(SIGSEGV, &v, NULL);

	char *file = argv[0];
	struct FHandle *fh;
	struct pmem2_config cfg;
	struct pmem2_source *src;

	/* read-only on file opened in read-only mode - should succeed */
	prepare_config(&cfg, &src, &fh, file, 0, 0, FH_READ);
	pmem2_config_set_protection(&cfg, PMEM2_PROT_READ);

	struct pmem2_map *map;
	int ret = pmem2_map(&cfg, src, &map);
	UT_ASSERTeq(ret, 0);

	pmem2_memcpy_fn memcpy_fn = pmem2_get_memcpy_fn(map);
	void *addr_map = pmem2_map_get_address(map);
	if (!ut_sigsetjmp(Jmp)) {
		/* same memcpy from above should now fail */
		memcpy_fn(addr_map, word1, strlen(word1), 0);
	} else {
		UT_OUT("memcpy successful");
	}

	unmap_map(map);
	FREE(map);
	UT_FH_CLOSE(fh);
	PMEM2_SOURCE_DELETE(&src);
	return 1;
}

/*
 * test_rw_mode_none_prot -- test R/W protection
 * pmem2_map() - should success
 * memcpy - should fail
 */
static int
test_rw_mode_none_prot(const struct test_case *tc,
		int argc, char *argv[])
{
	if (argc < 1)
		UT_FATAL("usage: test_map_prot <file>");

	/* arrange to catch SEGV */
	struct sigaction v;
	sigemptyset(&v.sa_mask);
	v.sa_flags = 0;
	v.sa_handler = signal_handler;
	SIGACTION(SIGSEGV, &v, NULL);

	char *file = argv[0];
	struct FHandle *fh;
	struct pmem2_config cfg;
	struct pmem2_source *src;

	/* read-only */
	prepare_config(&cfg, &src, &fh, file, 0, 0, FH_READ);
	pmem2_config_set_protection(&cfg, PMEM2_PROT_NONE);

	struct pmem2_map *map;
	int ret = pmem2_map(&cfg, src, &map);
	UT_ASSERTeq(ret, 0);

	pmem2_memcpy_fn memcpy_fn = pmem2_get_memcpy_fn(map);
	void *addr_map = pmem2_map_get_address(map);
	if (!ut_sigsetjmp(Jmp)) {
		/* same memcpy from above should now fail */
		memcpy_fn(addr_map, word1, strlen(word1), 0);
	} else {
		UT_OUT("memcpy successful");
	}

	unmap_map(map);
	FREE(map);
	UT_FH_CLOSE(fh);
	PMEM2_SOURCE_DELETE(&src);
	return 1;
}

/*
 * test_cases -- available test cases
 */
static struct test_case test_cases[] = {
	TEST_CASE(test_rw_mode_rw_prot),
	TEST_CASE(test_r_mode_rw_prot),
	TEST_CASE(test_rw_mode_r_prot),
	TEST_CASE(test_r_mode_r_prot),
	TEST_CASE(test_rw_mode_none_prot),
};

#define NTESTS (sizeof(test_cases) / sizeof(test_cases[0]))

int
main(int argc, char *argv[])
{
	START(argc, argv, "pmem2_map_prot");
	util_init();
	out_init("pmem2_map_prot", "TEST_LOG_LEVEL", "TEST_LOG_FILE", 0, 0);
	TEST_CASE_PROCESS(argc, argv, test_cases, NTESTS);
	out_fini();
	DONE(NULL);
}

#ifdef _MSC_VER
MSVC_CONSTR(libpmem2_init)
MSVC_DESTR(libpmem2_fini)
#endif
