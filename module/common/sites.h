// Verified patch-site table for the supported stock binary builds.
#pragma once

struct ProbeSite {
    const char* module;
    const char* id;
    unsigned rva;
    unsigned len;
    const unsigned char* stock;
    const unsigned char* patched;
    unsigned relocMask; // bit i: a relocated value starts at byte i
};

struct ProbeFile {
    const char* module;
    const char* stockSha;
    const char* patchedSha;
    unsigned long long preferredBase;
};

struct ProbeImage {  // Linux: load size and the ranges searched for strings / SendTables
    const char* module;
    unsigned size, roRva, roSize, rwRva, rwSize;
};

#if defined(_WIN32) && defined(_WIN64)
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
#endif // x64

#if defined(_WIN32) && !defined(_WIN64)
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
#endif // x86

#if defined(__linux__) && defined(__i386__)
static const unsigned char s_engine_srv_0[] = {0xF2};
static const unsigned char p_engine_srv_0[] = {0xF1};
static const unsigned char s_engine_srv_1[] = {0x14};
static const unsigned char p_engine_srv_1[] = {0x15};
static const unsigned char s_engine_srv_2[] = {0x0F};
static const unsigned char p_engine_srv_2[] = {0x10};
static const unsigned char s_engine_srv_3[] = {0x24, 0x08, 0x29, 0x00};
static const unsigned char p_engine_srv_3[] = {0xED, 0xDD, 0x29, 0x00};
static const unsigned char s_engine_srv_4[] = {0xEC, 0x7F};
static const unsigned char p_engine_srv_4[] = {0xF6, 0xFF};
static const unsigned char s_engine_srv_5[] = {0xA8, 0x92, 0x27, 0x00};
static const unsigned char p_engine_srv_5[] = {0x2D, 0xDB, 0x29, 0x00};
static const unsigned char s_engine_srv_6[] = {0xA8, 0x92, 0x27, 0x00};
static const unsigned char p_engine_srv_6[] = {0x2D, 0xDB, 0x29, 0x00};
static const unsigned char s_engine_srv_7[] = {0x24, 0x08, 0x29, 0x00};
static const unsigned char p_engine_srv_7[] = {0xED, 0xDD, 0x29, 0x00};
static const unsigned char s_engine_srv_8[] = {0x80, 0x00};
static const unsigned char p_engine_srv_8[] = {0x00, 0x01};
static const unsigned char s_engine_srv_9[] = {0x80, 0xC6};
static const unsigned char p_engine_srv_9[] = {0x00, 0xC7};
static const unsigned char s_engine_srv_10[] = {0x80, 0xC6};
static const unsigned char p_engine_srv_10[] = {0x00, 0xC7};
static const unsigned char s_engine_srv_11[] = {0x80, 0xC6};
static const unsigned char p_engine_srv_11[] = {0x00, 0xC7};
static const unsigned char s_engine_srv_12[] = {0x0C};
static const unsigned char p_engine_srv_12[] = {0x0D};
static const unsigned char s_engine_srv_13[] = {0x0C};
static const unsigned char p_engine_srv_13[] = {0x0D};
static const unsigned char s_engine_srv_14[] = {0x0C};
static const unsigned char p_engine_srv_14[] = {0x0D};
static const unsigned char s_engine_srv_15[] = {0x0C};
static const unsigned char p_engine_srv_15[] = {0x0D};
static const unsigned char s_engine_srv_16[] = {0x0C};
static const unsigned char p_engine_srv_16[] = {0x0D};
static const unsigned char s_engine_srv_17[] = {0x0C};
static const unsigned char p_engine_srv_17[] = {0x0D};
static const unsigned char s_engine_srv_18[] = {0x24, 0x08, 0x29, 0x00};
static const unsigned char p_engine_srv_18[] = {0xED, 0xDD, 0x29, 0x00};
static const unsigned char s_engine_srv_19[] = {0xA8, 0x92, 0x27, 0x00};
static const unsigned char p_engine_srv_19[] = {0x2D, 0xDB, 0x29, 0x00};
static const unsigned char s_engine_srv_20[] = {0x03};
static const unsigned char p_engine_srv_20[] = {0x04};
static const unsigned char s_engine_srv_21[] = {0x11};
static const unsigned char p_engine_srv_21[] = {0x12};
static const unsigned char s_engine_srv_22[] = {0x11};
static const unsigned char p_engine_srv_22[] = {0x12};
static const unsigned char s_engine_srv_23[] = {0x0D};
static const unsigned char p_engine_srv_23[] = {0x0E};
static const unsigned char s_engine_srv_24[] = {0x0E};
static const unsigned char p_engine_srv_24[] = {0x0F};
static const unsigned char s_engine_srv_25[] = {0x1F};
static const unsigned char p_engine_srv_25[] = {0x3F};
static const unsigned char s_engine_srv_26[] = {0x3F};
static const unsigned char p_engine_srv_26[] = {0x7F};
static const unsigned char s_engine_srv_27[] = {0x0D};
static const unsigned char p_engine_srv_27[] = {0x0E};
static const unsigned char s_engine_srv_28[] = {0x0E};
static const unsigned char p_engine_srv_28[] = {0x0F};
static const unsigned char s_engine_srv_29[] = {0x0D};
static const unsigned char p_engine_srv_29[] = {0x0E};
static const unsigned char s_engine_srv_30[] = {0x3F};
static const unsigned char p_engine_srv_30[] = {0x7F};
static const unsigned char s_engine_srv_31[] = {0x03};
static const unsigned char p_engine_srv_31[] = {0x04};
static const unsigned char s_engine_srv_32[] = {0x0E};
static const unsigned char p_engine_srv_32[] = {0x0F};
static const unsigned char s_engine_srv_33[] = {0x38};
static const unsigned char p_engine_srv_33[] = {0x78};
static const unsigned char s_engine_srv_34[] = {0x03};
static const unsigned char p_engine_srv_34[] = {0x04};
static const unsigned char s_engine_srv_35[] = {0xEC, 0x7F};
static const unsigned char p_engine_srv_35[] = {0xF6, 0xFF};
static const unsigned char s_engine_srv_36[] = {0xEC, 0x7F};
static const unsigned char p_engine_srv_36[] = {0xF6, 0xFF};
static const unsigned char s_engine_srv_37[] = {0x14};
static const unsigned char p_engine_srv_37[] = {0x15};
static const unsigned char s_engine_srv_38[] = {0x12};
static const unsigned char p_engine_srv_38[] = {0x13};
static const unsigned char s_engine_srv_39[] = {0x0F};
static const unsigned char p_engine_srv_39[] = {0x10};
static const unsigned char s_engine_srv_40[] = {0x14};
static const unsigned char p_engine_srv_40[] = {0x15};
static const unsigned char s_engine_srv_41[] = {0x13};
static const unsigned char p_engine_srv_41[] = {0x14};
static const unsigned char s_engine_srv_42[] = {0x11};
static const unsigned char p_engine_srv_42[] = {0x12};
static const unsigned char s_server_srv_0[] = {0x80, 0x46};
static const unsigned char p_server_srv_0[] = {0x00, 0x47};
static const unsigned char s_rt_0[] = {0x00, 0x00, 0x80, 0xC6, 0x00, 0x00, 0x80, 0xC6, 0x00, 0x00, 0x80, 0xC6};
static const unsigned char p_rt_0[] = {0x00, 0x00, 0x00, 0xC7, 0x00, 0x00, 0x00, 0xC7, 0x00, 0x00, 0x00, 0xC7};
static const unsigned char s_rt_1[] = {0x00, 0x00, 0x80, 0x46, 0x00, 0x00, 0x80, 0x46, 0x00, 0x00, 0x80, 0x46};
static const unsigned char p_rt_1[] = {0x00, 0x00, 0x00, 0x47, 0x00, 0x00, 0x00, 0x47, 0x00, 0x00, 0x00, 0x47};
static const ProbeSite kSites[] = {
    {"engine_srv.so", "P5-03a", 0x00126719, 1, s_engine_srv_0, p_engine_srv_0, 0x0},
    {"engine_srv.so", "P5-03b", 0x0012671C, 1, s_engine_srv_1, p_engine_srv_1, 0x0},
    {"engine_srv.so", "P5-03c", 0x0012671F, 1, s_engine_srv_2, p_engine_srv_2, 0x0},
    {"engine_srv.so", "P4-07a", 0x001BEF43, 4, s_engine_srv_3, p_engine_srv_3, 0x1},
    {"engine_srv.so", "P4-06b", 0x001BEFF9, 2, s_engine_srv_4, p_engine_srv_4, 0x0},
    {"engine_srv.so", "P4-07d", 0x001BF06D, 4, s_engine_srv_5, p_engine_srv_5, 0x1},
    {"engine_srv.so", "P4-07b", 0x001BF0E3, 4, s_engine_srv_6, p_engine_srv_6, 0x1},
    {"engine_srv.so", "P4-07c", 0x001BF132, 4, s_engine_srv_7, p_engine_srv_7, 0x1},
    {"engine_srv.so", "P4-08", 0x001BF2C2, 2, s_engine_srv_8, p_engine_srv_8, 0x0},
    {"engine_srv.so", "P4-09a", 0x001BF2DB, 2, s_engine_srv_9, p_engine_srv_9, 0x0},
    {"engine_srv.so", "P4-09b", 0x001BF2E5, 2, s_engine_srv_10, p_engine_srv_10, 0x0},
    {"engine_srv.so", "P4-09c", 0x001BF2EC, 2, s_engine_srv_11, p_engine_srv_11, 0x0},
    {"engine_srv.so", "P4-10d", 0x001CA36E, 1, s_engine_srv_12, p_engine_srv_12, 0x0},
    {"engine_srv.so", "P4-10e", 0x001CA3CF, 1, s_engine_srv_13, p_engine_srv_13, 0x0},
    {"engine_srv.so", "P4-10f", 0x001CA430, 1, s_engine_srv_14, p_engine_srv_14, 0x0},
    {"engine_srv.so", "P4-10a", 0x001CD0F0, 1, s_engine_srv_15, p_engine_srv_15, 0x0},
    {"engine_srv.so", "P4-10b", 0x001CD14E, 1, s_engine_srv_16, p_engine_srv_16, 0x0},
    {"engine_srv.so", "P4-10c", 0x001CD1AC, 1, s_engine_srv_17, p_engine_srv_17, 0x0},
    {"engine_srv.so", "P4-01", 0x00209A1F, 4, s_engine_srv_18, p_engine_srv_18, 0x1},
    {"engine_srv.so", "P4-02", 0x00209A33, 4, s_engine_srv_19, p_engine_srv_19, 0x1},
    {"engine_srv.so", "P5-05a", 0x0020C2ED, 1, s_engine_srv_20, p_engine_srv_20, 0x0},
    {"engine_srv.so", "P5-05b", 0x0020C403, 1, s_engine_srv_21, p_engine_srv_21, 0x0},
    {"engine_srv.so", "P5-05c", 0x0020C405, 1, s_engine_srv_22, p_engine_srv_22, 0x0},
    {"engine_srv.so", "P5-02a", 0x0020C52E, 1, s_engine_srv_23, p_engine_srv_23, 0x0},
    {"engine_srv.so", "P5-02b", 0x0020C600, 1, s_engine_srv_24, p_engine_srv_24, 0x0},
    {"engine_srv.so", "P5-02c", 0x0020C622, 1, s_engine_srv_25, p_engine_srv_25, 0x0},
    {"engine_srv.so", "P5-02d", 0x0020C638, 1, s_engine_srv_26, p_engine_srv_26, 0x0},
    {"engine_srv.so", "P5-01a", 0x0020D8A9, 1, s_engine_srv_27, p_engine_srv_27, 0x0},
    {"engine_srv.so", "P5-01c", 0x0020D8C6, 1, s_engine_srv_28, p_engine_srv_28, 0x0},
    {"engine_srv.so", "P5-01b", 0x0020D8CE, 1, s_engine_srv_29, p_engine_srv_29, 0x0},
    {"engine_srv.so", "P5-01d", 0x0020D8EE, 1, s_engine_srv_30, p_engine_srv_30, 0x0},
    {"engine_srv.so", "P5-06a", 0x0020DA81, 1, s_engine_srv_31, p_engine_srv_31, 0x0},
    {"engine_srv.so", "P5-06d", 0x0020DB9F, 1, s_engine_srv_32, p_engine_srv_32, 0x0},
    {"engine_srv.so", "P5-06c", 0x0020DBB0, 1, s_engine_srv_33, p_engine_srv_33, 0x0},
    {"engine_srv.so", "P5-07a", 0x0020DD8D, 1, s_engine_srv_34, p_engine_srv_34, 0x0},
    {"engine_srv.so", "P4-06", 0x00290829, 2, s_engine_srv_35, p_engine_srv_35, 0x0},
    {"engine_srv.so", "P4-05", 0x0029082D, 2, s_engine_srv_36, p_engine_srv_36, 0x0},
    {"engine_srv.so", "P5-07b", 0x0029C4FA, 1, s_engine_srv_37, p_engine_srv_37, 0x0},
    {"engine_srv.so", "P5-07b", 0x0029C4FE, 1, s_engine_srv_38, p_engine_srv_38, 0x0},
    {"engine_srv.so", "P5-04", 0x0029C500, 1, s_engine_srv_39, p_engine_srv_39, 0x0},
    {"engine_srv.so", "P5-04", 0x0029C508, 1, s_engine_srv_40, p_engine_srv_40, 0x0},
    {"engine_srv.so", "P5-06b", 0x0029C50E, 1, s_engine_srv_41, p_engine_srv_41, 0x0},
    {"engine_srv.so", "P5-06b", 0x0029C512, 1, s_engine_srv_42, p_engine_srv_42, 0x0},
    {"server_srv.so", "P4-13", 0x014AC072, 2, s_server_srv_0, p_server_srv_0, 0x0},
    {"engine_srv.so", "P4-03", 0x003ABD30, 12, s_rt_0, p_rt_0, 0x0},
    {"engine_srv.so", "P4-04", 0x003ABD20, 12, s_rt_1, p_rt_1, 0x0},
};
static const ProbeFile kFiles[] = {
    {"engine_srv.so", "32219560058808F50A82C9372C2C4B5F5A872508E5F6BFEC06799E9D97109175", "532A041F52FFF4E427966F84115DF084FD21A9527FC2393A4C5D5EE3E232596B", 0x0ull},
    {"server_srv.so", "936362EDFB2845C3495BE526A539E8ACF8856FC16CFF487D26997E736D8B7851", "48FD6C3AB813009A98D2414324E8A7E029C59A37B3AA571FB33570089712A5F3", 0x0ull},
};
static const ProbeImage kImages[] = {
    {"engine_srv.so", 0x00418660, 0x00271060, 0x00033354, 0x0030D9F0, 0x0010AC70},
    {"server_srv.so", 0x016758C0, 0x01092240, 0x0020C52C, 0x014A9F2C, 0x001CB994},
};
static const unsigned char a_crcfunc[] = {0x55, 0x89, 0xE5, 0x57, 0x56, 0x8D, 0x75, 0xE4, 0x53, 0x31, 0xDB, 0x83, 0xEC, 0x2C, 0x89, 0x34};
static const unsigned char a_getter[] = {0x55, 0xA1, 0x60, 0xC6, 0x37, 0x00, 0x89, 0xE5, 0x5D, 0xC3};
static const unsigned char a_store[] = {0xE8, 0x28, 0xFE, 0xFF, 0xFF, 0xA3, 0x60, 0xC6, 0x37, 0x00};
static const unsigned char a_hs_name[] = {0xC7, 0x44, 0x24, 0x08, 0x00, 0x01, 0x00, 0x00, 0xC7, 0x04, 0x24, 0x20, 0x0F, 0x38, 0x00, 0x89, 0x44, 0x24, 0x04, 0xE8, 0xF2, 0x80, 0x0C, 0x00};
static const unsigned char a_hs_state[] = {0xC7, 0x05, 0x04, 0x0F, 0x38, 0x00, 0x03, 0x00, 0x00, 0x00};
// Anchors: unchanged engine code (SendTable CRC; host-state map name), verified before use.
static const ProbeSite kAnchors[] = {
    {"engine_srv.so", "crcfunc", 0x00135120, 16, a_crcfunc, a_crcfunc, 0x0},
    {"engine_srv.so", "getter", 0x001351A0, 10, a_getter, a_getter, 0x4},
    {"engine_srv.so", "store", 0x001352F3, 10, a_store, a_store, 0x40},
    {"engine_srv.so", "hs_name", 0x0015A146, 24, a_hs_name, a_hs_name, 0x800},
    {"engine_srv.so", "hs_state", 0x0015A179, 10, a_hs_state, a_hs_state, 0x4},
};
static const unsigned kCrcFuncRva = 0x00135120;
static const unsigned kCrcGlobalRva = 0x0037C660;
static const unsigned kHostStateRva = 0x00380F00;
#endif // linux-x86
