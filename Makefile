CC = gcc
TEST_SOURCE = test.c hashmap.c dict.c
OUTPUT1 = zHashMap_test
EXAMPLE_SOURCE = example.c hashmap.c dict.c
OUTPUT2 = zHashMap_example

hashmap_test_make:
	$(CC) $(TEST_SOURCE) -o $(OUTPUT1)
	@echo "test complie Done!\n"
	$(CC) $(EXAMPLE_SOURCE) -o $(OUTPUT2)
	@echo "example complie Done!\n"