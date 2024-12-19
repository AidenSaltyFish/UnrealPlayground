#pragma once

#include "GameFramework/Actor.h"
#include "Components/PostProcessComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MotionBlurActor.generated.h"

UCLASS(Config=Game)
class AMotionBlurActor : public AActor
{
	GENERATED_BODY()
	
public:
	AMotionBlurActor();
	
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(VisibleAnywhere, Category = "PostProcess")
	UPostProcessComponent* PostProcessComponent;
};
