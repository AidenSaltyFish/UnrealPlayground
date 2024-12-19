#include "P:\GameDev_Unreal\Yoshi\Intermediate\Build\Win64\x64\YoshiEditor\Development\UnrealEd\SharedPCH.UnrealEd.Project.ValApi.Cpp20.h"
#include "MotionBlurActor.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AMotionBlurActor::AMotionBlurActor()
{
	// Enable ticking for this actor
	PrimaryActorTick.bCanEverTick = true;
		
	PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComponent"));
	PostProcessComponent->bUnbound = true; // Apply to the whole scene
	RootComponent = PostProcessComponent;

	// Configure motion blur settings
	PostProcessComponent->Settings.bOverride_MotionBlurAmount = true;
	PostProcessComponent->Settings.MotionBlurAmount = 0.5f; // Adjust as needed
	PostProcessComponent->Settings.bOverride_MotionBlurMax = true;
	PostProcessComponent->Settings.MotionBlurMax = 0.75f; // Adjust as needed
	PostProcessComponent->Settings.bOverride_MotionBlurPerObjectSize = true;
	PostProcessComponent->Settings.MotionBlurPerObjectSize = 0.1f; // Adjust as needed
}

void AMotionBlurActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	float curVel = PlayerCharacter->GetCharacterMovement()->Velocity.Size();
	
	// TArray<AActor*> PostProcessVolumes;
	// UGameplayStatics::GetAllActorsOfClass(GetWorld(), APostProcessVolume::StaticClass(), PostProcessVolumes);
	
	if (curVel >= 750)
	{
		PostProcessComponent->Settings.MotionBlurAmount = 0.75f;
	}
	else if (curVel >= 500)
	{
		PostProcessComponent->Settings.MotionBlurAmount = 0.25f;
	}
	else
	{
		PostProcessComponent->Settings.MotionBlurAmount = 0.0f;
	}

	UE_LOG(LogTemp, Warning, TEXT("Current motion blur: %s"),
		*FString::SanitizeFloat(PostProcessComponent->Settings.MotionBlurAmount));
}
