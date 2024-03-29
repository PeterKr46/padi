#include <SFML/Graphics.hpp>
#include "src/Controls.h"
#include "src/content/menu/MainMenu.h"
#include "SFML/Audio/Music.hpp"


int main(int argc, char * argv[]) {
    std::queue<std::pair<sf::VideoMode, uint8_t>> windowModes;
    windowModes.push({sf::VideoMode(1280, 720), sf::Style::Default});
    windowModes.push({sf::VideoMode::getFullscreenModes().front(), sf::Style::Fullscreen});
    windowModes.push({sf::VideoMode(1920, 1080), sf::Style::Default});

    sf::RenderWindow window(windowModes.front().first, "PAdI", windowModes.front().second);

    // Ambient sounds
    sf::Music ambient;
    //ambient.openFromFile("../media/audio/rain_hell/youfoundaclue - rain hell - OST -openjam 2020- - 03 charred.flac");
    ambient.openFromFile("../media/audio/shadydave/snowfall.ogg");
    //ambient.openFromFile("../media/audio/kelewin/ambient-intro-layered.ogg");
    ambient.setLoop(true);
    //ambient.setLoopPoints(sf::Music::TimeSpan(sf::Time(), ambient.getDuration() - sf::seconds(0.8)));
    ambient.setPitch(120.f / 125.8f * (80.f/60));
    //ambient.setPitch(120.f / 88.f);
    //ambient.setPitch(90.f / 91.7f);

    ambient.setVolume(12);
    ambient.play();

    std::shared_ptr<padi::Activity> activity = std::make_shared<padi::content::MainMenu>
            (
             "../media/ui.apollo",
             "../media/ui_sheet.png",
             argc > 1 ? argv[1] : nullptr
            );
    activity->handleResize(int(window.getSize().x), int(window.getSize().y));

    sf::Clock runtime;
    size_t frames = 0;
    while (window.isOpen()) {
        sf::Event event{};
        padi::Controls::resetKeyStates(); // Handles key events
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                activity->close();
                window.close();
            } else if (event.type == sf::Event::KeyPressed) {
                padi::Controls::keyDown(event.key.code);
            } else if (event.type == sf::Event::KeyReleased) {
                padi::Controls::keyReleased(event.key.code);
            } else if (event.type == sf::Event::TextEntered) {
                padi::Controls::textEntered(event.text.unicode);
            } else if (event.type == sf::Event::Resized) {
                activity->handleResize(int(event.size.width), int(event.size.height));
            } else if (event.type == sf::Event::MouseMoved) {
                sf::Vector2i pxPos{event.mouseMove.x, event.mouseMove.y};
                sf::Vector2f relPos{float(pxPos.x) / float(window.getSize().x), float(float(pxPos.y) / window.getSize().y)};
                padi::Controls::mouseMoved(relPos, pxPos);
            }
        }
        window.clear();

        activity->draw(&window);
        auto nextActivity = activity->handoff().lock();
         if (nextActivity != activity) {
            nextActivity->handleResize(int(window.getSize().x), int(window.getSize().y));
            activity = nextActivity;
        }

        window.display();
        ++frames;
        if(padi::Controls::wasKeyReleased(sf::Keyboard::F11)) {
            windowModes.push(windowModes.front());
            windowModes.pop();
            auto & mode = windowModes.front();
            window.create(mode.first, "Padi", mode.second);
            window.requestFocus();
            activity->handleResize(int(mode.first.width), int(mode.first.height));
        }
    }
    printf("%zu frames total in %.3f seconds\n", frames, runtime.getElapsedTime().asSeconds());
    printf("%.3f fps avg", float(frames) / runtime.getElapsedTime().asSeconds());
    return 0;
}
