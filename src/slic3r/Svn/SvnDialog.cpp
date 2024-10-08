#pragma once

#include "SvnDialog.hpp"
#include "SvnIntegration.hpp"

#include "slic3r/GUI/GUI_App.hpp"
#include "slic3r/GUI/Plater.hpp"
#include "slic3r/GUI/MainFrame.hpp"

#include <wx/msgdlg.h>
#include <wx/textdlg.h>
#include <wx/filedlg.h>

namespace Slic3r::Svn
{

static wxWindow* get_frame()
{
    return static_cast<wxWindow*>(Slic3r::GUI::wxGetApp().mainframe);
}

void error_dialog(const std::string &message)
{
    auto frame = get_frame();

    wxMessageDialog dialog(                     //
        frame, //
        message,          //
        "Error",                                //
        wxOK | wxICON_ERROR);

    dialog.ShowModal();
}

template<typename T>
static auto value_or_error_and_default(std::optional<T> &&opt_value, const T &default_value, std::string const &error_message)
{
    auto frame = get_frame();

    if (!opt_value.has_value()) {
        error_dialog(error_message);
        return default_value;
    }

    return opt_value.value();
}

bool save_and_commit_dialog()
{
    auto frame = get_frame();

    wxMessageDialog dialog(                                                   //
        frame,                                                                //
        "Do you want to save and commit the project file before preceeding?", //
        "Save and commit before preceding",                                   //
        wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);

    return dialog.ShowModal() == wxID_YES;
}

bool start_resolve_revison_dialog()
{
    auto frame = get_frame();

    wxMessageDialog dialog(                                                                         //
        frame,                                                                                      //
        "Do you want PrusaSlicer to resolve the svn revision of the selected model and project?",   //
        "Resolve model and project revision",                                                       //
        wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION);

    return dialog.ShowModal() == wxID_YES;
}

std::optional<std::string> commit_project_file_dialog()
{
    auto frame = get_frame();

    wxTextEntryDialog dialog(                                   //
        frame,                                                  //
        "Please enter a commit message for the project file.",  //
        "Enter commit message",                                 //
        "",                                                     //
        wxOK);

    if (dialog.ShowModal() != wxID_OK)
        return {};

    return dialog.GetValue().ToStdString();
}

std::optional<std::string> select_construction_source_dialog()
{
    auto frame = get_frame();
    auto last_dir = "";// wxGetApp().app_config->get_last_dir();

    wxFileDialog dialog(                        //
        frame,                                  //
        "Please choose construction source",    //
        last_dir,                               //
        "",                                     //
        "",                                     //
        wxFD_FILE_MUST_EXIST);

    if (dialog.ShowModal() != wxID_OK)
        return {};

    return dialog.GetPath().ToStdString();
}


std::optional<std::string> resolve_revison_dialog(std::string mod, std::string sep, std::string source_prefix, std::string project_prefix)
{
    auto& gui = Slic3r::GUI::wxGetApp();

    auto project_filepath = gui.plater()->get_project_filename(".3mf").ToStdString();
    auto is_dirty = gui.plater()->is_project_dirty();
    auto save_project = [&](){ gui.plater()->save_project_if_dirty(_L("Saving the project.")); };

    auto opt_project_status = svn_get_file_status(project_filepath);
    if (!opt_project_status.has_value()) {
        error_dialog("Could not get status of project file.");
        return {};
    }

    bool project_not_committed_in_newest_state = is_dirty ||
                                                 opt_project_status.value() != SvnFileStatus::UP_TO_DATE;

    if (project_not_committed_in_newest_state && save_and_commit_dialog()) {
        save_project();
        auto opt_commit_message = commit_project_file_dialog();

        if (!opt_commit_message.has_value())
            return {};

        if (!svn_commit_file(project_filepath, opt_commit_message.value())) {
            error_dialog("Could not commit project file.");
            return {};
        }
    }

    opt_project_status = svn_get_file_status(project_filepath);
    if (!opt_project_status.has_value()) {
        error_dialog("Could not get status of project file.");
        return {};
    }

    auto opt_project_revision = svn_get_revision_string(project_filepath);
    if (opt_project_status.value() != SvnFileStatus::UNVERSIONED && !opt_project_revision.has_value()) {
        error_dialog("Could not get revision of project file.");
        return {};
    }

    auto opt_construction_source_filepath = select_construction_source_dialog();
    if (!opt_construction_source_filepath.has_value())
        return {};

    auto opt_construction_source_status = svn_get_file_status(opt_construction_source_filepath.value());
    if (!opt_construction_source_status.has_value()) {
        error_dialog("Could not get status of construction source file.");
        return {};
    }
    if (opt_construction_source_status.value() == SvnFileStatus::UNVERSIONED) {
        error_dialog("The selected construction source is not under version control.");
        return {};
    }

    auto opt_construction_source_revision = svn_get_revision_string(opt_construction_source_filepath.value());
    if (!opt_construction_source_revision.has_value()) {
        error_dialog("Could not get revision of construction source file.");
        return {};
    }

    auto format_revision = [&](auto &opt_revision, auto &opt_status, std::string id) {
        if (opt_status.value() == SvnFileStatus::MODIFIED)
            return id + opt_revision.value() + mod;
        else
            return id + opt_revision.value_or(mod);
    };

    auto construction_source_revision_formatted = format_revision(opt_construction_source_revision, opt_construction_source_status, source_prefix);
    auto project_revision_formatted             = format_revision(opt_project_revision, opt_project_status, project_prefix);

    return construction_source_revision_formatted + sep + project_revision_formatted;
}

}