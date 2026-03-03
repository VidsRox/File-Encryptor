CXX = g++
CXXFLAGS = -std=c++17 -lpthread  

TARGET = encrypt

SRCS = main.cpp FileReader.cpp FileWriter.cpp Cryptor.cpp ProcessManager.cpp envReader.cpp SharedQueue.cpp HybridManager.cpp AsyncManager.cpp

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) -luring

clean:
	rm -f $(TARGET)