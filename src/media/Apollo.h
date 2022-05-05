//
// Created by Peter on 29/04/2022.
//

#pragma once

#include <map>
#include <string>
#include <memory>
#include "Animation.h"
#include <SFML/Audio/SoundBuffer.hpp>

namespace padi {

    typedef std::map<std::string, std::shared_ptr<padi::Animation>> AnimationSet;
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

        [[nodiscard]] std::shared_ptr<padi::Animation> lookupAnim(std::string const &ctxName, std::string const &animName) const;


        void addSoundBuffer(std::string const &name, std::shared_ptr<sf::SoundBuffer> sound);

        [[nodiscard]] std::shared_ptr<sf::SoundBuffer> lookupAudio(std::string const &audioName) const;

        [[nodiscard]] const padi::AnimationSet *lookupAnimContext(std::string const &charName) const;

    private:
        AudioSet m_generalAudio;
        AnimationSet m_generalAnimations;
        std::map<std::string, AnimationSet> m_animationContext;
    };


} // padi
