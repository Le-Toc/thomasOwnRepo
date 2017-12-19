#include "MusicPlayer.hpp"
//musicplayer constructor, member music, member music filenames, member music volume
MusicPlayer::MusicPlayer()
	:mMusic()
	, mFilenames()
	, mVolume(100.f)
{
	mFilenames[Music::MenuTheme] = "Media/Music/MenuTheme.ogg";
	mFilenames[Music::MissionTheme] = "Media/Music/MissionTheme.ogg";
}
//play theme with catch/throw - loop n play music
void MusicPlayer::play(Music::ID theme)
{
	std::string filename = mFilenames[theme];
	if (!mMusic.openFromFile(filename))
	{
		throw std::runtime_error("Music " + filename + " could not be loaded");
	}
	mMusic.setVolume(mVolume);
	mMusic.setLoop(true);
	mMusic.play();
}

void MusicPlayer::stop()
{
	mMusic.stop();
}

void MusicPlayer::setVolume(float volume)
{
	mVolume = volume;
}
//pause music if game paused
void MusicPlayer::setPaused(bool paused)
{
	if (paused)
	{
		mMusic.pause();
	}
	else
	{
		mMusic.play();
	}
}