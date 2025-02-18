#include <functional>
#include <string>
#include <optional>
#include <tuple>
#include <memory>
#include <queue>
#include <thread>
#include <mutex>

// Kept these std::function interfaces because they are easy to mock for
// the purpose of unit testing

/**
 * An interface to get the next line fron the file
 * If EOF is reached, should return 0.
 *
 * @param  buff   Output line buffer.
 *                If the line is successfully read, the a \n terminated
 *                string should be written to the buffer.
 *                Pass it as 0 to close the file.
 * @return        No. of bytes read in this read operation
*/
typedef std::function<uint8_t(char* buff)> FileLineReader;


/**
 * An interface to put a line into a file.
 *
 * @param  buff   Input buffer.
 * @param  len    No. of bytes to be but in the buffer.
*/
typedef std::function<void(const char* buff, const uint32_t len)> FileWriter;


/**
 * An interface to provide a FileLineReader(explained above), given an input filename.
 *
 * @param  file   Name of the file to be read
 * @return        A "FileLineReader"
*/
typedef std::function<FileLineReader(const std::string& file)> FileReaderProvider;

/**
 * An interface to provide a FileWriter(explained above), given an input filename.
 *
 * @param  file   Name of the file to be read
 * @return        A "FileWriter"
*/
typedef std::function<FileWriter(const std::string&)> FileWriterProvider;

/**
 * An interface to handle the merge completion of 2 files.
 * Passed a a callback to "FileMerger" explained below 
 *
 * @param  file1   1st of the 2 files that were merged
 * @param  file2   2nd of the 2 files that were merged
 * @param  outFile Name of the outfile containing the result of the merge
*/
typedef std::function<void(const std::string& file1, const std::string& file2, const std::string& outFile)> MergeNotificationHandler;

typedef std::tuple<std::string, std::string> MergeFilePair;

/**
 * An interface to merge 2 files and write output to a 3rd, intermediate file
 *
 * @param  mergeFileFetcher         An std::function to get the next 2 files to be merged
 *                                  If there is just 1 file, in the queue containg files to be merged,
 *                                  it will return std::nullopt which will mean that all the files have
 *                                  merged and the only file in the queue is the final Multiplexed file
 * @param  fileReaderProvider       A FileReaderProvider(explained above)
 * @param  fileWriterProvider       A FileWriterProvider(explained above)
 * @param  mergeNotificationHandler A MergeNotificationHandler(explained above)
 * @param  maxHeapSize              Max amount of hep it can allocate to hold intermediate results
 *                                  If the required memory exceeds this value, then the intermediate result is
 *                                  written to the outFile
 * @return                          Returns true if there were atleast 2 files in the queue and hence they were merged
 *                                  Returns false otherwise 
*/

typedef std::function<bool(const std::function<std::optional<MergeFilePair>()> mergeFileFetcher, 
                           const FileReaderProvider fileReaderProvider,
                           const FileWriterProvider fileWriterProvider,
                           const MergeNotificationHandler mergeNotificationHandler,
                           const uint64_t maxHeapSize)> FileMerger;

/**
 * This is the like the main function. Declared here to make the code unit testable
 * One can easily pass mock IO interfaces to this method for the purpose of Unit Testing
 *
 * @param  numThreads               Max no. of threads used to merge the files
 * @param  maxHeapSize              Max memory in GB to allocated to hold intermediate results
 * @param  filesToMerge             A input queue containing the files to be merged
 * @param  fileMerger               A FileMerger(explained above)
 * @param  fileReaderProvider       A FileReaderProvider(explained above)
 * @param  fileWriterProvider       A FileWriterProvider(explained above)
*/
void entryPoint(uint8_t numThreads,
                uint64_t maxHeapSize,
                std::shared_ptr<std::queue<std::string>> filesToMerge,
                std::shared_ptr<std::mutex> mutex,
                FileMerger fileMerger,
                FileReaderProvider fileReaderProvider,
                FileWriterProvider fileWriterProvider);

// Used in production implementation, defined in Solution.cpp details mentioned above implementation 
FileLineReader getFileLineReader(const std::string& filename);

// Used in production implementation, defined in Solution.cpp details mentioned above implementation 
FileWriter getFileWriter(const std::string& filename);

// Used in production implementation, defined in Solution.cpp details mentioned above implementation 
bool mergeFiles(const std::function<std::optional<MergeFilePair>()> filenameFetcher, 
                const FileReaderProvider fileReaderProvider,
                const FileWriterProvider fileWriterProvider,
                const MergeNotificationHandler outFileNotifier,
                const uint64_t maxHeapSize);