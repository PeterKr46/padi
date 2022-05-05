//
// Created by Peter on 29/04/2022.
//

#include "Apollo.h"

#include <utility>
#include <fstream>
#include <sstream>

namespace padi {
    std::shared_ptr<padi::Animation> Apollo::lookupAnim(const std::string &animName) const {
        auto iter = m_generalSet.find(animName);
        if (iter == m_generalSet.end()) return nullptr;
        return iter->second;
    }

    const padi::AnimationSet *Apollo::lookupContext(const std::string &charName) const {
        auto iter = m_contextMap.find(charName);
        if (iter == m_contextMap.end()) return nullptr;
        return &iter->second;
    }

    void Apollo::addAnimation(std::string const &name, std::shared_ptr<padi::Animation> anim) {
        m_generalSet[name] = std::move(anim);
    }

    bool Apollo::initializeContext(std::string const &name) {
        if (m_contextMap.find(name) != m_contextMap.end()) {
            return false;
        } else {
            m_contextMap[name] = {};
            return true;
        }
    }

    bool Apollo::addAnimation(std::string const &name, std::string const &animName,
                              const std::shared_ptr<padi::Animation> &anim) {
        auto ctx = m_contextMap.find(name);
        if (ctx != m_contextMap.end()) {
            ctx->second[animName] = anim;
            return true;
        } else {
            return false;
        }
    }

    std::shared_ptr<padi::Animation> Apollo::lookupAnim(const std::string &ctxName, const std::string &animName) const {
        auto ctx = m_contextMap.find(ctxName);
        if (ctx != m_contextMap.end()) {
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

        AnimationSet *block = &m_generalSet;
        std::string key{};
        if (config.is_open()) {
            while (getline(config, line)) {
                if (line.empty() || line.at(0) == '#') continue;

                if (line.substr(0, 5) == "block") {
                    block = &m_contextMap[line.substr(6)];
                    printf("[Apollo] BLOCK '%s'\n", line.substr(6).c_str());
                } else if (line.substr(0, 4) == "anim") {
                    auto secondspace = line.find(' ', 5);
                    key = line.substr(5,secondspace - 5);
                    printf("[Apollo] ANIM '%s'", key.c_str());
                    std::istringstream ints(line.substr(secondspace));
                    auto data = std::vector<int>(std::istream_iterator<int>(ints),
                                                 std::istream_iterator<int>());

                    if (data.size() < 7) {
                        printf("INVALID (%i values, need at least 7).\n", data.size());
                    } else if (data.size() == 7) {
                        printf(" %i frames.\n", data[6]);
                        block->insert({key,
                                       std::make_shared<SimpleAnimation>(
                                               StripAnimation(sf::Vector2i(data[0], data[1]), {data[2], data[3]},
                                                              {data[4], data[5]}, data[6]))
                                      });
                    } else {
                        printf(" %i frames.\n", data[6]);
                        block->insert({key,
                                       std::make_shared<SimpleAnimation>(
                                               StripAnimation(sf::Vector2i(data[0], data[1]), {data[2], data[3]},
                                                              {data[4], data[5]}, data[6], data[7]))
                                      });
                    }
                }
            }
            config.close();
        }
    }

} // padi