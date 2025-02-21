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
  4. There should be atleast 2 files passed as input arguments 
   
# Code implementation style overview:
  The implementation used higher order functions generously(using std::function)
  The IO interface to read/write files has been abstracted inside std::function objects. The entry point of the application is the "entryPoint" method.
  The production implementation and unit tests should pass IO interfaces to this method to provide an actual or mock interfaces.

  Futher granular details can be found in code comments once you uwalk though the code.

# Solution approach
  As far as the core algorithm is concerned, it uses the "merge" step of merge sort alogorithm to generate intermediate files which are then merged with other intermediate and input files until only 1 file remains which is there sult file: "MultiplexedFile.txt".
  
  For exapmple, if there are 3 input files, the input queue will be: {"symbol1.txt", "symbol2.txt", "symbol3.txt"}. Then it will:
  1. Merge "symbol1.txt" and "symbol2.txt" to create a temporary file with a random name, let's call it "temp.txt".
  2. Then it will pop "symbol1.txt" and "symbol2.txt" and push "temp2.txt" from the input queue so that input queue becomes {"temp.txt", "symbol3.txt"}
  3. It then runs step 1-2 on input queue again and so that "temp.txt" and "symbol3.txt" are popped and their merge file, "temp2.txt" is the only file ib the input queue, and this is the time when the algorithm exits as this files is the multiplexed file, before exiting, this file is renamed as "MultiplxedFile.txt"

  ## Parallel processing
  This algorithm is run in multiple threads to speed up the process, resources like memory and open files are equally distributed among all the threads.

  The input queue mentioned above is shared by all the threads so each thread access the queue in a critical section