#include "CommandQueue.hpp"
#include "SceneNode.hpp"
//functions for custome queue, details in hpp std::queue<Command>

void CommandQueue::push(const Command& command)
{
	mQueue.push(command);
}

Command CommandQueue::pop()
{
	Command command = mQueue.front();
	mQueue.pop();
	return command;
}

bool CommandQueue::isEmpty() const
{
	return mQueue.empty();
}
