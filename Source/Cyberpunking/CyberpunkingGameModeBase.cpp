// Fill out your copyright notice in the Description page of Project Settings.

#include "CyberpunkingGameModeBase.h"
#include "Hoverer.h"

ACyberpunkingGameModeBase::ACyberpunkingGameModeBase()
{

	DefaultPawnClass = AHoverer::StaticClass();
	//PlayerControllerClass = ASomePlayerController::StaticClass();
}


