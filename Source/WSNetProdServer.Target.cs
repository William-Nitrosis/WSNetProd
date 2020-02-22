// Insert Your Copyright Here

using UnrealBuildTool;
using System.Collections.Generic;

public class WSNetProdServerTarget : TargetRules
{
    public WSNetProdServerTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Server;

        ExtraModuleNames.Add("WSNetProd"); // This would be your project module if not BP only
    }
}