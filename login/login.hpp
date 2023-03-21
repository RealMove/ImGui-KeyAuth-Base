#pragma once
#include "auth.hpp"
#include "../protection/skStr.h"
#include <iostream>
#include <string>

using namespace KeyAuth;

std::string name = skCrypt("").decrypt(); // application name. right above the blurred text aka the secret on the licenses tab among other tabs
std::string ownerid = skCrypt("").decrypt(); // ownerid, found in account settings. click your profile picture on top right of dashboard and then account settings.
std::string secret = skCrypt("").decrypt(); // app secret, the blurred text on licenses tab and other tabs
std::string version = skCrypt("1.0").decrypt(); // leave alone unless you've changed version on website
std::string url = skCrypt("https://keyauth.win/api/1.2/").decrypt(); // change if you're self-hosting

api KeyAuthApp(name, ownerid, secret, version, url);

void login(std::string key)
{
	// Main login function, if you wanna add something different you can change here
	KeyAuthApp.init();
	if (KeyAuthApp.checkblack()) {
		abort();
	}
	KeyAuthApp.license(key);
	if (!KeyAuthApp.data.success)
	{
		std::cout << skCrypt("\n Status: ") << KeyAuthApp.data.message;
		Sleep(1500);
		exit(0);
	}
}