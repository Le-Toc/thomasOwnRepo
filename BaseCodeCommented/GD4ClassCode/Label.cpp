#include "Label.hpp"
#include "Utility.hpp"

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
//again, not 100% sure, but I think this is the label displayed in game for player & enemies???
namespace GUI
{

	Label::Label(const std::string& text, const FontHolder& fonts)
		: mText(text, fonts.get(Fonts::Main), 16)
	{
	}

	bool Label::isSelectable() const
	{
		return false;
	}

	void Label::handleEvent(const sf::Event&)
	{
	}

	void Label::draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		states.transform *= getTransform();
		target.draw(mText, states);
	}

	void Label::setText(const std::string& text)
	{
		mText.setString(text);
	}

}