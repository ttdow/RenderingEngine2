// Payload struct passed along rays.
struct Payload
{
    float3 color;
    bool allowReflection;
    bool missed;
};

// Scene acceleration structure (TLAS).
RaytracingAccelerationStructure scene : register(t0);

// UAV target to write final colours.
RWTexture2D<float4> uav : register(u0);

// Constants for camera position and lighting.
static const float3 camera = float3(0, 1.5, -7);
static const float3 light = float3(0, 200, 0);
static const float3 skyTop = float3(0.24, 0.44, 0.72);
static const float3 skyBottom = float3(0.75, 0.86, 0.93);

// -----------------------------------------------------------------------------
// RAY GENERATION SHADER
// -----------------------------------------------------------------------------

[shader("raygeneration")]
void RayGeneration()
{
    // Get current pixel coordinate and screen size.
    uint2 idx = DispatchRaysIndex().xy;
    float2 size = DispatchRaysDimensions().xy;
    
    // Compute UV coordinate and generate a ray toward the scene.
    float2 uv = idx / size;
    float3 target = float3((uv.x * 2 - 1) * 1.8 * (size.x / size.y),
                           (1 - uv.y) * 4 - 2 + camera.y,
                           0);
    
    // Set up the ray description.
    RayDesc ray;
    ray.Origin = camera;
    ray.Direction = target - camera;
    ray.TMin = 0.001;
    ray.TMax = 1000;
    
    // Initialize the payload.
    Payload payload;
    payload.allowReflection = true; // Only allow one reflection per ray.
    payload.missed = false;
    
    // Trace the ray through the scene.
    TraceRay(scene, 0x00, 0xFF, 0, 0, 0, ray, payload);
    
    // Write the final colour to the output image.
    uav[idx] = float4(payload.color, 1);
}

// -----------------------------------------------------------------------------
// MISS SHADER
// -----------------------------------------------------------------------------

[shader("miss")]
void Miss(inout Payload payload)
{
    // Fade from horizon colour to sky colour based on the ray direction.
    float slope = normalize(WorldRayDirection()).y;
    float t = saturate(slope * 5 + 0.5);
    payload.color = lerp(skyBottom, skyTop, t);
    
    payload.missed = true;
}

// -----------------------------------------------------------------------------
// CLOSEST HIT SHADER
// -----------------------------------------------------------------------------

// Forward declarations for per-instance material logic.
void HitCube(inout Payload payload, float2 uv);
void HitMirror(inout Payload payload, float2 uv);
void HitFloor(inout Payload payload, float2 uv);

[shader("closesthit")]
void ClosestHit(inout Payload payload, BuiltInTriangleIntersectionAttributes attribs)
{
    float2 uv = attribs.barycentrics;
    
    // Select shading behaviour based on instance ID.
    switch (InstanceID())
    {
        case 0:
            HitCube(payload, uv);
            break;
        case 1:
            HitMirror(payload, uv);
            break;
        case 2:
            HitFloor(payload, uv);
            break;
        default:
            payload.color = float3(1, 0, 1); // Debug magenta for unknown instance.
            break;
    }
}

// Cube shader with normal-based colouring and face borders.
void HitCube(inout Payload payload, float2 uv)
{
    // Estimate face normal from primitive index.
    uint tri = PrimitiveIndex();
    tri /= 2;
    float3 normal = (tri.xxx % 3 == uint3(0, 1, 2)) * (tri < 3 ? -1 : 1);
    float3 worldNormal = normalize(mul(normal, (float3x3) ObjectToWorld4x3()));
    
    // Base colour from absolute normal components.
    float3 color = abs(normal) / 3 + 0.5;
    if (uv.x < 0.03 || uv.y < 0.03)
    {
        color = 0.25.rrr;
    }
    
    // Simple directional lighting.
    color *= saturate(dot(worldNormal, normalize(light))) + 0.33;
    
    payload.color = color;
}

// Mirror shader with one-bounce recursive reflection.
void HitMirror(inout Payload payload, float2 uv)
{
    // Stop reflecting if recursion not allowed.
    if (!payload.allowReflection)
    {
        return;
    }
    
    // Compute world-space hit position and surface normal.
    float3 pos = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    float3 normal = normalize(mul(float3(0, 1, 0), (float3x3)ObjectToWorld4x3()));
    
    // Reflect ray direction around normal.
    float3 reflected = reflect(normalize(WorldRayDirection()), normal);
    
    // Construct a new ray.
    RayDesc mirrorRay;
    mirrorRay.Origin = pos;
    mirrorRay.Direction = reflected;
    mirrorRay.TMin = 0.001;
    mirrorRay.TMax = 1000;
    
    // Prevent further reflection.
    payload.allowReflection = false;
    
    // Trace the reflected ray with same payload.
    TraceRay(scene, 0x00, 0xFF, 0, 0, 0, mirrorRay, payload);
}

// Floor shader with checkerboard pattern and shadow ray.
void HitFloor(inout Payload payload, float2 uv)
{ 
    float3 pos = WorldRayOrigin() + WorldRayDirection() * RayTCurrent();
    
    // Checkerboard pattern using world-space XZ coordinates.
    bool2 pattern = frac(pos.xz) > 0.5;
    payload.color = (pattern.x ^ pattern.y ? 0.6 : 0.4).rrr;

    // Shadow ray setup.
    RayDesc shadowRay;
    shadowRay.Origin = pos;
    shadowRay.Direction = light - pos;
    shadowRay.TMin = 0.001;
    shadowRay.TMax = 1;
    
    // Trace a shadow ray to see if light is blocked.
    Payload shadow;
    shadow.allowReflection = false;
    shadow.missed = false;
    TraceRay(scene, 0x00, 0xFF, 0, 0, 0, shadowRay, shadow);
    
    // If the shadow ray hit something, darken the colour.
    if (!shadow.missed)
    {
        payload.color /= 2;
    }
}