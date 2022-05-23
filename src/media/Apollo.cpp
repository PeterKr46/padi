//
// Created by Peter on 29/04/2022.
//

#include "Apollo.h"

#include <utility>
#include <iterator>
#include <fstream>
#include <sstream>

namespace padi {
    std::shared_ptr<padi::Animation> Apollo::lookupAnim(const std::string &animName) const {
        auto iter = m_generalAnimations.find(animName);
        if (iter == m_generalAnimations.end()) return nullptr;
        return iter->second;
    }

    const padi::AnimationSet *Apollo::lookupAnimContext(const std::string &charName) const {
        auto iter = m_animationContext.find(charName);
        if (iter == m_animationContext.end()) return nullptr;
        return &iter->second;
    }

    void Apollo::addAnimation(std::string const &name, std::shared_ptr<padi::Animation> anim) {
        m_generalAnimations[name] = std::move(anim);
    }

    bool Apollo::initializeContext(std::string const &name) {
        if (m_animationContext.find(name) != m_animationContext.end()) {
            return false;
        } else {
            m_animationContext[name] = {};
            return true;
        }
    }

    bool Apollo::addAnimation(std::string const &name, std::string const &animName,
                              const std::shared_ptr<padi::Animation> &anim) {
        auto ctx = m_animationContext.find(name);
        if (ctx != m_animationContext.end()) {
            ctx->second[animName] = anim;
            return true;
        } else {
            return false;
        }
    }

    std::shared_ptr<padi::Animation> Apollo::lookupAnim(const std::string &ctxName, const std::string &animName) const {
        auto ctx = m_animationContext.find(ctxName);
        if (ctx != m_animationContext.end()) {
            auto anim = ctx->second.find(animName);
            if (anim != ctx->second.end()) {
                return anim->second;
            }
        }
        return {nullptr};
    }

    void Apollo::loadFromFile(std::string const &path) {
        std::ifstream config(path);
        std::string line;

        AnimationSet *block = &m_generalAnimations;
        std::string key{};
        if (config.is_open()) {
            while (getline(config, line)) {
                if (line.empty() || line.at(0) == '#') continue;

                if (line.substr(0, 5) == "block") {
                    block = &m_animationContext[line.substr(6)];
                    printf("[Apollo] BLOCK '%s'\n", line.substr(6).c_str());
                } else if (line.substr(0, 4) == "anim") {
                    auto secondspace = line.find(' ', 5);
                    key = line.substr(5, secondspace - 5);
                    printf("[Apollo] ANIM '%s'", key.c_str());
                    std::istringstream ints(line.substr(secondspace));
                    auto data = std::vector<int>(std::istream_iterator<int>(ints),
                                                 std::istream_iterator<int>());

                    if (data.size() == 4) {
                        printf(" static frame.\n");
                        block->insert({key,
                                       std::make_shared<StaticAnimation>(sf::Vector2i(data[0], data[1]),
                                                                         sf::Vector2f(data[2], data[3]))
                                      });
                    } else if (data.size() == 7) {
                        printf(" %i frames.\n", data[6]);
                        block->insert({key,
                                       std::make_shared<SimpleAnimation>(
                                               StripAnimation(sf::Vector2i(data[0], data[1]), {data[2], data[3]},
                                                              {data[4], data[5]}, data[6]))
                                      });
                    } else if (data.size() > 7) {
                        printf(" %i frames.\n", data[6]);
                        block->insert({key,
                                       std::make_shared<SimpleAnimation>(
                                               StripAnimation(sf::Vector2i(data[0], data[1]), {data[2], data[3]},
                                                              {data[4], data[5]}, data[6], data[7]))
                                      });
                    } else {
                        printf("INVALID (%llu values, need at least 7).\n", data.size());
                    }
                } else if (line.substr(0, 5) == "audio") {
                    auto secondspace = line.find(' ', 6);
                    key = line.substr(6, secondspace - 6);
                    auto soundPath = line.substr(line.find_last_of(' ') + 1);
                    printf("[Apollo] AUDIO '%s' - '%s'\n", key.c_str(), soundPath.c_str());
                    auto buf = m_generalAudio[key] = std::make_shared<sf::SoundBuffer>();
                    buf->loadFromFile(soundPath);
                } else if (line.substr(0, 6) == "shader") {
                    auto secondspace = line.find(' ', 7);
                    key = line.substr(7, secondspace - 7);
                    auto shaderPath = line.substr(line.find_last_of(' ') + 1);
                    printf("[Apollo] SHADER '%s' - '%s'\n", key.c_str(), shaderPath.c_str());
                    if (shaderPath.substr(shaderPath.size() - 4) == "vert") {
                        loadVertexShader(key, shaderPath);
                    } else if (shaderPath.substr(shaderPath.size() - 4) == "frag") {
                        loadFragmentShader(key, shaderPath);
                    } else
                        printf("[Apollo] SHADER TYPE UNRECOGNIZED '%s' - '%s'\n", key.c_str(), shaderPath.c_str());
                }

            }
        }
        config.close();
    }

    std::shared_ptr<sf::SoundBuffer> Apollo::lookupAudio(const std::string &audioName) const {
        auto iter = m_generalAudio.find(audioName);
        if (iter == m_generalAudio.end()) return nullptr;
        return iter->second;
    }

    void Apollo::addSoundBuffer(const std::string &name, std::shared_ptr<sf::SoundBuffer> sound) {
        m_generalAudio[name] = std::move(sound);
    }

    bool Apollo::loadShader(const std::string &name, const std::string &vert, const std::string &frag) {
        return m_shaders[name].loadFromFile(vert, frag);
    }

    const sf::Shader *Apollo::lookupShader(const std::string &name) const {
        return &m_shaders.at(name);
    }

    bool Apollo::loadVertexShader(const std::string &name, const std::string &vert) {
        return m_shaders[name].loadFromFile(vert, sf::Shader::Type::Vertex);
    }

    bool Apollo::loadFragmentShader(const std::string &name, const std::string &vert) {
        return m_shaders[name].loadFromFile(vert, sf::Shader::Type::Fragment);
    }

} // padi