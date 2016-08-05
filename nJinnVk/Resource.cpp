#include "stdafx.hpp"
#include "Resource.hpp"

#include "ResourceManager.hpp"

nJinn::Resource::Resource() :
	mIsLoaded(false)
{}

void nJinn::Resource::finishedLoading()
{
	mIsLoaded = true;
	resourceManager->runCallbacks(this);
}
