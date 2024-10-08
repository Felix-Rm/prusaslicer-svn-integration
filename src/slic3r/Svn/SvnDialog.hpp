#pragma once

#include <optional>
#include <string>


namespace Slic3r::Svn
{

void error_dialog(const std::string &message);

bool save_and_commit_dialog();
bool start_resolve_revison_dialog();

std::optional<std::string> commit_project_file_dialog();
std::optional<std::string> select_construction_source_dialog();
std::optional<std::string> resolve_revison_dialog(std::string mod = "*", std::string sep = "-", std::string source_prefix = "s", std::string project_prefix = "p");

}