// SPDX-License-Identifier: BSD-3-Clause
/* Copyright 2023, Intel Corporation */

/*
 * obj_alloc_mt.c -- multithreaded test of the allocator
 */
#include <stdint.h>

#include "file.h"
#include "libpmemobj.h"
#include "sys_util.h"
#include "unittest.h"

#define MAX_THREADS 32
#define MAX_OPS_PER_THREAD 500
#define ALLOC_SIZE 104
#define REALLOC_SIZE (ALLOC_SIZE * 3)
#define MIX_RERUNS 10

#define CHUNKSIZE (1 << 18)
#define CHUNKS_PER_THREAD 3

static unsigned Threads;
static unsigned Ops_per_thread;
static unsigned Tx_per_thread;

struct worker_args {
	PMEMobjpool *pop;
	PMEMoid oids[MAX_OPS_PER_THREAD];
	unsigned idx;
};

static void *
alloc_worker(void *arg)
{
	struct worker_args *a = arg;

	for (unsigned i = 0; i < Ops_per_thread; ++i) {
		a->oids[i] = OID_NULL;
		int err = pmemobj_alloc(a->pop, &a->oids[i], ALLOC_SIZE,
			0, NULL, NULL);
		UT_ASSERTeq(err, 0);
		UT_ASSERT(!OID_IS_NULL(a->oids[i]));
	}

	return NULL;
}

static void *
realloc_worker(void *arg)
{
	struct worker_args *a = arg;

	for (unsigned i = 0; i < Ops_per_thread; ++i) {
		int err = pmemobj_realloc(a->pop, &a->oids[i],
			REALLOC_SIZE, 0);
		UT_ASSERTeq(err, 0);
		UT_ASSERT(!OID_IS_NULL(a->oids[i]));
	}

	return NULL;
}

static void *
free_worker(void *arg)
{
	struct worker_args *a = arg;

	for (unsigned i = 0; i < Ops_per_thread; ++i) {
		pmemobj_free(&a->oids[i]);
		UT_ASSERT(OID_IS_NULL(a->oids[i]));
	}

	return NULL;
}

static void *
mix_worker(void *arg)
{
	struct worker_args *a = arg;

	/*
	 * The mix scenario is ran twice to increase the chances of run
	 * contention.
	 */
	for (unsigned j = 0; j < MIX_RERUNS; ++j) {
		for (unsigned i = 0; i < Ops_per_thread; ++i) {
			int err = pmemobj_alloc(a->pop, &a->oids[i],
				ALLOC_SIZE, 0, NULL, NULL);
			UT_ASSERTeq(err, 0);
			UT_ASSERT(!OID_IS_NULL(a->oids[i]));
		}

		for (unsigned i = 0; i < Ops_per_thread; ++i) {
			pmemobj_free(&a->oids[i]);
			UT_ASSERT(OID_IS_NULL(a->oids[i]));
		}
	}

	return NULL;
}

static void *
alloc_free_worker(void *arg)
{
	struct worker_args *a = arg;

	for (unsigned i = 0; i < Ops_per_thread; ++i) {
		int err = pmemobj_alloc(a->pop, &a->oids[i], ALLOC_SIZE,
				0, NULL, NULL);
		UT_ASSERTeq(err, 0);
		UT_ASSERT(!OID_IS_NULL(a->oids[i]));

		pmemobj_free(&a->oids[i]);
		UT_ASSERT(OID_IS_NULL(a->oids[i]));
	}

	return NULL;
}

static void
run_worker(void *(worker_func)(void *arg), struct worker_args args[])
{
	os_thread_t t[MAX_THREADS];

	for (unsigned i = 0; i < Threads; ++i)
		THREAD_CREATE(&t[i], NULL, worker_func, &args[i]);

	for (unsigned i = 0; i < Threads; ++i)
		THREAD_JOIN(&t[i], NULL);
}

int
main(int argc, char *argv[])
{
	START(argc, argv, "obj_alloc_mt");

	if (argc != 5)
		UT_FATAL("usage: %s <threads> <ops/t> <tx/t> [file]", argv[0]);

	PMEMobjpool *pop;

	Threads = ATOU(argv[1]);
	if (Threads > MAX_THREADS)
		UT_FATAL("Threads %d > %d", Threads, MAX_THREADS);
	Ops_per_thread = ATOU(argv[2]);
	if (Ops_per_thread > MAX_OPS_PER_THREAD)
		UT_FATAL("Ops per thread %d > %d", Threads, MAX_THREADS);
	Tx_per_thread = ATOU(argv[3]);

	int exists = util_file_exists(argv[4]);
	if (exists < 0)
		UT_FATAL("!util_file_exists");

	if (!exists) {
		pop = pmemobj_create(argv[4], "TEST", (PMEMOBJ_MIN_POOL) +
			(MAX_THREADS * CHUNKSIZE * CHUNKS_PER_THREAD),
		0666);

		if (pop == NULL)
			UT_FATAL("!pmemobj_create");
	} else {
		pop = pmemobj_open(argv[4], "TEST");

		if (pop == NULL)
			UT_FATAL("!pmemobj_open");
	}

	struct worker_args args[MAX_THREADS];

	for (unsigned i = 0; i < Threads; ++i) {
		args[i].pop = pop;
		args[i].idx = i;
	}

	run_worker(alloc_worker, args);
	run_worker(free_worker, args);

	run_worker(alloc_worker, args);
	run_worker(realloc_worker, args);
	run_worker(free_worker, args);

	run_worker(mix_worker, args);

	run_worker(alloc_free_worker, args);

	pmemobj_close(pop);

	DONE(NULL);
}
