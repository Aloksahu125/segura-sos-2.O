SRC = ../../segura_esp32
test: test_pipeline.cpp $(SRC)/feature_extract.cpp $(SRC)/edge_model.cpp $(SRC)/payload.cpp
	g++ -std=c++17 -O1 -Wall -Wextra -Wno-unused-function -Wno-unused-parameter -I$(SRC) $^ -o test_pipeline
	./test_pipeline parity_windows.csv
clean:
	rm -f test_pipeline
