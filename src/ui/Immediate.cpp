//
// Created by Peter on 02/06/2022.
//
#include "Immediate.h"

#include "UIContext.h"
#include "../Constants.h"
#include "../Controls.h"
#include "../entity/OneshotEntity.h"

namespace padi {


    bool Immediate::checkFocusSwitch(padi::UIContext *ctx, size_t id) {
        if (!ctx->m_navUsed) {
            if (padi::Controls::wasKeyReleased(sf::Keyboard::Down)) {
                ctx->m_focused = 0;
                ctx->m_navUsed = true;
                return true;
            }
            if (padi::Controls::wasKeyReleased(sf::Keyboard::Up)) {
                ctx->m_focused = ctx->m_pred;
                ctx->m_navUsed = true;
                return true;
            }
        }
        return false;
    }

    bool Immediate::Button(padi::UIContext *ctx, std::string const &label, sf::FloatRect const &size) {
        static auto hash = std::hash<std::string>();

        size_t id = hash(label);

        sf::Color color = sf::Color::White;
        size_t frame = 0;
        if (ctx->isFocused(id)) {
            // Tab to cycle... for now
            checkFocusSwitch(ctx, id);

            color = sf::Color::Yellow;
            frame = padi::Controls::pollKeyState(sf::Keyboard::Space);
        } else {
            if (ctx->m_focused == 0)
                ctx->m_focused = id;
        }
        ctx->m_numVerts += drawScalable(ctx, ctx->getApollo()->lookupAnim("scalable_button"), size, frame, color);
        ctx->m_pred = id;

        return frame == 3;
    }

    bool Immediate::Switch(padi::UIContext *ctx, std::string const &label, sf::FloatRect const &size, bool *ptr) {
        static auto hash = std::hash<std::string>();

        size_t id = hash(label);

        sf::Color color = sf::Color::White;
        size_t frame = ptr ? *ptr : 0;
        bool changed = false;

        if (ctx->isFocused(id)) {
            // Tab to cycle... for now
            checkFocusSwitch(ctx, id);

            if (padi::Controls::wasKeyPressed(sf::Keyboard::Space)) {
                changed = true;
                if (ptr) {
                    *ptr = !*ptr;
                    frame = *ptr;
                    if (*ptr) {
                        // TODO register
                        auto audio = std::make_shared<AudioPlayback>(ctx->getApollo()->lookupAudio("switch_on"));
                    } else {
                        auto audio = std::make_shared<AudioPlayback>(ctx->getApollo()->lookupAudio("switch_off"));
                    }
                }
            }
            color = sf::Color::Yellow;
        } else {
            color = sf::Color::White;
            if (ctx->m_focused == 0)
                ctx->m_focused = id;
        }
        ctx->m_numVerts += draw(ctx, ctx->getApollo()->lookupAnim("switch"), size, frame, color);
        ctx->m_pred = id;

        return changed;
    }

    bool Immediate::TextInput(padi::UIContext *ctx, const std::string &label, std::string *ptr, size_t max_len) {
        static auto hash = std::hash<std::string>();

        size_t id = hash(label);

        bool changed = false;

        if (ctx->isFocused(id)) {
            // Tab to cycle... for now
            checkFocusSwitch(ctx, id);
            Controls::textInput(*ptr, max_len);
            changed = true;
        } else {
            if (ctx->m_focused == 0)
                ctx->m_focused = id;
        }
        ctx->m_pred = id;
        return changed;
    }

    size_t Immediate::draw(padi::UIContext *ctx, const std::shared_ptr<Animation> &anim, sf::FloatRect const &bound,
                           uint8_t frame, sf::Color color = sf::Color::White) {
        while (ctx->m_vbo.getVertexCount() < ctx->m_numVerts + 9 * 4) {
            ctx->m_vbo.resize((ctx->m_vbo.getVertexCount() + 1) * 2);
        }
        sf::Vertex *quad = &ctx->m_vbo[ctx->m_numVerts];
        sf::Vector2f texSize{padi::UIPadding_px * 2, padi::UIPadding_px * 2};
        sf::Vector2f texOffset{0, 0};
        if (anim) {
            texSize = sf::Vector2f(anim->getResolution());
            texOffset = (*anim)[frame];
        }
        quad[0].position.x = quad[3].position.x = bound.left;
        quad[1].position.x = quad[2].position.x = bound.left + bound.width;

        quad[0].position.y = quad[1].position.y = bound.top;
        quad[2].position.y = quad[3].position.y = bound.top + bound.height;

        auto t = ctx->topTransform();
        quad[0].position = t.transformPoint(quad[0].position);
        quad[1].position = t.transformPoint(quad[1].position);
        quad[2].position = t.transformPoint(quad[2].position);
        quad[3].position = t.transformPoint(quad[3].position);

        quad[0].texCoords = texOffset;
        quad[1].texCoords = texOffset + sf::Vector2f(texSize.x, 0);
        quad[2].texCoords = texOffset + texSize;
        quad[3].texCoords = texOffset + sf::Vector2f(0, texSize.y);

        quad[0].color = quad[1].color = quad[2].color = quad[3].color = color;

        return 4;
    }

    void Immediate::Sprite(padi::UIContext *ctx, sf::FloatRect const &size, size_t frame,
                           const std::shared_ptr<Animation> &anim, sf::Color const &color) {
        ctx->m_numVerts += draw(ctx, anim, size, frame, color);
    }

    void Immediate::ScalableSprite(padi::UIContext *ctx, const sf::FloatRect &size, size_t frame,
                                   const std::shared_ptr<Animation> &anim, const sf::Color &color) {
        ctx->m_numVerts += drawScalable(ctx, anim, size, frame, color);
    }

    size_t
    Immediate::drawScalable(padi::UIContext *ctx, const std::shared_ptr<Animation> &anim, sf::FloatRect const &bound,
                            uint8_t frame, sf::Color color = sf::Color::White) {
        while (ctx->m_vbo.getVertexCount() < ctx->m_numVerts + 9 * 4) {
            ctx->m_vbo.resize((ctx->m_vbo.getVertexCount() + 1) * 2);
        }
        sf::Vertex *quad = &(ctx->m_vbo[ctx->m_numVerts]);
        sf::Vector2f texSize{padi::UIPadding_px * 2, padi::UIPadding_px * 2};
        sf::Vector2f texOffset{0, 0};
        if (anim) {
            texSize = sf::Vector2f(anim->getResolution());
            texOffset = (*anim)[frame];
        }
        float x_anchors[4]{
                bound.left,
                bound.left + padi::UIPadding_px,
                bound.left + bound.width - padi::UIPadding_px,
                bound.left + bound.width};
        float y_anchors[4]{
                bound.top,
                bound.top + padi::UIPadding_px,
                bound.top + bound.height - padi::UIPadding_px,
                bound.top + bound.height};
        float x_tex[4]{
                texOffset.x,
                texOffset.x + padi::UIPadding_px,
                texOffset.x + texSize.x - padi::UIPadding_px,
                texOffset.x + texSize.x};
        float y_tex[4]{
                texOffset.y,
                texOffset.y + padi::UIPadding_px,
                texOffset.y + texSize.y - padi::UIPadding_px,
                texOffset.y + texSize.y};

        auto t = ctx->topTransform();

        for (int y = 0; y < 3; ++y) {
            for (int x = 0; x < 3; ++x) {
                quad[0].position.x = x_anchors[x];
                quad[0].position.y = y_anchors[y];
                quad[0].texCoords.x = x_tex[x];
                quad[0].texCoords.y = y_tex[y];

                quad[1].position.x = x_anchors[x + 1];
                quad[1].position.y = y_anchors[y];
                quad[1].texCoords.x = x_tex[x + 1];
                quad[1].texCoords.y = y_tex[y];

                quad[2].position.x = x_anchors[x + 1];
                quad[2].position.y = y_anchors[y + 1];
                quad[2].texCoords.x = x_tex[x + 1];
                quad[2].texCoords.y = y_tex[y + 1];

                quad[3].position.x = x_anchors[x];
                quad[3].position.y = y_anchors[y + 1];
                quad[3].texCoords.x = x_tex[x];
                quad[3].texCoords.y = y_tex[y + 1];
                quad[0].color = quad[1].color = quad[2].color = quad[3].color = color;
                quad[0].position = t.transformPoint(quad[0].position);
                quad[1].position = t.transformPoint(quad[1].position);
                quad[2].position = t.transformPoint(quad[2].position);
                quad[3].position = t.transformPoint(quad[3].position);

                quad += 4;
            }
        }

        return 9 * 4;
    }

    bool Immediate::isFocused(padi::UIContext *ctx, const std::string &id) {
        static auto hash = std::hash<std::string>();
        return hash(id) == ctx->m_focused;
    }

    bool Immediate::isAnyFocused(padi::UIContext *ctx, std::string *ids, size_t num) {
        static auto hash = std::hash<std::string>();
        for(size_t i = 0; i < num; ++i) {
            if (ctx->m_focused == hash(ids[i])) return true;
        }
        return false;
    }

}