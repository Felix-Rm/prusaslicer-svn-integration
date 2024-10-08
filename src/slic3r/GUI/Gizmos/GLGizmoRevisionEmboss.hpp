///|/ Copyright (c) Prusa Research 2021 - 2023 Oleksandra Iushchenko @YuSanka, Lukáš Matěna @lukasmatena, Enrico Turri @enricoturri1966, Filip Sykala @Jony01
///|/ Copyright (c) Felix Reißmann 2024 @Felix-Rm
///|/
///|/ PrusaSlicer is released under the terms of the AGPLv3 or higher
///|/
#ifndef slic3r_GLGizmoRevisionEmboss_hpp_
#define slic3r_GLGizmoRevisionEmboss_hpp_

#include "GLGizmoEmbossBase.hpp"
#include "slic3r/GUI/GUI_App.hpp"

namespace Slic3r::GUI {
class GLGizmoRevisionEmboss : public GLGizmoEmbossBase
{
public:
    explicit GLGizmoRevisionEmboss(GLCanvas3D &parent);

protected:
    bool        setup_dialog_defaults() override;
    std::string on_get_name() const override;

private:
    void draw_window() override;
};

} // namespace Slic3r::GUI

#endif // slic3r_GLGizmoRevisionEmboss_hpp_
