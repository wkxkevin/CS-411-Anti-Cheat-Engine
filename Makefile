CXX = clang++
CXX_FLAGS = -std=c++20 -g -O0 -Wall -Wextra -Werror

EXEC = bin/exec
TESTS = bin/tests
SRC = edit_distance.cpp

.DEFAULT_GOAL := exec
.PHONY: clean exec tests

exec: $(EXEC)

tests: $(TESTS)

$(EXEC): $(SRC)
	mkdir -p bin
	$(CXX) $(CXX_FLAGS) $^ -o $@

$(TESTS): $(SRC)
	mkdir -p bin
	$(CXX) $(CXX_FLAGS) $^ -o $@

clean:
	rm -rf bin/*