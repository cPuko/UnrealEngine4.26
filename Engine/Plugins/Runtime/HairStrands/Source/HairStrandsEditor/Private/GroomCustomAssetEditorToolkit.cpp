// Copyright Epic Games, Inc. All Rights Reserved.

#include "GroomCustomAssetEditorToolkit.h"
#include "Modules/ModuleManager.h"
#include "EditorStyleSet.h"
#include "Widgets/Docking/SDockTab.h"
#include "PropertyEditorModule.h"
#include "IDetailsView.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Editor.h"
#include "AssetEditorModeManager.h"
#include "GroomAsset.h"
#include "GroomComponent.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "HairStrandsEditor.h"
#include "GroomEditorCommands.h"
#include "GroomEditorMode.h"
#include "GroomEditorStyle.h"
#include "GroomAssetDetails.h"
#include "GroomMaterialDetails.h"

#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"

#define GROOMEDITOR_ENABLE_COMPONENT_PANEL 0
#define LOCTEXT_NAMESPACE "GroomCustomAssetEditor"
const FName FGroomCustomAssetEditorToolkit::ToolkitFName(TEXT("GroomEditor"));

const FName FGroomCustomAssetEditorToolkit::TabId_Viewport(TEXT("GroomCustomAssetEditor_Render"));
const FName FGroomCustomAssetEditorToolkit::TabId_LODProperties(TEXT("GroomCustomAssetEditor_LODProperties"));
const FName FGroomCustomAssetEditorToolkit::TabId_InterpolationProperties(TEXT("GroomCustomAssetEditor_InterpolationProperties"));
const FName FGroomCustomAssetEditorToolkit::TabId_RenderingProperties(TEXT("GroomCustomAssetEditor_RenderProperties"));
const FName FGroomCustomAssetEditorToolkit::TabId_CardsProperties(TEXT("GroomCustomAssetEditor_CardsProperties"));
const FName FGroomCustomAssetEditorToolkit::TabId_MeshesProperties(TEXT("GroomCustomAssetEditor_MeshesProperties"));
const FName FGroomCustomAssetEditorToolkit::TabId_MaterialProperties(TEXT("GroomCustomAssetEditor_MaterialProperties"));
const FName FGroomCustomAssetEditorToolkit::TabId_PhysicsProperties(TEXT("GroomCustomAssetEditor_PhysicsProperties"));
const FName FGroomCustomAssetEditorToolkit::TabId_PreviewGroomComponent(TEXT("GroomCustomAssetEditor_PreviewGroomComponent"));

void FGroomCustomAssetEditorToolkit::RegisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenuGroomEditor", "Hair Strands Asset Editor"));

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(TabId_Viewport, FOnSpawnTab::CreateSP(this, &FGroomCustomAssetEditorToolkit::SpawnViewportTab))
		.SetDisplayName(LOCTEXT("ViewportTab", "Viewport"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef())
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Render"));

	InTabManager->RegisterTabSpawner(TabId_LODProperties, FOnSpawnTab::CreateSP(this, &FGroomCustomAssetEditorToolkit::SpawnTab_LODProperties))
		.SetDisplayName(LOCTEXT("LODPropertiesTab", "LOD"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef())
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(TabId_InterpolationProperties, FOnSpawnTab::CreateSP(this, &FGroomCustomAssetEditorToolkit::SpawnTab_InterpolationProperties))
		.SetDisplayName(LOCTEXT("InterpolationPropertiesTab", "Interpolation"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef())
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(TabId_RenderingProperties, FOnSpawnTab::CreateSP(this, &FGroomCustomAssetEditorToolkit::SpawnTab_RenderingProperties))
		.SetDisplayName(LOCTEXT("RenderingPropertiesTab", "Strands"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef())
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(TabId_PhysicsProperties, FOnSpawnTab::CreateSP(this, &FGroomCustomAssetEditorToolkit::SpawnTab_PhysicsProperties))
		.SetDisplayName(LOCTEXT("PhysicsPropertiesTab", "Physics"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef())
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(TabId_CardsProperties, FOnSpawnTab::CreateSP(this, &FGroomCustomAssetEditorToolkit::SpawnTab_CardsProperties))
		.SetDisplayName(LOCTEXT("CardsPropertiesTab", "Cards"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef())
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(TabId_MeshesProperties, FOnSpawnTab::CreateSP(this, &FGroomCustomAssetEditorToolkit::SpawnTab_MeshesProperties))
		.SetDisplayName(LOCTEXT("MeshesPropertiesTab", "Meshes"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef())
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

	InTabManager->RegisterTabSpawner(TabId_MaterialProperties, FOnSpawnTab::CreateSP(this, &FGroomCustomAssetEditorToolkit::SpawnTab_MaterialProperties))
		.SetDisplayName(LOCTEXT("MaterialPropertiesTab", "Material"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef())
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));

#if GROOMEDITOR_ENABLE_COMPONENT_PANEL
	InTabManager->RegisterTabSpawner(TabId_PreviewGroomComponent, FOnSpawnTab::CreateSP(this, &FGroomCustomAssetEditorToolkit::SpawnTab_PreviewGroomComponent))
		.SetDisplayName(LOCTEXT("PreviewGroomComponentTab", "Preview Component"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef())
		.SetIcon(FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.Tabs.Details"));
#endif
}

void FGroomCustomAssetEditorToolkit::UnregisterTabSpawners(const TSharedRef<class FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(TabId_Viewport);
	InTabManager->UnregisterTabSpawner(TabId_LODProperties);
	InTabManager->UnregisterTabSpawner(TabId_InterpolationProperties);
	InTabManager->UnregisterTabSpawner(TabId_RenderingProperties);
	InTabManager->UnregisterTabSpawner(TabId_CardsProperties);
	InTabManager->UnregisterTabSpawner(TabId_MeshesProperties);
	InTabManager->UnregisterTabSpawner(TabId_MaterialProperties);
	InTabManager->UnregisterTabSpawner(TabId_PhysicsProperties);
#if GROOMEDITOR_ENABLE_COMPONENT_PANEL
	InTabManager->UnregisterTabSpawner(TabId_PreviewGroomComponent);
#endif
}

FEdMode* FGroomCustomAssetEditorToolkit::GetEditorMode() const 
{
	static FGroomEditorMode *Mode = new FGroomEditorMode;
	return Mode;
}

static uint32 GOpenedGroomEditorCount = 0;

void FGroomCustomAssetEditorToolkit::DocPropChanged(UObject *InObject, FPropertyChangedEvent &Property)
{
	if (!GroomAsset.Get())
	{
		return;
	}
#if 0 //TODO
	UGroomDocument *Doc = FGroomDataIO::GetDocumentForAsset(GroomAsset.Get());

	if (Doc != nullptr && InObject==Doc)
	{		
		if (Property.GetPropertyName() == GET_MEMBER_NAME_CHECKED(UGroomDocument, GroomTarget))
		{
			FObjectProperty* prop = CastField<FObjectProperty>(Property.Property);
			UStaticMesh		*AsStaticMesh = Cast<UStaticMesh>(prop->GetPropertyValue_InContainer(InObject));
			USkeletalMesh	*AsSkeletalMesh = Cast<USkeletalMesh>(prop->GetPropertyValue_InContainer(InObject));
			
			if (AsStaticMesh)
			{
				OnStaticGroomTargetChanged(AsStaticMesh);
			} else if (AsSkeletalMesh)
			{
				OnSkeletalGroomTargetChanged(AsSkeletalMesh);
			}
			else
			{
				OnSkeletalGroomTargetChanged(nullptr);
				OnSkeletalGroomTargetChanged(nullptr);
			}
		}
	}	
#endif
}

void FGroomCustomAssetEditorToolkit::OnStaticGroomTargetChanged(UStaticMesh *NewTarget)
{	
	if (PreviewStaticMeshComponent != nullptr)
	{
		PreviewStaticMeshComponent->SetStaticMesh(NewTarget);
		PreviewSkeletalMeshComponent->SetVisibility(false);
		PreviewStaticMeshComponent->SetVisibility(NewTarget != nullptr);
	}	
}

void FGroomCustomAssetEditorToolkit::OnSkeletalGroomTargetChanged(USkeletalMesh *NewTarget)
{	
	if (PreviewSkeletalMeshComponent != nullptr)
	{
		PreviewSkeletalMeshComponent->SetSkeletalMesh(NewTarget);
		PreviewSkeletalMeshComponent->SetVisibility(NewTarget != nullptr);
		PreviewStaticMeshComponent->SetVisibility(false);
	}	
}

void FGroomCustomAssetEditorToolkit::ExtendToolbar()
{
	// Disable simulation toolbar as it is currently not hooked
#if 0 
	struct Local
	{
		static TSharedRef<SWidget> FillSimulationOptionsMenu(FGroomCustomAssetEditorToolkit* Toolkit)
		{
			FMenuBuilder MenuBuilder(true, Toolkit->GetToolkitCommands());

			MenuBuilder.AddMenuEntry( FGroomEditorCommands::Get().PlaySimulation );
			MenuBuilder.AddMenuEntry( FGroomEditorCommands::Get().PauseSimulation );
			MenuBuilder.AddMenuEntry( FGroomEditorCommands::Get().ResetSimulation );

			return MenuBuilder.MakeWidget();
		}

		static void FillToolbar(FToolBarBuilder& ToolbarBuilder, FGroomCustomAssetEditorToolkit* Toolkit)
		{			
			ToolbarBuilder.AddToolBarButton( FGroomEditorCommands::Get().Simulate );

			ToolbarBuilder.BeginSection( "PlaybackOptions" );
			{				
				ToolbarBuilder.AddComboButton(
					FUIAction(),
					FOnGetContent::CreateStatic(Local::FillSimulationOptionsMenu, Toolkit),
					LOCTEXT("SimulationOptions", "Simulation"),
					LOCTEXT("SimulationOptionsTooltip", "Simulation options"),
					FSlateIcon(FGroomEditorStyle::GetStyleName(), "GroomEditor.SimulationOptions")
				);
			}
			ToolbarBuilder.EndSection();
		}
	};

	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);

	ToolbarExtender->AddToolBarExtension(
		"Asset",
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateStatic(&Local::FillToolbar, this)
	);

	AddToolbarExtender(ToolbarExtender);

	FGroomEditor& GroomEditorModule = FModuleManager::LoadModuleChecked<FGroomEditor>("HairStrandsEditor"); // GroomEditor
#endif
}

void FGroomCustomAssetEditorToolkit::InitPreviewComponents()
{
	check(GroomAsset!=nullptr);
//	check(FGroomDataIO::GetDocumentForAsset(GroomAsset) != nullptr); TODO
	check(PreviewGroomComponent == nullptr);
	check(PreviewStaticMeshComponent == nullptr);
	check(PreviewSkeletalMeshComponent == nullptr);
	
#if 0 //TODO
	UGroomDocument *Doc = FGroomDataIO::GetDocumentForAsset(GroomAsset);

	// Update the document from the groom asset
	FGroomDataIO::UpdateDocumentFromGroomAsset(GroomAsset.Get(), Doc);
#endif

	PreviewGroomComponent = NewObject<UGroomComponent>(GetTransientPackage(), NAME_None, RF_Transient);
	PreviewGroomComponent->CastShadow = 1;
	PreviewGroomComponent->bCastDynamicShadow = 1;
	PreviewGroomComponent->SetGroomAsset(GroomAsset.Get());
	PreviewGroomComponent->Activate(true);
	
	if (PropertyListenDelegate.IsValid() == false)
	{		
		PropertyListenDelegate = FCoreUObjectDelegates::OnObjectPropertyChanged.AddRaw(this, &FGroomCustomAssetEditorToolkit::DocPropChanged);
	}

#if 0 //TODO
	PreviewStaticMeshComponent = NewObject<UStaticMeshComponent>(GetTransientPackage(), NAME_None, RF_Transient);
	PreviewSkeletalMeshComponent = NewObject<USkeletalMeshComponent>(GetTransientPackage(), NAME_None, RF_Transient);
	if (Doc->GetStaticGroomTarget())
	{
		PreviewStaticMeshComponent->SetStaticMesh(Doc->GetStaticGroomTarget());
	}
	else
	{
		PreviewStaticMeshComponent->SetVisibility(false);
	}

	if (Doc->GetSkeletalGroomTarget())
	{
		PreviewSkeletalMeshComponent->SetSkeletalMesh(Doc->GetSkeletalGroomTarget());
	}
	else
	{
		PreviewSkeletalMeshComponent->SetVisibility(false);
	}
#endif
}

bool FGroomCustomAssetEditorToolkit::OnRequestClose() 
{
	bool Removed = FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(PropertyListenDelegate);
	//check(Removed);

	PropertiesTab.Reset();
	ViewportTab.Reset();

	DetailView_LODProperties.Reset();
	DetailView_InterpolationProperties.Reset();
	DetailView_RenderingProperties.Reset();
	DetailView_PhysicsProperties.Reset();
	DetailView_CardsProperties.Reset();
	DetailView_MeshesProperties.Reset();
	DetailView_MaterialProperties.Reset();
#if GROOMEDITOR_ENABLE_COMPONENT_PANEL
	DetailView_PreviewGroomComponent.Reset();
#endif

	return FAssetEditorToolkit::OnRequestClose();
}

void FGroomCustomAssetEditorToolkit::InitCustomAssetEditor(const EToolkitMode::Type Mode, const TSharedPtr<class IToolkitHost>& InitToolkitHost, UGroomAsset* InCustomAsset)
{
	const bool bIsUpdatable		= false;
	const bool bAllowFavorites	= true;
	const bool bIsLockable		= false;

	PreviewGroomComponent		 = nullptr;
	PreviewStaticMeshComponent	 = nullptr;
	PreviewSkeletalMeshComponent = nullptr;

	SetCustomAsset(InCustomAsset);
	InitPreviewComponents();
	
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

	DetailView_LODProperties			= PropertyEditorModule.CreateDetailView(FDetailsViewArgs(bIsUpdatable, bIsLockable, true, FDetailsViewArgs::ObjectsUseNameArea, false));
	DetailView_InterpolationProperties	= PropertyEditorModule.CreateDetailView(FDetailsViewArgs(bIsUpdatable, bIsLockable, true, FDetailsViewArgs::ObjectsUseNameArea, false));
	DetailView_RenderingProperties		= PropertyEditorModule.CreateDetailView(FDetailsViewArgs(bIsUpdatable, bIsLockable, true, FDetailsViewArgs::ObjectsUseNameArea, false));
	DetailView_PhysicsProperties		= PropertyEditorModule.CreateDetailView(FDetailsViewArgs(bIsUpdatable, bIsLockable, true, FDetailsViewArgs::ObjectsUseNameArea, false));
	DetailView_CardsProperties			= PropertyEditorModule.CreateDetailView(FDetailsViewArgs(bIsUpdatable, bIsLockable, true, FDetailsViewArgs::ObjectsUseNameArea, false));
	DetailView_MeshesProperties			= PropertyEditorModule.CreateDetailView(FDetailsViewArgs(bIsUpdatable, bIsLockable, true, FDetailsViewArgs::ObjectsUseNameArea, false));
	DetailView_MaterialProperties		= PropertyEditorModule.CreateDetailView(FDetailsViewArgs(bIsUpdatable, bIsLockable, true, FDetailsViewArgs::ObjectsUseNameArea, false));
#if GROOMEDITOR_ENABLE_COMPONENT_PANEL
	DetailView_PreviewGroomComponent	= PropertyEditorModule.CreateDetailView(FDetailsViewArgs(bIsUpdatable, bIsLockable, true, FDetailsViewArgs::ObjectsUseNameArea, false));
#endif

	// Customization
	DetailView_CardsProperties->SetGenericLayoutDetailsDelegate(FOnGetDetailCustomizationInstance::CreateStatic(&FGroomRenderingDetails::MakeInstance, (IGroomCustomAssetEditorToolkit*)this, EMaterialPanelType::Cards));
	DetailView_MeshesProperties->SetGenericLayoutDetailsDelegate(FOnGetDetailCustomizationInstance::CreateStatic(&FGroomRenderingDetails::MakeInstance, (IGroomCustomAssetEditorToolkit*)this, EMaterialPanelType::Meshes));
	DetailView_RenderingProperties->SetGenericLayoutDetailsDelegate(FOnGetDetailCustomizationInstance::CreateStatic(&FGroomRenderingDetails::MakeInstance, (IGroomCustomAssetEditorToolkit*)this, EMaterialPanelType::Strands));
	DetailView_InterpolationProperties->SetGenericLayoutDetailsDelegate(FOnGetDetailCustomizationInstance::CreateStatic(&FGroomRenderingDetails::MakeInstance, (IGroomCustomAssetEditorToolkit*)this, EMaterialPanelType::Interpolation));
	DetailView_PhysicsProperties->SetGenericLayoutDetailsDelegate(FOnGetDetailCustomizationInstance::CreateStatic(&FGroomRenderingDetails::MakeInstance, (IGroomCustomAssetEditorToolkit*)this, EMaterialPanelType::Physics));
	DetailView_LODProperties->SetGenericLayoutDetailsDelegate(FOnGetDetailCustomizationInstance::CreateStatic(&FGroomRenderingDetails::MakeInstance, (IGroomCustomAssetEditorToolkit*)this, EMaterialPanelType::LODs));
	DetailView_MaterialProperties->SetGenericLayoutDetailsDelegate(FOnGetDetailCustomizationInstance::CreateStatic(&FGroomMaterialDetails::MakeInstance, (IGroomCustomAssetEditorToolkit*)this));

	SEditorViewport::FArguments args;
	ViewportTab = SNew(SGroomEditorViewport);
	
	// Default layout
	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_GroomAssetEditor_Layout_v14")
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewStack()
				->SetSizeCoefficient(0.1f)
				->SetHideTabWell(true)
				->AddTab(GetToolbarTabId(), ETabState::OpenedTab)
			)
			->Split
			(
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Horizontal)
				->SetSizeCoefficient(0.9f)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.8f)
					->SetHideTabWell(true)
					->AddTab(TabId_Viewport, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.2f)
					->AddTab(TabId_LODProperties,			ETabState::OpenedTab)
					->AddTab(TabId_InterpolationProperties,	ETabState::OpenedTab)
					->AddTab(TabId_RenderingProperties,		ETabState::OpenedTab)
					->AddTab(TabId_CardsProperties,			ETabState::OpenedTab)
					->AddTab(TabId_MeshesProperties,		ETabState::OpenedTab)
					->AddTab(TabId_MaterialProperties,		ETabState::OpenedTab)
					->AddTab(TabId_PhysicsProperties,		ETabState::OpenedTab)
				#if GROOMEDITOR_ENABLE_COMPONENT_PANEL
					->AddTab(TabId_PreviewGroomComponent,	ETabState::OpenedTab)
				#endif
				)
			)
		);

	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;

	FAssetEditorToolkit::InitAssetEditor(
		Mode,
		InitToolkitHost,
		FGroomEditor::GroomEditorAppIdentifier,
		StandaloneDefaultLayout,
		bCreateDefaultStandaloneMenu,
		bCreateDefaultToolbar,
		(UObject*)InCustomAsset);
	
	FProperty* P0 = FindFProperty<FProperty>(GroomAsset->GetClass(), GET_MEMBER_NAME_CHECKED(UGroomAsset, HairGroupsInterpolation));
	FProperty* P1 = FindFProperty<FProperty>(GroomAsset->GetClass(), GET_MEMBER_NAME_CHECKED(UGroomAsset, HairGroupsRendering));
	FProperty* P2 = FindFProperty<FProperty>(GroomAsset->GetClass(), GET_MEMBER_NAME_CHECKED(UGroomAsset, HairGroupsPhysics));
	FProperty* P3 = FindFProperty<FProperty>(GroomAsset->GetClass(), GET_MEMBER_NAME_CHECKED(UGroomAsset, HairGroupsCards));
	FProperty* P5 = FindFProperty<FProperty>(GroomAsset->GetClass(), GET_MEMBER_NAME_CHECKED(UGroomAsset, HairGroupsMeshes));
	FProperty* P6 = FindFProperty<FProperty>(GroomAsset->GetClass(), GET_MEMBER_NAME_CHECKED(UGroomAsset, HairGroupsMaterials));
	FProperty* P4 = FindFProperty<FProperty>(GroomAsset->GetClass(), GET_MEMBER_NAME_CHECKED(UGroomAsset, HairGroupsLOD));

	FProperty* P7 = FindFProperty<FProperty>(GroomAsset->GetClass(), GET_MEMBER_NAME_CHECKED(UGroomAsset, HairGroupsInfo));
	FProperty* P8 = FindFProperty<FProperty>(GroomAsset->GetClass(), GET_MEMBER_NAME_CHECKED(UGroomAsset, LODSelectionType));

	FProperty* P0_0 = FindFProperty<FProperty>(GroomAsset->GetClass(), GET_MEMBER_NAME_CHECKED(UGroomAsset, EnableGlobalInterpolation));
	FProperty* P0_1 = FindFProperty<FProperty>(GroomAsset->GetClass(), GET_MEMBER_NAME_CHECKED(UGroomAsset, HairInterpolationType));

	P7->SetMetaData(TEXT("Category"), TEXT("Hidden"));

	// Make the source file property hidden
	FProperty* SourceFileProperty = FindFProperty<FProperty>(GroomAsset->GetClass(), GET_MEMBER_NAME_CHECKED(UGroomAsset, AssetUserData));
	SourceFileProperty->RemoveMetaData(TEXT("ShowOnlyInnerProperties"));

	P0->RemoveMetaData(TEXT("ShowOnlyInnerProperties"));
	P1->RemoveMetaData(TEXT("ShowOnlyInnerProperties"));
	P2->RemoveMetaData(TEXT("ShowOnlyInnerProperties"));
	P3->RemoveMetaData(TEXT("ShowOnlyInnerProperties"));
	P4->RemoveMetaData(TEXT("ShowOnlyInnerProperties"));
	P5->RemoveMetaData(TEXT("ShowOnlyInnerProperties"));
	P6->RemoveMetaData(TEXT("ShowOnlyInnerProperties"));
	P7->RemoveMetaData(TEXT("ShowOnlyInnerProperties"));
	P8->RemoveMetaData(TEXT("ShowOnlyInnerProperties"));

	// Set the asset we are editing in the details view
	if (DetailView_InterpolationProperties.IsValid())
	{
		P0->SetMetaData(TEXT("Category"), TEXT("Interpolation"));
		P0_0->SetMetaData(TEXT("Category"), TEXT("Interpolation"));
		P0_1->SetMetaData(TEXT("Category"), TEXT("Interpolation"));
		P1->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P2->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P3->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P4->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P5->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P6->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P7->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P8->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		
		DetailView_InterpolationProperties->SetObject(Cast<UObject>(GroomAsset));
	}

	if (DetailView_RenderingProperties.IsValid())
	{
		P0->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P0_0->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P0_1->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P1->SetMetaData(TEXT("Category"), TEXT("Rendering"));
		P2->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P3->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P4->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P5->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P6->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P7->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P8->SetMetaData(TEXT("Category"), TEXT("Hidden"));

		DetailView_RenderingProperties->SetObject(Cast<UObject>(GroomAsset));
	}

	if (DetailView_PhysicsProperties.IsValid())
	{
		P0->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P0_0->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P0_1->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P1->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P2->SetMetaData(TEXT("Category"), TEXT("Physics"));
		P3->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P4->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P5->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P6->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P7->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P8->SetMetaData(TEXT("Category"), TEXT("Hidden"));

		DetailView_PhysicsProperties->SetObject(Cast<UObject>(GroomAsset));
	}

	if (DetailView_CardsProperties.IsValid())
	{
		P0->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P0_0->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P0_1->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P1->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P2->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P3->SetMetaData(TEXT("Category"), TEXT("Cards"));
		P4->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P5->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P6->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P7->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P8->SetMetaData(TEXT("Category"), TEXT("Hidden"));

		DetailView_CardsProperties->SetObject(Cast<UObject>(GroomAsset));
	}

	if (DetailView_MeshesProperties.IsValid())
	{
		P0->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P0_0->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P0_1->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P1->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P2->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P3->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P4->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P5->SetMetaData(TEXT("Category"), TEXT("Meshes"));
		P6->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P7->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P8->SetMetaData(TEXT("Category"), TEXT("Hidden"));

		DetailView_MeshesProperties->SetObject(Cast<UObject>(GroomAsset));
	}

	if (DetailView_LODProperties.IsValid())
	{
		P0->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P0_0->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P0_1->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P1->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P2->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P3->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P4->SetMetaData(TEXT("Category"), TEXT("LOD"));
		P5->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P6->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P7->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P8->SetMetaData(TEXT("Category"), TEXT("LOD"));

		DetailView_LODProperties->SetObject(Cast<UObject>(GroomAsset));
	}

	if (DetailView_MaterialProperties.IsValid())
	{
		P0->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P0_0->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P0_1->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P1->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P2->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P3->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P4->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P5->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P6->SetMetaData(TEXT("Category"), TEXT("Hidden")); // Disabled as we have a custom widget displaying the properties
		P7->SetMetaData(TEXT("Category"), TEXT("Hidden"));
		P8->SetMetaData(TEXT("Category"), TEXT("Hidden"));

		DetailView_MaterialProperties->SetObject(Cast<UObject>(GroomAsset));
	}

#if GROOMEDITOR_ENABLE_COMPONENT_PANEL
	if (DetailView_PreviewGroomComponent.IsValid())
	{
		DetailView_PreviewGroomComponent->SetObject(PreviewGroomComponent.Get());
	}
#endif

	ExtendToolbar();
	RegenerateMenusAndToolbars();

	const TSharedRef<FUICommandList>& CommandList = GetToolkitCommands();
		
	CommandList->MapAction(FGroomEditorCommands::Get().ResetSimulation,
			FExecuteAction::CreateSP(this, &FGroomCustomAssetEditorToolkit::OnResetSimulation),
			FCanExecuteAction::CreateSP(this, &FGroomCustomAssetEditorToolkit::CanResetSimulation));

	CommandList->MapAction(FGroomEditorCommands::Get().PauseSimulation,
			FExecuteAction::CreateSP(this, &FGroomCustomAssetEditorToolkit::OnPauseSimulation),
			FCanExecuteAction::CreateSP(this, &FGroomCustomAssetEditorToolkit::CanResetSimulation));

	CommandList->MapAction(FGroomEditorCommands::Get().PlaySimulation,
			FExecuteAction::CreateSP(this, &FGroomCustomAssetEditorToolkit::OnPlaySimulation),
			FCanExecuteAction::CreateSP(this, &FGroomCustomAssetEditorToolkit::CanPlaySimulation));
}

void FGroomCustomAssetEditorToolkit::OnPlaySimulation()
{
	//if (NiagaraComponent != nullptr)
	//{
	//	NiagaraComponent->SetPaused(false);
	//}
}
bool FGroomCustomAssetEditorToolkit::CanPlaySimulation() const
{
	return true;
}

void FGroomCustomAssetEditorToolkit::OnPauseSimulation()
{
	//if (NiagaraComponent != nullptr)
	//{
	//	NiagaraComponent->SetPaused(true);
	//}
}

bool FGroomCustomAssetEditorToolkit::CanPauseSimulation() const
{
	return true;
}

void FGroomCustomAssetEditorToolkit::OnResetSimulation()
{
	//if (NiagaraComponent != nullptr)
	//{
	//	NiagaraComponent->Activate(true);
	//}	
}

bool FGroomCustomAssetEditorToolkit::CanResetSimulation() const
{
	return true;
}

FGroomCustomAssetEditorToolkit::FGroomCustomAssetEditorToolkit()
{
	GroomAsset						= nullptr;	
	PreviewGroomComponent			= nullptr;	
	PreviewStaticMeshComponent		= nullptr;
	PreviewSkeletalMeshComponent	= nullptr;

	// Set LOD debug mode to show current LOD index & screen size
	if (GOpenedGroomEditorCount == 0)
	{
		SetHairScreenLODInfo(true);
	}
	++GOpenedGroomEditorCount;
}

FGroomCustomAssetEditorToolkit::~FGroomCustomAssetEditorToolkit()
{
	// Disable LOD debug mode info
	--GOpenedGroomEditorCount;
	if (GOpenedGroomEditorCount == 0)
	{
		SetHairScreenLODInfo(false);
	}
	check(GOpenedGroomEditorCount >= 0);
}

FText FGroomCustomAssetEditorToolkit::GetToolkitName() const
{
	return FText::FromString(GroomAsset->GetName());
}

FName FGroomCustomAssetEditorToolkit::GetToolkitFName() const
{
	return ToolkitFName;
}

FText FGroomCustomAssetEditorToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "Groom Asset Editor");
}

FText FGroomCustomAssetEditorToolkit::GetToolkitToolTipText() const
{
	return LOCTEXT("ToolTip", "Groom Asset Editor");
}

FString FGroomCustomAssetEditorToolkit::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "AnimationDatabase ").ToString();
}

FLinearColor FGroomCustomAssetEditorToolkit::GetWorldCentricTabColorScale() const
{
	return FColor::Red;
}

UGroomAsset* FGroomCustomAssetEditorToolkit::GetCustomAsset() const
{
	return GroomAsset.Get();
}

void FGroomCustomAssetEditorToolkit::SetCustomAsset(UGroomAsset* InCustomAsset)
{
	GroomAsset = InCustomAsset;
}

TSharedRef<SDockTab> FGroomCustomAssetEditorToolkit::SpawnTab_CardsProperties(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == TabId_CardsProperties);

	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("GenericEditor.Tabs.Properties"))
		.Label(LOCTEXT("CardsPropertiesTab", "Cards"))
		.TabColorScale(GetTabColorScale())
		[
			DetailView_CardsProperties.ToSharedRef()
		];
}

TSharedRef<SDockTab> FGroomCustomAssetEditorToolkit::SpawnTab_MeshesProperties(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == TabId_MeshesProperties);

	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("GenericEditor.Tabs.Properties"))
		.Label(LOCTEXT("MeshesPropertiesTab", "Meshes"))
		.TabColorScale(GetTabColorScale())
		[
			DetailView_MeshesProperties.ToSharedRef()
		];
}

TSharedRef<SDockTab> FGroomCustomAssetEditorToolkit::SpawnTab_MaterialProperties(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == TabId_MaterialProperties);

	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("GenericEditor.Tabs.Properties"))
		.Label(LOCTEXT("MaterialPropertiesTab", "Material"))
		.TabColorScale(GetTabColorScale())
		[
			DetailView_MaterialProperties.ToSharedRef()
		];
}

TSharedRef<SDockTab> FGroomCustomAssetEditorToolkit::SpawnTab_PhysicsProperties(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == TabId_PhysicsProperties);

	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("GenericEditor.Tabs.Properties"))
		.Label(LOCTEXT("PhysicsPropertiesTab", "Physics"))
		.TabColorScale(GetTabColorScale())
		[
			DetailView_PhysicsProperties.ToSharedRef()
		];
}

TSharedRef<SDockTab> FGroomCustomAssetEditorToolkit::SpawnTab_RenderingProperties(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == TabId_RenderingProperties);

	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("GenericEditor.Tabs.Properties"))
		.Label(LOCTEXT("RenderingPropertiesTab", "Strands"))
		.TabColorScale(GetTabColorScale())
		[
			DetailView_RenderingProperties.ToSharedRef()
		];
}

TSharedRef<SDockTab>  FGroomCustomAssetEditorToolkit::SpawnTab_InterpolationProperties(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == TabId_InterpolationProperties);

	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("GenericEditor.Tabs.Properties"))
		.Label(LOCTEXT("InterpolationPropertiesTab", "Interpolation"))
		.TabColorScale(GetTabColorScale())
		[
			DetailView_InterpolationProperties.ToSharedRef()
		];
}

TSharedRef<SDockTab>  FGroomCustomAssetEditorToolkit::SpawnTab_LODProperties(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == TabId_LODProperties);

	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("GenericEditor.Tabs.Properties"))
		.Label(LOCTEXT("LODPropertiesTab", "LOD"))
		.TabColorScale(GetTabColorScale())
		[
			DetailView_LODProperties.ToSharedRef()
		];
}

TSharedRef<SDockTab> FGroomCustomAssetEditorToolkit::SpawnTab_PreviewGroomComponent(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == TabId_PreviewGroomComponent);

	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("GenericEditor.Tabs.Properties"))
		.Label(LOCTEXT("GroomComponentTab", "Component"))
		.TabColorScale(GetTabColorScale())
		[
			DetailView_PreviewGroomComponent.ToSharedRef()
		];
}

UGroomComponent *FGroomCustomAssetEditorToolkit::GetPreview_GroomComponent() const
{	
	return PreviewGroomComponent.Get();
}

UStaticMeshComponent *FGroomCustomAssetEditorToolkit::GetPreview_StaticMeshComponent() const
{	
	return PreviewStaticMeshComponent.Get();
}

USkeletalMeshComponent *FGroomCustomAssetEditorToolkit::GetPreview_SkeletalMeshComponent() const
{
	return PreviewSkeletalMeshComponent.Get();
}

TSharedRef<SDockTab> FGroomCustomAssetEditorToolkit::SpawnViewportTab(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == TabId_Viewport);
	check(GetPreview_GroomComponent());
//	check(GetPreview_StaticMeshComponent());
//	check(GetPreview_SkeletalMeshComponent());

	ViewportTab->SetGroomComponent(GetPreview_GroomComponent());
	ViewportTab->SetStaticMeshComponent(GetPreview_StaticMeshComponent());
	ViewportTab->SetSkeletalMeshComponent(GetPreview_SkeletalMeshComponent());

	return SNew(SDockTab)
		.Icon(FEditorStyle::GetBrush("GenericEditor.Tabs.Render"))
		.Label(LOCTEXT("RenderTitle", "Render"))
		.TabColorScale(GetTabColorScale())
		[
			ViewportTab.ToSharedRef()
		];
}

#undef LOCTEXT_NAMESPACE
