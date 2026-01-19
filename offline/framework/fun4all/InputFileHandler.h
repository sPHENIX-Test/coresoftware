#ifndef FUN4ALL_INPUTFILEHANDLER_H
#define FUN4ALL_INPUTFILEHANDLER_H

#include <cstdint>
#include <list>
#include <string>
#include <vector>

/**
   * Construct an InputFileHandler with default state.
   */
  
  /**
   * Destroy the InputFileHandler.
   */
  
  /**
   * Open the specified input file.
   * @param filename Path to the file to open.
   * @returns An integer status code (implementation-defined).
   */
  
  /**
   * Close the currently opened file.
   * @returns `-1` by default to indicate failure or unimplemented behavior; derived classes may return other status codes.
   */
  
  /**
   * Reset the internal file list to its initial state.
   * @returns An integer status code (implementation-defined).
   */
  
  /**
   * Advance to and open the next file from the internal file list.
   * @returns An integer status code indicating success or failure.
   */
  
  /**
   * Add a filename parsed from a list (e.g., batch list) to the internal file list.
   * @param filename Filename to add.
   * @returns An integer status code indicating success or failure.
   */
  
  /**
   * Add a single filename to the internal file list.
   * @param filename Filename to add.
   * @returns An integer status code indicating success or failure.
   */
  
  /**
   * Record that the given filename has been opened by appending it to the opened-files list.
   * @param filename Filename that was opened.
   */
  
  /**
   * Print status or diagnostics about the handler.
   * @param what Selector for what to print (defaults to "ALL").
   */
  
  /**
   * Query whether a file is currently open.
   * @returns `1` if open, `0` if not (or other implementation-specific integer flags).
   */
  
  /**
   * Set the open state flag for the handler.
   * @param i Integer value to set the open state to.
   */
  
  /**
   * Set the verbosity level used by the handler.
   * @param i Verbosity level.
   */
  
  /**
   * Get the current verbosity level.
   * @returns The verbosity level.
   */
  
  /**
   * Update or synchronize the internal file list (e.g., copy or refresh working list).
   */
  
  /**
   * Set the current filename tracked by the handler.
   * @param fn Filename to set as current.
   */
  
  /**
   * Get the current filename tracked by the handler.
   * @returns Reference to the current filename string.
   */
  
  /**
   * Check whether the internal candidate file list is empty.
   * @returns `true` if the file list contains no entries, `false` otherwise.
   */
  
  /**
   * Set the repeat count or flag controlling how files are reprocessed.
   * @param i Repeat count or flag (defaults to -1).
   */
  class InputFileHandler
{
 public:
  InputFileHandler() = default;
  virtual ~InputFileHandler() = default;
  virtual int fileopen(const std::string & /*filename*/);  // { return 0; }
  virtual int fileclose() { return -1; }

  virtual int ResetFileList();

  int OpenNextFile();
  int AddListFile(const std::string &filename);
  int AddFile(const std::string &filename);
  void AddToFileOpened(const std::string &filename) { m_FileListOpened.push_back(filename); }
  void Print(const std::string &what = "ALL") const;
  virtual int IsOpen() const { return m_IsOpen; }
  void IsOpen(const int i) { m_IsOpen = i; }
  void SetVerbosity(const uint64_t i) { m_Verbosity = i; }
  uint64_t GetVerbosity() const { return m_Verbosity; }
  void UpdateFileList();
  void FileName(const std::string &fn) { m_FileName = fn; }
  const std::string &FileName() const { return m_FileName; }
  bool FileListEmpty() const { return m_FileList.empty(); }
  void Repeat(const int i = -1) { m_Repeat = i; }
  std::pair<std::list<std::string>::const_iterator, std::list<std::string>::const_iterator> FileOpenListBeginEnd() { return std::make_pair(m_FileListOpened.begin(), m_FileListOpened.end()); }
  const std::list<std::string> &GetFileList() const { return m_FileListCopy; }
  const std::list<std::string> &GetFileOpenedList() const { return m_FileListOpened; }
  /**
 * Set the script to run before opening files.
 * @param script Path or command string of the script to execute prior to opening a file.
 */
void SetOpeningScript(const std::string &script) { m_RunBeforeOpeningScript = script; }
  const std::string &GetOpeningScript() const { return m_RunBeforeOpeningScript; }
  /**
 * Set the argument string to pass to the script run before opening files.
 * @param args Space-separated or otherwise-formatted argument string to supply to the pre-opening script previously set via SetOpeningScript.
 */
void SetOpeningScriptArgs(const std::string &args) { m_OpeningArgs = args; }
  const std::string &GetOpeningScriptArgs() const { return m_OpeningArgs; }
  int RunBeforeOpening(const std::vector<std::string> &stringvec);

 private:
  int m_IsOpen{0};
  /**
 * Controls how many times file processing should be repeated.
 *
 * A value of 0 means files are processed once (no repetition). Higher values
 * request that files be reopened/processed that many additional times.
 */
int m_Repeat{0};
  /**
 * Verbosity level for logging and diagnostic output.
 *
 * Higher values increase the amount of diagnostic information produced.
 * Defaults to 0.
 */
uint64_t m_Verbosity{0};
  std::string m_FileName;
  std::string m_RunBeforeOpeningScript;
  std::string m_OpeningArgs;
  std::list<std::string> m_FileList;
  std::list<std::string> m_FileListCopy;
  std::list<std::string> m_FileListOpened;  // all files which were opened during running
};

#endif