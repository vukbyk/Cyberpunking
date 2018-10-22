// Fill out your copyright notice in the Description page of Project Settings.

#include "Hoverer.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/GameFramework/Actor.h"
#include "PhysicsEngine/PhysicsThrusterComponent.h"
#include "DrawDebugHelpers.h"
//#include "Projectile.h"

#include "EngineGlobals.h"
#include "GameFramework/WorldSettings.h"
#include "IHeadMountedDisplay.h"
#include "IXRTrackingSystem.h"
#include "ISpectatorScreenController.h"
#include "IXRSystemAssets.h"
#include "Components/PrimitiveComponent.h"
#include "Features/IModularFeatures.h"
#include "XRMotionControllerBase.h"

/* VR Includes */
#include "HeadMountedDisplay.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "IHeadMountedDisplay.h"
#include "XRMotionControllerBase.h"
#include "MotionControllerComponent.h"

//mine classes
#include "GroundEffectThrusterComponent.h"


AHoverer::AHoverer()
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinderOptional<UStaticMesh> Mesh;
		FConstructorStatics() : Mesh(TEXT("/Game/Meshes/Hoverboard/Hoverboard.Hoverboard"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	// Create static mesh component
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainMesh"));
	RootComponent = StaticMeshComponent;
	StaticMeshComponent->SetStaticMesh(ConstructorStatics.Mesh.Get());	// Set static mesh
	StaticMeshComponent->SetIsReplicated(true);
	//Physics
	StaticMeshComponent->SetSimulatePhysics(true);
	StaticMeshComponent->SetEnableGravity(true);
	StaticMeshComponent->SetMassOverrideInKg(NAME_None, mass, true);
	StaticMeshComponent->SetLinearDamping(.2);
	StaticMeshComponent->SetAngularDamping(3);
	StaticMeshComponent->BodyInstance.InertiaTensorScale = FVector( .75, 3, 1);
	StaticMeshComponent->SetCenterOfMass(FVector(0, 0, -175));
	StaticMeshComponent->SetMoveIgnoreMask(EComponentMobility::Movable);

	float pitch = -70;
	float yaw = 90;
	float roll = 90;

	// Create a spring arm component
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);	// Attach SpringArm to RootComponent
	SpringArm->TargetArmLength = 0.0f; // The camera follows at this distance behind the character	
	SpringArm->SocketOffset = springArmOfset;
	SpringArm->bEnableCameraLag = false;
	SpringArm->bEnableCameraRotationLag = false;
	SpringArm->bInheritRoll = false;
	SpringArm->bInheritPitch = false;

	//za normalnu kameru
	//SpringArm->bEnableCameraLag = true;
	//SpringArm->bEnableCameraRotationLag = true;
	//SpringArm->CameraLagSpeed = 1000.f;
	//FVector headOffset = FVector(-10, 0, 180);

	offsetHMD = CreateDefaultSubobject<USceneComponent>(TEXT("offsetHMD"));
	offsetHMD->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	//offsetHMD->SetupAttachment(RootComponent);
	//offsetHMD->SetRelativeLocation(FVector(-100, 0, 180));

	// Create camera component 
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	// Attach the camera
	//Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);	
	Camera->SetupAttachment(offsetHMD);
	Camera->SetRelativeLocation(FVector(0, 0, 0));
	//Camera->bUsePawnControlRotation = false; // Don't rotate camera with controller

	headMD = CreateDefaultSubobject<UHeadMountedDisplayFunctionLibrary>(TEXT("HMD"));

	LeftMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MCLeft"));
	LeftMotionController->MotionSource = FName(TEXT("Left"));
	LeftMotionController->SetupAttachment(offsetHMD);
	
	RightMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("MCRight"));
	RightMotionController->MotionSource = FName(TEXT("Right"));
	RightMotionController->SetupAttachment(offsetHMD);

	//RightHandComponent = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightHand"));
	//RightHandComponent->Hand = EControllerHand::Right;
	//RightHandComponent->AttachParent = VROriginComp;
	//StaticMeshComponent->BodyInstance.bLockXRotation = true;
	//StaticMeshComponent->BodyInstance.bLockYRotation = true;
	//StaticMeshComponent->BodyInstance.bLockZRotation = true;
	//StaticMeshComponent->BodyInstance.bLockXTranslation = true;
	//StaticMeshComponent->BodyInstance.bLockYTranslation = true;
	//StaticMeshComponent->BodyInstance.bLockZTranslation = true;
	//MainThrusterComponent = CreateDefaultSubobject<UPhysicsThrusterComponent>(TEXT("MainThruster"));
	//MainThrusterComponent->SetupAttachment(RootComponent);
	//MainThrusterComponent->SetWorldRotation(FRotator(-180, 0, 0));
	//MainThrusterComponent->ThrustStrength = 100000;
	//MainThrusterComponent->bAutoActivate = 1;

	thrusterOffset = FVector(150, 40, 0);
	float angle = 5;

	thrusterLF = CreateDefaultSubobject<UPhysicsThrusterComponent>(TEXT("thrusterLF"));
	thrusterLF->SetRelativeLocation(thrusterOffset*FVector(1, -1, 1));
	thrusterLF->SetupAttachment(RootComponent);
	thrusterLF->SetWorldRotation(FQuat(FVector(0, 1, 0), FMath::DegreesToRadians(90)) * FQuat(FVector(0, 0, 1), FMath::DegreesToRadians(-angle)));
	thrusterLF->ThrustStrength = 0;
	thrusterLF->bAutoActivate = 1;
	groundEffectLF = new GroundEffectThrusterComponent();//CreateDefaultSubobject<UGroundEffectThrusterComponent>(TEXT("groundEffectLF"));
	groundEffectLF->thruster=thrusterLF;
	groundEffectLF->lastLocation = thrusterLF->GetComponentLocation();
	groundEffectLF->maxForce = maxHoverForce;

	thrusterRF = CreateDefaultSubobject<UPhysicsThrusterComponent>(TEXT("thrusterRF"));
	thrusterRF->SetRelativeLocation(thrusterOffset);
	thrusterRF->SetupAttachment(RootComponent);
	thrusterRF->SetWorldRotation(FQuat(FVector(0, 1, 0), FMath::DegreesToRadians(90)) * FQuat(FVector(0, 0, 1), FMath::DegreesToRadians(angle)));
	thrusterRF->ThrustStrength = 0;
	thrusterRF->bAutoActivate = 1;
	groundEffectRF = new GroundEffectThrusterComponent();//CreateDefaultSubobject<UGroundEffectThrusterComponent>(TEXT("groundEffectRF"));
	groundEffectRF->thruster = thrusterRF;
	groundEffectRF->lastLocation = thrusterRF->GetComponentLocation();
	groundEffectRF->maxForce = maxHoverForce;

	thrusterLB = CreateDefaultSubobject<UPhysicsThrusterComponent>(TEXT("thrusterLB"));
	thrusterLB->SetRelativeLocation(thrusterOffset*FVector(-1,-1, 1));
	thrusterLB->SetupAttachment(RootComponent);
	thrusterLB->SetWorldRotation(FQuat(FVector(0, 1, 0), FMath::DegreesToRadians(90)) * FQuat(FVector(0, 0, 1), FMath::DegreesToRadians(-angle)));
	thrusterLB->ThrustStrength = 0;
	thrusterLB->bAutoActivate = 1;
	groundEffectLB = new GroundEffectThrusterComponent();//CreateDefaultSubobject<UGroundEffectThrusterComponent>(TEXT("groundEffectLB"));
	groundEffectLB->thruster = thrusterLB;
	groundEffectLB->lastLocation = thrusterLB->GetComponentLocation();
	groundEffectLB->maxForce = maxHoverForce;

	thrusterRB = CreateDefaultSubobject<UPhysicsThrusterComponent>(TEXT("thrusterRB"));
	thrusterRB->SetRelativeLocation(thrusterOffset*FVector (-1, 1, 1));
	thrusterRB->SetupAttachment(RootComponent);
	thrusterRB->SetWorldRotation(FQuat(FVector(0, 1, 0), FMath::DegreesToRadians(90)) * FQuat(FVector(0, 0, 1), FMath::DegreesToRadians(angle)));
	thrusterRB->ThrustStrength = 0;
	thrusterRB->bAutoActivate = 1;
	groundEffectRB = new GroundEffectThrusterComponent();//CreateDefaultSubobject<UGroundEffectThrusterComponent>(TEXT("groundEffectRB"));	
	groundEffectRB->thruster = thrusterRB;
	groundEffectRB->lastLocation = thrusterRB->GetComponentLocation();
	groundEffectRB->maxForce = maxHoverForce;

	bReplicates = true;
	bReplicateMovement = true;
	//PrimaryActorTick.bCanEverTick = false;
	SetTickGroup(ETickingGroup::TG_PostPhysics);
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}


// Called when the game starts or when spawned
void AHoverer::BeginPlay()
{
	Super::BeginPlay();
	ResetHMDOrigin();
}

// Called every frame
void AHoverer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Offset of thrusters&rayCasts
	float offsetFB = 150;
	float offsetLR = 40;
	float offsetH = 0;
	
	groundEffectLF->updateImpulse();
	groundEffectRF->updateImpulse();
	groundEffectLB->updateImpulse();
	groundEffectRB->updateImpulse();

	FRotator rotHMD;
	FVector posHMD;
	headMD->GetOrientationAndPosition(rotHMD, posHMD);
	const float factorp = .005f;
	FVector head = Camera->GetComponentLocation() - GetActorLocation() - springArmOfset;
	
	//Relative to camera
	float rf = FVector::DotProduct(RightMotionController->GetComponentLocation() - Camera->GetComponentLocation(), Camera->GetForwardVector());
	float rr = FVector::DotProduct(RightMotionController->GetComponentLocation() - Camera->GetComponentLocation(), Camera->GetRightVector());
	float ru = FVector::DotProduct(RightMotionController->GetComponentLocation() - Camera->GetComponentLocation(), Camera->GetUpVector());
	FVector rc = FVector(rf, rr, ru);
	float lf = FVector::DotProduct(LeftMotionController->GetComponentLocation() - Camera->GetComponentLocation(), Camera->GetForwardVector());
	float lr = FVector::DotProduct(LeftMotionController->GetComponentLocation() - Camera->GetComponentLocation(), Camera->GetRightVector());
	float lu = FVector::DotProduct(LeftMotionController->GetComponentLocation() - Camera->GetComponentLocation(), Camera->GetUpVector());
	FVector lc = FVector(lf, lr, lu);




	//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("%f"), FVector::DotProduct(GetVelocity(), GetActorRightVector()) ) );

	//FVector verticalSpeed = GetVelocity().ProjectOnTo(GetActorUpVector()); // *GetWorld()->GetDeltaSeconds();
	//FVector verticalForceAdd = verticalSpeed * - 70;
	//StaticMeshComponent->AddForce(verticalForceAdd);

	DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetVelocity()*1 + GetActorLocation(), 120.f, FColor::Red, false, -1, 2, 5.f);
	DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetVelocity().ProjectOnTo(FVector(0,0,1)) * .5 + GetActorLocation(), 120.f, FColor::Blue, false, -1, 2, 5.f);

	StaticMeshComponent->AddTorqueInRadians(GetActorRightVector() * torqeuPitchCoefficient * FMath::Clamp(pitchThrottle, -1.f, 1.f));
	StaticMeshComponent->AddTorqueInRadians(GetActorForwardVector() * -torqeuRollCoefficient * FMath::Clamp(rollThrottle, -1.f, 1.f));
	StaticMeshComponent->AddTorqueInRadians(GetActorUpVector()/*FVector(0, 0, 1)*/ * torqeuYawCoefficient * FMath::Clamp(yawThrottle, -1.f, 1.f));

	FVector toMakeNoseUp = FVector(GetActorRightVector().X, GetActorRightVector().Y, 0.f).GetSafeNormal();
	

	//ako se poklapa nagib sa smerom brzine upsori ga za koeficijent Z
	float direction = FVector::DotProduct(GetVelocity(), GetActorRightVector());

	if ((direction > 0 && GetActorRightVector().Z > 0)) //Left Turn
	{
		float rollCoof = -GetActorRightVector().Z;
		FVector sideSpeed = GetVelocity().ProjectOnTo(GetActorRightVector());
		FVector sideForceAdd = sideSpeed * sideDump * rollCoof;
		StaticMeshComponent->AddForce(sideForceAdd);
		DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), sideSpeed * 1.0f + GetActorLocation(), 120.f, FColor::Magenta, false, -1, 2, 5.f);
		DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), sideForceAdd * .01f + GetActorLocation(), 120.f, FColor::Cyan, false, -1, 2, 5.f);

		FVector toMakeNoseUp = FVector(GetActorRightVector().X, GetActorRightVector().Y, 0.f).GetSafeNormal();
		if (toMakeNoseUp != FVector(0, 0, 0))
		{
			StaticMeshComponent->AddTorqueInRadians(toMakeNoseUp * torqeuPitchCoefficient * rollCoof * 2);
		}
	}
	if ((direction < 0 && GetActorRightVector().Z < 0))
	{
		float rollCoof = GetActorRightVector().Z;
		FVector sideSpeed = GetVelocity().ProjectOnTo(GetActorRightVector());
		FVector sideForceAdd = sideSpeed * sideDump * rollCoof;
		StaticMeshComponent->AddForce(sideForceAdd);
		DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), sideSpeed * 1.0f + GetActorLocation(), 120.f, FColor::Magenta, false, -1, 2, 5.f);
		DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), sideForceAdd * .01f + GetActorLocation(), 120.f, FColor::Cyan, false, -1, 2, 5.f);

		FVector toMakeNoseUp = FVector(GetActorRightVector().X, GetActorRightVector().Y, 0.f).GetSafeNormal();
		if (toMakeNoseUp != FVector(0, 0, 0))
		{
			StaticMeshComponent->AddTorqueInRadians(toMakeNoseUp * torqeuPitchCoefficient * rollCoof * 2);
		}
	}

	//updateHoverImpulses(+offsetFB, +offsetLR, offsetH);
	//updateHoverImpulses(+offsetFB, -offsetLR, offsetH);
	//updateHoverImpulses(-offsetFB, +offsetLR, offsetH);
	//updateHoverImpulses(-offsetFB, -offsetLR, offsetH);
	//float linearDamping = 1.0 / 1;
	//FVector localVerticalSpeed = GetVelocity().ProjectOnTo(GetActorUpVector());
	//FVector verticalDumingSpeed = localVerticalSpeed * linearDamping * GetWorld()->GetDeltaSeconds();
	//StaticMeshComponent->SetPhysicsLinearVelocity(GetVelocity() - verticalDumingSpeed);
	//
	//StaticMeshComponent->AddTorqueInRadians(GetActorRightVector() * torqeuPitchCoefficient * FMath::Clamp(pitchForce, -1.f, 1.f));
	//StaticMeshComponent->AddTorqueInRadians(GetActorForwardVector() * -torqeuRollCoefficient * FMath::Clamp(rollForce, -1.f, 1.f));
	//StaticMeshComponent->AddTorqueInRadians(FVector(0, 0, 1) * torqeuYawCoefficient * FMath::Clamp(yawForce, -1.f, 1.f));
	//
	//FVector yawFv = FVector(0, 0, 1) * torqeuYawCoefficient * FMath::Clamp(posHMD.Y * FMath::Abs(posHMD.Y) * 0.005f, -1.f, 1.f);
	//FVector pitchFv = GetActorRightVector() * torqeuPitchCoefficient * FMath::Clamp(posHMD.X * FMath::Abs(posHMD.X) *.003f, -1.f, 1.f);
	//FVector rollFv = GetActorForwardVector() * -torqeuRollCoefficient * FMath::Clamp(posHMD.Y * FMath::Abs(posHMD.Y) * factorp, -1.f, 1.f);
	//
	//float rf = FVector::DotProduct(RightMotionController->GetComponentLocation() - GetActorLocation(), FRotationMatrix(rotHMD).GetScaledAxis(EAxis::X));
	//float rr = FVector::DotProduct(RightMotionController->GetComponentLocation() - GetActorLocation(), FRotationMatrix(rotHMD).GetScaledAxis(EAxis::Y));
	//float ru = FVector::DotProduct(RightMotionController->GetComponentLocation() - GetActorLocation(), FRotationMatrix(rotHMD).GetScaledAxis(EAxis::Z));
	//
	//float rf = FVector::DotProduct(RightMotionController->GetComponentLocation() - GetActorLocation(), Camera->GetForwardVector());
	//float rr = FVector::DotProduct(RightMotionController->GetComponentLocation() - GetActorLocation(), Camera->GetRightVector());
	//float ru = FVector::DotProduct(RightMotionController->GetComponentLocation() - GetActorLocation(), Camera->GetUpVector());
	//FVector rc = FVector(rf, rr, ru);
	//
	////board orientation
	//float rf = FVector::DotProduct(RightMotionController->GetComponentLocation() - GetActorLocation(), Camera->GetForwardVector());
	//float rr = FVector::DotProduct(RightMotionController->GetComponentLocation() - GetActorLocation(), Camera->GetRightVector());
	//float ru = FVector::DotProduct(RightMotionController->GetComponentLocation() - GetActorLocation(), Camera->GetUpVector());
	//FVector rc = FVector(rf, rr, ru);
	//float lf = FVector::DotProduct(LeftMotionController->GetComponentLocation() - GetActorLocation(), Camera->GetForwardVector());
	//float lr = FVector::DotProduct(LeftMotionController->GetComponentLocation() - GetActorLocation(), Camera->GetRightVector());
	//float lu = FVector::DotProduct(LeftMotionController->GetComponentLocation() - GetActorLocation(), Camera->GetUpVector());
	//FVector lc = FVector(lf, lr, lu);
	//FVector rmcPos;
	//FRotator rmcRot;
	//rmc->GetPositionOrientation(rmcPos, rmcRot);
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, .45f, FColor::Purple, FString::Printf(TEXT("Position: %f"), FMath::Clamp(posHMD.Y * FMath::Abs(posHMD.Y) * factorp, -1.f, 1.f) ) );
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, .1f, FColor::Yellow, FString::Printf(TEXT("D: %s"), *(posHMD).ToString()));
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, .1f, FColor::Purple, FString::Printf(TEXT("H: %s"), *head.ToString()));
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, .31f, FColor::Blue, FString::Printf(TEXT("L: %s"), *lc.ToString()));
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, .31f, FColor::Red, FString::Printf(TEXT("R: %s"), *rc.ToString()));
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, .31f, FColor::Yellow, FString::Printf(TEXT("R: %s"), *RightMotionController->GetComponentLocation().ToString()));
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, .1f, FColor::Orange, FString::Printf(TEXT("-: %s"), *(rc - rcc).ToString()));
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, .1f, FColor::Green, FString::Printf(TEXT("A: %s"), *GetActorLocation().ToString()));
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, .5f, FColor::Green, FString::Printf(TEXT("Position: %s"), *(/* pos - */StaticMeshComponent->GetComponentLocation()).ToString()));
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, .1f, FColor::Red, FString::Printf(TEXT("V: %f Vert: %f"), GetVelocity().Size() / 100 * 3.6, GetVelocity().Z / 100 * 3.6));
	//FVector verticalSpeed = FVector(0, 0, GetVelocity().Z);// GetVelocity().ProjectOnTo(GetActorUpVector());
	//StaticMeshComponent->AddForce(-verticalSpeed*100);
	//float ang = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(GetVelocity().GetSafeNormal(), GetActorUpVector())));
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, .1f, FColor::Red, FString::Printf(TEXT("%f "), FMath::Abs(FMath::Cos(ang))));
	//FVector verticalSpeed = GetVelocity().ProjectOnTo(GetActorUpVector()); // *GetWorld()->GetDeltaSeconds();
	//FVector verticalForceAdd = verticalSpeed * -150;
	//StaticMeshComponent->AddForce(verticalForceAdd);
	//DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetVelocity() + GetActorLocation(), 120.f, FColor::Red, false, -1, 2, 5.f);
	//DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), (verticalForceAdd) * .01 + GetActorLocation(), 120.f, FColor::Magenta, false, -1, 2, 5.f);
	//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, .1f, FColor::Red, FString::Printf(TEXT("Distance: %f"), GetVelocity().Size()));
	//StaticMeshComponent->ComponentVelocity = VectorZero();// GetVelocity()*.01;
	//linearVelocity *= 1.0f / (1.0f + GetWorld()->GetDeltaSeconds() * 1);
	//FVector position = FVector(-GetVelocity().X, GetVelocity().Y, 160).Normalize()*400;
	//SpringArm->SocketOffset = position;
	//Camera->SetRelativeLocation(position);
}

// Called to bind functionality to input
void AHoverer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	InputComponent->BindAxis("throttle", this, &AHoverer::moveX);
	InputComponent->BindAxis("yaw", this, &AHoverer::yaw);
	InputComponent->BindAxis("roll", this, &AHoverer::roll);
	InputComponent->BindAxis("pitch", this, &AHoverer::pitch);
	InputComponent->BindAction("resetvrp", IE_Pressed, this, &AHoverer::ResetHMDOrigin);
	//InputComponent->BindAction("Fire", IE_Pressed, this, &AHowerer::OnFire);
}


void AHoverer::moveX(float AxisValue)
{
	//changeto tick use currentThrottle to add froce
	currentThrottle = 0;
	if (AxisValue != 0)
	{
		//StaticMeshComponent->AddImpulse(GetActorForwardVector() * maxForwardForce * value * GetWorld()->GetDeltaSeconds());
		//StaticMeshComponent->AddForce(AxisValue * 100000 * GetActorForwardVector());
		//AddActorLocalOffset(FVector(AxisValue * 10, 0, 0));
		//CurrentVelocity = GetActorForwardVector() * FMath::Clamp(AxisValue, -1.0f, 1.0f) * 100.0f;
		//CurrentVelocity.X = FMath::Clamp(AxisValue, -1.0f, 1.0f) * 100.0f;
		//ServerMoveX(AxisValue);
		currentThrottle = AxisValue;
	}
	FMath::Clamp(currentThrottle, -1.0f, .0f);
	StaticMeshComponent->AddImpulse(GetActorForwardVector() * maxForwardForce * currentThrottle * GetWorld()->GetDeltaSeconds());
	//GEngine->AddOnScreenDebugMessage(-1, .25f, FColor::Yellow, FString::Printf(TEXT("th: %f"), currentThrottle));
}

void AHoverer::yaw(float AxisValue)
{
	if (AxisValue != 0)
	{
		StaticMeshComponent->AddTorqueInRadians(FVector(0,0,1) * -torqeuYawCoefficient * AxisValue);
		//ServerYaw(AxisValue);
		//StaticMeshComponent->AddTorqueInRadians(GetActorUpVector() * AxisValue * 1000000);
		//AddActorLocalRotation(FRotator(0, AxisValue, 0), true, 0, ETeleportType::None);
		// Move at 100 units per second right or left
		//CurrentVelocity.Y = FMath::Clamp(AxisValue, -1.0f, 1.0f) * 100.0f;
	}
}

void AHoverer::roll(float AxisValue)
{
	if (AxisValue != 0)
	{
		StaticMeshComponent->AddTorqueInRadians(GetActorForwardVector() * -torqeuRollCoefficient * AxisValue);
		//ServerRoll(AxisValue);
		//StaticMeshComponent->AddTorqueInRadians(GetActorUpVector() * AxisValue * 1000000);
		//AddActorLocalRotation(FRotator(0, AxisValue, 0), true, 0, ETeleportType::None);
		// Move at 100 units per second right or left
		//CurrentVelocity.Y = FMath::Clamp(AxisValue, -1.0f, 1.0f) * 100.0f;
	}
}

void AHoverer::pitch(float AxisValue)
{
	if (AxisValue != 0)
	{
		StaticMeshComponent->AddTorqueInRadians(GetActorRightVector() * torqeuPitchCoefficient * AxisValue);
		//ServerPitch(AxisValue);
		//StaticMeshComponent->AddTorqueInRadians(GetActorUpVector() * AxisValue * 1000000);
		//AddActorLocalRotation(FRotator(0, AxisValue, 0), true, 0, ETeleportType::None);
		// Move at 100 units per second right or left
		//CurrentVelocity.Y = FMath::Clamp(AxisValue, -1.0f, 1.0f) * 100.0f;
	}
}

void AHoverer::ResetHMDOrigin()
{
	//UHeadMountedDisplayFunctionLibrary::
	GEngine->XRSystem->ResetOrientationAndPosition();
}

void AHoverer::SetupVROptions()
{
}


/**
* function: getHMDType
* This method communicates with the Unreal Engine to determine what type of HMD
* is attached to host device.
*
* @return FString    name of the HMD device; "None" if there is no HMD
*/
FString AHoverer::getHMDType()
{
	//IHeadMountedDisplay* hmd = (IHeadMountedDisplay*)(GEngine->XRSystem->GetHMDDevice.Get());
	//if (hmd) {
	//	EHMDDeviceType::Type current_hmd = hmd->GetHMDDeviceType();
	//	switch (current_hmd) {
	//	case EHMDDeviceType::DT_ES2GenericStereoMesh:
	//		return FString("Generic");
	//		break;
	//	case EHMDDeviceType::DT_GearVR:
	//		return FString("GearVR");
	//		break;
	//	case EHMDDeviceType::DT_Morpheus:
	//		return FString("PSVR");
	//		break;
	//	case EHMDDeviceType::DT_OculusRift:
	//		return FString("OculusRift");
	//		break;
	//	case EHMDDeviceType::DT_SteamVR:
	//		return FString("SteamVR");
	//		break;
	//	default:
	//		return FString("None");
	//		break;
	//	}
	//}
	return FString("None");
}

void AHoverer::Destroyed()
{
	Super::Destroyed();

	delete groundEffectLF;
	delete groundEffectRF;
	delete groundEffectLB;
	delete groundEffectRB;
}

void AHoverer::updateHoverImpulses(float offsetFB, float offsetLR, float offsetH, float localVertSpeed)
{
	//FlushPersistentDebugLines(GetWorld());
	FCollisionQueryParams CollisionParams;
	FHitResult OutHit;

	FVector loc = GetActorLocation() + GetActorForwardVector() * offsetFB + GetActorRightVector() *  offsetLR + GetActorUpVector() * offsetH;
	FVector direction = GetActorUpVector().RotateAngleAxis(FMath::Sign(offsetLR) * 15, GetActorForwardVector());

	if (GetWorld()->LineTraceSingleByChannel(OutHit, loc - 5 * direction, -direction * 500 + loc, ECC_Visibility, CollisionParams))
	{
		if (OutHit.bBlockingHit)
		{
			float groundEffect = maxHoverForce - OutHit.Distance * OutHit.Distance * 6;
			float currentImpulse = FMath::Clamp(groundEffect, 0.0f, maxHoverForce);
			FVector impulse = GetActorUpVector() * currentImpulse;
			StaticMeshComponent->AddImpulseAtLocation(impulse * GetWorld()->GetDeltaSeconds(), loc);
			DrawDebugDirectionalArrow(GetWorld(), loc, (impulse) * .002 + loc, 120.f, FColor::Yellow, false, -1, 2, 5.f);
			//DrawDebugDirectionalArrow(GetWorld(), loc, (localVertSpeed) * .01 + lo, 120.f, FColor::Magenta, false, -1, 2, 5.f);
			//if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.f	s, FColor::Red, FString::Printf(TEXT("Distance: %f"), currentImpulse));
			//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("Distance: %f"), OutHit.Distance   ));
			//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("You are hitting: %s"), *OutHit.GetActor()->GetName()));
			//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("Impact Point: %s"), *OutHit.ImpactPoint.ToString()));
			//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("Normal Point: %s"), *OutHit.ImpactNormal.ToString()));
		}
	}
	DrawDebugDirectionalArrow(GetWorld(), loc, -direction * 500 + loc, 120.f, FColor::Green, false, -1, 2, 5.f);
}

void AHoverer::updateHoverThruster(UPhysicsThrusterComponent *thruster)
{
	////FlushPersistentDebugLines(GetWorld());
	//FCollisionQueryParams CollisionParams;
	//FHitResult OutHit;
	//if (GetWorld()->LineTraceSingleByChannel(OutHit, thruster->GetComponentLocation(), thruster->GetForwardVector() * 500 + thruster->GetComponentLocation(), ECC_Visibility, CollisionParams))
	//{
	//	if (OutHit.bBlockingHit)
	//	{
	//		float currentForce = FMath::Clamp(maxHoverForce - OutHit.Distance *OutHit.Distance + OutHit.Distance, 0.0f, maxHoverForce);
	//		thruster->ThrustStrength = currentForce;
	//		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("Distance: %f"), currentForce));
	//		//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("Distance: %f"), OutHit.Distance   ));
	//		//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("You are hitting: %s"), *OutHit.GetActor()->GetName()));
	//		//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("Impact Point: %s"), *OutHit.ImpactPoint.ToString()));
	//		//GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("Normal Point: %s"), *OutHit.ImpactNormal.ToString()));
	//	}
	//}
	////DrawDebugDirectionalArrow(GetWorld(), thruster->GetComponentLocation(), thruster->GetForwardVector() * 500 + thruster->GetComponentLocation(), 120.f, FColor::Magenta, false, -1, 2, 5.f);
}