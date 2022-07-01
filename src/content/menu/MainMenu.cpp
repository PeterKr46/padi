//
// Created by Peter on 06/05/2022.
//

#include "MainMenu.h"
#include <SFML/Graphics.hpp>
#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/TcpSocket.hpp"
#include "../../ui/Immediate.h"
#include "../../Controls.h"
#include "SFML/Network/Packet.hpp"
#include "../game/OnlineGame.h"
#include "../../level/LevelGenerator.h"
#include "../../Utils.h"

namespace padi::content {


    MainMenu::MainMenu(std::string const &apollo, std::string const &spritesheet)
            : m_chat({250, 194, 200, 60}) {

        m_uiContext.init(apollo, spritesheet);
        m_chat.init(&m_uiContext);
        m_chat.submit = [&](std::string const &msg) { sendChatMessage(msg); };

        m_crt.setShader(m_uiContext.getApollo()->lookupShader("fpa"));

        m_uiContext.pushTransform().translate(16, 32);

        m_uiContext.setText("play", "Play", {64 + 16, 12}, true);
        m_uiContext.updateTextSize("play", 1.5);
        m_uiContext.updateTextOutline("play", sf::Color::Black, 0.5);

        sf::Clock rand;
        m_uiContext.pushTransform().translate(240, 148);
        static const char defaultNames[][9] = {
                "Tom", "Jerry",
                "Garfunke", "Simon",
                "Lucky", "Unlucky",
                "Tadys", "Bleb"
        };
        m_uiContext.setText("nick_label", "Nick", {0, 0});
        m_uiContext.setText("nick_input", defaultNames[rand.getElapsedTime().asMicroseconds() % 8], {32, 0});
        m_uiContext.popTransform();

        switchMode(NUM_MODES, PlayTutorial);
        m_uiContext.popTransform();

        Immediate::setFocus(&m_uiContext, "play");
        m_target = PlayTutorial;
    }

    void MainMenu::switchMode(TargetMode from, TargetMode to) {
        switch (from) {
            case PlayTutorial:
                m_uiContext.removeText("tut_info");
                break;
            case PlayAlone:
                m_uiContext.removeText("seed_input");
                break;
            case PlayCoop:
                break;
            case OnlineHost:
                m_uiContext.removeText("host_title");
                m_uiContext.removeText("own_ip");
                m_uiContext.removeText("host_play");
                m_uiContext.removeText("num_clients");
                closeHostSession();
                break;
            case OnlineClient:
                m_uiContext.removeText("join_title");
                m_uiContext.removeText("ip_input");
                m_uiContext.removeText("connect");
                closeClientSession();
                break;
            case NUM_MODES:
                break;
        }
        switch (to) {
            case PlayTutorial:
                m_uiContext.updateTextString("play", "Tutorial");
                m_uiContext.setText("tut_info",
                                            "A short introduction\n"
                                                " to this wonderful game.", {0,40});
                break;
            case PlayAlone:
                m_uiContext.updateTextString("play", "Play Alone");
                m_uiContext.setText("seed_input", "Choose a Seed", {88, 52}, true);
                break;
            case PlayCoop:
                m_uiContext.updateTextString("play", "Local Co-Op");
                break;
            case OnlineHost:
                m_uiContext.updateTextString("play", "Host Game");

                m_uiContext.pushTransform().translate(0, 32);
                m_uiContext.setText("host_title", "HOST REMOTE PLAY", {88, 8}, true);
                m_uiContext.setText("own_ip", "", {88, 20}, true);
                m_uiContext.setText("host_play", "Not Hosting", {86, 38}, true);
                m_uiContext.setText("num_clients", "", {8, 56});
                m_uiContext.popTransform();
                break;
            case OnlineClient:
                m_uiContext.updateTextString("play", "Join Game");

                m_uiContext.pushTransform().translate(0, 32);
                m_uiContext.setText("join_title", "JOIN REMOTE PLAY", {88, 8}, true);
                m_uiContext.setText("ip_input", "127.0.0.1", {88, 20}, true);
                m_uiContext.setText("connect", "Connect", {8, 34});
                m_uiContext.popTransform();

                break;
            default:
                break;
        }
    }

    void MainMenu::draw(sf::RenderTarget *target) {
        {
            auto lvl = m_background.getLevel().lock();
            lvl->update(m_crt.asTarget());
            lvl->centerView({-3, 3});
            lvl->populateVBO();
        }

        m_crt.asTarget()->clear();
        auto states = sf::RenderStates::Default;
        states.transform.scale(
                sf::Vector2f(255.f / m_crt.asTarget()->getView().getSize().y,
                             255.f / m_crt.asTarget()->getView().getSize().y));
        m_crt.asTarget()->draw(m_background, states);
        m_uiContext.nextFrame();
        {
            m_uiContext.pushTransform().translate(16, 32);
            if (Immediate::isFocused(&m_uiContext, "play")) {
                if (Controls::wasKeyReleased(sf::Keyboard::Left)) {
                    auto newMode = TargetMode((m_target - 1 + NUM_MODES) % NUM_MODES);
                    switchMode(m_target, newMode);
                    m_target = newMode;
                } else if (Controls::wasKeyReleased(sf::Keyboard::Right)) {
                    auto newMode = TargetMode((m_target + 1) % NUM_MODES);
                    switchMode(m_target, newMode);
                    m_target = newMode;
                }
            }
            switch (m_target) {
                case PlayTutorial:
                    drawTutorialUI();
                    break;
                case PlayAlone:
                    drawSPUI();
                    break;
                case PlayCoop:
                    drawCoopUI();
                    break;
                case OnlineHost:
                    drawHostUI();
                    break;
                case OnlineClient:
                    drawClientUI();
                    break;
                case NUM_MODES:
                    break;
            }
            Immediate::Sprite(&m_uiContext,
                              sf::FloatRect{-16, 0, 32, 32},
                              0,
                              m_uiContext.getApollo()->lookupAnim("arrow_left"));
            Immediate::Sprite(&m_uiContext,
                              sf::FloatRect{150, 0, 32, 32},
                              0,
                              m_uiContext.getApollo()->lookupAnim("arrow_right"));

            // NICKNAME CONFIGURATION
            if (!clientRole.client && !hostRole.active) {
                m_uiContext.pushTransform().translate(240, 148);
                Immediate::ScalableSprite(&m_uiContext, {28, -2, 64, 12}, 0,
                                          m_uiContext.getApollo()->lookupAnim("scalable_textfield"));
                std::string t = m_uiContext.getTextString("nick_input");
                if (Immediate::isFocused(&m_uiContext, "nick_input")) {
                    m_uiContext.updateTextColor("nick_input", sf::Color::Yellow);
                } else {
                    m_uiContext.updateTextColor("nick_input", sf::Color::White);
                    if (t.empty()) {
                        t = "Red";
                        m_uiContext.updateTextString("nick_input", t);
                    }
                }
                if (Immediate::TextInput(&m_uiContext, "nick_input", &t, 8, AlphabeticNumericCharacterSet)) {
                    m_uiContext.updateTextString("nick_input", t);
                }
                Immediate::ScalableSprite(
                        &m_uiContext,
                        {96, -2, 12, 12},
                        0,
                        m_uiContext.getApollo()->lookupAnim("scalable_window"),
                        sf::Color(hsv(int(hash_c_string(t.c_str(), t.length())), 1.f, 0.8f))
                        );
                m_uiContext.popTransform();
            }
            m_uiContext.popTransform();
        }
        m_chat.draw(&m_uiContext);

        if (hostRole.active) {
            updateHost();
        }
        if (clientRole.client) {
            updateClient();
        }

        m_crt.asTarget()->draw(m_uiContext);

        target->draw(m_crt);
    }

    std::weak_ptr<padi::Activity> MainMenu::handoff() {
        return m_next ? m_next : shared_from_this();
    }

    void MainMenu::handleResize(int width, int height) {
        m_crt.handleResize(width, height);
    }

    void MainMenu::initializeHostSession() {
        hostRole.listener.setBlocking(false);
        if (hostRole.listener.listen(42069) != sf::Socket::Done) {
            printf("[padi::content::MainMenu] Error setting up Host.\n");
            m_uiContext.updateTextString("num_clients", "Host setup failed.");
            hostRole.active = false;
        } else {
            hostRole.nextClient = std::make_shared<sf::TcpSocket>();
            hostRole.nextClient->setBlocking(false);
            printf("[padi::content::MainMenu] Host session started.\n");
            m_uiContext.updateTextString("num_clients", "Host online!");
            m_uiContext.updateTextString("connect", "Unavailable");
            m_uiContext.updateTextString("host_play", "Start playing");
            sendChatMessage("Host session started.");
        }
    }

    void MainMenu::closeHostSession() {
        hostRole.listener.close();
        hostRole.active = false;
        for (auto &client: hostRole.clients) {
            client.getSocket().lock()->disconnect();
        }
        if (hostRole.nextClient) {
            hostRole.nextClient.reset();
        }
        hostRole.clients.clear();
        m_uiContext.updateTextString("num_clients", "");
        m_uiContext.updateTextString("connect", "Connect");
        m_uiContext.updateTextString("host_play", "Not Hosting");
        m_uiContext.updateTextString("own_ip", "");
        appendChatMessage("Host session closed.");
    }

    void MainMenu::updateHost() {
        auto result = hostRole.listener.accept(*hostRole.nextClient);
        if (result == sf::Socket::Done) {
            hostRole.clients.emplace_back(hostRole.nextClient);
            hostRole.nextClient = std::make_shared<sf::TcpSocket>();
            hostRole.nextClient->setBlocking(false);
            m_uiContext.updateTextString("num_clients", std::to_string(hostRole.clients.size()) + " client(s).");
            sendChatMessage("A player joined.");
        }

        auto clientIter = hostRole.clients.begin();
        while (clientIter != hostRole.clients.end()) {
            if (clientIter->receive() != -1) {
                ChatMessagePayload chatMsg;
                while (clientIter->fetch(chatMsg)) {
                    handleChatPacket(chatMsg);
                }
                clientIter++;
            } else {
                clientIter = hostRole.clients.erase(clientIter);
                sendChatMessage("A player left.");
            }
        }
        m_uiContext.updateTextString("num_clients", std::to_string(hostRole.clients.size()) + " client(s).");
    }

    void MainMenu::initializeClientSession() {
        if (!clientRole.client) {
            printf("[padi::content::MainMenu|Client] Starting Client...\n");
            auto hostIp = sf::IpAddress(m_uiContext.getTextString("ip_input"));
            if (hostIp == sf::IpAddress::None) {
                Immediate::setFocus(&m_uiContext, "ip_input");
                printf("[padi::content::MainMenu|Client] Invalid IP address!\n");
                appendChatMessage("Invalid Host address.");
            } else {
                clientRole.client = InOutBox(std::make_shared<sf::TcpSocket>());
                clientRole.client.getSocket().lock()->setBlocking(true);
                if (clientRole.client.getSocket().lock()->connect(hostIp, 42069, sf::seconds(0.5)) ==
                    sf::Socket::Done) {
                    clientRole.client.getSocket().lock()->setBlocking(false);
                    m_uiContext.updateTextString("play", "");
                    m_uiContext.updateTextString("host_play", "Unavailable");
                    m_uiContext.updateTextString("connect", "Disconnect");
                    appendChatMessage("Connected.");
                } else {
                    clientRole.client = InOutBox();
                    Immediate::setFocus(&m_uiContext, "ip_input");
                    printf("[padi::content::MainMenu|Client] Failed to connect.\n");
                    appendChatMessage("Failed to connect.");
                }
            }
        }
    }

    void MainMenu::closeClientSession() {
        if (clientRole.client) {
            clientRole.client.getSocket().lock()->disconnect();
            clientRole.client = InOutBox();
            m_uiContext.updateTextString("connect", "Connect");
            m_uiContext.updateTextString("num_clients", "");
            m_uiContext.updateTextString("connect", "Connect");
            m_uiContext.updateTextString("host_play", "Not Hosting");
            m_uiContext.updateTextString("own_ip", "");
            m_uiContext.updateTextString("play", "Play");
            appendChatMessage("Disconnected.");
        }
    }

    void MainMenu::updateClient() {
        if (clientRole.client.receive() == -1) {
            clientRole.client = InOutBox();
            appendChatMessage("Connection closed.");
            m_uiContext.updateTextString("play", "Play");
        } else {
            ChatMessagePayload chatMsg;
            GameStartPayload gameStart;
            if (clientRole.client.fetch(chatMsg)) { // CHAT
                handleChatPacket(chatMsg);
            } else if (clientRole.client.fetch(gameStart)) { // CHAT
                clientHandleGameStart(gameStart);
            }
        }
    }

    void MainMenu::appendChatMessage(const std::string &msg) {
        m_chat.write(&m_uiContext, msg);
        printf("[padi::content::MainMenu] CHAT '%s'\n", msg.c_str());
    }

    void MainMenu::handleChatPacket(ChatMessagePayload &packet) {
        if (hostRole.active) {
            sendChatMessage(std::string(packet.message, std::min(strlen(packet.message), sizeof(packet.message) / sizeof(char))));
        } else {
            appendChatMessage(std::string(packet.message, std::min(strlen(packet.message), sizeof(packet.message) / sizeof(char))));
        }
    }

    void MainMenu::sendChatMessage(const std::string &msg) {
        sf::Packet packet = PackagePayload(ChatMessagePayload(~0u, msg.c_str()));
        if (clientRole.client) {
            clientRole.client.send(packet);
        } else if (hostRole.active) {
            for (auto &client: hostRole.clients) {
                client.send(packet);
            }
            appendChatMessage(msg);
        } else {
            appendChatMessage(msg);
        }
    }

    void MainMenu::hostStartGame() {
        if (hostRole.active) {
            sf::Packet packet;
            for (auto &client: hostRole.clients) {
                client.send(PackagePayload(packet, GameStartPayload()));
            }
            appendChatMessage("Starting Game...");

            auto inboxes = std::vector<InOutBox>();
            inboxes.reserve(hostRole.clients.size());
            for (const auto &client: hostRole.clients) inboxes.emplace_back(client);

            auto game = std::make_shared<padi::content::HostGame>(inboxes, m_uiContext.getTextString("nick_input"),
                                                                  12345);
            m_next = game;
        }
    }

    void MainMenu::clientHandleGameStart(GameStartPayload const &payload) {
        if (clientRole.client) {
            appendChatMessage("Starting Game...");
            auto game = std::make_shared<padi::content::ClientGame>(clientRole.client,
                                                                    m_uiContext.getTextString("nick_input"));
            m_next = game;
        }
    }

    void MainMenu::drawTutorialUI() {
        if (Immediate::Button(&m_uiContext, "play", {16, 0, 128, 32})) {
            std::vector<InOutBox> nosocks;
            m_next = std::make_shared<padi::content::HostGame>(nosocks, m_uiContext.getTextString("nick_input"),
                                                               LevelGenerator::TutorialSeed);
        }
    }

    void MainMenu::drawSPUI() {
        std::string t = m_uiContext.getTextString("seed_input");
        m_uiContext.updateTextColor("seed_input",
                                    Immediate::isFocused(&m_uiContext, "seed_input") ? sf::Color::Yellow
                                                                                   : sf::Color::White);

        if (Immediate::Button(&m_uiContext, "play", {16, 0, 128, 32})) {
            std::vector<InOutBox> nosocks;
            m_next = std::make_shared<padi::content::HostGame>(nosocks, m_uiContext.getTextString("nick_input"), std::hash<std::string>()(t));
        }
        if (Immediate::TextInput(&m_uiContext, "seed_input", &t, 16, AlphabeticNumericCharacterSet)) {
            m_uiContext.updateTextString("seed_input", t);
        }
    }

    void MainMenu::drawCoopUI() {
        if (Immediate::Button(&m_uiContext, "play", {16, 0, 128, 32})) {
            std::vector<InOutBox> nosocks;
            // TODO
            m_next = std::make_shared<padi::content::HostGame>(nosocks, m_uiContext.getTextString("nick_input"), 1234);
        }
    }

    void MainMenu::drawHostUI() {
        Immediate::Button(&m_uiContext, "play", {16, 0, 128, 32}, true);

        m_uiContext.pushTransform().translate(0, 32);
        Immediate::ScalableSprite(&m_uiContext, {-4, 0, 176, 72}, 0,
                                  m_uiContext.getApollo()->lookupAnim("scalable_border"));
        m_uiContext.updateTextColor("host_play",
                                    Immediate::isFocused(&m_uiContext, "host_play") ? sf::Color::White
                                                                                    : sf::Color(0x999999ff));
        if (!clientRole.client) {
            if (Immediate::Switch(&m_uiContext, "host_switch", {-4, 24, 32, 32}, &hostRole.active)) {
                if (hostRole.active) {
                    initializeHostSession();
                } else {
                    closeHostSession();
                }
            }
            if (hostRole.active) {
                if (Immediate::Button(&m_uiContext, "host_play", {24, 28, 120, 24})) {
                    hostStartGame();
                }
                static const std::string hostUi_ids[2]{"host_play", "host_switch"};
                if (Immediate::isAnyFocused(&m_uiContext, hostUi_ids, 2)) {
                    m_uiContext.updateTextString("own_ip", sf::IpAddress::getLocalAddress().toString());
                } else {
                    m_uiContext.updateTextString("own_ip", "");
                }
            }
        }
        m_uiContext.popTransform();
    }

    void MainMenu::drawClientUI() {
        Immediate::Button(&m_uiContext, "play", {16, 0, 128, 32}, true);

        m_uiContext.pushTransform().translate(0, 32);
        Immediate::ScalableSprite(&m_uiContext, {-4, 0, 176, 56}, 0,
                                  m_uiContext.getApollo()->lookupAnim("scalable_border"));
        if (!(hostRole.active || clientRole.client)) {
            std::string t = m_uiContext.getTextString("ip_input");
            m_uiContext.updateTextColor("ip_input",
                                        Immediate::isFocused(&m_uiContext, "ip_input") ? sf::Color::Yellow
                                                                                       : sf::Color::White);
            if (Immediate::TextInput(&m_uiContext, "ip_input", &t, 15, AddressCharacterSet)) {
                m_uiContext.updateTextString("ip_input", t);
            }
            m_uiContext.updateTextColor("connect",
                                        Immediate::isFocused(&m_uiContext, "connect") ? sf::Color::White
                                                                                      : sf::Color(0x999999ff));
            if (Immediate::Button(&m_uiContext, "connect", {0, 28, 168, 24})) {
                m_uiContext.setFocus(0);
                initializeClientSession();
            }
        } else if (clientRole.client) {
            if (Immediate::Button(&m_uiContext, "connect", {0, 28, 168, 24})) {
                m_uiContext.setFocus(0);
                closeClientSession();
            }
        }
        m_uiContext.popTransform();
    }

} // content