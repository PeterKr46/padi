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

int main() {
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "PAdI");

    auto levelGen = padi::LevelGenerator();
    auto level = levelGen
            .loadSpriteMaster("../media/complete_sheet.png")
            .withSeed(1)
            .withArea({24, 24})
            .generate();
    sf::Font f{};
    f.setSmooth(false);
    f.loadFromFile("../media/prstartk.ttf");
    sf::Text t("Hello, world.",f,7);

    padi::Apollo apollo;
    /*if(apollo.initializeContext("cube")) {
        apollo.addAnimation("cube", "move_x_from", std::make_shared<padi::SimpleAnimation>(padi::StripAnimation({32, 32}, {0, 0}, {0, 48}, 12)));
        apollo.addAnimation("cube", "move_x_to", std::make_shared<padi::SimpleAnimation>(padi::StripAnimation({32, 32}, {16, 8}, {0, 48}, 12)));

        apollo.addAnimation("cube", "move_y_from", std::make_shared<padi::SimpleAnimation>(padi::StripAnimation({32, 32}, {64, 0}, {0, 48}, 12)));
        apollo.addAnimation("cube", "move_y_to", std::make_shared<padi::SimpleAnimation>(padi::StripAnimation({32, 32}, {48, 8}, {0, 48}, 12)));

        apollo.addAnimation("cube", "idle", std::make_shared<padi::SimpleAnimation>(padi::StripAnimation({32, 32}, {0, 560}, {0, 32}, 12)));
    }
    if(apollo.initializeContext("tetrahedron")) {
        apollo.addAnimation("tetrahedron", "move_x_from", std::make_shared<padi::SimpleAnimation>(padi::StripAnimation({32, 32}, {192, 0}, {0, 48}, 12)));
        apollo.addAnimation("tetrahedron", "move_x_to", std::make_shared<padi::SimpleAnimation>(padi::StripAnimation({32, 32}, {208, 8}, {0, 48}, 12)));

        apollo.addAnimation("tetrahedron", "move_y_from", std::make_shared<padi::SimpleAnimation>(padi::StripAnimation({32, 32}, {256, 0}, {0, 48}, 12)));
        apollo.addAnimation("tetrahedron", "move_y_to", std::make_shared<padi::SimpleAnimation>(padi::StripAnimation({32, 32}, {240, 8}, {0, 48}, 12)));

        apollo.addAnimation("tetrahedron", "idle", std::make_shared<padi::StaticAnimation>(sf::Vector2i {32,32}, sf::Vector2f {192,0}));
    }

    apollo.addAnimation("cursor",std::make_shared<padi::SimpleAnimation>(padi::StripAnimation({32, 32}, {160, 0}, {0, 32}, 6, 1)));
    apollo.addAnimation("air_strike",std::make_shared<padi::SimpleAnimation>(padi::StripAnimation({32, 32}, {288, 0}, {0, 32}, 12)));
    apollo.addAnimation("air_strike_large",std::make_shared<padi::SimpleAnimation>(padi::StripAnimation({32, 48}, {384, 0}, {0, 48}, 12)));
    apollo.addAnimation("fire",std::make_shared<padi::SimpleAnimation>(padi::StripAnimation({32, 40}, {352, 0}, {0, 40}, 12)));
    apollo.addAnimation("q_mark",std::make_shared<padi::SimpleAnimation>(padi::StripAnimation({32, 36}, {416, 0}, {0, 36}, 12)));
    apollo.addAnimation("debug",std::make_shared<padi::SimpleAnimation>(padi::StripAnimation({32, 41}, {992, 0}, {0, 32}, 12)));
    apollo.addAnimation("button",std::make_shared<padi::SimpleAnimation>(padi::StripAnimation({18, 24}, {96, 0}, {0, 24}, 4, 2)));*/
    apollo.loadFromFile("../media/strips.apollo");

    {
        int offset = 2;
        for (auto anim: {"air_strike", "air_strike_large", "fire", "q_mark", "debug", "cursor", "button"}) {
            auto slave = std::make_shared<padi::SlaveEntity>(sf::Vector2i{++offset, 0});
            slave->m_animation = apollo.lookupAnim(anim);
            level.getMap()->addEntity(slave);
        }
    }

    std::shared_ptr<padi::LivingEntity> livingEntity;
    livingEntity = std::make_shared<padi::LivingEntity>(apollo.lookupContext("cube"),
                                                        sf::Vector2i{rand() % 24, rand() % 24});
    livingEntity->setColor({255, 255, 255});
    auto leSpawn = std::make_shared<padi::SpawnEvent>(livingEntity, apollo.lookupAnim("air_strike_large"));
    leSpawn->dispatch(&level);

    auto button = std::make_shared<padi::Button>(sf::Vector2i{4,4}, apollo.lookupAnim("button"));
    level.getMap()->addUIObject(button);

    padi::Cursor cursor(apollo.lookupAnim("cursor"));

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
        sf::Vector2f wPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        sf::Vector2i tPos = level.getMap()->mapWorldPosToTile(wPos);

        button->update();
        button->active = cursor.getPosition() == button->getPosition();

        level.update(&window);
        cursor.update(&level);


        level.centerView(livingEntity->getPosition());

        t.setPosition(level.getMap()->mapTilePosToWorld(tPos) - t.getLocalBounds().getSize());
        t.setString(std::to_string(tPos.x) + " " + std::to_string(tPos.y));
        if(sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
            path.clear();
            livingEntity->intentStay();
        }
        if(sf::Mouse::isButtonPressed(sf::Mouse::Left) && path.empty()) {
            path = padi::FindPath(level.getMap(),livingEntity->getPosition(), tPos);
        }
        if(!path.empty()) {
            if(livingEntity->intentMove(path.front())) {
                path.erase(path.begin());
                t.setString(std::to_string(path.size()) + " more steps");
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
