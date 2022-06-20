//
// Created by Peter on 29/04/2022.
//

#pragma once

#include <map>
#include <string>
#include <memory>
#include "Animation.h"
#include "SFML/Graphics/Shader.hpp"
#include <SFML/Audio/SoundBuffer.hpp>

namespace padi {

    class Apollo;

    class AnimationSet {
    public:
        AnimationSet() = default;
        explicit AnimationSet(std::string  name);

        bool has(const std::string& name) const;

        [[nodiscard]] std::shared_ptr<padi::Animation> const& at(const std::string& name) const;

        [[nodiscard]] std::string const& getName() const;

    private:
        friend class Apollo;
        std::map<std::string, std::shared_ptr<padi::Animation>> m_map;
        std::string m_name;
    };

    typedef std::map<std::string, std::shared_ptr<sf::SoundBuffer>> AudioSet;

    class Apollo {
        /**
         * I just felt like naming it this (:
         */
    public:

        void loadFromFile(std::string const &path);

        bool initializeContext(std::string const &);

        void addAnimation(std::string const &name, std::shared_ptr<padi::Animation> anim);

        bool addAnimation(std::string const &context, std::string const &animName,
                          const std::shared_ptr<padi::Animation> &anim);

        [[nodiscard]] std::shared_ptr<padi::Animation> lookupAnim(std::string const &animName) const;

        [[nodiscard]] std::shared_ptr<padi::Animation>
        lookupAnim(std::string const &ctxName, std::string const &animName) const;

        [[nodiscard]] const padi::AnimationSet *lookupAnimContext(std::string const &charName) const;

        bool loadShader(std::string const &name, std::string const &vert, std::string const &frag);

        bool loadVertexShader(std::string const &name, std::string const &vert);

        bool loadFragmentShader(std::string const &name, std::string const &vert);

        [[nodiscard]] std::shared_ptr<sf::Shader> lookupShader(std::string const &name) const;

        void addSoundBuffer(std::string const &name, std::shared_ptr<sf::SoundBuffer> sound);

        [[nodiscard]] std::shared_ptr<sf::SoundBuffer> lookupAudio(std::string const &audioName) const;

    private:
        AudioSet m_generalAudio;
        AnimationSet m_generalAnimations;
        std::map<std::string, AnimationSet> m_animationContext;
        std::map<std::string, std::shared_ptr<sf::Shader>> m_shaders;
    };


} // padi
