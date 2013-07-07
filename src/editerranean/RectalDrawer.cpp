#include "RectalDrawer.h"

namespace sharopie {
  
  RectalDrawer::RectalDrawer(Material *mat)
  : vertexBuffer_(Buffer::BindingArray), indexBuffer_(Buffer::BindingIndex)
  , locOffset_(mat->getUniformLocation("uv2_offset"))
  , locScale_(mat->getUniformLocation("uv2_scale"))
  , locAtlas_(mat->getUniformLocation("us2_atlas")) {
    vertexBuffer_.retain();
    indexBuffer_.retain();
    batch_.setMaterial(mat);
    batch_.setAttribSource("av2_vtx", &vertexBuffer_,
                           2, offsetof(Vertex, vtx), sizeof(Vertex));
    batch_.setAttribSource("av2_tex", &vertexBuffer_,
                           2, offsetof(Vertex, tex), sizeof(Vertex));
  }
  
  RectalDrawer::~RectalDrawer() {
  }
  
  void RectalDrawer::reset(Context *ctx, const fontain::String *string) {
    vertices_.clear();
    indices_.clear();
    vertices_.reserve(string->length() * 4);
    indices_.reserve(string->length() * 6);
    
    const vec2f kTexScale(vec2f(string->atlas().meta().size).recip());
    for (u32 i = 0; i < string->length(); ++i) {
      const fontain::String::Glyph &g = string->glyphs()[i];
      const vec2f size(g.rectInAtlas.size());
      makeRect(rect2f(vec2f(g.offset), vec2f(g.offset) + size),
               rect2f(g.rectInAtlas)*kTexScale);
    }
    
    batch_.setGeometry(Batch::GeometryTriangleList, 0, indices_.size(),
                       Batch::IndexU16, &indexBuffer_);
    atlas_.reset(const_cast<Atlas*>(&string->atlas()));
  }

  void RectalDrawer::makeRect(rect2f vtx, rect2f tex) {
    u16 index =
    vertices_.push_back(Vertex(vtx.topLeft(), tex.topLeft()));
    vertices_.push_back(Vertex(vtx.bottomLeft(), tex.bottomLeft()));
    vertices_.push_back(Vertex(vtx.topRight(), tex.topRight()));
    vertices_.push_back(Vertex(vtx.bottomRight(), tex.bottomRight()));
    indices_.push_back(index);
    indices_.push_back(index+1);
    indices_.push_back(index+2);
    indices_.push_back(index+2);
    indices_.push_back(index+1);
    indices_.push_back(index+3);
  }

  void RectalDrawer::draw(Context *ctx, vec2f offset, vec2f scale) {
    if (vertices_.size() > 0) {
      vertexBuffer_.load(ctx, vertices_.get(0), vertices_.totalSizeInBytes());
      indexBuffer_.load(ctx, indices_.get(0), indices_.totalSizeInBytes());
      vertices_.clear();
      indices_.clear();
    }
    batch_.getMaterial()->getUniforms().setUniform(locOffset_, offset);
    batch_.getMaterial()->getUniforms().setUniform(locScale_, scale);
    batch_.getMaterial()->getUniforms().setUniform(locAtlas_,
                                                   const_cast<Sampler*>(atlas_->getSampler(ctx)));
    batch_.draw(ctx);
  }

} // namespace sharopie
