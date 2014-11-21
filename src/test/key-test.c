#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "test_util.h"
#include "../page_key.h"
#include "../rlite.h"

static int expect_key(rlite *db, unsigned char *key, long keylen, char type, long page)
{
	unsigned char type2;
	long page2;
	int retval = rl_key_get(db, key, keylen, &type2, NULL, &page2);
	if (retval != RL_FOUND) {
		fprintf(stderr, "Unable to get key %d\n", retval);
		return 1;
	}

	if (type != type2) {
		fprintf(stderr, "Expected type %c, got %c instead\n", type, type2);
		return 1;
	}

	if (page != page2) {
		fprintf(stderr, "Expected page %ld, got %ld instead\n", page, page2);
		return 1;
	}
	return 0;
}

int basic_test_set_get(int _commit)
{
	int retval = 0;
	fprintf(stderr, "Start basic_test_set_get %d\n", _commit);

	rlite *db;
	RL_CALL_VERBOSE(setup_db, RL_OK, &db, _commit, 1);
	unsigned char *key = (unsigned char *)"my key";
	long keylen = strlen((char *)key);
	unsigned char type = 'A';
	long page = 23;
	retval = rl_key_set(db, key, keylen, type, page);
	if (retval != RL_OK) {
		fprintf(stderr, "Unable to set key %d\n", retval);
		goto cleanup;
	}

	if (_commit) {
		rl_commit(db);
	}

	retval = expect_key(db, key, keylen, type, page);
	if (retval != 0) {
		goto cleanup;
	}

	fprintf(stderr, "End basic_test_set_get\n");
	rl_close(db);
	retval = 0;
cleanup:
	return retval;
}

int basic_test_get_unexisting()
{
	int retval = 0;
	fprintf(stderr, "Start basic_test_get_unexisting\n");

	rlite *db;
	RL_CALL_VERBOSE(setup_db, RL_OK, &db, 0, 1);

	unsigned char *key = (unsigned char *)"my key";
	long keylen = strlen((char *)key);
	retval = rl_key_get(db, key, keylen, NULL, NULL, NULL);
	if (retval != RL_NOT_FOUND) {
		fprintf(stderr, "Expected not to find key %d\n", retval);
		goto cleanup;
	}

	fprintf(stderr, "End basic_test_get_unexisting\n");
	rl_close(db);
	retval = 0;
cleanup:
	return retval;
}

int basic_test_set_delete()
{
	int retval = 0;
	fprintf(stderr, "Start basic_test_set_delete\n");

	rlite *db;
	RL_CALL_VERBOSE(setup_db, RL_OK, &db, 0, 1);


	unsigned char *key = (unsigned char *)"my key";
	long keylen = strlen((char *)key);
	unsigned char type = 'A';
	long page = 23;
	retval = rl_key_set(db, key, keylen, type, page);
	if (retval != RL_OK) {
		fprintf(stderr, "Unable to set key %d\n", retval);
		goto cleanup;
	}

	retval = rl_key_delete(db, key, keylen);
	if (retval != RL_OK) {
		fprintf(stderr, "Unable to delete key %d\n", retval);
		goto cleanup;
	}

	retval = rl_key_get(db, key, keylen, NULL, NULL, NULL);
	if (retval != RL_NOT_FOUND) {
		fprintf(stderr, "Expected not to find key, got %d instead\n", retval);
		goto cleanup;
	}

	fprintf(stderr, "End basic_test_set_delete\n");
	rl_close(db);
	retval = 0;
cleanup:
	return retval;
}

int basic_test_get_or_create(int _commit)
{
	int retval = 0;
	fprintf(stderr, "Start basic_test_get_or_create %d\n", _commit);

	rlite *db;
	RL_CALL_VERBOSE(setup_db, RL_OK, &db, _commit, 1);
	unsigned char *key = (unsigned char *)"my key";
	long keylen = strlen((char *)key);
	unsigned char type = 'A';
	long page = 100, page2 = 200; // set dummy values for != assert
	retval = rl_key_get_or_create(db, key, keylen, type, &page);
	if (retval != RL_NOT_FOUND) {
		fprintf(stderr, "Unable to set key %d\n", retval);
		goto cleanup;
	}

	if (_commit) {
		rl_commit(db);
	}

	retval = rl_key_get_or_create(db, key, keylen, type, &page2);
	if (retval != RL_FOUND) {
		fprintf(stderr, "Unable to find existing key %d\n", retval);
		goto cleanup;
	}
	if (page != page2) {
		fprintf(stderr, "Expected page2 %ld to match page %ld\n", page2, page);
		goto cleanup;
	}

	if (_commit) {
		rl_commit(db);
	}

	retval = rl_key_get_or_create(db, key, keylen, type + 1, &page2);
	if (retval != RL_WRONG_TYPE) {
		fprintf(stderr, "Expected get_or_create to return wrong type, got %d instead\n", retval);
		goto cleanup;
	}

	fprintf(stderr, "End basic_test_get_or_create\n");
	rl_close(db);
	retval = 0;
cleanup:
	return retval;
}

RL_TEST_MAIN_START(key_test)
{
	RL_TEST(basic_test_set_get, 0);
	RL_TEST(basic_test_set_get, 1);
	RL_TEST(basic_test_get_unexisting, 0);
	RL_TEST(basic_test_set_delete, 0);
	RL_TEST(basic_test_get_or_create, 0);
	RL_TEST(basic_test_get_or_create, 1);
}
RL_TEST_MAIN_END