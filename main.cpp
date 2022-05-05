#include <iostream>
#include <SFML/Graphics.hpp>
#include "src/entity/LivingEntity.h"
#include "src/animation/Apollo.h"
#include "src/AStar.h"
#include "lib/PerlinNoise/PerlinNoise.hpp"
#include "src/level/LevelGenerator.h"
#include "src/level/SpawnEvent.h"
#include "src/Controls.h"
#include "src/level/Cursor.h"
#include "src/ui/Button.h"
#include "src/player/Ability.h"

int main() {
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "PAdI");

    auto levelGen = padi::LevelGenerator();
    auto level = levelGen
            .withSpritesheet("../media/complete_sheet.png")
            .withApollo("../media/strips.apollo")
            .withSeed(3)
            .withArea({100, 100})
            .generate();
    auto apollo = level.getApollo();
    sf::Font f{};
    f.setSmooth(false);
    f.loadFromFile("../media/prstartk.ttf");
    sf::Text t("Hello, world.",f,7);

    {
        int offset = 2;
        for (auto anim: {"air_strike", "air_strike_large", "fire", "q_mark", "debug", "cursor", "button"}) {
            auto slave = std::make_shared<padi::SlaveEntity>(sf::Vector2i{++offset, 0});
            slave->m_animation = apollo->lookupAnim(anim);
            level.getMap()->addEntity(slave);
        }
    }

    std::shared_ptr<padi::LivingEntity> livingEntity;
    livingEntity = std::make_shared<padi::LivingEntity>(apollo->lookupContext("cube"), sf::Vector2i{0, 0});
    livingEntity->setColor({255, 255, 255});
    auto leSpawn = std::make_shared<padi::SpawnEvent>(livingEntity, apollo->lookupAnim("bubble"));
    leSpawn->dispatch(&level);

    auto button = std::make_shared<padi::Button>(sf::Vector2i{4,4}, apollo->lookupAnim("button"));
    level.getMap()->addUIObject(button);

    padi::Cursor cursor(apollo->lookupAnim("cursor"));

    std::vector<sf::Vector2i> path;

    sf::Clock clock;
    size_t frames = 0;
    sf::Clock frame_clock;
    float longest = 0;
    while (window.isOpen())
    {
        sf::Event event{};
        padi::Controls::resetKeyStates(); // Handles key events
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            else if(event.type == sf::Event::KeyPressed) {
                padi::Controls::keyDown(event.key.code);
            }
            else if(event.type == sf::Event::KeyReleased) {
                padi::Controls::keyReleased(event.key.code);
            }
        }

        button->update();
        button->active = cursor.getPosition() == button->getPosition();

        level.update(&window);
        cursor.update(&level);

        if(padi::Controls::isKeyPressed(sf::Keyboard::S)) {
            padi::content::AirStrike().cast(&level, cursor.getPosition());
        }

        level.centerView(livingEntity->getPosition());

        t.setPosition(level.getMap()->mapTilePosToWorld(cursor.getPosition()) - sf::Vector2f(t.getLocalBounds().getSize().x / 2, -12));
        t.setString(std::to_string(cursor.getPosition().x) + " " + std::to_string(cursor.getPosition().y));

        if(padi::Controls::isKeyDown(sf::Keyboard::Q)) {
            path.clear();
            livingEntity->intentStay();
        } else if(padi::Controls::isKeyDown(sf::Keyboard::W) && path.empty()) {
            path = padi::FindPath(level.getMap(),livingEntity->getPosition(), cursor.getPosition());
        }
        if(!path.empty()) {
            if(livingEntity->intentMove(path.front())) {
                path.erase(path.begin());
            }
        }

        level.populateVBO();

        window.clear();
        window.draw(level);
        window.draw(t);
        window.display();
        float t = frame_clock.restart().asSeconds();
        if(t > 0.013) {
            std::cout << "Drop detected: " << 1.f/t << " FPS (" << t << ")" << std::endl;
        }
        if(frames > 20 && t > longest) {
            longest = t;
        }
        ++frames;
    }
    printf("Slowest frame took %.3f s, i.e. %.3f FPS\n", longest, 1.f/longest);
    printf("%i quads final\n", level.getMap()->numQuads());
    printf("%i frames total in %.3f seconds\n", frames, clock.getElapsedTime().asSeconds());
    printf("%.3f fps avg", float(frames) / clock.getElapsedTime().asSeconds());
    return 0;
}
