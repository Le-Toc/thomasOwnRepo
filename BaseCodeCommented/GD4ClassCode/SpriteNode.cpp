#include "SpriteNode.hpp"

#include <SFML/Graphics/RenderTarget.hpp>

//sprite node ...how sprites fit into scene graph???
SpriteNode::SpriteNode(const sf::Texture& texture) 
	: mSprite(texture)
{
}

SpriteNode::SpriteNode(const sf::Texture& texture, const sf::IntRect& textureRect) 
	: mSprite(texture, textureRect)
{
}

void SpriteNode::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(mSprite, states);
}