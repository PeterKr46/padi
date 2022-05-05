#include <iostream>
#include <SFML/Graphics.hpp>
#include "src/entity/LivingEntity.h"
#include "src/animation/Apollo.h"
#include "src/AStar.h"
#include "lib/PerlinNoise/PerlinNoise.hpp"

int main() {
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "PAdI");

    padi::Level level({10, 10});
    sf::Texture tex;
    tex.loadFromFile("../media/complete_sheet.png");
    level.setMasterSheet(tex);
    //level.scale(4,4);
    level.centerView({0,0});
    {
        const siv::PerlinNoise::seed_type seed = 123456u;
        const siv::PerlinNoise perlin{ seed };
        sf::Vector2i tile;
        for (tile.x = 0; tile.x < 10; tile.x++) {
            for (tile.y = 0; tile.y < 10; tile.y++) {
                auto z = float(perlin.octave2D_01(tile.x * 0.2, tile.y * 0.2, 5));
                auto t = std::make_shared<padi::Tile>(tile);
                t->m_color.r = z * 255;
                t->m_color.g = z * 255;
                t->m_color.b = z * 255;
                level.getMap()->addTile(t);
            }
        }
    }
    sf::Font f{};
    f.setSmooth(false);
    f.loadFromFile("../media/prstartk.ttf");
    sf::Text t("Hello, world.",f,7);

    padi::Apollo apollo;
    if(apollo.initializeContext("cube")) {
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

    apollo.addAnimation("air_strike",std::make_shared<padi::SimpleAnimation>(padi::StripAnimation({32, 32}, {288, 0}, {0, 32}, 12)));
    apollo.addAnimation("air_strike_large",std::make_shared<padi::SimpleAnimation>(padi::StripAnimation({32, 48}, {384, 0}, {0, 48}, 12)));
    apollo.addAnimation("fire",std::make_shared<padi::SimpleAnimation>(padi::StripAnimation({32, 40}, {352, 0}, {0, 40}, 12)));
    apollo.addAnimation("q_mark",std::make_shared<padi::SimpleAnimation>(padi::StripAnimation({32, 36}, {416, 0}, {0, 36}, 12)));
    apollo.addAnimation("debug",std::make_shared<padi::SimpleAnimation>(padi::StripAnimation({32, 32}, {992, 0}, {0, 32}, 12)));

    {
        int offset = 2;
        for (auto anim: {"air_strike", "air_strike_large", "fire", "q_mark", "debug"}) {
            auto slave = std::make_shared<padi::SlaveEntity>(sf::Vector2i{++offset, 0});
            slave->m_animation = apollo.lookupAnim(anim);
            level.getMap()->addEntity(slave);
        }
    }
    
    auto livingEntity = std::make_shared<padi::LivingEntity>(apollo.lookupContext("cube"), sf::Vector2i{1, 1});
    livingEntity->setColor({255, 255, 255});
    level.getMap()->addEntity(livingEntity);
    level.addCycleBeginListener(livingEntity);
    level.addCycleEndListener(livingEntity);

    auto randomEntity = std::make_shared<padi::LivingEntity>(apollo.lookupContext("tetrahedron"), sf::Vector2i{10, 10});
    randomEntity->setColor({255, 255, 255});
    level.getMap()->addEntity(randomEntity);
    
    std::vector<sf::Vector2i> path;
    
    auto fire = std::make_shared<padi::SlaveEntity>(sf::Vector2i{2, 2});
    fire->m_animation = apollo.lookupAnim("fire");
    fire->m_color = sf::Color(255, 127, 0);
    
    auto strike = std::make_shared<padi::SlaveEntity>(sf::Vector2i{0, 0});
    strike->m_animation = apollo.lookupAnim("air_strike_large");
    strike->m_color = sf::Color(255, 127, 127);

    sf::Clock clock;
    size_t frames = 0;
    sf::Clock frame_clock;
    float longest = 0;
    while (window.isOpen())
    {
        sf::Event event{};
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        sf::Vector2f wPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        sf::Vector2i tPos = level.getMap()->mapWorldPosToTile(wPos);
        //}
        level.update(&window);

        t.setPosition(level.getMap()->mapTilePosToWorld(tPos) - t.getLocalBounds().getSize());
        t.setString(std::to_string(tPos.x) + " " + std::to_string(tPos.y));
        if(sf::Mouse::isButtonPressed(sf::Mouse::Left) && path.empty()) {
            path = padi::FindPath(level.getMap(),livingEntity->getPosition(), tPos);
        }
        if(!path.empty()) {
            if(livingEntity->moveIntent(path.front())) {
                path.erase(path.begin());
                t.setString(std::to_string(path.size()) + " more steps");
            }
        }

        //if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) livingEntity->moveIntent(&map, sf::Vector2i{-1, 0});
        //else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) livingEntity->moveIntent(&map, sf::Vector2i{1, 0});
        //else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) livingEntity->moveIntent(&map, sf::Vector2i{0, -1});
        //else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) livingEntity->moveIntent(&map, sf::Vector2i{0, 1});

        //if(map.getMap()->getCurrentCycleFrames() == 0) {
        //    sf::Vector2i dir;
        //    switch (rand() & 0b11) {
        //        case 0: dir={1,0}; break;
        //        case 1: dir={-1,0}; break;
        //        case 2: dir={0,1}; break;
        //        case 3: dir={0,-1}; break;
        //    }
        //    if(randomEntity->moveIntent(&map, dir)) {
        //        auto tile = map.getMap()->getTile(randomEntity->getPosition());
        //        if(tile) {
        //            tile->m_color -= sf::Color(randomEntity->getColor().r / 16, randomEntity->getColor().g / 16,
        //                                       randomEntity->getColor().b / 16);
        //            tile->m_color.a = 255;
        //            if(tile->m_color.r+tile->m_color.g+tile->m_color.b < 128) {
        //                map.getMap()->removeTile(randomEntity->getPosition());
        //            }
        //        }
        //    }
        //}
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
    printf("%i entities final\n", level.getMap()->numQuads());
    printf("%i frames total in %.3f seconds\n", frames, clock.getElapsedTime().asSeconds());
    printf("%.3f fps avg", float(frames) / clock.getElapsedTime().asSeconds());
    return 0;
}
