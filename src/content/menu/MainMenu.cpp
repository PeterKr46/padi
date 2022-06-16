//
// Created by Peter on 06/05/2022.
//

#include "MainMenu.h"
#include <SFML/Graphics.hpp>
#include "SFML/Network/IpAddress.hpp"
#include "SFML/Network/TcpSocket.hpp"
#include "../../ui/Immediate.h"
#include "../game/Game.h"
#include "../../Controls.h"
#include "SFML/Network/Packet.hpp"
#include "../game/OnlineGame.h"
#include "../../Constants.h"

namespace padi::content {


    MainMenu::MainMenu(sf::RenderTarget *renderTarget, std::string const &apollo, std::string const &spritesheet) {
        m_crt.handleResize(int(float(renderTarget->getSize().x) / renderTarget->getSize().y * 256), 256);
        m_uiContext.init(apollo, spritesheet);
        m_crt.setShader(m_uiContext.getApollo()->lookupShader("fpa"));

        m_uiContext.pushTransform().translate(16, 32);

        m_uiContext.setText("play", "Play", {72, 12}, true);
        m_uiContext.updateTextSize("play", 1.5);
        m_uiContext.updateTextOutline("play", sf::Color::Black, 0.5);

        m_uiContext.pushTransform().translate(0, 32);
        m_uiContext.setText("join_title", "JOIN REMOTE PLAY", {72, 8}, true);
        m_uiContext.setText("ip_label", "IP", {8, 16});
        m_uiContext.setText("ip_input", "127.0.0.1", {24, 16});
        m_uiContext.setText("connect", "Connect", {8, 34});
        m_uiContext.popTransform();

        m_uiContext.pushTransform().translate(0, 90);
        m_uiContext.setText("host_title", "HOST REMOTE PLAY", {72, 8}, true);
        m_uiContext.setText("own_ip", "", {72, 20}, true);
        m_uiContext.setText("host_play", "Offline", {86, 38}, true);
        m_uiContext.setText("num_clients", "", {8, 56});
        m_uiContext.popTransform();

        m_uiContext.pushTransform().translate(0, 170);
        m_uiContext.setText("nick_label", "Nick", {0, 0});
        m_uiContext.setText("nick_input", "Red", {32, 0});
        m_uiContext.popTransform();

        m_uiContext.popTransform();

        m_uiContext.pushTransform().translate(235, 180);
        m_uiContext.setText("chat_title", "CHAT", {0, -12});
        m_uiContext.setText("chat_log", "                  ,-.    \n"
                                                "          ,      ( {o\\   \n"
                                                "          {`\"=,___) (`~ \n"
                                                "           \\  ,_.-   )   \n"
                                                "~^~^~^~^~^~^`- ~^ ~^ '~^~^~^", {0, 0});
        m_uiContext.setText("chat_input", "", {0, 60});
        m_uiContext.popTransform();
    }

    void MainMenu::draw(sf::RenderTarget *target) {
        m_background.getLevel()->update(m_crt.asTarget());
        m_background.getLevel()->centerView({-3, 3});
        m_background.getLevel()->populateVBO();
        m_crt.asTarget()->clear();

        auto states = sf::RenderStates::Default;
        states.transform.scale(
                sf::Vector2f(256.f / m_crt.asTarget()->getView().getSize().y,
                             256.f / m_crt.asTarget()->getView().getSize().y));
        m_crt.asTarget()->draw(m_background, states);

        m_uiContext.nextFrame();

        {
            m_uiContext.pushTransform().translate(16, 32);
            if (!(hostRole.active || clientRole.client)) {
                m_uiContext.updateTextColor("play",
                                            Immediate::isFocused(&m_uiContext, "play") ? sf::Color::White : sf::Color(
                                                    0x999999ff));
                if (Immediate::Button(&m_uiContext, "play", {-6, 0, 152, 32})) {
                    m_next = std::make_shared<padi::content::Game>();
                }
            }
            { // CLIENT CONFIGURATION
                m_uiContext.pushTransform().translate(0, 32);
                Immediate::ScalableSprite(&m_uiContext, {-4, 0, 148, 56}, 0,
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
                    if (Immediate::Button(&m_uiContext, "connect", {0, 28, 140, 24})) {
                        m_uiContext.setFocus(0);
                        initializeClientSession();
                    }
                }
                m_uiContext.popTransform();
            }
            { // HOST CONFIGURATION
                m_uiContext.pushTransform().translate(0, 90);
                Immediate::ScalableSprite(&m_uiContext, {-4, 0, 148, 72}, 0,
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
            // NICKNAME CONFIGURATION
            if (!clientRole.client && !hostRole.active) {
                m_uiContext.pushTransform().translate(0, 170);
                Immediate::ScalableSprite(&m_uiContext, {28, -2, 116, 12}, 0,
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
                m_uiContext.popTransform();
            }
            m_uiContext.popTransform();
        }
        // CHAT
        {
            m_uiContext.pushTransform().translate(235, 180);
            Immediate::ScalableSprite(&m_uiContext, {-4, -4, 208, 60}, 0,
                                      m_uiContext.getApollo()->lookupAnim("scalable_border"));
            Immediate::ScalableSprite(&m_uiContext, {-2, 56, 204, 14}, 0,
                                      m_uiContext.getApollo()->lookupAnim("scalable_textfield"));
            Immediate::ScalableSprite(&m_uiContext, {-4, 54, 208, 16}, 0,
                                      m_uiContext.getApollo()->lookupAnim("scalable_border"),
                                      Immediate::isFocused(&m_uiContext, "chat_input") ? sf::Color::Yellow
                                                                                       : sf::Color::White);
            m_uiContext.popTransform();
            std::string t = m_uiContext.getTextString("chat_input");
            m_uiContext.updateTextColor("chat_input",
                                        Immediate::isFocused(&m_uiContext, "chat_input") ? sf::Color::Yellow
                                                                                         : sf::Color::White);
            if (Immediate::TextInput(&m_uiContext, "chat_input", &t, 18, SimpleCharacterSet)) {
                m_uiContext.updateTextString("chat_input", t);
            }
            if (Immediate::isFocused(&m_uiContext, "chat_input") &&
                padi::Controls::wasKeyReleased(sf::Keyboard::Enter)) {
                sendChatMessage(m_uiContext.getTextString("nick_input") + ": " + t);
                m_uiContext.updateTextString("chat_input", "");
            }
        }
        if (hostRole.active) {
            updateHost();
        }
        if (clientRole.client) {
            updateClient();
        }

        m_crt.asTarget()->draw(m_uiContext);

        //target->setView(m_renderTarget->getDefaultView());
        target->draw(m_crt);
    }

    std::shared_ptr<padi::Activity> MainMenu::handoff() {
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
            m_uiContext.updateTextString("play", "");
            sendChatMessage("Host session started.");
        }
    }

    void MainMenu::closeHostSession() {
        hostRole.listener.close();
        for (auto &client: hostRole.clients) {
            client->disconnect();
        }
        hostRole.clients.clear();
        m_uiContext.updateTextString("num_clients", "");
        m_uiContext.updateTextString("connect", "Connect");
        m_uiContext.updateTextString("host_play", "Offline");
        m_uiContext.updateTextString("own_ip", "");
        m_uiContext.updateTextString("play", "Play");
        sendChatMessage("Host session closed.");
    }

    void MainMenu::updateHost() {
        auto result = hostRole.listener.accept(*hostRole.nextClient);
        if (result == sf::Socket::Done) {
            hostRole.clients.push_back(hostRole.nextClient);
            hostRole.nextClient = std::make_shared<sf::TcpSocket>();
            hostRole.nextClient->setBlocking(false);
            m_uiContext.updateTextString("num_clients", std::to_string(hostRole.clients.size()) + " client(s).");
            sendChatMessage("A player joined.");
        }

        sf::Packet packet;
        for (auto &client: hostRole.clients) {
            result = client->receive(packet);
            if (result == sf::Socket::Done) {
                auto data = reinterpret_cast<const uint8_t *>(packet.getData());
                if (data[0] == 0) { // CHAT
                    handleChatPacket(packet);
                } else {
                    printf("[padi::content::MainMenu] Unknown Packet type %u!\n", data[0]);
                }
            }
        }
    }

    void MainMenu::initializeClientSession() {
        if (!clientRole.client) {
            printf("[padi::content::MainMenu|Client] Starting Client...\n");
            auto hostIp = sf::IpAddress(m_uiContext.getTextString("ip_input"));
            if (hostIp == sf::IpAddress::None) {
                Immediate::setFocus(&m_uiContext, "ip_input");
                printf("[padi::content::MainMenu|Client] Invalid IP address!\n");
                sendChatMessage("Invalid Host address.");
            } else {
                clientRole.client = std::make_shared<sf::TcpSocket>();
                clientRole.client->setBlocking(true);
                if (clientRole.client->connect(hostIp, 42069, sf::seconds(0.5)) == sf::Socket::Done) {
                    clientRole.client->setBlocking(false);
                    m_uiContext.updateTextString("play", "");
                } else {
                    clientRole.client.reset();
                    Immediate::setFocus(&m_uiContext, "ip_input");
                    printf("[padi::content::MainMenu|Client] Failed to connect.\n");
                    sendChatMessage("Failed to connect.");
                }
            }
        }
    }

    void MainMenu::updateClient() {
        sf::Packet packet;
        auto result = clientRole.client->receive(packet);
        if (result == sf::Socket::Disconnected) {
            clientRole.client.reset();
            sendChatMessage("Connection closed.");
            m_uiContext.updateTextString("play", "Play");
        } else if (result == sf::Socket::Done) {
            auto data = reinterpret_cast<const uint8_t *>(packet.getData());
            if (data[0] == 0) { // CHAT
                handleChatPacket(packet);
            } else if (data[0] == 1) {
                clientHandleGameStart(packet);
            } else {
                printf("[padi::content::MainMenu] Unknown Packet type %u!\n", data[0]);
            }
        }
    }

    void MainMenu::appendChatMessage(const std::string &msg) {
        auto chatlog = m_uiContext.getTextString("chat_log");
        chatlog = chatlog.substr(chatlog.find_first_of('\n') + 1) + '\n' + msg;
        m_uiContext.updateTextString("chat_log", chatlog);
        printf("[padi::content::MainMenu] CHAT '%s'\n", msg.c_str());
    }

    void MainMenu::handleChatPacket(sf::Packet &packet) {
        auto data = reinterpret_cast<const uint8_t *>(packet.getData());
        if (packet.getDataSize() < 2) {
            printf("[padi::content::MainMenu] Corrupted CHAT package!\n");
        } else {
            auto len = data[1];
            auto msg = std::string(reinterpret_cast<const char *>(data) + 2, len);
            if (hostRole.active) {
                sendChatMessage(msg);
            } else {
                appendChatMessage(msg);
            }
        }
    }

    void MainMenu::sendChatMessage(const std::string &msg) {
        sf::Packet packet;
        uint8_t type = 0;
        uint8_t len = msg.length();
        packet.append(&type, 1);
        packet.append(&len, 1);
        packet.append(msg.c_str(), msg.length());
        if (clientRole.client) {
            clientRole.client->send(packet);
        } else if (hostRole.active) {
            for (auto &client: hostRole.clients) {
                client->send(packet);
            }
            appendChatMessage(msg);
        } else {
            appendChatMessage(msg);
        }
    }

    void MainMenu::hostStartGame() {
        if (hostRole.active) {
            sf::Packet packet;
            uint8_t type = 1;
            packet.append(&type, 1);
            for (auto &client: hostRole.clients) {
                client->send(packet);
            }
            appendChatMessage("Starting Game...");
            auto game = std::make_shared<padi::content::OnlineGame>(hostRole.clients, true,
                                                                    m_uiContext.getTextString("nick_input"));
            m_next = game;
        }
    }

    void MainMenu::clientHandleGameStart(const sf::Packet &packet) {
        if (clientRole.client) {
            appendChatMessage("Starting Game...");
            auto game = std::make_shared<padi::content::OnlineGame>(std::vector{clientRole.client}, false,
                                                                    m_uiContext.getTextString("nick_input"));
            m_next = game;
        }

    }
} // content