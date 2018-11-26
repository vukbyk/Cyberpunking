// Fill out your copyright notice in the Description page of Project Settings.

#include "GroundEffectThrusterComponent.h"
#include "PhysicsEngine/PhysicsThrusterComponent.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineGlobals.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"


GroundEffectThrusterComponent::GroundEffectThrusterComponent()
{
}

void GroundEffectThrusterComponent::updateImpulse()
{
	//Thruster forward where force go (down for now)
	//FlushPersistentDebugLines(GetWorld());
	
	float vertV = FVector::DotProduct((lastLocation - thruster->GetComponentLocation()), -thruster->GetForwardVector()) / thruster->GetWorld()->GetDeltaSeconds();
	lastLocation = thruster->GetComponentLocation();

	float side = FMath::Sign(thruster->RelativeLocation.Y);
	float addAngRay = 6;
	FVector rayDir = -thruster->GetForwardVector().RotateAngleAxis(side * addAngRay, thruster->GetUpVector()); //+rotating on basic
	FVector startRay = thruster->GetComponentLocation() - 5.0 * rayDir + side * -5 * thruster->GetRightVector();
	FVector endRay = -rayDir * 500 + thruster->GetComponentLocation();
	float force = 0;

	FHitResult OutHit;
	FCollisionQueryParams CollisionParams;
	if (thruster->GetWorld()->LineTraceSingleByChannel(OutHit, startRay, endRay, ECC_WorldStatic, CollisionParams))
	{
		if (OutHit.bBlockingHit && OutHit.Distance<=500)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("%f"), OutHit.Distance));
			float groundEffect = maxForce - OutHit.Distance * OutHit.Distance * reduceHoverF;
			float damping = 500;
			force = FMath::Clamp(groundEffect + vertV * damping, 0.0f, maxForce);
		}
	}
	//force += vertV * 10;
	//force = FMath::Clamp(force, 0/*-maxForce*/, maxForce);
	thruster->ThrustStrength = force;

	FVector forceDir = -thruster->GetForwardVector();
	//DrawDebugDirectionalArrow(thruster->GetWorld(), thruster->GetComponentLocation(), forceDir * force * .001 + thruster->GetComponentLocation(), 120.f, FColor::Yellow, false, -1, 2, 5.f);
	DrawDebugDirectionalArrow(thruster->GetWorld(), startRay, endRay, 120.f, FColor::Green, false, -1, 2, 5.f);
	DrawDebugDirectionalArrow(thruster->GetWorld(), thruster->GetComponentLocation(), forceDir * vertV * 0.1 + thruster->GetComponentLocation(), 120.f, FColor::Purple, false, -1, 2, 5.f);
	//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("%f"), vertV *.01));
	//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("4")));
}