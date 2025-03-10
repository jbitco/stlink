/* == nightwalker-87: TODO: CONTENT AND USE OF THIS SOURCE FILE IS TO BE VERIFIED (07.06.2023) == */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stlink.h>
#include <flash.h>
#include <flash_opts.h>

#if defined(_MSC_VER)
#include <malloc.h>
#endif

struct Test {
    const char * cmd_line;
    int32_t res;
    struct flash_opts opts;
};

static bool cmp_strings(const char * s1, const char * s2) {
    if(s1 == NULL || s2 == NULL) {
        return (s1 == s2);
    } else {
        return (0 == strcmp(s1, s2));
    }
}

static bool cmp_mem(const uint8_t * s1, const uint8_t * s2, uint32_t size) {
    if(s1 == NULL || s2 == NULL) {
        return (s1 == s2);
    } else {
        return (0 == memcmp(s1, s2, size));
    }
}

static bool execute_test(const struct Test * test) {
    int32_t ac = 0;
    char* av[32];

    /* parse (tokenize) the test command line */
    #if defined(_MSC_VER)
    char *cmd_line = alloca(strlen(test->cmd_line) + 1);
    #else
    char cmd_line[strlen(test->cmd_line) + 1];
    #endif

    strcpy(cmd_line, test->cmd_line);

    for(char * tok = strtok(cmd_line, " "); tok; tok = strtok(NULL, " ")) {
        if((size_t) ac >= sizeof(av) / sizeof(av[0])) return (false);

        av[ac] = tok;
        ++ac;
    }

    /* Call */
    struct flash_opts opts;
    int32_t res = flash_get_opts(&opts, ac, av);

    /* Compare results */
    bool ret = (res == test->res);

    if(ret && (res == 0)) {
        ret &= (opts.cmd == test->opts.cmd);
        ret &= cmp_mem(opts.serial, test->opts.serial, sizeof(opts.serial));
        ret &= cmp_strings(opts.filename, test->opts.filename);
        ret &= (opts.addr == test->opts.addr);
        ret &= (opts.size == test->opts.size);
        ret &= (opts.reset == test->opts.reset);
        ret &= (opts.log_level == test->opts.log_level);
        ret &= (opts.freq == test->opts.freq);
        ret &= (opts.format == test->opts.format);
    }

    printf("[%s] (%d) %s\n", ret ? "OK" : "ERROR", res, test->cmd_line);
    return (ret);
}

static struct Test tests[] = {
    { "", -1, FLASH_OPTS_INITIALIZER },
    { "--debug --reset read test.bin 0x80000000 0x1000", 0,
      { .cmd = FLASH_CMD_READ,
        .serial = { 0 },
        .filename = "test.bin",
        .addr = 0x80000000,
        .size = 0x1000,
        .reset = 1,
        .log_level = DEBUG_LOG_LEVEL,
        .freq = 0,
        .format = FLASH_FORMAT_BINARY }
    },
    { "--debug --mass-erase --reset write test.bin 0x80000000", 0,
      { .cmd = FLASH_CMD_WRITE,
        .serial = { 0 },
        .filename = "test.bin",
        .addr = 0x80000000,
        .size = 0,
        .reset = 1,
        .log_level = DEBUG_LOG_LEVEL,
        .mass_erase = 1,
        .freq = 0,
        .format = FLASH_FORMAT_BINARY }
    },
    { "--debug --freq 5k --reset write test.bin 0x80000000", 0,
      { .cmd = FLASH_CMD_WRITE,
        .serial = { 0 },
        .filename = "test.bin",
        .addr = 0x80000000,
        .size = 0,
        .reset = 1,
        .log_level = DEBUG_LOG_LEVEL,
        .freq = 5,
        .format = FLASH_FORMAT_BINARY }
    },
    { "--debug --freq 15k --reset write test.bin 0x80000000", 0,
      { .cmd = FLASH_CMD_WRITE,
        .serial = { 0 },
        .filename = "test.bin",
        .addr = 0x80000000,
        .size = 0,
        .reset = 1,
        .log_level = DEBUG_LOG_LEVEL,
        .freq = 15,
        .format = FLASH_FORMAT_BINARY }
    },
    { "--debug --freq=5k --reset write test.bin 0x80000000", 0,
      { .cmd = FLASH_CMD_WRITE,
        .serial = { 0 },
        .filename = "test.bin",
        .addr = 0x80000000,
        .size = 0,
        .reset = 1,
        .log_level = DEBUG_LOG_LEVEL,
        .freq = 5,
        .format = FLASH_FORMAT_BINARY }
    },
    { "--debug --reset read test.bin 0x80000000 1000", 0,
      { .cmd = FLASH_CMD_READ,
        .serial = { 0 },
        .filename = "test.bin",
        .addr = 0x80000000,
        .size = 1000,
        .reset = 1,
        .log_level = DEBUG_LOG_LEVEL,
        .freq = 0,
        .format = FLASH_FORMAT_BINARY }
    },
    { "--debug --reset read test.bin 0x80000000 1k", 0,
      { .cmd = FLASH_CMD_READ,
        .serial = { 0 },
        .filename = "test.bin",
        .addr = 0x80000000,
        .size = 1024,
        .reset = 1,
        .log_level = DEBUG_LOG_LEVEL,
        .freq = 0,
        .format = FLASH_FORMAT_BINARY }
    },
    { "--debug --reset read test.bin 0x80000000 1M", 0,
      { .cmd = FLASH_CMD_READ,
        .serial = { 0 },
        .filename = "test.bin",
        .addr = 0x80000000,
        .size = 1048576,
        .reset = 1,
        .log_level = DEBUG_LOG_LEVEL,
        .freq = 0,
        .format = FLASH_FORMAT_BINARY }
    },
    { "--debug --reset write test.bin 0x80000000", 0,
      { .cmd = FLASH_CMD_WRITE,
        .serial = { 0 },
        .filename = "test.bin",
        .addr = 0x80000000,
        .size = 0,
        .reset = 1,
        .log_level = DEBUG_LOG_LEVEL,
        .freq = 0,
        .format = FLASH_FORMAT_BINARY }
    },
    { "erase", 0,
      { .cmd = FLASH_CMD_ERASE,
        .serial = { 0 },
        .filename = NULL,
        .addr = 0,
        .size = 0,
        .reset = 0,
        .log_level = STND_LOG_LEVEL,
        .mass_erase = 1,
        .freq = 0,
        .format = FLASH_FORMAT_BINARY }
    },
    { "--mass-erase erase", 0,
      { .cmd = FLASH_CMD_ERASE,
        .serial = { 0 },
        .filename = NULL,
        .addr = 0,
        .size = 0,
        .reset = 0,
        .log_level = STND_LOG_LEVEL,
        .mass_erase = 1,
        .freq = 0,
        .format = FLASH_FORMAT_BINARY }
    },
    { "--debug --reset --format=ihex write test.hex", 0,
      { .cmd = FLASH_CMD_WRITE,
        .serial = { 0 },
        .filename = "test.hex",
        .addr = 0,
        .size = 0,
        .reset = 1,
        .log_level = DEBUG_LOG_LEVEL,
        .freq = 0,
        .format = FLASH_FORMAT_IHEX }
    },
    { "--debug --reset --format=binary write test.hex", -1, FLASH_OPTS_INITIALIZER },
    { "--debug --reset --format=ihex write test.hex 0x80000000", -1, FLASH_OPTS_INITIALIZER },
    { "--debug --reset write test.hex sometext", -1, FLASH_OPTS_INITIALIZER },
    { "--serial ABCEFF544851717867216044 erase sometext", -1, FLASH_OPTS_INITIALIZER },
    { "--serial ABCEFF544851717867216044 erase", 0,
      { .cmd = FLASH_CMD_ERASE,
        .serial = "ABCEFF544851717867216044",
        .filename = NULL,
        .addr = 0,
        .size = 0,
        .reset = 0,
        .log_level = STND_LOG_LEVEL,
        .freq = 0,
        .format = FLASH_FORMAT_BINARY }
    },
    { "--serial=ABCEFF544851717867216044 erase", 0,
      { .cmd = FLASH_CMD_ERASE,
        .serial = "ABCEFF544851717867216044",
        .filename = NULL,
        .addr = 0,
        .size = 0,
        .reset = 0,
        .log_level = STND_LOG_LEVEL,
        .freq = 0,
        .format = FLASH_FORMAT_BINARY }
    },
};

int32_t main() {
    bool allOk = true;

    for(uint32_t i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i)
        if(!execute_test(&tests[i]))
            allOk = false;

    return (allOk ? 0 : 1);
}
