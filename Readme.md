# Buiding the solution
Make a directory in root directory, let's say i create the the directory called build.
While being in the root directory, run the command:
mkdir build; cd build; cmake ..; make

If you want to build debug binaries, run the following command:
mkdir build; cd build; cmake .. _DCMAKE_BUILD_TYPE=Debug; make

# Targets:
  Following are the build targets:
  ## Solution
    Main binary which merges the files
    location: <root_directory>/build
  ## Unit tests
    Runs various unit tests to test the code
    location: <root_directory>/build/UnitTests

# Using the executable
  Run the executable "Solution" using command line params in the following way
  <path to "Solution" executable> <max threads to run> <max open files> <maxAllowed memory to use> <space separated list of files>
  for example, f we are already in the build directory we can write:
  ./Solution 4 100 4 CSCO.txt MSFT.txt APPL.txt AMAZ.txt

  Please note that memory mentioned here doesn't include the memory occupied by internal buffers occpied by file handling apis
  like std::ifstream, std::ofstream etc.

# Assumtions:
  1. Every lines is <= 256 in length, including the new line
  2. All the filenames provided in the commandline are valid files
     if a wrong filename is provided, results are undefined