// Windows x86 target: bin of game app 4000 build 25375506 and dedicated-server app 4020 build 25375525.
// Used by the game and the dedicated server (dedicated.dll / filesystem_stdio.dll provide the file system).
#pragma once

namespace bm
{

// ---- Generated: patch sites, file identities, engine anchors and game hooks ----
// Site ids (P4-xx, P5-xx) refer to the research project's patch log.
static const unsigned char s_engine_0[] = {0x0C};
static const unsigned char p_engine_0[] = {0x0D};
static const unsigned char s_engine_1[] = {0x0C};
static const unsigned char p_engine_1[] = {0x0D};
static const unsigned char s_engine_2[] = {0x0C};
static const unsigned char p_engine_2[] = {0x0D};
static const unsigned char s_engine_3[] = {0x0F};
static const unsigned char p_engine_3[] = {0x10};
static const unsigned char s_engine_4[] = {0x0C};
static const unsigned char p_engine_4[] = {0x0D};
static const unsigned char s_engine_5[] = {0x0C};
static const unsigned char p_engine_5[] = {0x0D};
static const unsigned char s_engine_6[] = {0x0C};
static const unsigned char p_engine_6[] = {0x0D};
static const unsigned char s_engine_7[] = {0x74, 0x53, 0x33, 0x10};
static const unsigned char p_engine_7[] = {0x58, 0x4D, 0x3A, 0x10};
static const unsigned char s_engine_8[] = {0x38, 0xA2, 0x31, 0x10};
static const unsigned char p_engine_8[] = {0xA8, 0x97, 0x31, 0x10};
static const unsigned char s_engine_9[] = {0x74, 0x53, 0x33, 0x10};
static const unsigned char p_engine_9[] = {0x58, 0x4D, 0x3A, 0x10};
static const unsigned char s_engine_10[] = {0x38, 0xA2, 0x31, 0x10};
static const unsigned char p_engine_10[] = {0xA8, 0x97, 0x31, 0x10};
static const unsigned char s_engine_11[] = {0x80, 0x00};
static const unsigned char p_engine_11[] = {0x00, 0x01};
static const unsigned char s_engine_12[] = {0x80, 0xC6};
static const unsigned char p_engine_12[] = {0x00, 0xC7};
static const unsigned char s_engine_13[] = {0x80, 0xC6};
static const unsigned char p_engine_13[] = {0x00, 0xC7};
static const unsigned char s_engine_14[] = {0x80, 0xC6};
static const unsigned char p_engine_14[] = {0x00, 0xC7};
static const unsigned char s_engine_15[] = {0x74, 0x53, 0x33, 0x10};
static const unsigned char p_engine_15[] = {0x58, 0x4D, 0x3A, 0x10};
static const unsigned char s_engine_16[] = {0x38, 0xA2, 0x31, 0x10};
static const unsigned char p_engine_16[] = {0xA8, 0x97, 0x31, 0x10};
static const unsigned char s_engine_17[] = {0x0E};
static const unsigned char p_engine_17[] = {0x0F};
static const unsigned char s_engine_18[] = {0x0E};
static const unsigned char p_engine_18[] = {0x0F};
static const unsigned char s_engine_19[] = {0x0D};
static const unsigned char p_engine_19[] = {0x0E};
static const unsigned char s_engine_20[] = {0x3F};
static const unsigned char p_engine_20[] = {0x7F};
static const unsigned char s_engine_21[] = {0x0F};
static const unsigned char p_engine_21[] = {0x10};
static const unsigned char s_engine_22[] = {0x38};
static const unsigned char p_engine_22[] = {0x78};
static const unsigned char s_engine_23[] = {0x0E};
static const unsigned char p_engine_23[] = {0x0F};
static const unsigned char s_engine_24[] = {0x0F};
static const unsigned char p_engine_24[] = {0x10};
static const unsigned char s_engine_25[] = {0x0E};
static const unsigned char p_engine_25[] = {0x0F};
static const unsigned char s_engine_26[] = {0x0E};
static const unsigned char p_engine_26[] = {0x0F};
static const unsigned char s_engine_27[] = {0x1F};
static const unsigned char p_engine_27[] = {0x3F};
static const unsigned char s_engine_28[] = {0x3F};
static const unsigned char p_engine_28[] = {0x7F};
static const unsigned char s_engine_29[] = {0x11};
static const unsigned char p_engine_29[] = {0x12};
static const unsigned char s_engine_30[] = {0x14};
static const unsigned char p_engine_30[] = {0x15};
static const unsigned char s_engine_31[] = {0x11};
static const unsigned char p_engine_31[] = {0x12};
static const unsigned char s_engine_32[] = {0xEC, 0x7F};
static const unsigned char p_engine_32[] = {0xF6, 0xFF};
static const unsigned char s_engine_33[] = {0xEC, 0x7F};
static const unsigned char p_engine_33[] = {0xF6, 0xFF};
static const unsigned char s_engine_34[] = {0x13};
static const unsigned char p_engine_34[] = {0x14};
static const unsigned char s_engine_35[] = {0x11};
static const unsigned char p_engine_35[] = {0x12};
static const unsigned char s_engine_36[] = {0x0F};
static const unsigned char p_engine_36[] = {0x10};
static const unsigned char s_engine_37[] = {0x14};
static const unsigned char p_engine_37[] = {0x15};
static const unsigned char s_engine_38[] = {0x14};
static const unsigned char p_engine_38[] = {0x15};
static const unsigned char s_engine_39[] = {0x12};
static const unsigned char p_engine_39[] = {0x13};
static const unsigned char s_engine_40[] = {0x80, 0xC6};
static const unsigned char p_engine_40[] = {0x00, 0xC7};
static const unsigned char s_engine_41[] = {0x80, 0xC6};
static const unsigned char p_engine_41[] = {0x00, 0xC7};
static const unsigned char s_engine_42[] = {0x80, 0xC6};
static const unsigned char p_engine_42[] = {0x00, 0xC7};
static const unsigned char s_engine_43[] = {0x80, 0x46};
static const unsigned char p_engine_43[] = {0x00, 0x47};
static const unsigned char s_engine_44[] = {0x80, 0x46};
static const unsigned char p_engine_44[] = {0x00, 0x47};
static const unsigned char s_engine_45[] = {0x80, 0x46};
static const unsigned char p_engine_45[] = {0x00, 0x47};
static const unsigned char s_server_0[] = {0x90, 0x3A, 0x78, 0x10};
static const unsigned char p_server_0[] = {0x3C, 0x62, 0x78, 0x10};
static const unsigned char s_server_1[] = {0x54, 0x0D, 0x7C, 0x10};
static const unsigned char p_server_1[] = {0xF9, 0x1E, 0x7B, 0x10};
static const unsigned char s_server_2[] = {0x13};
static const unsigned char p_server_2[] = {0x14};
static const unsigned char s_server_3[] = {0x90, 0x3A, 0x78, 0x10};
static const unsigned char p_server_3[] = {0x3C, 0x62, 0x78, 0x10};
static const unsigned char s_server_4[] = {0x54, 0x0D, 0x7C, 0x10};
static const unsigned char p_server_4[] = {0xF9, 0x1E, 0x7B, 0x10};
static const unsigned char s_server_5[] = {0x11};
static const unsigned char p_server_5[] = {0x12};
static const unsigned char s_server_6[] = {0x90, 0x3A, 0x78, 0x10};
static const unsigned char p_server_6[] = {0x3C, 0x62, 0x78, 0x10};
static const unsigned char s_server_7[] = {0x54, 0x0D, 0x7C, 0x10};
static const unsigned char p_server_7[] = {0xF9, 0x1E, 0x7B, 0x10};
static const unsigned char s_server_8[] = {0x11};
static const unsigned char p_server_8[] = {0x12};
static const unsigned char s_server_9[] = {0x90, 0x3A, 0x78, 0x10};
static const unsigned char p_server_9[] = {0x3C, 0x62, 0x78, 0x10};
static const unsigned char s_server_10[] = {0x54, 0x0D, 0x7C, 0x10};
static const unsigned char p_server_10[] = {0xF9, 0x1E, 0x7B, 0x10};
static const unsigned char s_server_11[] = {0x11};
static const unsigned char p_server_11[] = {0x12};
static const unsigned char s_server_12[] = {0x90, 0x3A, 0x78, 0x10};
static const unsigned char p_server_12[] = {0x3C, 0x62, 0x78, 0x10};
static const unsigned char s_server_13[] = {0x54, 0x0D, 0x7C, 0x10};
static const unsigned char p_server_13[] = {0xF9, 0x1E, 0x7B, 0x10};
static const unsigned char s_server_14[] = {0x11};
static const unsigned char p_server_14[] = {0x12};
static const unsigned char s_server_15[] = {0x90, 0x3A, 0x78, 0x10};
static const unsigned char p_server_15[] = {0x3C, 0x62, 0x78, 0x10};
static const unsigned char s_server_16[] = {0x54, 0x0D, 0x7C, 0x10};
static const unsigned char p_server_16[] = {0xF9, 0x1E, 0x7B, 0x10};
static const unsigned char s_server_17[] = {0x11};
static const unsigned char p_server_17[] = {0x12};
static const unsigned char s_server_18[] = {0x90, 0x3A, 0x78, 0x10};
static const unsigned char p_server_18[] = {0x3C, 0x62, 0x78, 0x10};
static const unsigned char s_server_19[] = {0x54, 0x0D, 0x7C, 0x10};
static const unsigned char p_server_19[] = {0xF9, 0x1E, 0x7B, 0x10};
static const unsigned char s_server_20[] = {0x11};
static const unsigned char p_server_20[] = {0x12};
static const unsigned char s_server_21[] = {0x80, 0x46};
static const unsigned char p_server_21[] = {0x00, 0x47};
static const unsigned char s_client_0[] = {0x31, 0x36, 0x33, 0x38, 0x34};
static const unsigned char p_client_0[] = {0x33, 0x32, 0x37, 0x36, 0x38};
static const unsigned char s_client_1[] = {0x80, 0x46};
static const unsigned char p_client_1[] = {0x00, 0x47};
static const ProbeSite kSites[] = {
    {"engine.dll", "P4-10a", 0x0013FE13, 1, s_engine_0, p_engine_0, 0x0},
    {"engine.dll", "P4-10b", 0x0013FE8C, 1, s_engine_1, p_engine_1, 0x0},
    {"engine.dll", "P4-10c", 0x0013FF01, 1, s_engine_2, p_engine_2, 0x0},
    {"engine.dll", "P5-03", 0x00181E1D, 1, s_engine_3, p_engine_3, 0x0},
    {"engine.dll", "P4-11a", 0x001DD40B, 1, s_engine_4, p_engine_4, 0x0},
    {"engine.dll", "P4-11b", 0x001DD440, 1, s_engine_5, p_engine_5, 0x0},
    {"engine.dll", "P4-11c", 0x001DD475, 1, s_engine_6, p_engine_6, 0x0},
    {"engine.dll", "P4-07a", 0x001DEF7F, 4, s_engine_7, p_engine_7, 0x1},
    {"engine.dll", "P4-07b", 0x001DEF95, 4, s_engine_8, p_engine_8, 0x1},
    {"engine.dll", "P4-07c", 0x001DF077, 4, s_engine_9, p_engine_9, 0x1},
    {"engine.dll", "P4-07d", 0x001DF098, 4, s_engine_10, p_engine_10, 0x1},
    {"engine.dll", "P4-08", 0x001E0956, 2, s_engine_11, p_engine_11, 0x0},
    {"engine.dll", "P4-09a", 0x001E0976, 2, s_engine_12, p_engine_12, 0x0},
    {"engine.dll", "P4-09b", 0x001E097D, 2, s_engine_13, p_engine_13, 0x0},
    {"engine.dll", "P4-09c", 0x001E0984, 2, s_engine_14, p_engine_14, 0x0},
    {"engine.dll", "P4-01", 0x001F6154, 4, s_engine_15, p_engine_15, 0x1},
    {"engine.dll", "P4-02", 0x001F6167, 4, s_engine_16, p_engine_16, 0x1},
    {"engine.dll", "P5-01a", 0x0026056C, 1, s_engine_17, p_engine_17, 0x0},
    {"engine.dll", "P5-01c", 0x00260585, 1, s_engine_18, p_engine_18, 0x0},
    {"engine.dll", "P5-01b", 0x00260588, 1, s_engine_19, p_engine_19, 0x0},
    {"engine.dll", "P5-01d", 0x002605BC, 1, s_engine_20, p_engine_20, 0x0},
    {"engine.dll", "P5-06a", 0x002607D5, 1, s_engine_21, p_engine_21, 0x0},
    {"engine.dll", "P5-06c", 0x002608E9, 1, s_engine_22, p_engine_22, 0x0},
    {"engine.dll", "P5-06d", 0x002608F7, 1, s_engine_23, p_engine_23, 0x0},
    {"engine.dll", "P5-07a", 0x002609A6, 1, s_engine_24, p_engine_24, 0x0},
    {"engine.dll", "P5-02a", 0x00261434, 1, s_engine_25, p_engine_25, 0x0},
    {"engine.dll", "P5-02b", 0x00261442, 1, s_engine_26, p_engine_26, 0x0},
    {"engine.dll", "P5-02c", 0x0026144F, 1, s_engine_27, p_engine_27, 0x0},
    {"engine.dll", "P5-02d", 0x0026148E, 1, s_engine_28, p_engine_28, 0x0},
    {"engine.dll", "P5-05a", 0x002615DB, 1, s_engine_29, p_engine_29, 0x0},
    {"engine.dll", "P5-05c", 0x0026160F, 1, s_engine_30, p_engine_30, 0x0},
    {"engine.dll", "P5-05b", 0x00261615, 1, s_engine_31, p_engine_31, 0x0},
    {"engine.dll", "P4-05", 0x00355F39, 2, s_engine_32, p_engine_32, 0x0},
    {"engine.dll", "P4-06", 0x00355F41, 2, s_engine_33, p_engine_33, 0x0},
    {"engine.dll", "P5-06b", 0x003B0812, 1, s_engine_34, p_engine_34, 0x0},
    {"engine.dll", "P5-06b", 0x003B0816, 1, s_engine_35, p_engine_35, 0x0},
    {"engine.dll", "P5-04", 0x003B0818, 1, s_engine_36, p_engine_36, 0x0},
    {"engine.dll", "P5-04", 0x003B0820, 1, s_engine_37, p_engine_37, 0x0},
    {"engine.dll", "P5-07b", 0x003B0826, 1, s_engine_38, p_engine_38, 0x0},
    {"engine.dll", "P5-07b", 0x003B082A, 1, s_engine_39, p_engine_39, 0x0},
    {"engine.dll", "P4-03", 0x00425AF2, 2, s_engine_40, p_engine_40, 0x0},
    {"engine.dll", "P4-03", 0x00425AF6, 2, s_engine_41, p_engine_41, 0x0},
    {"engine.dll", "P4-03", 0x00425AFA, 2, s_engine_42, p_engine_42, 0x0},
    {"engine.dll", "P4-04", 0x00425AFE, 2, s_engine_43, p_engine_43, 0x0},
    {"engine.dll", "P4-04", 0x00425B02, 2, s_engine_44, p_engine_44, 0x0},
    {"engine.dll", "P4-04", 0x00425B06, 2, s_engine_45, p_engine_45, 0x0},
    {"server.dll", "P5-09a", 0x00280BA8, 4, s_server_0, p_server_0, 0x1},
    {"server.dll", "P5-09b", 0x00280BBD, 4, s_server_1, p_server_1, 0x1},
    {"server.dll", "P5-08", 0x00280BCA, 1, s_server_2, p_server_2, 0x0},
    {"server.dll", "P4-16a", 0x002ABF5A, 4, s_server_3, p_server_3, 0x1},
    {"server.dll", "P4-16g", 0x002ABF6F, 4, s_server_4, p_server_4, 0x1},
    {"server.dll", "P4-15a", 0x002ABF79, 1, s_server_5, p_server_5, 0x0},
    {"server.dll", "P4-16b", 0x002ABF8F, 4, s_server_6, p_server_6, 0x1},
    {"server.dll", "P4-16h", 0x002ABFA4, 4, s_server_7, p_server_7, 0x1},
    {"server.dll", "P4-15b", 0x002ABFAE, 1, s_server_8, p_server_8, 0x0},
    {"server.dll", "P4-16c", 0x002ABFC4, 4, s_server_9, p_server_9, 0x1},
    {"server.dll", "P4-16i", 0x002ABFD9, 4, s_server_10, p_server_10, 0x1},
    {"server.dll", "P4-15c", 0x002ABFE3, 1, s_server_11, p_server_11, 0x0},
    {"server.dll", "P4-16d", 0x002ABFF9, 4, s_server_12, p_server_12, 0x1},
    {"server.dll", "P4-16j", 0x002AC00E, 4, s_server_13, p_server_13, 0x1},
    {"server.dll", "P4-15d", 0x002AC018, 1, s_server_14, p_server_14, 0x0},
    {"server.dll", "P4-16e", 0x002AC02E, 4, s_server_15, p_server_15, 0x1},
    {"server.dll", "P4-16k", 0x002AC043, 4, s_server_16, p_server_16, 0x1},
    {"server.dll", "P4-15e", 0x002AC04D, 1, s_server_17, p_server_17, 0x0},
    {"server.dll", "P4-16f", 0x002AC06E, 4, s_server_18, p_server_18, 0x1},
    {"server.dll", "P4-16l", 0x002AC078, 4, s_server_19, p_server_19, 0x1},
    {"server.dll", "P4-15f", 0x002AC082, 1, s_server_20, p_server_20, 0x0},
    {"server.dll", "P4-13", 0x009A1726, 2, s_server_21, p_server_21, 0x0},
    {"client.dll", "P4-12", 0x005C8A64, 5, s_client_0, p_client_0, 0x0},
    {"client.dll", "P4-14", 0x006C500E, 2, s_client_1, p_client_1, 0x0},
};
static const ProbeFile kFiles[] = {
    {"engine.dll", "91416606D167440A48CCA9E68FFC0898CE350A879DADD074835336C199404DE4", "B850E70D79157CCFABEA290394713C2BE8A291285CC46C7C4433F80B4BE4FECC", 0x10000000ull},
    {"server.dll", "06688DD4C3499170951926A69BE91B6FC77B1DD428E4477566CB2EC6A13BE943", "62FB5FB0E4FB35C7E4E883B2EEFF152426FB0234B9CE2A9EC17121BDCC63F9D8", 0x10000000ull},
    {"client.dll", "A1B56FDCB6E70C00241698C45EB145EED77477D8F760320B0A53BFEC330E53E6", "332C71F80A29B963D0F99850FD8922F52996FA9A45942CD138B4E6B119567933", 0x10000000ull},
};
static const unsigned char a_crcfunc[] = {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x2C, 0x8D, 0x45, 0xFC, 0x50, 0xE8, 0xA1, 0x5B, 0x0D, 0x00, 0xA1};
static const unsigned char a_getter[] = {0xA1, 0xB8, 0xAA, 0x74, 0x10, 0xC3};
static const unsigned char a_store[] = {0xE8, 0x6B, 0xF7, 0xFF, 0xFF, 0xA3, 0xB8, 0xAA, 0x74, 0x10};
static const unsigned char a_hs_name[] = {0x68, 0x00, 0x01, 0x00, 0x00, 0xFF, 0x75, 0x08, 0x68, 0xA8, 0xE7, 0x74, 0x10, 0xE8, 0x6F, 0xB3, 0x0A, 0x00};
static const unsigned char a_hs_state[] = {0x83, 0xC4, 0x18, 0xC7, 0x05, 0x8C, 0xE7, 0x74, 0x10, 0x03, 0x00, 0x00, 0x00, 0x5D, 0xC3};
// Anchors: unchanged engine code (SendTable CRC; host-state map name), verified before use.
static const ProbeSite kAnchors[] = {
    {"engine.dll", "crcfunc", 0x0018A320, 16, a_crcfunc, a_crcfunc, 0x0},
    {"engine.dll", "getter", 0x0018AA50, 6, a_getter, a_getter, 0x2},
    {"engine.dll", "store", 0x0018ABB0, 10, a_store, a_store, 0x40},
    {"engine.dll", "hs_name", 0x001AB88F, 18, a_hs_name, a_hs_name, 0x200},
    {"engine.dll", "hs_state", 0x001AB8B3, 15, a_hs_state, a_hs_state, 0x20},
};
static const unsigned kCrcFuncRva = 0x0018A320;
static const unsigned kCrcGlobalRva = 0x0074AAB8;
static const unsigned kHostStateRva = 0x0074E788;
static const unsigned char a_spawn[] = {0x55, 0x8B, 0xEC, 0x81, 0xEC, 0x28, 0x03, 0x00, 0x00};
static const unsigned char a_clnew[] = {0x55, 0x8B, 0xEC, 0x81, 0xEC, 0xD0, 0x00, 0x00, 0x00};
// Client-module hook prologues (replaced by a jump once hooked; not part of kAnchors).
static const ProbeSite kHooks[] = {
    {"engine.dll", "spawn", 0x0014A200, 9, a_spawn, a_spawn, 0x0},
    {"engine.dll", "clnew", 0x000BC560, 9, a_clnew, a_clnew, 0x0},
};
static const unsigned kClStateOff = 0x130;
static const unsigned kClLevelOff = 0x1B0;

// ---- SendProp / SendTable field offsets in server.dll ----
// Re-derived from the binaries (research PATCH_LOG, SendProp layout). kPropVarName follows the public SDK member
// order and is cross-checked by FindSendTable's stride search; kPropFlags only bounds that search.
const size_t kPropBits = 0x0C, kPropLow = 0x10, kPropHigh = 0x14, kPropVarName = 0x30, kPropHLMul = 0x34,
             kPropFlags = 0x38;
const size_t kTblCount = 0x04, kTblName = 0x08;

// ---- Engine file system providers (map size check) ----
const FsProvider kFsProviders[] = {
    {"dedicated.dll", "2882D259DB67B195D0FCF07845D54B1BE47A872A11E5D885A018E0B38A076E9B", 0x0011F330, 0x000F0394,
     {0x00019B30, 0x0001D1E0, 0x00018940, 0x000109C0, 0x0001BA00}},
    {"filesystem_stdio.dll", "4F7EEDB8FE096B6DADDB06162FB243032EDEC72FE9AAE4492C2685787AAED925", 0, 0x00093B24,
     {0x000192F0, 0x0001CB00, 0x00017F70, 0x00010000, 0x0001B1C0}},
};

// ---- r_mapextents ConVar in client.dll (game only) ----
// Located from its registration code (0x1005B2B0) and the ConVar constructor's field stores.
const MapExtentsVar kMapExtents = {0x007EB1E0, 0x005C8A6C, 0x005C8A64, 0x0C, 0x1C, 0x20, 0x24, 0x28, 0x2C, 0x30};

}  // namespace bm
