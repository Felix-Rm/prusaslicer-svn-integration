#include "SvnIntegration.hpp"

#include <cstdio>
#include <vector>


namespace Slic3r::Svn {

SubprocessResult subprocess_call(std::string const &command)
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#define RevisionEmbossPopen _popen
#define RevisionEmbossPclose _pclose
#else
#define RevisionEmbossPopen popen
#define RevisionEmbossPclose(pipe) WEXITSTATUS(pclose(pipe))
#endif

    static constexpr size_t allocation_size = 128;

    FILE *pipe = RevisionEmbossPopen(command.c_str(), "r");

    if (!pipe)
        return {{}, ~0};

    std::vector<char> result(allocation_size, 0);
    size_t            size = 0;

    while (true) {
        // NOTE: Leaving one byte null termination at the end
        auto read = fread(result.data(), sizeof(char), result.size() - size - 1, pipe);
        if (read == 0)
            break;
        size += read;
        if (size == result.size() - 1)
            result.resize(size + allocation_size, 0);
    }

    return {result.data(), RevisionEmbossPclose(pipe)};

#undef RevisionEmbossPopen
#undef RevisionEmbossPclose
}

// NOTE: Will return UNVERSIONED if file does not exist
std::optional<SvnFileStatus> svn_get_file_status(const std::string &filepath)
{
    // NOTE: redirect stderr into pipe so we get errors and warnings
    std::string command  = "svn status \"" + filepath + "\" 2>&1";
    auto [output, rcode] = subprocess_call(command);

    if (rcode != 0)
        return {};

    if (output.empty())
        return SvnFileStatus::UP_TO_DATE;
    if (output.rfind("svn:", 0) != std::string::npos) // NOTE: Error / Warning prefix
        return SvnFileStatus::UNVERSIONED;
    if (output[0] == '?')
        return SvnFileStatus::UNVERSIONED;
    if (output[0] == 'A')
        return SvnFileStatus::VERSIONED;
    if (output[0] == 'M')
        return SvnFileStatus::MODIFIED;

    return {};
}

std::optional<std::string> svn_get_revision_string(const std::string &filepath)
{
    std::string command    = "svn info --show-item=revision \"" + filepath + "\"";
    auto [revision, rcode] = subprocess_call(command);

    if (rcode != 0)
        return {};

    auto numeric_end = revision.find_first_not_of("0123456789");
    return revision.substr(0, numeric_end);
}

// FIXME: This function is not transactional
bool svn_commit_file(const std::string &filepath, const std::string &message)
{
    // NOTE: This will fail in most cases, but it is probably faster than checking first
    std::string add_command = "svn add \"" + filepath + "\"";
    subprocess_call(add_command);

    std::string commit_command = "svn commit -m \"" + message + "\" \"" + filepath + "\"";
    auto [_, commit_rcode]     = subprocess_call(commit_command);
    return commit_rcode == 0;
}

} // namespace Slic3r::Svn