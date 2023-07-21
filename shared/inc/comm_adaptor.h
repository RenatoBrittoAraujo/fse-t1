#ifndef COMM_ADAPTER_TEST
#define COMM_ADAPTER_TEST 1

#ifndef TEST_MODE
#define TEST_MODE 1
#endif

int listen_for_main_call(char **buff);

int call_deps(int proc_id, char *buff);

#endif
