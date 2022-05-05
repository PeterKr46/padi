//
// Created by Peter on 02/05/2022.
//

#ifndef PADI_BUTTON_H
#define PADI_BUTTON_H

#include "../entity/LivingEntity.h"

namespace padi {
    class Button;

    class ButtonListener {
    public:
        virtual bool onRelease(std::shared_ptr<padi::Button> button) = 0;
    };

    class Button : public UIObject, public std::enable_shared_from_this<Button> {
    public:
        Button(const sf::Vector2i &pos, std::shared_ptr<padi::Animation> sprite);

        size_t
        populate(const padi::Map *map, sf::VertexArray &array, size_t vertexOffset, uint8_t frame) const override;

        void update();

    public:
        sf::Vector2f offset{0, -8};
        bool active{true};
        struct {
            sf::Color inactive{186, 186, 186};
            sf::Color active{255, 255, 255};
            sf::Color pressed{255, 255, 86};
        } color;
    private:
        std::shared_ptr<padi::Animation> m_sprites;
        uint8_t m_state{0}; // 0 = none, 1 = down, 2 = pressed, 3 = up
        std::vector<std::shared_ptr<ButtonListener>> m_listeners;
    };

} // padi

#endif //PADI_BUTTON_H
