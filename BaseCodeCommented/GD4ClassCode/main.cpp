#include "Application.hpp"

#include <stdexcept>
#include <iostream>
//this is the entry point - calls application and has simple try catch
int main()
{
	try {
		Application app;
		app.run();
	}
	catch (std::exception& e)
	{
		std::cout << "Exception: " << e.what() << std::endl;
		std::cin.ignore();
	}
}