#include "PauseState.hpp"
#include "Button.hpp"
#include "Utility.hpp"
#include "MusicPlayer.hpp"
#include "ResourceHolder.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
//handling pause with pause state - what happens or doesn't happen when game is paused
//constructor
PauseState::PauseState(StateStack& stack, Context context, bool letUpdatesThrough)
	: State(stack, context)
	, mBackgroundSprite()
	, mPausedText()
	, mGUIContainer()
	, mLetUpdatesThrough(letUpdatesThrough)
{
	sf::Font& font = context.fonts->get(Fonts::Main);
	sf::Vector2f windowSize(context.window->getSize());

	mPausedText.setFont(font);
	//printing pause text
	mPausedText.setString("Game Paused");
	mPausedText.setCharacterSize(70);
	centerOrigin(mPausedText);
	//half way across, and just shy of half down with big char size
	mPausedText.setPosition(0.5f * windowSize.x, 0.4f * windowSize.y);
	//button to resume
	auto returnButton = std::make_shared<GUI::Button>(context);
	returnButton->setPosition(0.5f * windowSize.x - 100, 0.4f * windowSize.y + 75);
	returnButton->setText("Return");
	returnButton->setCallback([this]()
	{
		requestStackPop();
	});
	//go to menu
	auto backToMenuButton = std::make_shared<GUI::Button>(context);
	backToMenuButton->setPosition(0.5f * windowSize.x - 100, 0.4f * windowSize.y + 125);
	backToMenuButton->setText("Back to menu");
	backToMenuButton->setCallback([this]()
	{
		requestStackClear();
		requestStackPush(States::Menu);
	});

	mGUIContainer.pack(returnButton);
	mGUIContainer.pack(backToMenuButton);

	getContext().music->setPaused(true);
}

PauseState::~PauseState()
{
	getContext().music->setPaused(false);
}
//what to draw in pause
void PauseState::draw()
{
	sf::RenderWindow& window = *getContext().window;
	window.setView(window.getDefaultView());

	sf::RectangleShape backgroundShape;
	backgroundShape.setFillColor(sf::Color(0, 0, 0, 150));
	backgroundShape.setSize(window.getView().getSize());

	window.draw(backgroundShape);
	window.draw(mPausedText);
	window.draw(mGUIContainer);
}
//pause with updates, seems like player pause doesn't pause game cuz other players still going? So what does pause in 2 player local do? need overrides for single 
//player??
bool PauseState::update(sf::Time)
{
	return mLetUpdatesThrough;
}

bool PauseState::handleEvent(const sf::Event& event)
{
	mGUIContainer.handleEvent(event);
	return false;
}