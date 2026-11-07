// Linux x86 target: srcds_linux of dedicated-server app 4020 build 25375525 (dedicated server only; no client,
// so no game hooks and no r_mapextents). ELF modules are linked at base 0, hence preferredBase 0.
#pragma once

namespace bm
{

// ---- Generated: patch sites, file identities, engine anchors, ELF image ranges ----
// Site ids (P4-xx, P5-xx) refer to the research project's patch log.
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

// ---- SendProp / SendTable field offsets in server_srv.so ----
// GCC build with the same 32-bit layout as Windows x86 (SendPropFloat, SendTable::Construct; LINUX_PORT.md).
const size_t kPropBits = 0x0C, kPropLow = 0x10, kPropHigh = 0x14, kPropVarName = 0x30, kPropHLMul = 0x34,
             kPropFlags = 0x38;
const size_t kTblCount = 0x04, kTblName = 0x08;

// ---- Engine file system provider (map size check) ----
// dedicated_srv.so's FileSystemFactory returns g_pBaseFileSystem (= &g_FileSystem_Stdio + 4, the IBaseFileSystem
// subobject); vtable = CFileSystem_Stdio's secondary vtable, slots = the _ZThn4_ thunks of
// CBaseFileSystem::Read/Write/Open/Close/Seek(void*, long long, FileSystemSeek_t) (LINUX_PORT.md).
const FsProvider kFsProviders[] = {
    {"dedicated_srv.so", "FA4B05805D2ABFF723867C73C2EE748134393E1CD0595B794F5CE9FB1759F28F", 0x00156438, 0x0011ECD4,
     {0x00045C00, 0x0004B560, 0x00045BB0, 0x00049880, 0x0004C4D0}},
};

}  // namespace bm
