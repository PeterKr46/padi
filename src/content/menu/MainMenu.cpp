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

namespace padi::content {


    MainMenu::MainMenu(sf::RenderTarget *renderTarget, std::string const &apollo, std::string const &spritesheet)
            : m_renderTarget(renderTarget), m_runtime() {
        if (!m_vfxBuffer.create(float(renderTarget->getSize().x) / renderTarget->getSize().y * 256, 256)) {
            printf("[padi::content::MainMenu] Failed to create vfxBuffer.\n");
        }
        m_uiContext.init(apollo, spritesheet);

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

        m_uiContext.pushTransform().translate(0, 168);
        m_uiContext.setText("nick_label", "Nick", {0, 0});
        m_uiContext.setText("nick_input", "Red", {32, 0});
        m_uiContext.popTransform();

        m_uiContext.popTransform();

        m_uiContext.pushTransform().translate(235, 180);
        m_uiContext.setText("chat_title", "CHAT", {0, -12});
        m_uiContext.setText("chat_log", "Welcome!\n\n\n\n\n", {0, 0});
        m_uiContext.setText("chat_input", "", {0, 60});
        m_uiContext.popTransform();
    }

    void MainMenu::draw() {
        m_background.getLevel()->update(&m_vfxBuffer);
        m_background.getLevel()->centerView({-3, 3});
        m_background.getLevel()->populateVBO();
        m_vfxBuffer.clear();

        auto states = sf::RenderStates::Default;
        states.transform.scale(
                sf::Vector2f(256.f / m_vfxBuffer.getView().getSize().y, 256.f / m_vfxBuffer.getView().getSize().y));
        m_vfxBuffer.draw(m_background, states);

        m_uiContext.nextFrame();

        {
            m_uiContext.pushTransform().translate(16, 32);
            if ( !(hostRole.active || clientRole.client)) {
                m_uiContext.updateTextColor("play",Immediate::isFocused(&m_uiContext, "play") ? sf::Color::White : sf::Color(0x999999ff));
                if (Immediate::Button(&m_uiContext, "play", {-6, 0, 152, 32})) {
                    m_next = std::make_shared<padi::content::Game>(m_renderTarget);
                }
            }
            { // CLIENT CONFIGURATION
                m_uiContext.pushTransform().translate(0, 32);
                Immediate::ScalableSprite(&m_uiContext, {-4, 0, 148, 56}, 0,
                                          m_uiContext.getApollo()->lookupAnim("scalable_border"));
                if (!(hostRole.active || clientRole.client)) {
                    std::string t = m_uiContext.getText("ip_input");
                    m_uiContext.updateTextColor("ip_input",
                                                Immediate::isFocused(&m_uiContext, "ip_input") ? sf::Color::Yellow
                                                                                               : sf::Color::White);
                    if (Immediate::TextInput(&m_uiContext, "ip_input", &t, 15)) {
                        m_uiContext.updateTextString("ip_input", t);
                    }
                    m_uiContext.updateTextColor("connect",
                                                Immediate::isFocused(&m_uiContext, "connect") ? sf::Color::White
                                                                                              : sf::Color(0x999999ff));
                    if (Immediate::Button(&m_uiContext, "connect", {0, 28, 140, 24})) {
                        initializeClientSession(); // TODO prevent acting as host AND client
                        m_uiContext.setFocus(0);
                    }
                }
                m_uiContext.popTransform();
            }
            { // HOST CONFIGURATION
                m_uiContext.pushTransform().translate(0, 90);
                Immediate::ScalableSprite(&m_uiContext, {-4, 0, 148, 72}, 0,
                                          m_uiContext.getApollo()->lookupAnim("scalable_border"));
                m_uiContext.updateTextColor("host_play",
                                            Immediate::isFocused(&m_uiContext, "host_play") ? sf::Color::White : sf::Color(0x999999ff));
                if(!clientRole.client) {
                    if (Immediate::Switch(&m_uiContext, "host_switch", {-4, 24, 32, 32}, &hostRole.active)) {
                        if (hostRole.active) {
                            initializeHostSession(); // TODO prevent acting as host AND client
                        } else {
                            closeHostSession();
                        }
                    }
                    if(hostRole.active) {
                        if (Immediate::Button(&m_uiContext, "host_play", {24, 28, 120, 24})) {
                            printf("asd");
                        }
                        if (Immediate::isFocused(&m_uiContext, "host_play")) {
                            m_uiContext.updateTextString("own_ip", sf::IpAddress::getLocalAddress().toString());
                        } else {
                            m_uiContext.updateTextString("own_ip", "<IP Hidden>");
                        }
                    }
                }
                m_uiContext.popTransform();
            }
            m_uiContext.popTransform();
        }
        // NICKNAME CONFIGURATION
        if (!clientRole.client && !hostRole.active) {
            std::string t = m_uiContext.getText("nick_input");
            if (Immediate::isFocused(&m_uiContext, "nick_input")) {
                m_uiContext.updateTextColor("nick_input", sf::Color::Yellow);
            } else {
                m_uiContext.updateTextColor("nick_input", sf::Color::White);
                if (t.empty()) {
                    t = "Red";
                    m_uiContext.updateTextString("nick_input", t);
                }
            }
            if (Immediate::TextInput(&m_uiContext, "nick_input", &t, 8)) {
                m_uiContext.updateTextString("nick_input", t);
            }
        }
        // CHAT
        {
            m_uiContext.pushTransform().translate(235, 180);
            Immediate::ScalableSprite(&m_uiContext, {-4, -4, 208, 60}, 0,
                                      m_uiContext.getApollo()->lookupAnim("scalable_border"));
            Immediate::ScalableSprite(&m_uiContext, {-4, 54, 208, 16}, 0,
                                      m_uiContext.getApollo()->lookupAnim("scalable_border"),
                                      Immediate::isFocused(&m_uiContext, "chat_input") ? sf::Color::Yellow
                                                                                       : sf::Color::White);
            m_uiContext.popTransform();
            std::string t = m_uiContext.getText("chat_input");
            m_uiContext.updateTextColor("chat_input",
                                        Immediate::isFocused(&m_uiContext, "chat_input") ? sf::Color::Yellow
                                                                                         : sf::Color::White);
            if (Immediate::TextInput(&m_uiContext, "chat_input", &t, 18)) {
                m_uiContext.updateTextString("chat_input", t);
            }
            if (Immediate::isFocused(&m_uiContext, "chat_input") &&
                padi::Controls::wasKeyReleased(sf::Keyboard::Enter)) {
                sendChatMessage(m_uiContext.getText("nick_input") + ": " + t);
                m_uiContext.updateTextString("chat_input", "");
            }
        }
        if (hostRole.active) {
            updateHost();
        }
        if (clientRole.client) {
            updateClient();
        }

        m_vfxBuffer.draw(*this);

        auto rState = sf::RenderStates::Default;
        auto shader = m_background.getLevel()->getApollo()->lookupShader("fpa");
        shader->setUniform("time", m_runtime.getElapsedTime().asSeconds());
        shader->setUniform("paused", m_background.getLevel()->isPaused());
        rState.shader = shader.get();
        rState.texture = &m_vfxBuffer.getTexture();
        m_renderTarget->setView(m_renderTarget->getDefaultView());
        m_renderTarget->draw(m_screenQuad, rState);
    }

    void MainMenu::draw(sf::RenderTarget &target, sf::RenderStates states) const {
        m_uiContext.draw(target, states);
        states.transform.translate(target.getView().getCenter() - target.getView().getSize() / 2.f);
        // TODO
        states.transform.scale(sf::Vector2f(target.getView().getSize().y / 256, target.getView().getSize().y / 256));
    }

    std::shared_ptr<padi::Activity> MainMenu::handoff() {
        return m_next ? m_next : shared_from_this();
    }

    void MainMenu::handleResize(int width, int height) {
        sf::Vector2f halfSize{float(width), float(height)};
        halfSize /= 2.f;
        sf::Vector2f imgSize{m_vfxBuffer.getSize()};
        m_screenQuad[0].position = {0, 0};
        m_screenQuad[0].texCoords = {0, imgSize.y};

        m_screenQuad[1].position = {0, float(height)};
        m_screenQuad[1].texCoords = {0, 0};

        m_screenQuad[2].position = {float(width), float(height)};
        m_screenQuad[2].texCoords = {imgSize.x, 0};

        m_screenQuad[3].position = {float(width), 0};
        m_screenQuad[3].texCoords = imgSize;
    }

    void MainMenu::initializeHostSession() {
        hostRole.listener.setBlocking(false);
        printf("[padi::content::MainMenu] Begin Hosting!\n");
        if (hostRole.listener.listen(42069) != sf::Socket::Done) {
            printf("[padi::content::MainMenu] Error setting up TCP Listener.\n");
            m_uiContext.updateTextString("num_clients", "Host setup failed.");
            hostRole.active = false;
        } else {
            hostRole.nextClient = std::make_shared<sf::TcpSocket>();
            hostRole.nextClient->setBlocking(false);
            printf("[padi::content::MainMenu] TCP Listener active.\n");
            m_uiContext.updateTextString("num_clients", "Host online!");
            m_uiContext.updateTextString("connect", "Unavailable");
            m_uiContext.updateTextString("host_play", "Start playing");
        }
    }

    void MainMenu::closeHostSession() {
        printf("[padi::content::MainMenu] Stop Hosting!\n");
        hostRole.listener.close();
        for (auto &client: hostRole.clients) {
            client->disconnect();
        }
        hostRole.clients.clear();
        m_uiContext.updateTextString("num_clients", "");
        m_uiContext.updateTextString("connect", "Connect");
        m_uiContext.updateTextString("host_play", "Offline");
        printf("[padi::content::MainMenu] TCP Listener inactive.\n");
        sendChatMessage("Host session closed.");
    }

    void MainMenu::updateHost() {
        auto result = hostRole.listener.accept(*hostRole.nextClient);
        if (result == sf::Socket::Done) {
            printf("[padi::content::MainMenu] Accepted a new client!\n");
            printf("[padi::content::MainMenu] Remote IP: %s\n",
                   hostRole.nextClient->getRemoteAddress().toString().c_str());
            hostRole.clients.push_back(hostRole.nextClient);
            hostRole.nextClient = std::make_shared<sf::TcpSocket>();
            hostRole.nextClient->setBlocking(false);
            m_uiContext.updateTextString("num_clients", std::to_string(hostRole.clients.size()) + " clients");
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
            printf("[padi::content::MainMenu] Start Client!\n");
            auto hostIp = sf::IpAddress(m_uiContext.getText("ip_input"));
            if (hostIp == sf::IpAddress::None) {
                Immediate::setFocus(&m_uiContext, "ip_input");
                printf("[padi::content::MainMenu] Invalid IP Address!\n");
            } else {
                clientRole.client = std::make_shared<sf::TcpSocket>();
                clientRole.client->setBlocking(true);
                if (clientRole.client->connect(hostIp, 42069, sf::seconds(0.5)) == sf::Socket::Done) {
                    printf("[padi::content::MainMenu] Client connected!\n");
                    clientRole.client->setBlocking(false);
                } else {
                    printf("[padi::content::MainMenu] Client failed to connect.\n");
                }
            }
        }
    }

    void MainMenu::updateClient() {
        sf::Packet packet;
        auto result = clientRole.client->receive(packet);
        if (result == sf::Socket::Disconnected) {
            clientRole.client.reset();
            printf("[padi::content::MainMenu] Client is disconnected!\n");
            sendChatMessage("Connection closed.");
        } else if (result == sf::Socket::Done) {
            auto data = reinterpret_cast<const uint8_t *>(packet.getData());
            if (data[0] == 0) { // CHAT
                handleChatPacket(packet);
            } else {
                printf("[padi::content::MainMenu] Unknown Packet type %u!\n", data[0]);
            }
        }
    }

    void MainMenu::appendChatMessage(const std::string &msg) {
        auto chatlog = m_uiContext.getText("chat_log");
        chatlog = chatlog.substr(chatlog.find_first_of('\n') + 1) + '\n' + msg;
        m_uiContext.updateTextString("chat_log", chatlog);
    }

    void MainMenu::handleChatPacket(sf::Packet &packet) {
        auto data = reinterpret_cast<const uint8_t *>(packet.getData());
        if (packet.getDataSize() < 2) {
            printf("[padi::content::MainMenu] Corrupted CHAT package!\n");
        } else {
            auto len = data[1];
            auto msg = std::string(reinterpret_cast<const char *>(data) + 2, len);
            if(hostRole.active) {
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
} // content