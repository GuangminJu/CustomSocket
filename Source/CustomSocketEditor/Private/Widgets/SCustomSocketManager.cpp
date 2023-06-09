// Copyright Epic Games, Inc. All Rights Reserved.

#include "SCustomSocketManager.h"
#include "Widgets/Layout/SSplitter.h"
#include "UObject/UnrealType.h"
#include "Engine/StaticMesh.h"
#include "Modules/ModuleManager.h"
#include "UObject/UObjectHash.h"
#include "UObject/UObjectIterator.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Views/SListView.h"
#include "EditorStyleSet.h"
#include "Components/StaticMeshComponent.h"
#include "Editor/UnrealEdEngine.h"
#include "Engine/StaticMeshSocket.h"
#include "UnrealEdGlobals.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Editor/StaticMeshEditor/Public/IStaticMeshEditor.h"

#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"

#include "ScopedTransaction.h"

#include "Runtime/Analytics/Analytics/Public/Interfaces/IAnalyticsProvider.h"
#include "EngineAnalytics.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "Framework/Commands/GenericCommands.h"
#include "SeatSocket/SeatSocket.h"
#include "Windows/WindowsPlatformApplicationMisc.h"

#define LOCTEXT_NAMESPACE "SSCSSocketManagerEditor"

struct SocketListItem
{
public:
	SocketListItem(USeatSocket* InSocket)
		: Socket(InSocket)
	{
	}

	/** The static mesh socket this represents */
	USeatSocket* Socket;

	/** Delegate for when the context menu requests a rename */
	DECLARE_DELEGATE(FOnRenameRequested);
	FOnRenameRequested OnRenameRequested;
};

class SSocketDisplayItem : public STableRow<TSharedPtr<FString>>
{
public:
	SLATE_BEGIN_ARGS(SSocketDisplayItem)
		{
		}

		/** The socket this item displays. */
		SLATE_ARGUMENT(TWeakPtr< SocketListItem >, SocketItem)

		/** Pointer back to the socket manager */
		SLATE_ARGUMENT(TWeakPtr< SCustomSocketManager >, SocketManagerPtr)
	SLATE_END_ARGS()

	/**
	 * Construct the widget
	 *
	 * @param InArgs   A declaration from which to construct the widget
	 */
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
	{
		SocketItem = InArgs._SocketItem;
		SocketManagerPtr = InArgs._SocketManagerPtr;

		TSharedPtr<SInlineEditableTextBlock> InlineWidget;

		this->ChildSlot
		    .Padding(0.0f, 3.0f, 6.0f, 3.0f)
		    .VAlign(VAlign_Center)
		[
			SAssignNew(InlineWidget, SInlineEditableTextBlock)
				.Text(this, &SSocketDisplayItem::GetSocketName)
				.OnVerifyTextChanged(this, &SSocketDisplayItem::OnVerifySocketNameChanged)
				.OnTextCommitted(this, &SSocketDisplayItem::OnCommitSocketName)
				.IsSelected(this, &STableRow<TSharedPtr<FString>>::IsSelectedExclusively)
		];

		TSharedPtr<SocketListItem> SocketItemPinned = SocketItem.Pin();
		if (SocketItemPinned.IsValid())
		{
			SocketItemPinned->OnRenameRequested.BindSP(InlineWidget.Get(), &SInlineEditableTextBlock::EnterEditingMode);
		}

		STableRow<TSharedPtr<FString>>::ConstructInternal(
			STableRow::FArguments()
			.ShowSelection(true),
			InOwnerTableView
		);
	}

private:
	/** Returns the socket name */
	FText GetSocketName() const
	{
		TSharedPtr<SocketListItem> SocketItemPinned = SocketItem.Pin();
		return SocketItemPinned.IsValid() ? FText::FromName(SocketItemPinned->Socket->Name) : FText();
	}

	bool OnVerifySocketNameChanged(const FText& InNewText, FText& OutErrorMessage)
	{
		bool bVerifyName = true;

		FText NewText = FText::TrimPrecedingAndTrailing(InNewText);
		if (NewText.IsEmpty())
		{
			OutErrorMessage = LOCTEXT("EmptySocketName_Error", "Sockets must have a name!");
			bVerifyName = false;
		}
		else
		{
			TSharedPtr<SocketListItem> SocketItemPinned = SocketItem.Pin();
			TSharedPtr<SCustomSocketManager> SocketManagerPinned = SocketManagerPtr.Pin();

			if (SocketItemPinned.IsValid() && SocketItemPinned->Socket != nullptr && SocketItemPinned->Socket->
				Name.ToString() != NewText.ToString() &&
				SocketManagerPinned.IsValid() && SocketManagerPinned->CheckForDuplicateSocket(NewText.ToString()))
			{
				OutErrorMessage = LOCTEXT("DuplicateSocket_Error", "Socket name in use!");
				bVerifyName = false;
			}
		}

		return bVerifyName;
	}

	void OnCommitSocketName(const FText& InText, ETextCommit::Type CommitInfo)
	{
		FText NewText = FText::TrimPrecedingAndTrailing(InText);

		TSharedPtr<SocketListItem> PinnedSocketItem = SocketItem.Pin();
		if (PinnedSocketItem.IsValid())
		{
			USeatSocket* SelectedSocket = PinnedSocketItem->Socket;
			if (SelectedSocket != NULL)
			{
				FScopedTransaction Transaction(LOCTEXT("SetSocketName", "Set Socket Name"));

				FProperty* ChangedProperty = FindFProperty<FProperty>(UStaticMeshSocket::StaticClass(), "SocketName");

				// Pre edit, calls modify on the object
				SelectedSocket->PreEditChange(ChangedProperty);

				// Edit the property itself
				SelectedSocket->Name = FName(*NewText.ToString());

				// Post edit
				FPropertyChangedEvent PropertyChangedEvent(ChangedProperty);
				SelectedSocket->PostEditChangeProperty(PropertyChangedEvent);
			}
		}
	}

private:
	/** The Socket to display. */
	TWeakPtr<SocketListItem> SocketItem;

	/** Pointer back to the socket manager */
	TWeakPtr<SCustomSocketManager> SocketManagerPtr;
};

void SCustomSocketManager::Construct(const FArguments& InArgs)
{
	StaticMeshSocketEditor = InArgs._StaticMeshSocketEditor;

	OnSocketSelectionChanged = InArgs._OnSocketSelectionChanged;
	SeatMap = InArgs._SeatMap;

	StaticMeshSocketEditor->OnStaticMeshChanged.AddRaw(this, &SCustomSocketManager::SetStaticMesh);

	FDetailsViewArgs Args;
	Args.bHideSelectionTip = true;
	Args.bLockable = false;
	Args.bAllowSearch = false;
	Args.bShowOptions = false;
	Args.NotifyHook = this;
	Args.NameAreaSettings = FDetailsViewArgs::HideNameArea;

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	SocketDetailsView = PropertyModule.CreateDetailView(Args);

	WorldSpaceRotation = FVector::ZeroVector;

	this->ChildSlot
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		[
			SNew(SSplitter)
			.Orientation(Orient_Horizontal)

			+ SSplitter::Slot()
			.Value(.3f)
			[
				SNew(SBorder)
				.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					  .AutoHeight()
					  .Padding(0, 0, 0, 4)
					[
						SNew(SButton)
						.ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
						.ForegroundColor(FLinearColor::White)
						.Text(LOCTEXT("CopySeats", "Copy Seats"))
						.OnClicked(this, &SCustomSocketManager::CopySeats_Execute)
						.HAlign(HAlign_Center)
					]

					+ SVerticalBox::Slot()
					  .AutoHeight()
					  .Padding(0, 0, 0, 4)
					[
						SNew(SButton)
						.ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
						.ForegroundColor(FLinearColor::White)
						.Text(LOCTEXT("CreateSocket", "Create Seat Socket"))
						.OnClicked(this, &SCustomSocketManager::CreateSeatSocket_Execute)
						.HAlign(HAlign_Center)
					]

					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					[
						SAssignNew(SocketListView, SListView<TSharedPtr< SocketListItem > >)

						.SelectionMode(ESelectionMode::Single)

						.ListItemsSource(&SocketList)

						// Generates the actual widget for a tree item
						.OnGenerateRow(this, &SCustomSocketManager::MakeWidgetFromOption)

						// Find out when the user selects something in the tree
						.OnSelectionChanged(this, &SCustomSocketManager::SocketSelectionChanged_Execute)

						// Allow for some spacing between items with a larger item height.
						.ItemHeight(20.0f)

						.OnContextMenuOpening(this, &SCustomSocketManager::OnContextMenuOpening)
						.OnItemScrolledIntoView(this, &SCustomSocketManager::OnItemScrolledIntoView)

						.HeaderRow
						(
							SNew(SHeaderRow)
							.Visibility(EVisibility::Collapsed)
							+ SHeaderRow::Column(TEXT("Socket"))
						)
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SSeparator)
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(this, &SCustomSocketManager::GetSocketHeaderText)
					]
				]
			]

			+ SSplitter::Slot()
			.Value(.7f)
			[
				SNew(SOverlay)

				+ SOverlay::Slot()
				[
					SNew(SBorder)
					.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.Visibility(this, &SCustomSocketManager::GetSelectSocketMessageVisibility)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("NoSocketSelected", "Select a Socket"))
					]
				]

				+ SOverlay::Slot()
				[
					SocketDetailsView.ToSharedRef()
				]
			]
		]
	];

	RefreshSocketList();

	AddPropertyChangeListenerToSockets();
}

SCustomSocketManager::~SCustomSocketManager()
{
	RemovePropertyChangeListenerFromSockets();
}

USeatSocket* SCustomSocketManager::GetSelectedSocket() const
{
	if (SocketListView->GetSelectedItems().Num())
	{
		return SocketListView->GetSelectedItems()[0]->Socket;
	}

	return nullptr;
}

EVisibility SCustomSocketManager::GetSelectSocketMessageVisibility() const
{
	return SocketListView->GetSelectedItems().Num() > 0 ? EVisibility::Hidden : EVisibility::Visible;
}

void SCustomSocketManager::SetSelectedSocket(USeatSocket* InSelectedSocket)
{
	if (InSelectedSocket)
	{
		for (int32 i = 0; i < SocketList.Num(); i++)
		{
			if (SocketList[i]->Socket == InSelectedSocket)
			{
				SocketListView->SetSelection(SocketList[i]);

				SocketListView->RequestListRefresh();

				SocketSelectionChanged(InSelectedSocket);

				break;
			}
		}
	}
	else
	{
		SocketListView->ClearSelection();

		SocketListView->RequestListRefresh();

		SocketSelectionChanged(NULL);
	}
}

TSharedRef<ITableRow> SCustomSocketManager::MakeWidgetFromOption(TSharedPtr<SocketListItem> InItem,
                                                                 const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SSocketDisplayItem, OwnerTable)
				.SocketItem(InItem)
				.SocketManagerPtr(SharedThis(this));
}

void SCustomSocketManager::CreateSeatSocket()
{
	if (StaticMeshSocketEditor)
	{
		UStaticMesh* CurrentStaticMesh = StaticMeshSocketEditor->GetStaticMesh();

		const FScopedTransaction Transaction(LOCTEXT("CreateSocket", "Create Socket"));

		USeatSocket* NewSocket = NewObject<USeatSocket>(SeatMap);
		check(NewSocket);

		if (FEngineAnalytics::IsAvailable())
		{
			FEngineAnalytics::GetProvider().RecordEvent(TEXT("Editor.Usage.StaticMesh.CreateSocket"));
		}

		FString SocketNameString = TEXT("Socket");
		FName SocketName = FName(*SocketNameString);

		// Make sure the new name is valid
		int32 Index = 0;
		while (CheckForDuplicateSocket(SocketName.ToString()))
		{
			SocketName = FName(*FString::Printf(TEXT("%s%i"), *SocketNameString, Index));
			++Index;
		}


		NewSocket->Name = SocketName;
		NewSocket->SetFlags(RF_Transactional);
		NewSocket->OnPropertyChanged().AddSP(this, &SCustomSocketManager::OnSocketPropertyChanged);

		SeatMap->PreEditChange(NULL);
		SeatMap->AddSeat(CurrentStaticMesh, NewSocket);
		SeatMap->PostEditChange();
		SeatMap->MarkPackageDirty();

		TSharedPtr<SocketListItem> SocketItem = MakeShareable(new SocketListItem(NewSocket));
		SocketList.Add(SocketItem);
		SocketListView->RequestListRefresh();

		SocketListView->SetSelection(SocketItem);
		RequestRenameSelectedSocket();
	}
}

void SCustomSocketManager::CopySeat()
{
	FSeats Seats = SeatMap->GetSeats(StaticMesh.Get());

	FString Names;
	FString Types;
	FString Positions;
	FString Rotations;
	FString Postures;
	FString Scopes;

	for (const USeatSocket* Seat : Seats.Seats)
	{
		if (!Names.IsEmpty())
			Names.Append(",");

		Names.Append(Seat->Name.ToString());

		if (!Types.IsEmpty())
			Types.Append(",");

		Types.Append(FString::FromInt(static_cast<int32>(Seat->SeatType)));

		if (!Positions.IsEmpty())
			Positions.Append(",");

		Positions.Append(FString::Printf(TEXT("(%f,%f,%f)"), Seat->RelativeLocation.X, Seat->RelativeLocation.Y,
		                                 Seat->RelativeLocation.Z));

		if (!Rotations.IsEmpty())
			Rotations.Append(",");

		Rotations.Append(FString::Printf(TEXT("(%f,%f,%f)"),
		                                 Seat->RelativeRotation.Pitch, Seat->RelativeRotation.Yaw,
		                                 Seat->RelativeRotation.Roll));

		if (!Postures.IsEmpty())
			Postures.Append(",");

		Postures.Append(FString::FromInt(static_cast<int32>(Seat->Posture)));

		if (!Scopes.IsEmpty())
			Scopes.Append(",");

		Scopes.Append(FString::Printf(TEXT("(%f,%f,%f,%f)"), Seat->PitchScope * 0.5, Seat->PitchScope * -0.5,
		                              Seat->YawScope * 0.5, Seat->YawScope * -0.5));
	}

	FString SeatString = Names + "\t" + Types + "\t" + Positions + "\t" + Rotations + "\t" + Postures + "\t" + Scopes;
	FPlatformApplicationMisc::ClipboardCopy(*SeatString);
}

void SCustomSocketManager::DuplicateSelectedSocket()
{
	USeatSocket* SelectedSocket = GetSelectedSocket();

	if (StaticMeshSocketEditor && SelectedSocket)
	{
		const FScopedTransaction Transaction(LOCTEXT("SocketManager_DuplicateSocket", "Duplicate Socket"));

		UStaticMesh* CurrentStaticMesh = StaticMeshSocketEditor->GetStaticMesh();

		USeatSocket* NewSocket = DuplicateObject(SelectedSocket, CurrentStaticMesh);

		// Create a unique name for this socket
		NewSocket->Name = MakeUniqueObjectName(CurrentStaticMesh, UStaticMeshSocket::StaticClass(),
		                                       NewSocket->Name);

		// Add the new socket to the static mesh
		SeatMap->PreEditChange(NULL);

		SeatMap->AddSeat(CurrentStaticMesh, NewSocket);
		SeatMap->PostEditChange();
		SeatMap->MarkPackageDirty();

		RefreshSocketList();

		// Select the duplicated socket
		SetSelectedSocket(NewSocket);
	}
}


void SCustomSocketManager::UpdateStaticMesh()
{
	RefreshSocketList();
}

void SCustomSocketManager::RequestRenameSelectedSocket()
{
	if (SocketListView->GetSelectedItems().Num() == 1)
	{
		TSharedPtr<SocketListItem> SocketItem = SocketListView->GetSelectedItems()[0];
		SocketListView->RequestScrollIntoView(SocketItem);
		DeferredRenameRequest = SocketItem;
	}
}

void SCustomSocketManager::DeleteSelectedSocket()
{
	if (SocketListView->GetSelectedItems().Num())
	{
		const FScopedTransaction Transaction(LOCTEXT("DeleteSocket", "Delete Socket"));

		if (StaticMeshSocketEditor)
		{
			UStaticMesh* CurrentStaticMesh = StaticMeshSocketEditor->GetStaticMesh();
			SeatMap->PreEditChange(NULL);
			USeatSocket* SelectedSocket = SocketListView->GetSelectedItems()[0]->Socket;
			SelectedSocket->OnPropertyChanged().RemoveAll(this);
			SeatMap->RemoveSeat(CurrentStaticMesh, SelectedSocket);
			SeatMap->PostEditChange();

			RefreshSocketList();
		}
	}
}

void SCustomSocketManager::RefreshSocketList()
{
	if (StaticMeshSocketEditor)
	{
		bool bIsSameStaticMesh = true;
		UStaticMesh* CurrentStaticMesh = StaticMeshSocketEditor->GetStaticMesh();
		if (!StaticMesh.IsValid() || CurrentStaticMesh != StaticMesh.Get())
		{
			StaticMesh = CurrentStaticMesh;
			bIsSameStaticMesh = false;
		}
		// Only rebuild the socket list if it differs from the static meshes socket list
		// This is done so that an undo on a socket property doesn't cause the selected
		// socket to be de-selected, thus hiding the socket properties on the detail view.
		// NB: Also force a rebuild if the underlying StaticMesh has been changed.
		const TArray<USeatSocket*>& Sockets = SeatMap->GetSeats(CurrentStaticMesh).Seats;
		if (Sockets.Num() != SocketList.Num() || !bIsSameStaticMesh)
		{
			SocketList.Empty();
			for (int32 i = 0; i < Sockets.Num(); i++)
			{
				USeatSocket* Socket = Sockets[i];
				SocketList.Add(MakeShareable(new SocketListItem(Socket)));
			}

			SocketListView->RequestListRefresh();
		}

		// Set the socket on the detail view to keep it in sync with the sockets properties
		if (SocketListView->GetSelectedItems().Num())
		{
			TArray<UObject*> ObjectList;
			ObjectList.Add(SocketListView->GetSelectedItems()[0]->Socket);
			SocketDetailsView->SetObjects(ObjectList, true);
		}

		// TODO
		// StaticMeshEditorPinned->RefreshViewport();
	}
	else
	{
		SocketList.Empty();
		SocketListView->ClearSelection();
		SocketListView->RequestListRefresh();
	}
}

bool SCustomSocketManager::CheckForDuplicateSocket(const FString& InSocketName)
{
	for (int32 i = 0; i < SocketList.Num(); i++)
	{
		if (SocketList[i]->Socket->Name.ToString() == InSocketName)
		{
			return true;
		}
	}

	return false;
}

void SCustomSocketManager::SocketSelectionChanged(USeatSocket* InSocket)
{
	TArray<UObject*> SelectedObject;

	if (InSocket)
	{
		SelectedObject.Add(InSocket);
	}

	SocketDetailsView->SetObjects(SelectedObject);

	// Notify listeners
	OnSocketSelectionChanged.ExecuteIfBound();
}

void SCustomSocketManager::SocketSelectionChanged_Execute(TSharedPtr<SocketListItem> InItem,
                                                          ESelectInfo::Type /*SelectInfo*/)
{
	if (InItem.IsValid())
	{
		SocketSelectionChanged(InItem->Socket);
	}
	else
	{
		SocketSelectionChanged(NULL);
	}
}

FReply SCustomSocketManager::CreateSeatSocket_Execute()
{
	CreateSeatSocket();

	return FReply::Handled();
}

FReply SCustomSocketManager::CopySeats_Execute()
{
	CopySeat();

	return FReply::Handled();
}

FText SCustomSocketManager::GetSocketHeaderText() const
{
	UStaticMesh* CurrentStaticMesh = nullptr;

	if (StaticMeshSocketEditor)
	{
		CurrentStaticMesh = StaticMeshSocketEditor->GetStaticMesh();
	}
	return FText::Format(
		LOCTEXT("SocketHeader_TotalFmt", "{0} sockets"),
		FText::AsNumber((CurrentStaticMesh != nullptr) ? CurrentStaticMesh->Sockets.Num() : 0));
}

void SCustomSocketManager::SocketName_TextChanged(const FText& InText)
{
	CheckForDuplicateSocket(InText.ToString());
}

TSharedPtr<SWidget> SCustomSocketManager::OnContextMenuOpening()
{
	const bool bShouldCloseWindowAfterMenuSelection = true;

	if (!StaticMeshSocketEditor)
	{
		return TSharedPtr<SWidget>();
	}

	// TODO
	FMenuBuilder MenuBuilder(bShouldCloseWindowAfterMenuSelection,
	                         TSharedPtr<const FUICommandList>()/*StaticMeshEditorPinned->GetToolkitCommands()*/);

	{
		MenuBuilder.BeginSection("BasicOperations");
		{
			MenuBuilder.AddMenuEntry(FGenericCommands::Get().Delete);
			MenuBuilder.AddMenuEntry(FGenericCommands::Get().Duplicate);
			MenuBuilder.AddMenuEntry(FGenericCommands::Get().Rename);
		}
		MenuBuilder.EndSection();
	}

	return MenuBuilder.MakeWidget();
}

void SCustomSocketManager::NotifyPostChange(const FPropertyChangedEvent& PropertyChangedEvent,
                                            FProperty* PropertyThatChanged)
{
	TArray<TSharedPtr<SocketListItem>> SelectedList = SocketListView->GetSelectedItems();
	if (SelectedList.Num())
	{
		if (PropertyThatChanged->GetName() == TEXT("Pitch") || PropertyThatChanged->GetName() == TEXT("Yaw") ||
			PropertyThatChanged->GetName() == TEXT("Roll"))
		{
			const USeatSocket* Socket = SelectedList[0]->Socket;
			WorldSpaceRotation.Set(Socket->RelativeRotation.Pitch, Socket->RelativeRotation.Yaw,
			                       Socket->RelativeRotation.Roll);
		}
	}
}

void SCustomSocketManager::AddPropertyChangeListenerToSockets()
{
	if (StaticMeshSocketEditor)
	{
		UStaticMesh* CurrentStaticMesh = StaticMeshSocketEditor->GetStaticMesh();
		FSeats Seats = SeatMap->GetSeats(CurrentStaticMesh);
		for (int32 i = 0; i < Seats.Seats.Num(); ++i)
		{
			Seats.Seats[i]->OnPropertyChanged().AddSP(
				this, &SCustomSocketManager::OnSocketPropertyChanged);
		}
	}
}

void SCustomSocketManager::RemovePropertyChangeListenerFromSockets()
{
	if (StaticMeshSocketEditor)
	{
		UStaticMesh* CurrentStaticMesh = StaticMeshSocketEditor->GetStaticMesh();
		if (CurrentStaticMesh)
		{
			for (int32 i = 0; i < CurrentStaticMesh->Sockets.Num(); ++i)
			{
				CurrentStaticMesh->Sockets[i]->OnPropertyChanged().RemoveAll(this);
			}
		}
	}
}

void SCustomSocketManager::OnSocketPropertyChanged(const USeatSocket* Socket, const FProperty* ChangedProperty)
{
	static FName RelativeRotationName(TEXT("RelativeRotation"));
	static FName RelativeLocationName(TEXT("RelativeLocation"));

	check(Socket != nullptr);

	FName ChangedPropertyName = ChangedProperty->GetFName();

	if (ChangedPropertyName == RelativeRotationName)
	{
		const USeatSocket* SelectedSocket = GetSelectedSocket();

		if (Socket == SelectedSocket)
		{
			WorldSpaceRotation.Set(Socket->RelativeRotation.Pitch, Socket->RelativeRotation.Yaw,
			                       Socket->RelativeRotation.Roll);
		}
	}

	if (!StaticMeshSocketEditor)
	{
		return;
	}

	if (ChangedPropertyName == RelativeRotationName || ChangedPropertyName == RelativeLocationName)
	{
		// If socket location or rotation is changed, update the position of any actors attached to it in instances of this mesh
		UStaticMesh* CurrentStaticMesh = StaticMeshSocketEditor->GetStaticMesh();
		if (CurrentStaticMesh != nullptr)
		{
			bool bUpdatedChild = false;

			for (TObjectIterator<UStaticMeshComponent> It; It; ++It)
			{
				if (It->GetStaticMesh() == CurrentStaticMesh)
				{
					const AActor* Actor = It->GetOwner();
					if (Actor != nullptr)
					{
						const USceneComponent* Root = Actor->GetRootComponent();
						if (Root != nullptr)
						{
							for (USceneComponent* Child : Root->GetAttachChildren())
							{
								if (Child != nullptr && Child->GetAttachSocketName() == Socket->Name)
								{
									Child->UpdateComponentToWorld();
									bUpdatedChild = true;
								}
							}
						}
					}
				}
			}

			if (bUpdatedChild)
			{
				GUnrealEd->RedrawLevelEditingViewports();
			}
		}
	}
}

void SCustomSocketManager::PostUndo()
{
	RefreshSocketList();
}

void SCustomSocketManager::OnItemScrolledIntoView(TSharedPtr<SocketListItem> InItem,
                                                  const TSharedPtr<ITableRow>& InWidget)
{
	TSharedPtr<SocketListItem> DeferredRenameRequestPinned = DeferredRenameRequest.Pin();
	if (DeferredRenameRequestPinned.IsValid())
	{
		DeferredRenameRequestPinned->OnRenameRequested.ExecuteIfBound();
		DeferredRenameRequest.Reset();
	}
}

void SCustomSocketManager::SetStaticMesh(UStaticMesh* InStaticMesh)
{
	RefreshSocketList();
}

#undef LOCTEXT_NAMESPACE
