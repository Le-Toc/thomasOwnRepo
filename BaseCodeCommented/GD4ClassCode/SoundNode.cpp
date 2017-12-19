#include "SoundNode.hpp"
#include "SoundPlayer.hpp"
//sound note for attaching sounds to events in game?
SoundNode::SoundNode(SoundPlayer& player)
	:SceneNode()
	,mSounds(player)
{

}

void SoundNode::playSound(SoundEffect::ID sound, sf::Vector2f position)
{
	mSounds.play(sound, position);
}

unsigned int SoundNode::getCategory() const
{
	return Category::SoundEffect;
}