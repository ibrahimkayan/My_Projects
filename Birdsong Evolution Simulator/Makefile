EXEC=evochirp
SRC=src/evochirp.s
TEST_DIR=test-cases
OUTPUT_DIR=my-outputs

default: $(EXEC)

$(EXEC): $(SRC)
	@echo "Assembling $< into $@..."
	gcc -nostartfiles -no-pie -o $(EXEC) $(SRC)

testcases: $(EXEC)
	@echo "Running test cases..."
	@mkdir -p $(OUTPUT_DIR)
	@for infile in $(TEST_DIR)/input_*.txt; do \
		base=$$(basename $$infile); \
		outfile=$$(echo $$base | sed 's/input_/output_/'); \
		echo "  > $$infile -> $(OUTPUT_DIR)/$$outfile"; \
		./$(EXEC) < $$infile > $(OUTPUT_DIR)/$$outfile; \
	done
grade: testcases
	python3 test/grader.py ./$(EXEC) $(TEST_DIR)
