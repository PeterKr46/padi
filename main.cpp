#include <iostream>
#include <SFML/Graphics.hpp>
#include "src/Stage.h"
#include "src/LivingEntity.h"
#include "src/Apollo.h"

int main()
{
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "PAdI");
    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);


    padi::Stage map;
    if (!map.generate("spritesheet.png", sf::Vector2u(32, 32), 32))
        return -1;

    map.scale(4.f, 4.5f);

    padi::Apollo apollo;
    if(apollo.initializeContext("cube")) {
        apollo.addAnimation("cube", "move_x_from", std::make_shared<padi::SimpleAnimation>(padi::StripAnimation({32, 32}, {0, 0}, {0, 48}, 12)));
        apollo.addAnimation("cube", "move_x_to", std::make_shared<padi::SimpleAnimation>(padi::StripAnimation({32, 32}, {16, 8}, {0, 48}, 12)));

        apollo.addAnimation("cube", "move_y_from", std::make_shared<padi::SimpleAnimation>(padi::StripAnimation({32, 32}, {64, 0}, {0, 48}, 12)));
        apollo.addAnimation("cube", "move_y_to", std::make_shared<padi::SimpleAnimation>(padi::StripAnimation({32, 32}, {48, 8}, {0, 48}, 12)));

        apollo.addAnimation("cube", "idle", std::make_shared<padi::StaticAnimation>(sf::Vector2i {32,32}, sf::Vector2f {0,0}));
    }

    auto livingEntity = std::make_shared<padi::LivingEntity>(apollo.lookupChar("cube"), sf::Vector2i{1, 1});
    livingEntity->setColor({255, 255, 255});
    map.getMap()->addEntity(livingEntity);

    sf::Vector2f clickPos{0,0};
    bool clicked{false};
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
        sf::Vector2f wPos{-1,-1};
        //if(sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            wPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        //}
        map.update(wPos, clock.getElapsedTime());
        if(!clicked && sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
            clicked = true;
            clickPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        }
        else if(clicked) {
            sf::Vector2f curPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
            map.move(curPos-clickPos);
            clickPos=curPos;
            if(!sf::Mouse::isButtonPressed(sf::Mouse::Right))
                clicked = false;
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) livingEntity->move(&map, sf::Vector2i{-1, 0});
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) livingEntity->move(&map, sf::Vector2i{1, 0});
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) livingEntity->move(&map, sf::Vector2i{0, -1});
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) livingEntity->move(&map, sf::Vector2i{0, 1});

        window.clear();
        window.draw(map);
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
    printf("%i entities final\n", map.getMap()->numQuads());
    printf("%i frames total in %.3f seconds\n", frames, clock.getElapsedTime().asSeconds());
    printf("%.3f fps avg", float(frames) / clock.getElapsedTime().asSeconds());
    return 0;
}
