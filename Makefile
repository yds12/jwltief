# Makefile

# Build the test application
test: game.cpp enemy.cpp stage.cpp
	g++ -fpermissive -o jwltief game.cpp enemy.cpp stage.cpp -lagl

# Cleans the directories, removing executables and objects
clean:
	rm *.o jwltief
