#include <iostream>
#include <SFML/Graphics.hpp>
#include "src/Stage.h"
#include "src/LivingEntity.h"

int main()
{
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "PAdI");
    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);


    padi::Stage map;
    if (!map.generate("spritesheet.png", sf::Vector2u(32, 32), 32))
        return -1;

    map.scale(4.f, 4.5f);

    auto roll_XA = padi::StripAnimation({32, 32}, {0, 0}, {0, 48}, 12);
    auto roll_XB = padi::StripAnimation({32, 32}, {16, 8}, {0, 48}, 12);
    auto roll_YA = padi::StripAnimation({32, 32}, {64, 0}, {0, 48}, 12);
    auto roll_YB = padi::StripAnimation({32, 32}, {48, 8}, {0, 48}, 12);

    {

        auto livingEntity = new padi::LivingEntity({1, 1});
        livingEntity->setAnimation(new padi::ReverseAnimation(&roll_XA));
        livingEntity->setSlaveAnimation(new padi::ReverseAnimation(&roll_XB));
        livingEntity->setColor({255, 255, 255});
        livingEntity->move(&map, {1, 0});
        map.getMap()->addEntity(livingEntity);

    }{

        auto livingEntity = new padi::LivingEntity({0, 1});
        livingEntity->setAnimation(&roll_XA);
        livingEntity->setSlaveAnimation(&roll_XB);
        livingEntity->setColor({127, 255, 127});
        livingEntity->move(&map, {1, 0});
        map.getMap()->addEntity(livingEntity);

    }
    {

        auto livingEntity = new padi::LivingEntity({1, 1});
        livingEntity->setAnimation(new padi::ReverseAnimation(&roll_YA));
        livingEntity->setSlaveAnimation(new padi::ReverseAnimation(&roll_YB));
        livingEntity->setColor({255, 255, 127});
        livingEntity->move(&map, {0, 1});
        map.getMap()->addEntity(livingEntity);

    }
    {

        auto livingEntity = new padi::LivingEntity({1, 0});
        livingEntity->setAnimation(&roll_YA);
        livingEntity->setSlaveAnimation(&roll_YB);
        livingEntity->setColor({127, 255, 255});
        livingEntity->move(&map, {0, 1});
        map.getMap()->addEntity(livingEntity);

    }

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
