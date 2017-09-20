/**
 * @brief Sample code to play with!
 */

#include <memory>
#include <iostream>
#include <string>
#include <cassert>

#include <eventbus/EventBus.h>
#include <eventbus/EventCollector.h>

namespace Event //Example namespace for events
{

struct Gold { int value = 0; }; //Event that will be proceed when our gold changes

}

enum class Monster
{
	Frog, Tux, Shark
};

class CharacterController // some kind of character controller
{
public:
	CharacterController(const std::shared_ptr<Dexode::EventBus>& eventBus)
			: _bus{eventBus}
	{
	}

	void kill(Monster monsterType)
	{
		if (Monster::Frog == monsterType)
		{
			_gold += 1;
		}
		else if (Monster::Tux == monsterType)
		{
			_gold += 100;
		}
		else if (Monster::Shark == monsterType)
		{
			_gold += 25;
		}
		_bus->notify(Event::Gold{_gold});
	}

private:
	int _gold = 0;//Controller stores how much gold we have
	std::shared_ptr<Dexode::EventBus> _bus;
};

class UIWallet
{
public:
	UIWallet(const std::shared_ptr<Dexode::EventBus>& eventBus)
			: _listener{eventBus}
	{
	}

	void onDraw()//Example "draw" of UI
	{
		std::cout << "Gold:" << _gold << std::endl;
	}

	void onEnter()// We could also do such things in ctor and dtor but a lot of UI has something like this
	{
		_listener.listen<Event::Gold>([this](const auto& event)
									  {
										  _gold = std::to_string(event.value);
									  });
	}

	void onExit()
	{
		_listener.unlistenAll();
	}

private:
	std::string _gold = "0";
	Dexode::EventCollector _listener;
};

class ShopButton // Shop button is only enabled when we have some gold (odd decision but for sample good :P)
{
public:
	ShopButton(const std::shared_ptr<Dexode::EventBus>& eventBus)
			: _listener{eventBus}
	{
	}

	void onEnter()
	{
		//We can use lambda or bind your choice
		_listener.listen<Event::Gold>(std::bind(&ShopButton::onGoldUpdated, this, std::placeholders::_1));
	}

	void onExit()
	{
		_listener.unlistenAll();//unlistenAll is also called when listener is destroyed
	}

	bool isEnabled() const
	{
		return _isEnabled;
	}

private:
	Dexode::EventCollector _listener;
	bool _isEnabled = false;

	void onGoldUpdated(const Event::Gold& event)
	{
		_isEnabled = event.value > 0;
		std::cout << "Shop button is:" << _isEnabled << std::endl;//some kind of logs
	}
};

int main(int argc, char* argv[])
{
	std::shared_ptr<Dexode::EventBus> eventBus = std::shared_ptr<Dexode::EventBus>(new Dexode::EventBus{});

	CharacterController characterController{eventBus};

	UIWallet wallet{eventBus};//UIWallet doesn't know anything about character or even who store gold
	ShopButton shopButton{eventBus};//ShopButton doesn't know anything about character
	{
		wallet.onEnter();
		shopButton.onEnter();
	}

	characterController.kill(Monster::Tux);

	wallet.onDraw();

	//It is easy to test UI eg.
	eventBus->notify(Event::Gold{1});
	assert(shopButton.isEnabled() == true);
	eventBus->notify(Event::Gold{0});
	assert(shopButton.isEnabled() == false);

	return 0;
}
