#include <iostream>
#include <SFML/Graphics.hpp>
#include <utility>
#include "src/entity/LivingEntity.h"
#include "src/media/Apollo.h"
#include "lib/PerlinNoise/PerlinNoise.hpp"
#include "src/level/LevelGenerator.h"
#include "src/level/SpawnEvent.h"
#include "src/Controls.h"
#include "src/level/Cursor.h"
#include "src/player/Ability.h"
#include "SFML/Audio/SoundBuffer.hpp"
#include "src/entity/StaticEntity.h"
#include "src/ui/UIContext.h"
#include "src/ui/Button.h"
#include "src/ui/Switch.h"
#include "SFML/Audio/Music.hpp"
#include "src/content/abilities/Abilities.h"


int main() {
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "PAdI");
    sf::RenderTexture rawImage;
    sf::Shader crtShader;
    crtShader.loadFromFile("../src/shaders/crt.vert","../src/shaders/crt.frag");

    if (!rawImage.create(1920, 1080)) {
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
    level.initCursor("cursor");
    auto apollo = level.getApollo();
    sf::Font f{};
    f.setSmooth(false);
    f.loadFromFile("../media/prstartk.ttf");
    sf::Text t("Hello, world.", f, 7);

    sf::Music ambient;
    ambient.openFromFile("../media/audio/rat_in_hell/youfoundaclue - rain hell - OST -openjam 2020- - 03 charred.flac");
    ambient.setLoop(true);
    ambient.setLoopPoints(sf::Music::TimeSpan(sf::Time(), ambient.getDuration() - sf::seconds(0.8)));
    //ambient.setPlayingOffset(sf::seconds(30));
    ambient.setPitch(120.f/125.8f);
    ambient.play();
    std::shared_ptr<padi::LivingEntity> livingEntity;
    livingEntity = std::make_shared<padi::LivingEntity>(apollo->lookupAnimContext("cube"), sf::Vector2i{0, 0});
    livingEntity->setColor({255, 255, 255});
    auto leSpawn = std::make_shared<padi::SpawnEvent>(livingEntity, apollo->lookupAnim("bubble"));
    leSpawn->dispatch(&level);

    auto ui = padi::UIContext();
    ui.initTextures("../media/ui.apollo", "../media/ui_sheet.png");
    ui.addObject(std::make_shared<padi::Button>(&ui));
    auto uiSwitch = std::make_shared<padi::Switch>(&ui);
    ui.addObject(uiSwitch);

    auto walkAbility = std::make_shared<padi::content::Walk>();
    walkAbility->user = livingEntity;
    auto tpAbility = std::make_shared<padi::content::Teleport>();
    tpAbility->user = livingEntity;
    auto asAbility = std::make_shared<padi::content::AirStrike>();
    //auto darkAbility = std::make_shared<padi::content::Darken>();

    std::shared_ptr<padi::Ability> activeAbility{nullptr};

    auto dbg = std::make_shared<padi::StaticEntity>(sf::Vector2i(3,-3));
    dbg->m_animation = level.getApollo()->lookupAnim("debug");
level.getMap()->addEntity(dbg);

    sf::VertexArray quad(sf::Quads, 4);
    {
        sf::Vector2f halfSize{window.getSize()};
        halfSize /= 2.f;
        sf::Vector2f imgSize{rawImage.getSize()};
        quad[0].position = -halfSize;
        quad[0].texCoords = {0, imgSize.y};

        quad[1].position = {-halfSize.x, halfSize.y};
        quad[1].texCoords = {0,0};

        quad[2].position = halfSize;
        quad[2].texCoords = {imgSize.x, 0};

        quad[3].position = {halfSize.x, -halfSize.y};
        quad[3].texCoords = imgSize;
    }
    auto view = window.getView();
    view.setSize(sf::Vector2f (rawImage.getSize()));
    view.setCenter(0,0);//sf::Vector2f (rawImage.getSize()) / 2.f);
    window.setView(view);

    sf::Clock clock;
    size_t frames = 0;
    sf::Clock frame_clock;
    float longest = 0;
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

        //button->update();

        level.update(&rawImage);

        t.setString(std::to_string(level.getMap()->numQuads()));
        if (padi::Controls::isKeyDown(sf::Keyboard::Home)) {
            level.moveCursor(livingEntity->getPosition());
        }
        if (activeAbility) {
            activeAbility->castIndicator(&level);
            if (padi::Controls::isKeyDown(sf::Keyboard::Space)) {
                livingEntity->intentCast(activeAbility, level.getCursorLocation());
                activeAbility = nullptr;
            } else if (padi::Controls::isKeyDown(sf::Keyboard::Escape)) {
                activeAbility = nullptr;
                level.hideCursor();
            }
        } else if (padi::Controls::isKeyDown(sf::Keyboard::Q)) {
            activeAbility = asAbility;
        } else if (padi::Controls::isKeyDown(sf::Keyboard::E)) {
            activeAbility = tpAbility;
        } else if (padi::Controls::isKeyDown(sf::Keyboard::W)) {
            activeAbility = walkAbility;
        }/* else if (padi::Controls::isKeyDown(sf::Keyboard::R)) {
            activeAbility = darkAbility;
        }*/

        level.centerView(livingEntity->getPosition());

        t.setPosition(level.getMap()->mapTilePosToWorld(level.getCursorLocation()));// - sf::Vector2f(t.getLocalBounds().getSize().x / 2, -12));

        level.populateVBO();
        ui.populateVBO();

        rawImage.clear();
        auto states = sf::RenderStates::Default;
        states.transform.scale(sf::Vector2f( 256.f /rawImage.getView().getSize().y, 256.f / rawImage.getView().getSize().y));
        rawImage.draw(level, states);
        rawImage.draw(t, states);
        rawImage.draw(ui);

        window.clear();
        auto rState = sf::RenderStates::Default;
        rState.shader = &crtShader;
        rState.texture = &rawImage.getTexture();
        window.draw(quad,rState);
        window.display();

        float t = frame_clock.restart().asSeconds();
        if (frames > 20 && t > 0.013) {
            std::cout << "Drop detected: " << 1.f / t << " FPS (" << t << ")" << std::endl;
        }
        ++frames;
    }
    printf("Slowest frame took %.3f s, i.e. %.3f FPS\n", longest, 1.f / longest);
    printf("%i / %i quads final\n", level.getMap()->numQuads(), level.getVBOCapacity());
    printf("%i frames total in %.3f seconds\n", frames, clock.getElapsedTime().asSeconds());
    printf("%.3f fps avg", float(frames) / clock.getElapsedTime().asSeconds());
    return 0;
}
