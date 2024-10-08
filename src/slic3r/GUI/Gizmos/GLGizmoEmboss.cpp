#include "GLGizmoEmboss.hpp"
#include "slic3r/GUI/GUI_App.hpp"
#include "slic3r/GUI/ImGuiWrapper.hpp"
#include "slic3r/GUI/Gizmos/GLGizmosManager.hpp"
 
using namespace Slic3r;
using namespace Slic3r::Emboss;
using namespace Slic3r::GUI;
using namespace Slic3r::GUI::Emboss;

GLGizmoEmboss::GLGizmoEmboss(GLCanvas3D &parent) : GLGizmoEmbossBase(parent, EmbossType::EMBOSS, WXK_CONTROL_T) //
{}
 
bool GLGizmoEmboss::setup_dialog_defaults()
{
    m_text = "Embossed Text";
    return true;
}
 
std::string GLGizmoEmboss::on_get_name() const { return _u8L("Emboss"); }
 
void GLGizmoEmboss::draw_window()
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

    #ifdef SHOW_WX_FONT_DESCRIPTOR
    if (is_selected_style)
        m_imgui->text_colored(ImGuiPureWrap::COL_GREY_DARK, m_style_manager.get_style().path);
#endif // SHOW_WX_FONT_DESCRIPTOR

#ifdef SHOW_CONTAIN_3MF_FIX
    if (m_volume!=nullptr &&
        m_volume->text_configuration.has_value() &&
        m_volume->text_configuration->fix_3mf_tr.has_value()) {
        ImGui::SameLine();
        m_imgui->text_colored(ImGuiPureWrap::COL_GREY_DARK, ".3mf");
        if (ImGui::IsItemHovered()) {
            Transform3d &fix = *m_volume->text_configuration->fix_3mf_tr;
            std::stringstream ss;
            ss << fix.matrix();            
            std::string filename = (m_volume->source.input_file.empty())? "unknown.3mf" :
                                   m_volume->source.input_file + ".3mf";
            ImGui::SetTooltip("Text configuation contain \n"
                              "Fix Transformation Matrix \n"
                              "%s\n"
                              "loaded from \"%s\" file.",
                              ss.str().c_str(), filename.c_str()
                );
        }
    }
#endif // SHOW_CONTAIN_3MF_FIX
#ifdef SHOW_ICONS_TEXTURE    
    auto &t = m_icons_texture;
    ImGui::Image((void *) t.get_id(), ImVec2(t.get_width(), t.get_height()));
#endif //SHOW_ICONS_TEXTURE
#ifdef SHOW_IMGUI_ATLAS
    const auto &atlas = m_style_manager.get_atlas();
    ImGui::Image(atlas.TexID, ImVec2(atlas.TexWidth, atlas.TexHeight));
#endif // SHOW_IMGUI_ATLAS

#ifdef ALLOW_OPEN_NEAR_VOLUME
    ImGui::SameLine();
    if (ImGui::Checkbox("##ALLOW_OPEN_NEAR_VOLUME", &m_allow_open_near_volume)) {
        if (m_allow_open_near_volume)
            m_set_window_offset = calc_fine_position(m_parent.get_selection(), get_minimal_window_size(), m_parent.get_canvas_size());
    } else if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", ((m_allow_open_near_volume) ? 
            "Fix settings position":
            "Allow floating window near text").c_str());
    }
#endif // ALLOW_FLOAT_WINDOW
}