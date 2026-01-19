#ifndef FUN4ALL_INPUTFILEHANDLER_H
#define FUN4ALL_INPUTFILEHANDLER_H

#include <cstdint>
#include <list>
#include <string>
#include <vector>

/**
   * Attempt to open the specified file for processing.
   * @param filename Name of the file to open.
   * @returns `0` on success, a negative error code on failure.
   */
  /**
   * Close the currently opened file.
   * @returns `-1` by default; override to return `0` on success or a negative error code on failure.
   */
  /**
   * Reset the internal file list state to its initial condition.
   * @returns `0` on success, a negative error code on failure.
   */
  /**
   * Advance to and open the next file from the internal file list.
   * @returns `0` on success, a negative error code if no file could be opened or on error.
   */
  /**
   * Add all filenames listed in the provided file to the internal file list.
   * @param filename Path to a file that contains a list of filenames to add.
   * @returns `0` on success, a negative error code on failure.
   */
  /**
   * Add a single filename to the internal file list.
   * @param filename Filename to append to the file list.
   * @returns `0` on success, a negative error code on failure.
   */
  /**
   * Record a filename as having been opened during this run.
   * @param filename Filename to append to the opened-files record.
   */
  /**
   * Print internal state or file lists.
   * @param what Determines what to print; implementation-specific values (default: "ALL").
   */
  /**
   * Get whether a file is currently open.
   * @returns `m_IsOpen` (nonzero if a file is open, `0` otherwise).
   */
  /**
   * Set the open state indicator.
   * @param i New open state value (`0` for closed, nonzero for open).
   */
  /**
   * Set the verbosity level used for logging or printing.
   * @param i Verbosity level.
   */
  /**
   * Get the current verbosity level.
   * @returns The verbosity level.
   */
  /**
   * Refresh or rebuild the internal file list from configured sources.
   */
  /**
   * Set the current filename in focus.
   * @param fn Filename to set as current.
   */
  /**
   * Get the current filename in focus.
   * @returns The current filename.
   */
  /**
   * Check whether the internal file list is empty.
   * @returns `true` if the file list contains no entries, `false` otherwise.
   */
  /**
   * Set how many times files should be repeated when iterating.
   * @param i Repeat count (`-1` indicates default/unspecified behavior).
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
 * @param script Path or command string to be executed prior to opening files.
 */
void SetOpeningScript(const std::string &script) { m_RunBeforeOpeningScript = script; }
  const std::string &GetOpeningScript() const { return m_RunBeforeOpeningScript; }
  /**
 * Set the arguments for the opening script.
 * @param args A single string containing arguments that will be supplied to the opening script; stored internally.
 */
void SetOpeningScriptArgs(const std::string &args) { m_OpeningArgs = args; }
  const std::string &GetOpeningScriptArgs() const { return m_OpeningArgs; }
  int RunBeforeOpening(const std::vector<std::string> &stringvec);

 private:
  int m_IsOpen{0};
  /**
 * Number of times to repeat processing the file list.
 */
int m_Repeat{0};
  /**
 * Verbosity level used to control amount of diagnostic or informational output.
 *
 * Higher values produce more detailed output; treat as an application-defined,
 * unitless verbosity/verbosity-mask value.
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