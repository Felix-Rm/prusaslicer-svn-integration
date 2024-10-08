#pragma once

#include <optional>
#include <string>

namespace Slic3r::Svn
{

struct SubprocessResult
{
    std::string output;
    int         rcode;
};

SubprocessResult subprocess_call(std::string const &command);

enum class SvnFileStatus { UNVERSIONED, UP_TO_DATE, VERSIONED, MODIFIED };

std::optional<SvnFileStatus> svn_get_file_status(const std::string &filepath);
std::optional<std::string> svn_get_revision_string(const std::string &filepath);
bool svn_commit_file(const std::string &filepath, const std::string &message);

} // namespace Slic3r::Svn