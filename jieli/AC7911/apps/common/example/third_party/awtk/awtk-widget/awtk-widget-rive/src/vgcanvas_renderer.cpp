#include "vgcanvas_renderer.hpp"
#include "rive/math/vec2d.hpp"
#include "rive/shapes/paint/color.hpp"

using namespace rive;

void VGCanvasRenderPath::fillRule(FillRule value)
{
}

void VGCanvasRenderPath::reset()
{
    vg_path_destroy(m_Path);
    m_Path = NULL;
}

void VGCanvasRenderPath::addRenderPath(RenderPath *path, const Mat2D &t)
{
    const vg_path_t *src = dynamic_cast<VGCanvasRenderPath *>(path)->path();
    if (src != NULL) {
        matrix_t m;
        vg_path_t *iter = vg_path_dup(src, TRUE);

        matrix_set(&m, t[0], t[1], t[2], t[3], t[4], t[5]);
        vg_path_transform(iter, &m);
        m_Path = vg_path_append(m_Path, iter);
    }
}

void VGCanvasRenderPath::moveTo(float x, float y)
{
    m_Path = vg_path_add_move_to(m_Path, x, y);
}

void VGCanvasRenderPath::lineTo(float x, float y)
{
    m_Path = vg_path_add_line_to(m_Path, x, y);
}

void VGCanvasRenderPath::cubicTo(float ox, float oy, float ix, float iy, float x, float y)
{
    m_Path = vg_path_add_cubic_to(m_Path, ox, oy, ix, iy, x, y);
}

void VGCanvasRenderPath::close()
{
    m_Path = vg_path_add_close(m_Path);
}

VGCanvasRenderPaint::VGCanvasRenderPaint()
{
    m_Paint = vg_paint_create();
}
VGCanvasRenderPaint::~VGCanvasRenderPaint()
{
    vg_paint_destroy(m_Paint);
    m_Paint = NULL;
}

void VGCanvasRenderPaint::style(RenderPaintStyle style)
{
    switch (style) {
    case RenderPaintStyle::fill:
        vg_paint_set_type(m_Paint, VG_PAINT_FILL);
        break;
    case RenderPaintStyle::stroke:
        vg_paint_set_type(m_Paint, VG_PAINT_STROKE);
        break;
    }
}

void VGCanvasRenderPaint::color(unsigned int value)
{
    color_t c = color_init(colorRed(value), colorGreen(value), colorBlue(value), colorAlpha(value));
    vg_paint_set_color(m_Paint, c);
}

void VGCanvasRenderPaint::thickness(float value)
{
    vg_paint_set_line_width(m_Paint, value);
}

void VGCanvasRenderPaint::join(StrokeJoin value)
{
    switch (value) {
    case StrokeJoin::miter: {
        vg_paint_set_join_type(m_Paint, VG_JOIN_MITTER);
        break;
    }
    case StrokeJoin::bevel: {
        vg_paint_set_join_type(m_Paint, VG_JOIN_BEVEL);
        break;
    }
    case StrokeJoin::round: {
        vg_paint_set_join_type(m_Paint, VG_JOIN_ROUND);
        break;
    }
    default:
        break;
    }
}
void VGCanvasRenderPaint::cap(StrokeCap value)
{
    switch (value) {
    case StrokeCap::butt: {
        vg_paint_set_cap_type(m_Paint, VG_CAP_BUTT);
        break;
    }
    case StrokeCap::square: {
        vg_paint_set_cap_type(m_Paint, VG_CAP_SQUARE);
        break;
    }
    case StrokeCap::round: {
        vg_paint_set_cap_type(m_Paint, VG_CAP_ROUND);
        break;
    }
    default:
        break;
    }
}

void VGCanvasRenderPaint::linearGradient(float sx, float sy, float ex, float ey)
{
    vg_paint_set_linear_gradient(m_Paint, sx, sy, ex, ey);
}

void VGCanvasRenderPaint::radialGradient(float sx, float sy, float ex, float ey)
{
    vg_paint_set_radial_gradient(m_Paint, sx, sy, ex, ey);
}

void VGCanvasRenderPaint::addStop(unsigned int color, float stop)
{
    color_t c;
    c.color = color;

    vg_paint_add_gradient_stop(m_Paint, c, stop);
}

void VGCanvasRenderPaint::completeGradient()
{
}

void VGCanvasRenderPaint::blendMode(BlendMode value)
{
    if (value != rive::BlendMode::srcOver) {
        log_debug("not supported blendMode: %d\n", value);
    }
}

void VGCanvasRenderer::save()
{
    vg_renderer_save(m_Renderer);
}

void VGCanvasRenderer::restore()
{
    vg_renderer_restore(m_Renderer);
}

void VGCanvasRenderer::transform(const Mat2D &t)
{
    if (t[0] == 0 && t[1] == 0 && t[2] == 0 && t[3] == 0) {
        /*XXX:invertible matrix will make cairo into error status*/
        vg_renderer_transform(m_Renderer, 1, 0, 0, 1, t[4], t[5]);
    } else {
        vg_renderer_transform(m_Renderer, t[0], t[1], t[2], t[3], t[4], t[5]);
    }
}

void VGCanvasRenderer::drawPath(RenderPath *path, RenderPaint *paint)
{
    vg_renderer_draw_path(m_Renderer, dynamic_cast<VGCanvasRenderPath *>(path)->path(),
                          dynamic_cast<VGCanvasRenderPaint *>(paint)->paint());
}

void VGCanvasRenderer::clipPath(RenderPath *path)
{
    vg_renderer_clip_path(m_Renderer, dynamic_cast<VGCanvasRenderPath *>(path)->path());
}

namespace rive
{
RenderPath *makeRenderPath()
{
    return new VGCanvasRenderPath();
}

RenderPaint *makeRenderPaint()
{
    return new VGCanvasRenderPaint();
}
}  // namespace rive
