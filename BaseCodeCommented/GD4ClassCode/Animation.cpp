#include "Animation.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>

//constructor
Animation::Animation()
	: mSprite()
	, mFrameSize()
	, mNumFrames(0)
	, mCurrentFrame(0)
	, mDuration(sf::Time::Zero)
	, mElapsedTime(sf::Time::Zero)
	, mRepeat(false)
{
}
//overload constructor
Animation::Animation(const sf::Texture& texture)
	: mSprite(texture)
	, mFrameSize()
	, mNumFrames(0)
	, mCurrentFrame(0)
	, mDuration(sf::Time::Zero)
	, mElapsedTime(sf::Time::Zero)
	, mRepeat(false)
{
}
//sets texture
void Animation::setTexture(const sf::Texture& texture)
{
	mSprite.setTexture(texture);
}
//pointer to texture
const sf::Texture* Animation::getTexture() const
{
	return mSprite.getTexture();
}
//setter for frame size
void Animation::setFrameSize(sf::Vector2i frameSize)
{
	mFrameSize = frameSize;
}
//getter frame size
sf::Vector2i Animation::getFrameSize() const
{
	return mFrameSize;
}
//set num frames
void Animation::setNumFrames(std::size_t numFrames)
{
	mNumFrames = numFrames;
}
//get num frames
std::size_t Animation::getNumFrames() const
{
	return mNumFrames;
}

void Animation::setDuration(sf::Time duration)
{
	mDuration = duration;
}

sf::Time Animation::getDuration() const
{
	return mDuration;
}

void Animation::setRepeating(bool flag)
{
	mRepeat = flag;
}

bool Animation::isRepeating() const
{
	return mRepeat;
}
//restart animation
void Animation::restart()
{
	mCurrentFrame = 0;
}

bool Animation::isFinished() const
{
	return mCurrentFrame >= mNumFrames;
}
//bounding box size
sf::FloatRect Animation::getLocalBounds() const
{
	return sf::FloatRect(getOrigin(), static_cast<sf::Vector2f>(getFrameSize()));
}
//NOT SURE - diff between local and global bounds
sf::FloatRect Animation::getGlobalBounds() const
{
	return getTransform().transformRect(getLocalBounds());
}
//update moves between animation frames and images on spritesheet I think??
void Animation::update(sf::Time dt)
{
	sf::Time timePerFrame = mDuration / static_cast<float>(mNumFrames);
	mElapsedTime += dt;

	sf::Vector2i textureBounds(mSprite.getTexture()->getSize());
	sf::IntRect textureRect = mSprite.getTextureRect();

	if (mCurrentFrame == 0)
		textureRect = sf::IntRect(0, 0, mFrameSize.x, mFrameSize.y);

	// While we have a frame to process
	while (mElapsedTime >= timePerFrame && (mCurrentFrame <= mNumFrames || mRepeat))
	{
		// Move the texture rect left
		textureRect.left += textureRect.width;

		// If we reach the end of the texture
		if (textureRect.left + textureRect.width > textureBounds.x)
		{
			// Move it down one line
			textureRect.left = 0;
			textureRect.top += textureRect.height;
		}

		// And progress to next frame
		mElapsedTime -= timePerFrame;
		if (mRepeat)
		{
			mCurrentFrame = (mCurrentFrame + 1) % mNumFrames;

			if (mCurrentFrame == 0)
				textureRect = sf::IntRect(0, 0, mFrameSize.x, mFrameSize.y);
		}
		else
		{
			mCurrentFrame++;
		}
	}

	mSprite.setTextureRect(textureRect);
}

void Animation::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();
	target.draw(mSprite, states);
}