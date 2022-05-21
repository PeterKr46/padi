#include <SFML/Graphics.hpp>
#include "src/entity/LivingEntity.h"
#include "lib/PerlinNoise/PerlinNoise.hpp"
#include "src/level/LevelGenerator.h"
#include "src/level/SpawnEvent.h"
#include "src/Controls.h"
#include "src/level/Cursor.h"
#include "src/player/Ability.h"
#include "src/entity/StaticEntity.h"
#include "src/ui/UIContext.h"
#include "SFML/Audio/Music.hpp"
#include "src/content/abilities/Abilities.h"
#include "src/content/menu/MainMenu.h"
#include <chrono>
#include <thread>
#include <memory>

#define SHOW_MENU

int main() {
    std::vector<sf::VideoMode> modes = sf::VideoMode::getFullscreenModes();
    auto mode = modes[0];
    // mode.width = 960;
    // mode.height = 1060;
    sf::RenderWindow window(
            mode
            // sf::VideoMode(960, 1080)
            , "PAdI", sf::Style::Fullscreen
    );

    sf::Shader crtShader;
    crtShader.loadFromFile("../src/shaders/crt.vert", "../src/shaders/crt.frag");



    sf::Font font{};
    font.setSmooth(false);
    font.loadFromFile("../media/prstartk.ttf");
    sf::Text quadCounter("Hello, world.", font, 7);

    // Ambient sounds
    sf::Music ambient;
    ambient.openFromFile("../media/audio/rain_hell/youfoundaclue - rain hell - OST -openjam 2020- - 03 charred.flac");
    ambient.setLoop(true);
    ambient.setLoopPoints(sf::Music::TimeSpan(sf::Time(), ambient.getDuration() - sf::seconds(0.8)));
    ambient.setPitch(120.f / 125.8f);
    ambient.setVolume(20);
    ambient.play();

    std::shared_ptr<padi::LivingEntity> livingEntity;

    auto walkAbility = std::make_shared<padi::content::Walk>();
    walkAbility->user = livingEntity;
    auto tpAbility = std::make_shared<padi::content::Teleport>();
    tpAbility->user = livingEntity;
    auto lightenAbility = std::make_shared<padi::content::Lighten>();

    auto darkenAbility = std::make_shared<padi::content::Darken>();

    std::shared_ptr<padi::Ability> activeAbility{nullptr};

    auto view = window.getView();
    view.setSize(sf::Vector2f(window.getSize()));
    view.setCenter(0, 0);//sf::Vector2f (rawImage.getSize()) / 2.font);
    window.setView(view);

    std::shared_ptr<padi::Activity> activity = std::make_shared<padi::content::MainMenu>
            (&window,
             "../media/ui.apollo",
             "../media/ui_sheet.png"
            );

    sf::Clock clock;
    sf::Clock frameClock;
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
                activity->handleResize(event.size.width, event.size.height);
            }
        }
        window.clear();

#ifdef SHOW_MENU
        activity->draw();
        activity = activity->handoff();
#else
        level->update(&rawImage);

        if (padi::Controls::isKeyDown(sf::Keyboard::Home)) {
            level->moveCursor(livingEntity->getPosition());
        }
        if (activeAbility) {
            activeAbility->castIndicator(&(*level));
            if (padi::Controls::isKeyDown(sf::Keyboard::Space)) {
                livingEntity->intentCast(activeAbility, level->getCursorLocation());
                activeAbility = nullptr;
            } else if (padi::Controls::isKeyDown(sf::Keyboard::Escape)) {
                activeAbility = nullptr;
                level->hideCursor();
            }
        } else if (padi::Controls::isKeyDown(sf::Keyboard::Q)) {
            activeAbility = lightenAbility;
        } else if (padi::Controls::isKeyDown(sf::Keyboard::E)) {
            activeAbility = tpAbility;
        } else if (padi::Controls::wasKeyPressed(sf::Keyboard::W)) {
            activeAbility = walkAbility;
        } else if (padi::Controls::wasKeyPressed(sf::Keyboard::R)) {
            activeAbility = darkenAbility;
        }

        level->centerView(livingEntity->getPosition());

        quadCounter.setString(std::to_string(level->getMap()->numQuads()));
        quadCounter.setPosition(level->getMap()->mapTilePosToWorld(level->getCursorLocation()));

        level->populateVBO();

        rawImage.clear();
        auto states = sf::RenderStates::Default;
        states.transform.scale(sf::Vector2f( 256.f /rawImage.getView().getSize().y, 256.f / rawImage.getView().getSize().y));
        rawImage.draw(*level, states);
        rawImage.draw(quadCounter, states);

        auto rState = sf::RenderStates::Default;
        rState.shader = &crtShader;
        rState.texture = &rawImage.getTexture();
        window.draw(screenQuad,rState);
#endif
        window.display();

        ++frames;
        size_t ms = frameClock.restart().asMicroseconds();
        if (ms < 1024) std::this_thread::sleep_for(std::chrono::microseconds(1024 - ms));
    }
    // printf("%zu / %zu quads final\n", level->getMap()->numQuads(), level->getVBOCapacity());
    printf("%zu frames total in %.3f seconds\n", frames, clock.getElapsedTime().asSeconds());
    printf("%.3f fps avg", float(frames) / clock.getElapsedTime().asSeconds());
    return 0;
}
