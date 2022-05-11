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

//#define SHOW_MENU

int main() {
    std::vector<sf::VideoMode> modes = sf::VideoMode::getFullscreenModes();
    auto mode = modes[0];
    // mode.width = 960;
    // mode.height = 1060;
    sf::RenderWindow window(
            mode
            // sf::VideoMode(960, 1080)
            ,"PAdI"
            //,sf::Style::Fullscreen
            );
    sf::RenderTexture rawImage;

    sf::Shader crtShader;
    crtShader.loadFromFile("../src/shaders/crt.vert", "../src/shaders/crt.frag");

    if (!rawImage.create(mode.width, mode.height)) {
        return -1;
    }

    auto levelGen = padi::LevelGenerator();
    time_t seed;
    time(&seed);
    auto level = levelGen
            .withSpritesheet("../media/level_sheet.png")
            .withApollo("../media/level.apollo")
            .withSeed(seed)
            .withArea({100, 100})
            .generate();
    level->initCursor("cursor");
    auto apollo = level->getApollo();

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
    livingEntity = std::make_shared<padi::LivingEntity>(apollo->lookupAnimContext("cube"), sf::Vector2i{0, 0});
    livingEntity->setColor({255, 255, 255});
    auto leSpawn = std::make_shared<padi::SpawnEvent>(livingEntity);
    leSpawn->dispatch(level);

    auto walkAbility = std::make_shared<padi::content::Walk>();
    walkAbility->user = livingEntity;
    auto tpAbility = std::make_shared<padi::content::Teleport>();
    tpAbility->user = livingEntity;
    auto lightenAbility = std::make_shared<padi::content::Lighten>();

    auto darkenAbility = std::make_shared<padi::content::Darken>();

    std::shared_ptr<padi::Ability> activeAbility{nullptr};

    // Cycle indicator
    auto cycleIndicator = std::make_shared<padi::StaticEntity>(sf::Vector2i(3, -3));
    cycleIndicator->m_animation = level->getApollo()->lookupAnim("debug");
    level->getMap()->addEntity(cycleIndicator);

    for(int i = 0; i < 10; ++i) {
        auto laser = std::make_shared<padi::StaticEntity>(sf::Vector2i(3, i));
        laser->m_animation = level->getApollo()->lookupAnim("laser_hold");
        level->getMap()->addEntity(laser);
    }

    sf::VertexArray screenQuad(sf::Quads, 4);
    {
        sf::Vector2f halfSize{window.getSize()};
        halfSize /= 2.f;
        sf::Vector2f imgSize{rawImage.getSize()};
        screenQuad[0].position = -halfSize;
        screenQuad[0].texCoords = {0, imgSize.y};

        screenQuad[1].position = {-halfSize.x, halfSize.y};
        screenQuad[1].texCoords = {0, 0};

        screenQuad[2].position = halfSize;
        screenQuad[2].texCoords = {imgSize.x, 0};

        screenQuad[3].position = {halfSize.x, -halfSize.y};
        screenQuad[3].texCoords = imgSize;
    }
    auto view = window.getView();
    view.setSize(sf::Vector2f(rawImage.getSize()));
    view.setCenter(0, 0);//sf::Vector2f (rawImage.getSize()) / 2.font);
    window.setView(view);

    padi::content::MainMenu menu(&window, "../media/ui.apollo", "../media/ui_sheet.png");

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
            }
        }
        window.clear();

#ifdef SHOW_MENU
        menu.clear();
        menu.draw();
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
    printf("%zu / %zu quads final\n", level->getMap()->numQuads(), level->getVBOCapacity());
    printf("%zu frames total in %.3f seconds\n", frames, clock.getElapsedTime().asSeconds());
    printf("%.3f fps avg", float(frames) / clock.getElapsedTime().asSeconds());
    return 0;
}
