#include "InputFileHandler.h"
#include "InputFileHandlerReturnCodes.h"

#include <phool/phool.h>

#include <TSystem.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

/**
 * @brief Add a filename to the handler's input list.
 *
 * Appends the given filename to the active file queue and to the preserved copy
 * used for resetting the file list.
 *
 * @param filename Path or identifier of the input file to add.
 * @return int `0` on success.
 */
int InputFileHandler::AddFile(const std::string &filename)
{
  if (GetVerbosity() > 0)
  {
    std::cout << "Adding " << filename << " to list of input files" << std::endl;
  }
  m_FileList.push_back(filename);
  m_FileListCopy.push_back(filename);
  return 0;
}

int InputFileHandler::AddListFile(const std::string &filename)
{
  // checking filesize to see if we have a text file
  if (std::filesystem::exists(filename.c_str()))
  {
    if (std::filesystem::is_regular_file(filename.c_str()))
    {
      //      uintmax_t fsize = std::filesystem::file_size(filename.c_str());
    }
    else
    {
      std::cout << filename << " is not a regular file" << std::endl;
      return -1;
    }
  }
  else
  {
    std::cout << PHWHERE << "Could not open " << filename << std::endl;
    return -1;
  }
  std::ifstream infile;
  infile.open(filename, std::ios_base::in);
  if (!infile)
  {
    std::cout << PHWHERE << "Could not open " << filename << std::endl;
    return -1;
  }
  std::string FullLine;
  int nfiles = 0;
  getline(infile, FullLine);
  while (!infile.eof())
  {
    if (!std::all_of(FullLine.begin(), FullLine.end(), ::isprint))
    {
      std::cout << PHWHERE << "file " << filename
                << " contains non printable characters, it is likely a binary file" << std::endl;
      return -1;
    }
    if (!FullLine.empty() && FullLine[0] != '#')  // remove comments
    {
      AddFile(FullLine);
      nfiles++;
    }
    else if (!FullLine.empty())
    {
      if (GetVerbosity() > 0)
      {
        std::cout << "Found Comment: " << FullLine << std::endl;
      }
    }
    getline(infile, FullLine);
  }
  infile.close();
  if (nfiles == 0)
  {
    std::cout << " listfile " << filename << " does not contain filenames "
              << "if this is the only list you load into this Input Manager your code will exit very soon" << std::endl;
  }
  return 0;
}

/**
 * Advance to and open the next file in the internal file list.
 *
 * If a pre-opening script is configured, it is invoked with the candidate filename
 * (and the configured file name, if present) before attempting to open the file.
 * If opening the candidate file fails, the file is removed from the list and the
 * handler proceeds to the next entry.
 *
 * @return InputFileHandlerReturnCodes::SUCCESS if a file was successfully opened and is ready for processing,
 *         InputFileHandlerReturnCodes::FAILURE if no files remain to open.
 */
int InputFileHandler::OpenNextFile()
{
  while (!m_FileList.empty())
  {
    std::list<std::string>::const_iterator iter = m_FileList.begin();
    if (GetVerbosity())
    {
      std::cout << PHWHERE << " opening next file: " << *iter << std::endl;
    }
    if (!GetOpeningScript().empty())
    {
      std::vector<std::string> stringvec;
      stringvec.push_back(*iter);
      if (!m_FileName.empty())
      {
        stringvec.push_back(m_FileName);
      }
      if (RunBeforeOpening(stringvec))
      {
        std::cout << PHWHERE << " RunBeforeOpening() failed" << std::endl;
      }
    }
    if (fileopen(*iter))
    {
      std::cout << PHWHERE << " could not open file: " << *iter << std::endl;
      m_FileList.pop_front();
    }
    else
    {
      return InputFileHandlerReturnCodes::SUCCESS;
    }
  }
  return InputFileHandlerReturnCodes::FAILURE;
}

void InputFileHandler::Print(const std::string & /* what */) const
{
  std::cout << "file list: " << std::endl;
  for (const auto &iter : m_FileList)
  {
    std::cout << iter << std::endl;
  }
}

void InputFileHandler::UpdateFileList()
{
  if (!m_FileList.empty())
  {
    if (m_Repeat)
    {
      m_FileList.push_back(*(m_FileList.begin()));
      if (m_Repeat > 0)
      {
        m_Repeat--;
      }
    }
    m_FileList.pop_front();
  }
  return;
}

int InputFileHandler::ResetFileList()
{
  if (m_FileListCopy.empty())
  {
    std::cout << "ResetFileList can only be used with filelists" << std::endl;
    return -1;
  }
  m_FileList.clear();
  m_FileList = m_FileListCopy;
  return 0;
}

/**
 * @brief Attempt to open the input file identified by fname.
 *
 * Performs whatever setup is required to make the file ready for processing by
 * the InputFileHandler (e.g., open file handle, validate format).
 *
 * @param fname Path to the input file to open.
 * @return int `0` on success, non-zero error code on failure.
 */
int InputFileHandler::fileopen(const std::string &fname)
{
  std::cout << "InputFileHandler::fileopen opening " << fname << std::endl;
  return 0;
}

/**
 * @brief Execute a configured pre-open script with provided arguments.
 *
 * If a pre-opening script is configured, validates the script exists and has owner-execute permission,
 * then executes it with the handler's configured opening arguments followed by the entries in @p stringvec.
 *
 * @param stringvec Additional arguments appended to the script command (typically filenames to be opened).
 * @return int Exit status of the script: `0` on success; `-1` if the script is not configured, not found, or not owner-executable; otherwise the script's exit code (normalized to the conventional low-order byte).
 */
int InputFileHandler::RunBeforeOpening(const std::vector<std::string> &stringvec)
{
  if (m_RunBeforeOpeningScript.empty())
  {
    return 0;
  }
  if (!std::filesystem::exists(m_RunBeforeOpeningScript))
  {
    std::cout << PHWHERE << " script " << m_RunBeforeOpeningScript << " not found"
              << std::endl;
    return -1;
  }
  if (!((std::filesystem::status(m_RunBeforeOpeningScript).permissions() & std::filesystem::perms::owner_exec) == std::filesystem::perms::owner_exec))
  {
    std::cout << PHWHERE << "RunBeforeOpeningScript script "
              << m_RunBeforeOpeningScript << " is not owner executable" << std::endl;
    return -1;
  }
  std::string fullcmd = m_RunBeforeOpeningScript + " " + m_OpeningArgs;
  for (const auto& iter : stringvec)
  {
    fullcmd += " " + iter;
  }

  if (m_Verbosity > 1)
  {
    std::cout << PHWHERE << " running " << fullcmd << std::endl;
  }
  unsigned int iret = gSystem->Exec(fullcmd.c_str());

  if (iret)
  {
    iret = iret >> 8U;
  }
  return static_cast<int> (iret);
}
