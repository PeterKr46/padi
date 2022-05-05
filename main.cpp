#include <iostream>
#include <SFML/Graphics.hpp>
#include "src/Stage.h"

int main()
{
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "PAdI");
    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);


    padi::Stage map;
    if (!map.generate("T32_Hover.png", sf::Vector2u(32, 32), 100))
        return -1;
    map.scale(4.f, 4.f);

    sf::Vector2i selected{2,2};
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
        if(frames > 20 && t > longest) {
            longest = t;
        }
        ++frames;
    }
    printf("Slowest frame took %.3f s, i.e. %.3f FPS\n", longest, 1.f/longest);
    printf("%i entities final\n", map.map.numQuads());
    printf("%i frames total in %.3f seconds\n", frames, clock.getElapsedTime().asSeconds());
    printf("%.3f fps avg", float(frames) / clock.getElapsedTime().asSeconds());
    return 0;
}
