#include "State.hpp"
#include "StateStack.hpp"
//state def and constructor

State::Context::Context(sf::RenderWindow& window, TextureHolder& textures, FontHolder& fonts, 
	MusicPlayer& music, SoundPlayer& sounds, KeyBinding& keys1, KeyBinding& keys2)
	: window(&window)
	, textures(&textures)
	, fonts(&fonts)
	, music(&music)
	, sounds(&sounds)
	, keys1(&keys1)
	, keys2(&keys2)
{
}

State::State(StateStack& stack, Context context)
	: mStack(&stack)
	, mContext(context)
{
}

State::~State()
{
}

void State::requestStackPush(States::ID stateID)
{
	mStack->pushState(stateID);
}

void State::requestStackPop()
{
	mStack->popState();
}

void State::requestStackClear()
{
	mStack->clearStates();
}

State::Context State::getContext() const
{
	return mContext;
}

void State::onActivate()
{

}

void State::onDestroy()
{

}
