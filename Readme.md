# Buiding the solution
Make a directory in root directory, let's say i create the the directory called build.
While being in the root directory, run the command:<br>
mkdir build; cd build; cmake ..; make

If you want to build debug binaries, run the following command:<br>
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
  Run the executable "Solution" using command line params in the following way:<br>
  \<path to executable\> \<max threads to run\> \<max open files\> \<max allowed memory in GB> \<space separated list of files\>
  for example, if we are already in the build directory we can write:<br>
  ./Solution 4 100 4 CSCO.txt MSFT.txt APPL.txt AMAZ.txt

  Please note that memory mentioned here doesn't include the memory occupied by internal buffers occpied by file handling apis
  like std::ifstream, std::ofstream etc.

# Assumtions/Scope/limitations:
  1. Every lines is <= 256 in length, including the new line
  2. All the filenames provided in the commandline are valid files
     if a wrong filename is provided, results are undefined
  3. The format of the input files will not be validated, it is expected the format will be correct. Wrong behavior will result in undefined behavior

# Code implementation style overview:
  The implementation used higher order functions generously(using std::function)
  The IO interface to read/write files has been abstracted inside std::function objects. The entry point of the application is the "entryPoint" method.
  The production implementation and unit tests should pass IO interfaces to this method to provide an actual or mock interfaces.

  Futher granular details can be found in code comments once yo uwalk though the code.