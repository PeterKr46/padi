#include <SFML/Graphics.hpp>
#include "src/Controls.h"
#include "src/content/menu/MainMenu.h"
#include "SFML/Audio/Music.hpp"


int main() {
    std::vector<sf::VideoMode> modes = sf::VideoMode::getFullscreenModes();
    auto mode = modes[0];
    mode.width = 1920;
    mode.height = 1440;
    sf::RenderWindow window(
            mode
            // sf::VideoMode(960, 1080)
            , "PAdI"//, sf::Style::Fullscreen
    );

    // Ambient sounds
    sf::Music ambient;
    ambient.openFromFile("../media/audio/rain_hell/youfoundaclue - rain hell - OST -openjam 2020- - 03 charred.flac");
    ambient.setLoop(true);
    ambient.setLoopPoints(sf::Music::TimeSpan(sf::Time(), ambient.getDuration() - sf::seconds(0.8)));
    ambient.setPitch(120.f / 125.8f);
    ambient.setVolume(20);
    ambient.play();

    std::shared_ptr<padi::Activity> activity = std::make_shared<padi::content::MainMenu>
            (&window,
             "../media/ui.apollo",
             "../media/ui_sheet.png"
            );
    activity->handleResize(int(mode.width), int(mode.height));

    sf::Clock runtime;
    size_t frames = 0;
    while (window.isOpen()) {
        sf::Event event{};
        padi::Controls::resetKeyStates(); // Handles key events
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::KeyPressed) {
                padi::Controls::keyDown(event.key.code);
            } else if (event.type == sf::Event::KeyReleased) {
                padi::Controls::keyReleased(event.key.code);
            } else if (event.type == sf::Event::Resized) {
                activity->handleResize(int(event.size.width), int(event.size.height));
            }
        }
        window.clear();

        activity->draw();
        auto nextActivity = activity->handoff();
        if(nextActivity != activity) {
            nextActivity->handleResize(int(window.getSize().x), int(window.getSize().y));
            activity = nextActivity;
        }

        window.display();
        ++frames;
    }
    printf("%zu frames total in %.3f seconds\n", frames, runtime.getElapsedTime().asSeconds());
    printf("%.3f fps avg", float(frames) / runtime.getElapsedTime().asSeconds());
    return 0;
}
