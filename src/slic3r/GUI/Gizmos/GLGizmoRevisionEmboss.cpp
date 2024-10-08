#include "GLGizmoRevisionEmboss.hpp"
#include "slic3r/GUI/GLCanvas3D.hpp"
#include "slic3r/GUI/GUI.hpp"
#include "slic3r/GUI/Gizmos/GLGizmosManager.hpp"
#include "slic3r/GUI/GUI_App.hpp"
#include "slic3r/GUI/ImGuiWrapper.hpp"
#include "slic3r/GUI/MainFrame.hpp"
#include "slic3r/GUI/MsgDialog.hpp"
#include "slic3r/GUI/Plater.hpp"

#include "slic3r/Svn/SvnIntegration.hpp"
#include "slic3r/Svn/SvnDialog.hpp"


#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

using namespace Slic3r;
using namespace Slic3r::Emboss;
using namespace Slic3r::GUI;
using namespace Slic3r::GUI::Emboss;

GLGizmoRevisionEmboss::GLGizmoRevisionEmboss(GLCanvas3D &parent)
    : GLGizmoEmbossBase(parent, GLGizmoEmbossBase::EmbossType::REVISION, WXK_CONTROL_V) //
{}

bool GLGizmoRevisionEmboss::setup_dialog_defaults()
{
    auto opt_text = Slic3r::Svn::resolve_revison_dialog();

    if (!opt_text.has_value()) {
        return false;
    }
    
    // Set default size and text for revision emboss
    m_text                                       = opt_text.value();
    m_style_manager.get_font_prop().size_in_mm   = 5;
    m_style_manager.get_style().projection.depth = 1;

    return true;
}

std::string GLGizmoRevisionEmboss::on_get_name() const { return _u8L("Revision Emboss"); }

void GLGizmoRevisionEmboss::draw_window()
{
    // Setter of indent must be befor disable !!!
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, m_gui_cfg->indent);
    ScopeGuard indent_sc([]() { ImGui::PopStyleVar(/*ImGuiStyleVar_IndentSpacing*/); });

    // Disable all except selection of font, when open text from 3mf with unknown font
    m_imgui->disabled_begin(m_is_unknown_font);
    ScopeGuard unknown_font_sc([imgui = m_imgui]() { imgui->disabled_end(/*m_is_unknown_font*/); });

    draw_text_input();

    ImGui::Indent();
    // When unknown font is inside .3mf only font selection is allowed
    m_imgui->disabled_end(/*m_is_unknown_font*/);
    draw_font_list_line();
    m_imgui->disabled_begin(m_is_unknown_font);
    bool use_inch = wxGetApp().app_config->get_bool("use_inches");
    draw_height(use_inch);
    draw_depth(use_inch);
    ImGui::Unindent();

    // close advanced style property when unknown font is selected
    if (m_is_unknown_font && m_is_advanced_edit_style)
        ImGui::SetNextTreeNodeOpen(false);

    if (ImGui::TreeNode(_u8L("Advanced").c_str())) {
        if (!m_is_advanced_edit_style) {
            set_minimal_window_size(true);
        } else {
            draw_advanced();
        }
        ImGui::TreePop();
    } else if (m_is_advanced_edit_style)
        set_minimal_window_size(false);

    ImGui::Separator();

    draw_style_list();

    // Do not select volume type, when it is text object
    if (!m_volume->is_the_only_one_part()) {
        ImGui::Separator();
        draw_model_type();
    }
}