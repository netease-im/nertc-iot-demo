#ifndef _RIVE_SKIA_RENDERER_HPP_
#define _RIVE_SKIA_RENDERER_HPP_

#include "base/vgcanvas.h"
#include "base/vg_renderer.h"
#include "rive/renderer.hpp"
#include <vector>

namespace rive {
class VGCanvasRenderPath : public RenderPath {
 private:
  vg_path_t* m_Path;

 public:
  VGCanvasRenderPath() {
    m_Path = NULL;
  }
  ~VGCanvasRenderPath() {
    this->reset();
    m_Path = NULL;
  }

  const vg_path_t* path() const {
    return m_Path;
  }
  void reset() override;
  void addRenderPath(RenderPath* path, const Mat2D& transform) override;
  void fillRule(FillRule value) override;
  void moveTo(float x, float y) override;
  void lineTo(float x, float y) override;
  void cubicTo(float ox, float oy, float ix, float iy, float x, float y) override;
  virtual void close() override;
};

class VGCanvasRenderPaint : public RenderPaint {
 private:
  vg_paint_t* m_Paint;

 public:
  const vg_paint_t* paint() const {
    return m_Paint;
  }
  VGCanvasRenderPaint();
  ~VGCanvasRenderPaint();
  void style(RenderPaintStyle style) override;
  void color(unsigned int value) override;
  void thickness(float value) override;
  void join(StrokeJoin value) override;
  void cap(StrokeCap value) override;
  void blendMode(BlendMode value) override;

  void linearGradient(float sx, float sy, float ex, float ey) override;
  void radialGradient(float sx, float sy, float ex, float ey) override;
  void addStop(unsigned int color, float stop) override;
  void completeGradient() override;
};

class VGCanvasRenderer : public Renderer {
 private:
  vg_renderer_t* m_Renderer;

 public:
  VGCanvasRenderer(vg_renderer_t* renderer) : m_Renderer(renderer) {
  }
  void save() override;
  void restore() override;
  void transform(const Mat2D& transform) override;
  void drawPath(RenderPath* path, RenderPaint* paint) override;
  void clipPath(RenderPath* path) override;
};
}  // namespace rive
#endif
