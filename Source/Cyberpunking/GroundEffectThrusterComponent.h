// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//#include "CoreMinimal.h"
//#include "PhysicsEngine/PhysicsThrusterComponent.h"
//#include "GroundEffectThrusterComponent.generated.h"

/**
 * 
 */
//UCLASS()
class /*CYBERPUNKING_API U*/GroundEffectThrusterComponent// : public UObject/*UPhysicsThrusterComponent*/
{
	//GENERATED_BODY()
public:
	GroundEffectThrusterComponent();

	//UPROPERTY(Category = physx, EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UPhysicsThrusterComponent *thruster;
	
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	FVector lastLocation;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float maxForce=400000;

	//UFUNCTION()
	void updateImpulse();

};
