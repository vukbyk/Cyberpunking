// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Hoverer.generated.h"

UCLASS()
class CYBERPUNKING_API AHoverer : public APawn
{
	GENERATED_BODY()

	// StaticMesh component that will be the visuals for our flying pawn
	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent *StaticMeshComponent;
	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USceneComponent *offsetHMD;
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent *SpringArm;
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent *Camera;

	//UPROPERTY(Category = physx, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	//class UPhysicsThrusterComponent *MainThrusterComponent;

	UPROPERTY(Category = physx, EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UPhysicsThrusterComponent *thrusterLF;
	UPROPERTY(Category = physx, EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UPhysicsThrusterComponent *thrusterRF;
	UPROPERTY(Category = physx, EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UPhysicsThrusterComponent *thrusterLB;
	UPROPERTY(Category = physx, EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UPhysicsThrusterComponent *thrusterRB;
	UFUNCTION()
	void updateHoverThruster(UPhysicsThrusterComponent *thruster);
	UFUNCTION()
	void updateHoverImpulses(float offsetFB, float offsetLR, float offsetH, float localVertSpeed = 0);

public:
	// Sets default values for this pawn's properties
	AHoverer();


	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float maxForwardForce = 150000;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float maxHoverForce = 300000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float torqeuYawCoefficient   = 20000000;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float torqeuRollCoefficient  = 13000000;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float torqeuPitchCoefficient = 20000000;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float torqueY;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float mass = 200;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float health = 100;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	FVector springArmOfset = FVector(-100.0f, 0.0f, 180.0f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	float currentThrottle = 0;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/* Component to specify origin for the HMD */
	UPROPERTY(VisibleAnywhere, Category = "VR")
	USceneComponent* VROriginComp;
	UPROPERTY(EditDefaultsOnly, Category = "VR")
	bool bPositionalHeadTracking;

	/* Motion Controllers */
	//UPROPERTY(EditDefaultsOnly, Category = "VR")
	UPROPERTY(Category = "VR", VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UHeadMountedDisplayFunctionLibrary *headMD;
	//UPROPERTY(EditDefaultsOnly, Category = "VR")
	UPROPERTY(Category = "VR", VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMotionControllerComponent *LeftMotionController;
	//UPROPERTY(EditDefaultsOnly, Category = "VR")
	UPROPERTY(Category = "VR", VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UMotionControllerComponent *RightMotionController;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Gameplay)
	float pitchThrottle;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Gameplay)
	float rollThrottle;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Gameplay)
	float yawThrottle;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Gameplay)
	FVector thrusterOffset;

	//UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Gameplay)
	class GroundEffectThrusterComponent *groundEffectLF;
	//UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Gameplay)
	class GroundEffectThrusterComponent *groundEffectRF;
	//UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Gameplay)
	class GroundEffectThrusterComponent *groundEffectLB;
	//UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Gameplay)
	class GroundEffectThrusterComponent *groundEffectRB;

	//UFUNCTION()
	//void DoDamage(float damageValue);
	//UFUNCTION()
	//float getHealth();
	FORCEINLINE class UStaticMeshComponent* GetMesh() const { return StaticMeshComponent; }
	FORCEINLINE class USpringArmComponent* GetSpringArm() const { return SpringArm; }
	FORCEINLINE class UCameraComponent* GetCamera() const { return Camera; }

	//// Spawning class
	//UPROPERTY(EditDefaultsOnly, Category = Projectile)
	//class TSubclassOf<class AProjectile> ProjectileClass;
	// Sound to play each time we fire
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Projectile)
	//class USoundBase *FireSound;
	// Spawning point for projectiles 
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	//	float MuzzleOffset = 250;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void moveX(float AxisValue);
	UFUNCTION()
	void yaw(float AxisValue);
	UFUNCTION()
	void roll(float AxisValue);
	UFUNCTION()
	void pitch(float AxisValue);
	UFUNCTION()
	void ResetHMDOrigin();// Resets HMD Origin position and orientation
	UFUNCTION()
	FString getHMDType();
	UFUNCTION()
	void SetupVROptions();

	virtual void Destroyed() override;

	//UFUNCTION(Reliable, Server, WithValidation)
	//void ServerMoveX(float value);
	//void ServerMoveX_Implementation(float value);
	//bool ServerMoveX_Validate(float value);
	//UFUNCTION(Reliable, Server, WithValidation)
	//void ServerRoll(float value);
	//void ServerRoll_Implementation(float value);
	//bool ServerRoll_Validate(float value);
	//UFUNCTION(Reliable, Server, WithValidation)
	//void ServerYaw(float value);
	//void ServerYaw_Implementation(float value);
	//bool ServerYaw_Validate(float value);
	//UFUNCTION(Reliable, Server, WithValidation)
	//void ServerPitch(float value);
	//void ServerPitch_Implementation(float value);
	//bool ServerPitch_Validate(float value);

	//UFUNCTION()
	//void OnFire();
	//UFUNCTION()
	//void SpawnProjectile();

	//UFUNCTION(Reliable, Server, WithValidation)
	//void ServerFire();
	//void ServerFire_Implementation();
	//bool ServerFire_Validate();
};
