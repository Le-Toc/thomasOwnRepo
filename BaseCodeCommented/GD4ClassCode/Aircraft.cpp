#include "Aircraft.hpp"
#include "DataTables.hpp"
#include "Utility.hpp"
#include "Pickup.hpp"
#include "CommandQueue.hpp"
#include "SoundNode.hpp"
#include "NetworkNode.hpp"
#include "ResourceHolder.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

#include <cmath>

using namespace std::placeholders;

//anonymous namespace
namespace
{
	const std::vector<AircraftData> Table = initializeAircraftData();
}
/*Aircraft constructor - aircraft type, textureholder, fontholder, hitpoints from table, sprite (specific texture from table),
explosion texture, etc */
Aircraft::Aircraft(Type type, const TextureHolder& textures, const FontHolder& fonts)
	: Entity(Table[type].hitpoints)
	, mType(type)
	, mSprite(textures.get(Table[type].texture), Table[type].textureRect)
	, mExplosion(textures.get(Textures::Explosion))
	, mFireCommand()
	, mMissileCommand()
	, mFireCountdown(sf::Time::Zero)
	, mIsFiring(false)
	, mIsLaunchingMissile(false)
	, mShowExplosion(true)
	, mExplosionBegan(false)
	, mSpawnedPickup(false)
	, mPickupsEnabled(true)
	, mFireRateLevel(1)
	, mSpreadLevel(1)
	, mMissileAmmo(2)
	, mDropPickupCommand()
	, mTravelledDistance(0.f)
	, mDirectionIndex(0)
	, mMissileDisplay(nullptr)
	, mIdentifier(0)
{
	//explosion size in pixels?
	mExplosion.setFrameSize(sf::Vector2i(256, 256));
	//animation played out in N frames
	mExplosion.setNumFrames(16);
	//time of expl animation
	mExplosion.setDuration(sf::seconds(1));
	//center of sprite for transformations
	centerOrigin(mSprite);
	//center of sprite for transformations
	centerOrigin(mExplosion);
	//this happens in airLayer
	mFireCommand.category = Category::SceneAirLayer;
	//something happens in the game screen over time?
	mFireCommand.action = [this, &textures](SceneNode& node, sf::Time)
	{
		//bullets fired: location and texture
		createBullets(node, textures);
	};

	mMissileCommand.category = Category::SceneAirLayer;
	//missile fired, instanciated where and when with texture...
	mMissileCommand.action = [this, &textures](SceneNode& node, sf::Time)
	{
		createProjectile(node, Projectile::Missile, 0.f, 0.5f, textures);
	};

	//destroyed aircraft drops something - in the air layer so can collide with player
	mDropPickupCommand.category = Category::SceneAirLayer;
	mDropPickupCommand.action = [this, &textures](SceneNode& node, sf::Time)
	{
		createPickup(node, textures);
	};
	//health display for each note, attached as child to target node
	std::unique_ptr<TextNode> healthDisplay(new TextNode(fonts, ""));
	mHealthDisplay = healthDisplay.get();
	attachChild(std::move(healthDisplay));
	//additional details exposed for players
	if (getCategory() == Category::PlayerAircraft)
	{
		std::unique_ptr<TextNode> missileDisplay(new TextNode(fonts, ""));
		missileDisplay->setPosition(0, 70);
		mMissileDisplay = missileDisplay.get();
		attachChild(std::move(missileDisplay));
	}

	updateTexts();
}
//getter for num of missiles
int Aircraft::getMissileAmmo() const
{
	return mMissileAmmo;
}
//setter
void Aircraft::setMissileAmmo(int ammo)
{
	mMissileAmmo = ammo;
}
//draw current aircraft - if aircraft is destroyed, draw explosion; else draw sprite
void Aircraft::drawCurrent(sf::RenderTarget& target, sf::RenderStates states) const
{
	if (isDestroyed() && mShowExplosion)
		target.draw(mExplosion, states);
	else
		target.draw(mSprite, states);
}
//not sure, think this is so enemies don't pickup stuff???
void Aircraft::disablePickups()
{
	mPickupsEnabled = false;
}
//update aircraft nodes, based on time and commands??
void Aircraft::updateCurrent(sf::Time dt, CommandQueue& commands)
{
	// Update texts and roll animation
	updateTexts();
	updateRollAnimation();

	// Entity has been destroyed: Possibly drop pickup, mark for removal
	if (isDestroyed())
	{
		checkPickupDrop(commands);
		mExplosion.update(dt);

		// Play explosion sound only once
		if (!mExplosionBegan)
		{
			SoundEffect::ID soundEffect = (randomInt(2) == 0) ? SoundEffect::Explosion1 : SoundEffect::Explosion2;
			playLocalSound(commands, soundEffect);

			// Emit network game action for enemy explosions
			if (!isAllied())
			{
				sf::Vector2f position = getWorldPosition();

				Command command;
				command.category = Category::Network;
				command.action = derivedAction<NetworkNode>([position](NetworkNode& node, sf::Time)
				{
					node.notifyGameAction(GameActions::EnemyExplode, position);
				});

				commands.push(command);
			}

			mExplosionBegan = true;
		}
		return;
	}

	// Check if bullets or missiles are fired
	checkProjectileLaunch(dt, commands);

	// Update enemy movement pattern; apply velocity
	updateMovementPattern(dt);
	Entity::updateCurrent(dt, commands);
}
//gets aircraft faction
unsigned int Aircraft::getCategory() const
{
	if (isAllied())
		return Category::PlayerAircraft;
	else
		return Category::EnemyAircraft;
}
//get sprite bounding box
sf::FloatRect Aircraft::getBoundingRect() const
{
	return getWorldTransform().transformRect(mSprite.getGlobalBounds());
}
//garbage collect at end of boom!
bool Aircraft::isMarkedForRemoval() const
{
	return isDestroyed() && (mExplosion.isFinished() || !mShowExplosion);
}
//trash node
void Aircraft::remove()
{
	Entity::remove();
	mShowExplosion = false;
}

bool Aircraft::isAllied() const
{
	return mType == Eagle;
}

float Aircraft::getMaxSpeed() const
{
	return Table[mType].speed;
}
//pickup buff
void Aircraft::increaseFireRate()
{
	if (mFireRateLevel < 10)
		++mFireRateLevel;
}
//pickup buff
void Aircraft::increaseSpread()
{
	if (mSpreadLevel < 3)
		++mSpreadLevel;
}
//pickup increase missile ammo
void Aircraft::collectMissiles(unsigned int count)
{
	mMissileAmmo += count;
}
//shoot gun
void Aircraft::fire()
{
	// Only ships with fire interval != 0 are able to fire
	if (Table[mType].fireInterval != sf::Time::Zero)
		mIsFiring = true;
}
//fire missile if available
void Aircraft::launchMissile()
{
	if (mMissileAmmo > 0)
	{
		mIsLaunchingMissile = true;
		--mMissileAmmo;
	}
}
//playy sounds
void Aircraft::playLocalSound(CommandQueue& commands, SoundEffect::ID effect)
{
	sf::Vector2f worldPosition = getWorldPosition();

	Command command;
	command.category = Category::SoundEffect;
	command.action = derivedAction<SoundNode>(
		[effect, worldPosition](SoundNode& node, sf::Time)
	{
		node.playSound(effect, worldPosition);
	});

	commands.push(command);
}

int	Aircraft::getIdentifier()
{
	return mIdentifier;
}

void Aircraft::setIdentifier(int identifier)
{
	mIdentifier = identifier;
}
//enemy movement AI
void Aircraft::updateMovementPattern(sf::Time dt)
{
	// Enemy airplane: Movement pattern
	const std::vector<Direction>& directions = Table[mType].directions;
	if (!directions.empty())
	{
		// Moved long enough in current direction: Change direction
		if (mTravelledDistance > directions[mDirectionIndex].distance)
		{
			mDirectionIndex = (mDirectionIndex + 1) % directions.size();
			mTravelledDistance = 0.f;
		}

		// Compute velocity from direction
		float radians = toRadian(directions[mDirectionIndex].angle + 90.f);
		float vx = getMaxSpeed() * std::cos(radians);
		float vy = getMaxSpeed() * std::sin(radians);

		setVelocity(vx, vy);

		mTravelledDistance += getMaxSpeed() * dt.asSeconds();
	}
}
//is there a pickup?
void Aircraft::checkPickupDrop(CommandQueue& commands)
{
	if (!isAllied() && randomInt(3) == 0 && !mSpawnedPickup)
		commands.push(mDropPickupCommand);

	mSpawnedPickup = true;
}

void Aircraft::checkProjectileLaunch(sf::Time dt, CommandQueue& commands)
{
	// Enemies try to fire all the time
	if (!isAllied())
		fire();

	// Check for automatic gunfire, allow only in intervals
	if (mIsFiring && mFireCountdown <= sf::Time::Zero)
	{
		// Interval expired: We can fire a new bullet
		commands.push(mFireCommand);
		playLocalSound(commands, isAllied() ? SoundEffect::AlliedGunfire : SoundEffect::EnemyGunfire);

		mFireCountdown += Table[mType].fireInterval / (mFireRateLevel + 1.f);
		mIsFiring = false;
	}
	else if (mFireCountdown > sf::Time::Zero)
	{
		// Interval not expired: Decrease it further
		mFireCountdown -= dt;
		mIsFiring = false;
	}

	// Check for missile launch
	if (mIsLaunchingMissile)
	{
		commands.push(mMissileCommand);
		playLocalSound(commands, SoundEffect::LaunchMissile);

		mIsLaunchingMissile = false;
	}
}
//start shooting - creating bullets - calls createProjectile
void Aircraft::createBullets(SceneNode& node, const TextureHolder& textures) const
{
	//lambda
	Projectile::Type type = isAllied() ? Projectile::AlliedBullet : Projectile::EnemyBullet;

	switch (mSpreadLevel)
	{
	case 1:
		createProjectile(node, type, 0.0f, 0.5f, textures);
		break;

	case 2:
		createProjectile(node, type, -0.33f, 0.33f, textures);
		createProjectile(node, type, +0.33f, 0.33f, textures);
		break;

	case 3:
		createProjectile(node, type, -0.5f, 0.33f, textures);
		createProjectile(node, type, 0.0f, 0.5f, textures);
		createProjectile(node, type, +0.5f, 0.33f, textures);
		break;
	}
}
//more creating bullets - this is the core shit here
void Aircraft::createProjectile(SceneNode& node, Projectile::Type type, float xOffset, float yOffset, const TextureHolder& textures) const
{
	std::unique_ptr<Projectile> projectile(new Projectile(type, textures));

	sf::Vector2f offset(xOffset * mSprite.getGlobalBounds().width, yOffset * mSprite.getGlobalBounds().height);
	sf::Vector2f velocity(0, projectile->getMaxSpeed());

	float sign = isAllied() ? -1.f : +1.f;
	projectile->setPosition(getWorldPosition() + offset * sign);
	projectile->setVelocity(velocity * sign);
	node.attachChild(std::move(projectile));
}
//make a pickup
void Aircraft::createPickup(SceneNode& node, const TextureHolder& textures) const
{
	auto type = static_cast<Pickup::Type>(randomInt(Pickup::TypeCount));

	std::unique_ptr<Pickup> pickup(new Pickup(type, textures));
	pickup->setPosition(getWorldPosition());
	pickup->setVelocity(0.f, 1.f);
	node.attachChild(std::move(pickup));
}
//updating aircraft texts
void Aircraft::updateTexts()
{
	// Display hitpoints
	if (isDestroyed())
		mHealthDisplay->setString("");
	else
		mHealthDisplay->setString(toString(getHitpoints()) + " HP");
	mHealthDisplay->setPosition(0.f, 50.f);
	mHealthDisplay->setRotation(-getRotation());

	// Display missiles, if available
	if (mMissileDisplay)
	{
		if (mMissileAmmo == 0 || isDestroyed())
			mMissileDisplay->setString("");
		else
			mMissileDisplay->setString("M: " + toString(mMissileAmmo));
	}
}
//roll animation
void Aircraft::updateRollAnimation()
{
	if (Table[mType].hasRollAnimation)
	{
		sf::IntRect textureRect = Table[mType].textureRect;

		// Roll left: Texture rect offset once
		if (getVelocity().x < 0.f)
			textureRect.left += textureRect.width;

		// Roll right: Texture rect offset twice
		else if (getVelocity().x > 0.f)
			textureRect.left += 2 * textureRect.width;

		mSprite.setTextureRect(textureRect);
	}
}