// Small line-based interface to the pinned mGBA core. No GUI or user saves.
#include <mgba/flags.h>
#include <mgba/core/core.h>
#include <mgba/core/log.h>
#include <mgba-util/vfs.h>

static void log_message(struct mLogger* logger, int category, enum mLogLevel level,
                        const char* format, va_list args) {
    (void) logger;
    if (level > mLOG_WARN) return;
    fprintf(stderr, "%s: ", mLogCategoryName(category));
    vfprintf(stderr, format, args);
    fputc('\n', stderr);
    if (level == mLOG_FATAL) exit(1);
}

int main(int argc, char** argv) {
    if (argc != 2 && argc != 3) { fprintf(stderr, "Usage: emerald-emulator ROM [SAVE]\n"); return 2; }
    struct mLogger logger = { .log = log_message, .filter = NULL };
    mLogSetDefaultLogger(&logger);
    struct mCore* core = mCoreFind(argv[1]);
    if (!core || !core->init(core)) return 1;
    mCoreInitConfig(core, "emerald-tests");
    // Do not load desktop configuration, BIOS, cheats, or autoload saves.
    mCoreConfigSetIntValue(&core->config, "useBios", 0);
    mCoreConfigSetIntValue(&core->config, "skipBios", 1);
    mCoreLoadForeignConfig(core, &core->config);
    unsigned width, height;
    core->desiredVideoDimensions(core, &width, &height);
    color_t* pixels = calloc(width * height, sizeof(color_t));
    if (!pixels || !mCoreLoadFile(core, argv[1])) return 1;
    // An optional save file, opened temporarily so the copy on disk isn't changed.
    if (argc == 3 && !mCoreLoadSaveFile(core, argv[2], true)) return 1;
    core->setVideoBuffer(core, pixels, width);
    core->rtc.override = RTC_FIXED;
    core->rtc.value = 946684800;
    core->reset(core);
    setvbuf(stdout, NULL, _IOLBF, 0);
    puts("READY");
    char line[4096];
    int result = 0;
    while (fgets(line, sizeof(line), stdin)) {
        unsigned frames, keys, address, bits;
        if (sscanf(line, "run %u %u", &frames, &keys) == 2 && frames <= 36000 && keys <= 1023) {
            core->setKeys(core, keys);
            for (unsigned i = 0; i < frames; ++i) core->runFrame(core);
            printf("OK %u\n", core->frameCounter(core));
        } else if (sscanf(line, "read %u %x", &bits, &address) == 2 && (bits == 8 || bits == 16 || bits == 32)) {
            unsigned value = bits == 8 ? core->busRead8(core, address) :
                bits == 16 ? core->busRead16(core, address) : core->busRead32(core, address);
            printf("VALUE %u\n", value);
        } else if (sscanf(line, "write %u %x %u", &bits, &address, &frames) == 3 && (bits == 8 || bits == 16 || bits == 32)) {
            if (bits == 8) core->busWrite8(core, address, frames);
            else if (bits == 16) core->busWrite16(core, address, frames);
            else core->busWrite32(core, address, frames);
            puts("OK");
        } else if (!strncmp(line, "screenshot ", 11)) {
            char* path = line + 11;
            path[strcspn(path, "\r\n")] = 0;
            struct VFile* vf = VFileOpen(path, O_WRONLY | O_CREAT | O_TRUNC);
            bool ok = vf && mCoreTakeScreenshotVF(core, vf);
            if (vf) vf->close(vf);
            puts(ok ? "OK" : "ERROR screenshot");
        } else if (!strcmp(line, "quit\n")) {
            puts("OK");
            break;
        } else {
            puts("ERROR invalid command");
            result = 2;
            break;
        }
    }
    mCoreConfigDeinit(&core->config);
    core->deinit(core);
    free(pixels);
    return result;
}
