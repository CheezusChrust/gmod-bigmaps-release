local minx, miny, minz = -32768, -32768, -32768
local maxx, maxy, maxz = 32768, 32768, 32768
local clamp = math.Clamp
local function clampPos(pos)
    local x, y, z = pos:Unpack()

    return Vector(clamp(x, minx, maxx), clamp(y, miny, maxy), clamp(z, minz, maxz))
end

hook.Add("InitPosEntity", "BigMapsCompatInitPosEntity", function()
    if WireLib then
        WireLib.clampPos = clampPos
    end

    if SF then
        SF.clampPos = clampPos
    end
end)