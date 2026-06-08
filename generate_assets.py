import json
from pathlib import Path

root = Path(__file__).resolve().parent
assets = root / "assets"

paths = [
    assets / "audio",
    assets / "models",
    assets / "textures",
    assets / "shaders",
    assets / "levels" / "mohenjo_daro",
]

for path in paths:
    path.mkdir(parents=True, exist_ok=True)
    if path.name in {"audio", "models", "textures"}:
        gitkeep = path / ".gitkeep"
        gitkeep.write_text("")

level_json = {
    "title": "Les Larmes de la Terre",
    "objectives": [
        {"id": "hydraulic_puzzle", "description": "Restore the flooded temple channels"},
        {"id": "cartel_patrol", "description": "Survive the first Cartel patrol"},
        {"id": "mirror_throne", "description": "Align the throne room mirrors"},
        {"id": "crystal_retrieve", "description": "Recover the First Echo Crystal"}
    ]
}

level_path = assets / "levels" / "mohenjo_daro" / "les_larmes_de_la_terre.json"
level_path.write_text(json.dumps(level_json, indent=4, ensure_ascii=False))

shader_path = assets / "shaders" / "basic.hlsl"
if not shader_path.exists():
    shader_path.write_text(
        "// Placeholder basic shader\n"
        "struct VSInput { float3 position : POSITION; float4 color : COLOR; };\n"
        "struct PSInput { float4 position : SV_POSITION; float4 color : COLOR; };\n"
        "PSInput VSMain(VSInput input) { PSInput output; output.position = float4(input.position, 1.0); output.color = input.color; return output; }\n"
        "float4 PSMain(PSInput input) : SV_TARGET { return input.color; }\n"
    )

print(f"Assets generated at: {assets}")
print(f"Level JSON written: {level_path}")
print(f"Shader present: {shader_path}")
