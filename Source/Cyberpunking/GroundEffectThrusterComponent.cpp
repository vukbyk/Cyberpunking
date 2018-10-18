// Fill out your copyright notice in the Description page of Project Settings.

#include "GroundEffectThrusterComponent.h"
#include "PhysicsEngine/PhysicsThrusterComponent.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineGlobals.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"


UGroundEffectThrusterComponent::UGroundEffectThrusterComponent()
{
}

void UGroundEffectThrusterComponent::updateImpulse()
{
	//Thruster forward where force go (down for now)
	//FlushPersistentDebugLines(GetWorld());
	
	float vertV = FVector::DotProduct((lastLocation - thruster->GetComponentLocation()), -thruster->GetForwardVector()) / GetWorld()->GetDeltaSeconds();
	lastLocation = thruster->GetComponentLocation();

	FVector rayDir = -thruster->GetForwardVector().RotateAngleAxis(FMath::Sign(thruster->RelativeLocation.Y) * 5, thruster->GetUpVector()); //+rotating on basic
	FVector forceDir = -thruster->GetForwardVector();

	DrawDebugDirectionalArrow(GetWorld(), thruster->GetComponentLocation(), forceDir * vertV * 0.1 + thruster->GetComponentLocation(), 120.f, FColor::Purple, false, -1, 2, 5.f);
	float groundEffect = 0;

	FCollisionQueryParams CollisionParams;
	FHitResult OutHit;
	float force=0;
	if (GetWorld()->LineTraceSingleByChannel(OutHit, thruster->GetComponentLocation() - 5.0 * rayDir, -rayDir * 500 + thruster->GetComponentLocation(), ECC_Visibility, CollisionParams))
	{
		if (OutHit.bBlockingHit)
		{
			groundEffect = maxForce - OutHit.Distance * OutHit.Distance * 6;
			force = FMath::Clamp(groundEffect + vertV * 100, 0.0f, maxForce);
		}
	}
	force += vertV * 50;
	force = FMath::Clamp(force, -maxForce, maxForce);
	thruster->ThrustStrength = force;
	DrawDebugDirectionalArrow(GetWorld(), thruster->GetComponentLocation(), forceDir * force * .001 + thruster->GetComponentLocation(), 120.f, FColor::Yellow, false, -1, 2, 5.f);
	DrawDebugDirectionalArrow(GetWorld(), thruster->GetComponentLocation(), -rayDir * 500 + thruster->GetComponentLocation(), 120.f, FColor::Green, false, -1, 2, 5.f);
	//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("%f"), vertV *.01));
}