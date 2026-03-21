#include "McpVersionCompatibility.h"
#include "McpHandlerUtils.h"
#include "McpAutomationBridgeGlobals.h"
#include "Dom/JsonObject.h"
#include "McpAutomationBridgeHelpers.h"
#include "McpAutomationBridgeSubsystem.h"
#include "McpHandlerDeclarations.h"

#if WITH_EDITOR
#include "PCGGraph.h"
#include "PCGNode.h"
#include "PCGEdge.h"
#include "PCGComponent.h"
#include "PCGSettings.h"
#include "Metadata/PCGMetadata.h"
#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "PackageTools.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

namespace McpHandlers
{
namespace PCG
{

bool PCGAction(const FString& RequestId, const FString& Action, const TSharedPtr<FJsonObject>& Payload, TSharedPtr<FMcpBridgeWebSocket> Socket)
{
    FString SubAction;
    if (!Payload->TryGetStringField(TEXT("action"), SubAction))
    {
        return McpHandlerUtils::SendErrorResponse(RequestId, Socket, TEXT("Missing sub-action in PCG payload"));
    }

    if (SubAction == TEXT("create_graph"))
    {
        FString Name, Path;
        if (!Payload->TryGetStringField(TEXT("name"), Name) || !Payload->TryGetStringField(TEXT("path"), Path))
        {
            return McpHandlerUtils::SendErrorResponse(RequestId, Socket, TEXT("Missing name or path for create_graph"));
        }

        FString PackagePath = Path / Name;
        UPackage* Package = CreatePackage(*PackagePath);
        if (!Package)
        {
            return McpHandlerUtils::SendErrorResponse(RequestId, Socket, TEXT("Failed to create package for PCG graph"));
        }

        UPCGGraph* NewGraph = NewObject<UPCGGraph>(Package, *Name, RF_Public | RF_Standalone);
        if (!NewGraph)
        {
            return McpHandlerUtils::SendErrorResponse(RequestId, Socket, TEXT("Failed to create PCG graph object"));
        }

        FAssetRegistryModule::AssetCreated(NewGraph);
        NewGraph->MarkPackageDirty();

        TSharedPtr<FJsonObject> Response = McpHandlerUtils::CreateBaseResponse(true, TEXT("PCG Graph created"));
        Response->SetStringField(TEXT("assetPath"), NewGraph->GetPathName());
        return McpHandlerUtils::SendResponse(RequestId, Socket, Response);
    }

    if (SubAction == TEXT("add_node"))
    {
        FString AssetPath, NodeType;
        if (!Payload->TryGetStringField(TEXT("assetPath"), AssetPath) || !Payload->TryGetStringField(TEXT("nodeType"), NodeType))
        {
            return McpHandlerUtils::SendErrorResponse(RequestId, Socket, TEXT("Missing assetPath or nodeType for add_node"));
        }

        UPCGGraph* Graph = LoadObject<UPCGGraph>(nullptr, *AssetPath);
        if (!Graph)
        {
            return McpHandlerUtils::SendErrorResponse(RequestId, Socket, TEXT("Failed to load PCG graph"));
        }

        UClass* NodeClass = McpHandlerUtils::ResolveClassByName(NodeType, TEXT("PCGSettings"));
        if (!NodeClass)
        {
            return McpHandlerUtils::SendErrorResponse(RequestId, Socket, FString::Printf(TEXT("Failed to resolve PCG node class: %s"), *NodeType));
        }

        UPCGSettings* Settings = NewObject<UPCGSettings>(Graph, NodeClass);
        UPCGNode* NewNode = Graph->AddNode(Settings);
        if (!NewNode)
        {
            return McpHandlerUtils::SendErrorResponse(RequestId, Socket, TEXT("Failed to add node to PCG graph"));
        }

        Graph->MarkPackageDirty();

        TSharedPtr<FJsonObject> Response = McpHandlerUtils::CreateBaseResponse(true, TEXT("PCG Node added"));
        Response->SetStringField(TEXT("nodeId"), NewNode->GetPathName());
        return McpHandlerUtils::SendResponse(RequestId, Socket, Response);
    }

    if (SubAction == TEXT("connect_nodes"))
    {
        FString AssetPath, SourceNodeId, TargetNodeId, SourcePin, TargetPin;
        if (!Payload->TryGetStringField(TEXT("assetPath"), AssetPath) ||
            !Payload->TryGetStringField(TEXT("sourceNodeId"), SourceNodeId) ||
            !Payload->TryGetStringField(TEXT("targetNodeId"), TargetNodeId) ||
            !Payload->TryGetStringField(TEXT("sourcePin"), SourcePin) ||
            !Payload->TryGetStringField(TEXT("targetPin"), TargetPin))
        {
            return McpHandlerUtils::SendErrorResponse(RequestId, Socket, TEXT("Missing connection parameters"));
        }

        UPCGGraph* Graph = LoadObject<UPCGGraph>(nullptr, *AssetPath);
        if (!Graph)
        {
            return McpHandlerUtils::SendErrorResponse(RequestId, Socket, TEXT("Failed to load PCG graph"));
        }

        UPCGNode* SourceNode = LoadObject<UPCGNode>(nullptr, *SourceNodeId);
        UPCGNode* TargetNode = LoadObject<UPCGNode>(nullptr, *TargetNodeId);

        if (!SourceNode || !TargetNode)
        {
            return McpHandlerUtils::SendErrorResponse(RequestId, Socket, TEXT("Failed to resolve source or target nodes"));
        }

        if (Graph->AddEdge(SourceNode, FName(*SourcePin), TargetNode, FName(*TargetPin)))
        {
            Graph->MarkPackageDirty();
            return McpHandlerUtils::SendSuccessResponse(RequestId, Socket, TEXT("PCG nodes connected"));
        }

        return McpHandlerUtils::SendErrorResponse(RequestId, Socket, TEXT("Failed to connect PCG nodes"));
    }

    if (SubAction == TEXT("generate"))
    {
        FString ActorName;
        if (!Payload->TryGetStringField(TEXT("name"), ActorName))
        {
             return McpHandlerUtils::SendErrorResponse(RequestId, Socket, TEXT("Missing name (Actor name) for generate"));
        }

        AActor* Actor = McpHandlerUtils::FindActorByName(McpHandlerUtils::GetEditorWorld(), ActorName);
        if (!Actor)
        {
            return McpHandlerUtils::SendErrorResponse(RequestId, Socket, TEXT("Failed to find actor for PCG generation"));
        }

        UPCGComponent* PCGComp = Actor->FindComponentByClass<UPCGComponent>();
        if (!PCGComp)
        {
            return McpHandlerUtils::SendErrorResponse(RequestId, Socket, TEXT("Actor has no PCG component"));
        }

        PCGComp->Generate();
        return McpHandlerUtils::SendSuccessResponse(RequestId, Socket, TEXT("PCG Generation triggered"));
    }

    return McpHandlerUtils::SendErrorResponse(RequestId, Socket, FString::Printf(TEXT("Unsupported PCG sub-action: %s"), *SubAction));
}

} // namespace PCG
} // namespace McpHandlers
#endif
