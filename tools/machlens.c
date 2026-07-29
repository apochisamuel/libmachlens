#include "machlens.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage(const char *program) {
    fprintf(stderr,
            "usage: %s [--json|--commands|--security|--layout] FILE\n",
            program);
}

static uint8_t *read_file(const char *path, size_t *size_out) {
    FILE *stream;
    long length;
    uint8_t *bytes;
    size_t read_count;
    if (path == NULL || size_out == NULL) {
        return NULL;
    }
    stream = fopen(path, "rb");
    if (stream == NULL) {
        fprintf(stderr, "%s: %s\n", path, strerror(errno));
        return NULL;
    }
    if (fseek(stream, 0, SEEK_END) != 0 || (length = ftell(stream)) < 0 ||
        fseek(stream, 0, SEEK_SET) != 0) {
        fprintf(stderr, "%s: unable to determine file size\n", path);
        fclose(stream);
        return NULL;
    }
    if ((unsigned long)length > (unsigned long)SIZE_MAX) {
        fprintf(stderr, "%s: file is too large\n", path);
        fclose(stream);
        return NULL;
    }
    bytes = (uint8_t *)malloc((size_t)length == 0 ? 1 : (size_t)length);
    if (bytes == NULL) {
        fprintf(stderr, "%s: unable to allocate input buffer\n", path);
        fclose(stream);
        return NULL;
    }
    read_count = fread(bytes, 1, (size_t)length, stream);
    fclose(stream);
    if (read_count != (size_t)length) {
        fprintf(stderr, "%s: incomplete read\n", path);
        free(bytes);
        return NULL;
    }
    *size_out = read_count;
    return bytes;
}

static char *render_image(const ml_image *image, const char *mode) {
    if (strcmp(mode, "json") == 0) {
        return ml_image_manifest_json(image);
    }
    if (strcmp(mode, "commands") == 0) {
        return ml_image_commands_report(image);
    }
    if (strcmp(mode, "security") == 0) {
        return ml_image_security_report(image);
    }
    if (strcmp(mode, "layout") == 0) {
        return ml_image_layout_report(image);
    }
    return ml_image_report_text(image);
}

int main(int argc, char **argv) {
    const char *mode = "summary";
    const char *path;
    uint8_t *bytes;
    size_t size;
    ml_context *context;
    ml_file *file;
    size_t index;
    if (argc == 2) {
        path = argv[1];
    } else if (argc == 3 && strncmp(argv[1], "--", 2) == 0) {
        mode = argv[1] + 2;
        path = argv[2];
        if (strcmp(mode, "json") != 0 && strcmp(mode, "commands") != 0 &&
            strcmp(mode, "security") != 0 && strcmp(mode, "layout") != 0) {
            usage(argv[0]);
            return 2;
        }
    } else {
        usage(argv[0]);
        return 2;
    }
    bytes = read_file(path, &size);
    if (bytes == NULL) {
        return 1;
    }
    context = ml_context_create();
    if (context == NULL) {
        free(bytes);
        fprintf(stderr, "unable to create parser context\n");
        return 1;
    }
    file = ml_file_parse_memory(context, bytes, size);
    free(bytes);
    if (file == NULL) {
        fprintf(stderr, "%s: %s\n", path, ml_context_error(context));
        ml_context_destroy(context);
        return 1;
    }
    if (strcmp(mode, "summary") == 0) {
        char *report = ml_file_summary_report(file);
        if (report != NULL) {
            fputs(report, stdout);
            ml_string_free(report);
        }
    } else {
        for (index = 0; index < ml_file_image_count(file); ++index) {
            const ml_image *image = ml_file_image_at(file, index);
            char *report = render_image(image, mode);
            if (ml_file_image_count(file) > 1) {
                printf("--- image %zu: %s ---\n", index,
                       ml_image_arch_name(image));
            }
            if (report != NULL) {
                fputs(report, stdout);
                if (report[0] != '\0' && report[strlen(report) - 1] != '\n') {
                    putchar('\n');
                }
                ml_string_free(report);
            }
        }
    }
    ml_file_destroy(file);
    ml_context_destroy(context);
    return 0;
}
