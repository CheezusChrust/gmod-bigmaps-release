// Windows x64 target: bin/win64 of game app 4000 build 25375506 and dedicated-server app 4020 build 25375525.
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
static const unsigned char s_engine_7[] = {0x13, 0xD9, 0x1A};
static const unsigned char p_engine_7[] = {0xDC, 0x9B, 0x25};
static const unsigned char s_engine_8[] = {0x5D, 0x36};
static const unsigned char p_engine_8[] = {0x05, 0x2A};
static const unsigned char s_engine_9[] = {0xD5, 0xD6, 0x1A};
static const unsigned char p_engine_9[] = {0x9E, 0x99, 0x25};
static const unsigned char s_engine_10[] = {0x2B, 0x34};
static const unsigned char p_engine_10[] = {0xD3, 0x27};
static const unsigned char s_engine_11[] = {0x5C, 0xD6, 0x1A};
static const unsigned char p_engine_11[] = {0x25, 0x99, 0x25};
static const unsigned char s_engine_12[] = {0xB2, 0x33};
static const unsigned char p_engine_12[] = {0x5A, 0x27};
static const unsigned char s_engine_13[] = {0xE4, 0xD5, 0x1A};
static const unsigned char p_engine_13[] = {0xAD, 0x98, 0x25};
static const unsigned char s_engine_14[] = {0x06, 0x33};
static const unsigned char p_engine_14[] = {0xAE, 0x26};
static const unsigned char s_engine_15[] = {0x80, 0x00};
static const unsigned char p_engine_15[] = {0x00, 0x01};
static const unsigned char s_engine_16[] = {0x80, 0xC6};
static const unsigned char p_engine_16[] = {0x00, 0xC7};
static const unsigned char s_engine_17[] = {0x80, 0xC6};
static const unsigned char p_engine_17[] = {0x00, 0xC7};
static const unsigned char s_engine_18[] = {0x80, 0xC6};
static const unsigned char p_engine_18[] = {0x00, 0xC7};
static const unsigned char s_engine_19[] = {0xC2, 0x0E, 0x19};
static const unsigned char p_engine_19[] = {0x8B, 0xD1, 0x23};
static const unsigned char s_engine_20[] = {0x1E, 0x6C};
static const unsigned char p_engine_20[] = {0xC6, 0x5F};
static const unsigned char s_engine_21[] = {0x0E};
static const unsigned char p_engine_21[] = {0x0F};
static const unsigned char s_engine_22[] = {0x0D};
static const unsigned char p_engine_22[] = {0x0E};
static const unsigned char s_engine_23[] = {0x0E};
static const unsigned char p_engine_23[] = {0x0F};
static const unsigned char s_engine_24[] = {0xAB};
static const unsigned char p_engine_24[] = {0xAF};
static const unsigned char s_engine_25[] = {0x0F};
static const unsigned char p_engine_25[] = {0x10};
static const unsigned char s_engine_26[] = {0x38};
static const unsigned char p_engine_26[] = {0x78};
static const unsigned char s_engine_27[] = {0x0E};
static const unsigned char p_engine_27[] = {0x0F};
static const unsigned char s_engine_28[] = {0x0F};
static const unsigned char p_engine_28[] = {0x10};
static const unsigned char s_engine_29[] = {0x0E};
static const unsigned char p_engine_29[] = {0x0F};
static const unsigned char s_engine_30[] = {0x0E};
static const unsigned char p_engine_30[] = {0x0F};
static const unsigned char s_engine_31[] = {0x1F};
static const unsigned char p_engine_31[] = {0x3F};
static const unsigned char s_engine_32[] = {0x3F};
static const unsigned char p_engine_32[] = {0x7F};
static const unsigned char s_engine_33[] = {0x11};
static const unsigned char p_engine_33[] = {0x12};
static const unsigned char s_engine_34[] = {0x11};
static const unsigned char p_engine_34[] = {0x12};
static const unsigned char s_engine_35[] = {0x14};
static const unsigned char p_engine_35[] = {0x15};
static const unsigned char s_engine_36[] = {0xEC, 0x7F};
static const unsigned char p_engine_36[] = {0xF6, 0xFF};
static const unsigned char s_engine_37[] = {0xEC, 0x7F};
static const unsigned char p_engine_37[] = {0xF6, 0xFF};
static const unsigned char s_engine_38[] = {0x13};
static const unsigned char p_engine_38[] = {0x14};
static const unsigned char s_engine_39[] = {0x11};
static const unsigned char p_engine_39[] = {0x12};
static const unsigned char s_engine_40[] = {0x0F};
static const unsigned char p_engine_40[] = {0x10};
static const unsigned char s_engine_41[] = {0x14};
static const unsigned char p_engine_41[] = {0x15};
static const unsigned char s_engine_42[] = {0x14};
static const unsigned char p_engine_42[] = {0x15};
static const unsigned char s_engine_43[] = {0x12};
static const unsigned char p_engine_43[] = {0x13};
static const unsigned char s_engine_44[] = {0x80, 0xC6};
static const unsigned char p_engine_44[] = {0x00, 0xC7};
static const unsigned char s_engine_45[] = {0x80, 0xC6};
static const unsigned char p_engine_45[] = {0x00, 0xC7};
static const unsigned char s_engine_46[] = {0x80, 0xC6};
static const unsigned char p_engine_46[] = {0x00, 0xC7};
static const unsigned char s_engine_47[] = {0x80, 0x46};
static const unsigned char p_engine_47[] = {0x00, 0x47};
static const unsigned char s_engine_48[] = {0x80, 0x46};
static const unsigned char p_engine_48[] = {0x00, 0x47};
static const unsigned char s_engine_49[] = {0x80, 0x46};
static const unsigned char p_engine_49[] = {0x00, 0x47};
static const unsigned char s_server_0[] = {0x33, 0x67, 0x5E};
static const unsigned char p_server_0[] = {0xDF, 0xBC, 0x5C};
static const unsigned char s_server_1[] = {0xDF, 0x51, 0x61};
static const unsigned char p_server_1[] = {0x34, 0x21, 0x60};
static const unsigned char s_server_2[] = {0x13};
static const unsigned char p_server_2[] = {0x14};
static const unsigned char s_server_3[] = {0x4D, 0x3B, 0x5B};
static const unsigned char p_server_3[] = {0xF9, 0x90, 0x59};
static const unsigned char s_server_4[] = {0xF2, 0x25, 0x5E};
static const unsigned char p_server_4[] = {0x47, 0xF5, 0x5C};
static const unsigned char s_server_5[] = {0x11};
static const unsigned char p_server_5[] = {0x12};
static const unsigned char s_server_6[] = {0x11};
static const unsigned char p_server_6[] = {0x12};
static const unsigned char s_server_7[] = {0x11};
static const unsigned char p_server_7[] = {0x12};
static const unsigned char s_server_8[] = {0x11};
static const unsigned char p_server_8[] = {0x12};
static const unsigned char s_server_9[] = {0x11};
static const unsigned char p_server_9[] = {0x12};
static const unsigned char s_server_10[] = {0x11};
static const unsigned char p_server_10[] = {0x12};
static const unsigned char s_server_11[] = {0x80, 0x46};
static const unsigned char p_server_11[] = {0x00, 0x47};
static const unsigned char s_client_0[] = {0x31, 0x36, 0x33, 0x38, 0x34};
static const unsigned char p_client_0[] = {0x33, 0x32, 0x37, 0x36, 0x38};
static const unsigned char s_client_1[] = {0x80, 0x46};
static const unsigned char p_client_1[] = {0x00, 0x47};
static const ProbeSite kSites[] = {
    {"engine.dll", "P4-10a", 0x00159E24, 1, s_engine_0, p_engine_0, 0x0},
    {"engine.dll", "P4-10b", 0x00159EAF, 1, s_engine_1, p_engine_1, 0x0},
    {"engine.dll", "P4-10c", 0x00159F3A, 1, s_engine_2, p_engine_2, 0x0},
    {"engine.dll", "P5-03", 0x001A87F9, 1, s_engine_3, p_engine_3, 0x0},
    {"engine.dll", "P4-11a", 0x002177C7, 1, s_engine_4, p_engine_4, 0x0},
    {"engine.dll", "P4-11b", 0x0021780C, 1, s_engine_5, p_engine_5, 0x0},
    {"engine.dll", "P4-11c", 0x0021784F, 1, s_engine_6, p_engine_6, 0x0},
    {"engine.dll", "P4-07a", 0x00218FCD, 3, s_engine_7, p_engine_7, 0x0},
    {"engine.dll", "P4-07b", 0x00218FE7, 2, s_engine_8, p_engine_8, 0x0},
    {"engine.dll", "P4-07c", 0x0021920B, 3, s_engine_9, p_engine_9, 0x0},
    {"engine.dll", "P4-07d", 0x00219219, 2, s_engine_10, p_engine_10, 0x0},
    {"engine.dll", "P4-07e", 0x00219284, 3, s_engine_11, p_engine_11, 0x0},
    {"engine.dll", "P4-07f", 0x00219292, 2, s_engine_12, p_engine_12, 0x0},
    {"engine.dll", "P4-07g", 0x002192FC, 3, s_engine_13, p_engine_13, 0x0},
    {"engine.dll", "P4-07h", 0x0021933E, 2, s_engine_14, p_engine_14, 0x0},
    {"engine.dll", "P4-08", 0x0021BCF8, 2, s_engine_15, p_engine_15, 0x0},
    {"engine.dll", "P4-09a", 0x0021BD1C, 2, s_engine_16, p_engine_16, 0x0},
    {"engine.dll", "P4-09b", 0x0021BD24, 2, s_engine_17, p_engine_17, 0x0},
    {"engine.dll", "P4-09c", 0x0021BD2C, 2, s_engine_18, p_engine_18, 0x0},
    {"engine.dll", "P4-01", 0x00235A1E, 3, s_engine_19, p_engine_19, 0x0},
    {"engine.dll", "P4-02", 0x00235A26, 2, s_engine_20, p_engine_20, 0x0},
    {"engine.dll", "P5-01a", 0x002B344B, 1, s_engine_21, p_engine_21, 0x0},
    {"engine.dll", "P5-01b", 0x002B3463, 1, s_engine_22, p_engine_22, 0x0},
    {"engine.dll", "P5-01c", 0x002B346D, 1, s_engine_23, p_engine_23, 0x0},
    {"engine.dll", "P5-01d", 0x002B3499, 1, s_engine_24, p_engine_24, 0x0},
    {"engine.dll", "P5-06a", 0x002B3708, 1, s_engine_25, p_engine_25, 0x0},
    {"engine.dll", "P5-06c", 0x002B3826, 1, s_engine_26, p_engine_26, 0x0},
    {"engine.dll", "P5-06d", 0x002B3835, 1, s_engine_27, p_engine_27, 0x0},
    {"engine.dll", "P5-07a", 0x002B3931, 1, s_engine_28, p_engine_28, 0x0},
    {"engine.dll", "P5-02a", 0x002B4438, 1, s_engine_29, p_engine_29, 0x0},
    {"engine.dll", "P5-02b", 0x002B4446, 1, s_engine_30, p_engine_30, 0x0},
    {"engine.dll", "P5-02c", 0x002B445D, 1, s_engine_31, p_engine_31, 0x0},
    {"engine.dll", "P5-02d", 0x002B4476, 1, s_engine_32, p_engine_32, 0x0},
    {"engine.dll", "P5-05a", 0x002B45DD, 1, s_engine_33, p_engine_33, 0x0},
    {"engine.dll", "P5-05b", 0x002B4616, 1, s_engine_34, p_engine_34, 0x0},
    {"engine.dll", "P5-05c", 0x002B4631, 1, s_engine_35, p_engine_35, 0x0},
    {"engine.dll", "P4-05", 0x003EB8E1, 2, s_engine_36, p_engine_36, 0x0},
    {"engine.dll", "P4-06", 0x003EB8E9, 2, s_engine_37, p_engine_37, 0x0},
    {"engine.dll", "P5-06b", 0x0040D552, 1, s_engine_38, p_engine_38, 0x0},
    {"engine.dll", "P5-06b", 0x0040D556, 1, s_engine_39, p_engine_39, 0x0},
    {"engine.dll", "P5-04", 0x0040D558, 1, s_engine_40, p_engine_40, 0x0},
    {"engine.dll", "P5-04", 0x0040D560, 1, s_engine_41, p_engine_41, 0x0},
    {"engine.dll", "P5-07b", 0x0040D56A, 1, s_engine_42, p_engine_42, 0x0},
    {"engine.dll", "P5-07b", 0x0040D56E, 1, s_engine_43, p_engine_43, 0x0},
    {"engine.dll", "P4-03", 0x004F16DA, 2, s_engine_44, p_engine_44, 0x0},
    {"engine.dll", "P4-03", 0x004F16DE, 2, s_engine_45, p_engine_45, 0x0},
    {"engine.dll", "P4-03", 0x004F16E2, 2, s_engine_46, p_engine_46, 0x0},
    {"engine.dll", "P4-04", 0x004F16EA, 2, s_engine_47, p_engine_47, 0x0},
    {"engine.dll", "P4-04", 0x004F16EE, 2, s_engine_48, p_engine_48, 0x0},
    {"engine.dll", "P4-04", 0x004F16F2, 2, s_engine_49, p_engine_49, 0x0},
    {"server.dll", "P5-09a", 0x00303481, 3, s_server_0, p_server_0, 0x0},
    {"server.dll", "P5-09b", 0x00303489, 3, s_server_1, p_server_1, 0x0},
    {"server.dll", "P5-08", 0x003034AA, 1, s_server_2, p_server_2, 0x0},
    {"server.dll", "P4-16a", 0x00336067, 3, s_server_3, p_server_3, 0x0},
    {"server.dll", "P4-16b", 0x00336076, 3, s_server_4, p_server_4, 0x0},
    {"server.dll", "P4-15a", 0x003360A8, 1, s_server_5, p_server_5, 0x0},
    {"server.dll", "P4-15b", 0x003360E1, 1, s_server_6, p_server_6, 0x0},
    {"server.dll", "P4-15c", 0x00336119, 1, s_server_7, p_server_7, 0x0},
    {"server.dll", "P4-15d", 0x00336151, 1, s_server_8, p_server_8, 0x0},
    {"server.dll", "P4-15e", 0x00336189, 1, s_server_9, p_server_9, 0x0},
    {"server.dll", "P4-15f", 0x003361C1, 1, s_server_10, p_server_10, 0x0},
    {"server.dll", "P4-13", 0x00CA63EA, 2, s_server_11, p_server_11, 0x0},
    {"client.dll", "P4-12", 0x006F009C, 5, s_client_0, p_client_0, 0x0},
    {"client.dll", "P4-14", 0x00878F7A, 2, s_client_1, p_client_1, 0x0},
};
static const ProbeFile kFiles[] = {
    {"engine.dll", "F722107111C4301A329BADF43B039D776E061CAE5F9F8AB5013ADB4E06709394", "64CD00E2D00220572AB9B41F8341ABE66820532BEC3F103EBECF53654277AF23", 0x180000000ull},
    {"server.dll", "067A2F292204B63B56CDED9DD8119CFCB4991FBED29808DAD0F7ABD922767D55", "1AFE5D83D61DCE6C91102E58AB1755EE6F404910EEE7C045D4BFA23366EC49E3", 0x180000000ull},
    {"client.dll", "DDA3F6B7AD831F2B9D46CC963F1AA427F0F8E21612797FC186AFBB33C4A0BBBA", "E1EE72F9A89CCEDB823E02D622A0F1EEEFC842E18DFC93A83F24BDBB894D2E21", 0x180000000ull},
};
static const unsigned char a_crcfunc[] = {0x40, 0x55, 0x41, 0x54, 0x48, 0x8B, 0xEC, 0x48, 0x83, 0xEC, 0x58, 0x48, 0x8D, 0x4D, 0x18, 0xE8};
static const unsigned char a_getter[] = {0x8B, 0x05, 0xFA, 0x39, 0x6F, 0x00, 0xC3};
static const unsigned char a_store[] = {0xE8, 0x04, 0xF6, 0xFF, 0xFF, 0x89, 0x05, 0xFE, 0x38, 0x6F, 0x00};
static const unsigned char a_hs_name[] = {0x41, 0xB8, 0x00, 0x01, 0x00, 0x00, 0x48, 0x8D, 0x0D, 0x36, 0x0E, 0x6D, 0x00, 0x48, 0x8B, 0xD3, 0xE8, 0x0E, 0xC0, 0x0C, 0x00};
static const unsigned char a_hs_state[] = {0x48, 0x8B, 0x5C, 0x24, 0x30, 0xC7, 0x05, 0xEE, 0x0D, 0x6D, 0x00, 0x03, 0x00, 0x00, 0x00, 0x48, 0x83, 0xC4, 0x20, 0x5F, 0xC3};
// Anchors: unchanged engine code (SendTable CRC; host-state map name), verified before use.
static const ProbeSite kAnchors[] = {
    {"engine.dll", "crcfunc", 0x001B25F0, 16, a_crcfunc, a_crcfunc, 0x0},
    {"engine.dll", "getter", 0x001B2EF0, 7, a_getter, a_getter, 0x0},
    {"engine.dll", "store", 0x001B2FE7, 11, a_store, a_store, 0x0},
    {"engine.dll", "hs_name", 0x001DB3AD, 21, a_hs_name, a_hs_name, 0x0},
    {"engine.dll", "hs_state", 0x001DB3D7, 21, a_hs_state, a_hs_state, 0x0},
};
static const unsigned kCrcFuncRva = 0x001B25F0;
static const unsigned kCrcGlobalRva = 0x008A68F0;
static const unsigned kHostStateRva = 0x008AC1D0;
static const unsigned char a_spawn[] = {0x48, 0x89, 0x5C, 0x24, 0x08, 0x48, 0x89, 0x74, 0x24, 0x10, 0x48, 0x89, 0x7C, 0x24, 0x18, 0x55};
static const unsigned char a_clnew[] = {0x40, 0x57, 0x48, 0x81, 0xEC, 0x10, 0x01, 0x00, 0x00, 0x83, 0xB9, 0x4C, 0x01, 0x00, 0x00, 0x03};
// Client-module hook prologues (replaced by a jump once hooked; not part of kAnchors).
static const ProbeSite kHooks[] = {
    {"engine.dll", "spawn", 0x00166EA0, 16, a_spawn, a_spawn, 0x0},
    {"engine.dll", "clnew", 0x000B0860, 16, a_clnew, a_clnew, 0x0},
};
static const unsigned kClStateOff = 0x14C;
static const unsigned kClLevelOff = 0x1C8;

// ---- SendProp / SendTable field offsets in server.dll ----
// Re-derived from the binaries (research PATCH_LOG, SendProp layout). kPropVarName follows the public SDK member
// order and is cross-checked by FindSendTable's stride search; kPropFlags only bounds that search.
const size_t kPropBits = 0x14, kPropLow = 0x18, kPropHigh = 0x1C, kPropVarName = 0x48, kPropHLMul = 0x50,
             kPropFlags = 0x54;
const size_t kTblCount = 0x08, kTblName = 0x10;

// ---- Engine file system providers (map size check) ----
const FsProvider kFsProviders[] = {
    {"dedicated.dll", "571DAF9858DE4BC2088686EB4F5CBC585032EAA4943C4971FAB3DA5BC10757C6", 0x0017E530, 0x00129770,
     {0x00020800, 0x00024870, 0x0001EF70, 0x00013ED0, 0x00022D90}},
    {"filesystem_stdio.dll", "BD03C74FEA015603616EBF3AD8CA0A4A9435C2D50459622F5CFA6EECCF41F7FB", 0, 0x000B2B10,
     {0x00020970, 0x00024B50, 0x0001EF80, 0x00013CC0, 0x00022F00}},
};

// ---- r_mapextents ConVar in client.dll (game only) ----
// Located from its registration code (0x18003CF30) and the ConVar constructor's field stores.
const MapExtentsVar kMapExtents = {0x00A36950, 0x006F00A8, 0x006F009C, 0x18, 0x38, 0x40, 0x48, 0x50, 0x54, 0x58};

}  // namespace bm
