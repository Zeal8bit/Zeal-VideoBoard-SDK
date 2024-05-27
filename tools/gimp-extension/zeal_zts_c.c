/**
 * SPDX-FileCopyrightText: 2024 Zeal 8-bit Computer <contact@zeal8bit.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

/**
 * @brief `O_BINARY` is only defined on Windows, define it for Linux too
 */
#ifndef O_BINARY
#define O_BINARY 0
#endif

#define ZEAL_TILE_HEIGHT    16
#define ZEAL_TILE_WIDTH     16
/* Maximum length of a sequence in RLE */
#define MAX_RLE_SEQ 128

static void run (
   const gchar      *name,
   gint              nparams,
   const GimpParam  *param,
   gint             *nreturn_vals,
   GimpParam       **return_vals);
static void query (void);


GimpPlugInInfo PLUG_IN_INFO = {
    NULL,
    NULL,
    query,
    run
};


static gboolean compress_dialog(void)
{
    GtkWidget *dialog;
    GtkWidget *label;
    GtkWidget *checkbox;

    gimp_ui_init("ZealTileset Exporter", FALSE);

    // Create a new dialog
    dialog = gimp_dialog_new("Zeal Exporter", "zeal_exporter",
                             NULL, GTK_DIALOG_MODAL, NULL, NULL,
                            GTK_STOCK_OK,     GTK_RESPONSE_OK,
                            NULL);

    // Create and add a checkbox to the dialog
    checkbox = gtk_check_button_new_with_label("Compress tileset colors to 2-bit or 4-bit mode");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbox), TRUE); // Set initial state
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), checkbox, FALSE, FALSE, 0);

    // Show all the widgets in the dialog
    gtk_widget_show_all(dialog);

    // Run the dialog
    gint response = gimp_dialog_run(GIMP_DIALOG(dialog));

    // Handle the response if needed
    assert(response == GTK_RESPONSE_OK);

    gboolean checked = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbox));

    // Destroy the dialog
    gtk_widget_destroy(dialog);

    return checked;
}

/* ----------------------------- PALETTE RELATED ---------------------------- */

uint16_t convert_to_rgb565(uint8_t r, uint8_t g, uint8_t b) {
    uint16_t red   = (r >> 3) & 0x1F;   // 5 bits for red
    uint16_t green = (g >> 2) & 0x3F; // 6 bits for green
    uint16_t blue  = (b >> 3) & 0x1F;  // 5 bits for blue

    return (red << 11) | (green << 5) | blue;
}

static int palette_save(const char* filename, const gint32 image)
{
    /* Get the current palette of the image */
    gint num_colors = 0;
    guchar* rgb888_colors  = gimp_image_get_colormap(image, &num_colors);

    if (num_colors == 0) {
        g_message("No color palette detected, only indexed mode is supported!\n");
        return 0;
    } else if (num_colors > 256) {
        g_message("Too many colors in the palette, the maximum is 256!\n");
        return 0;
    }

    int fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT | O_BINARY, 0644);
    if (fd < 0) {
        g_message("Could not create file\n");
        return 0;
    }

    /* We will have at most 256 RGB565 colors */
    uint8_t palette[256*2];
    int palette_i = 0;

    for (int i = 0; i < num_colors * 3; i += 3) {
        const guchar r = rgb888_colors[i];
        const guchar g = rgb888_colors[i + 1];
        const guchar b = rgb888_colors[i + 2];

        uint16_t color = convert_to_rgb565(r, g, b);

        /* Little-endian output */
        palette[palette_i++] = color & 0xff;
        palette[palette_i++] = (color >> 8) & 0xff;
    }

    /* Copy the palette to the file */
    int wr = write(fd, palette, palette_i);
    if (wr < 0) {
        g_message("Could not write the palette file\n");
        close(fd);
        return 0;
    }

    close(fd);
    return num_colors;
}

/* ----------------------------- TILESET RELATED ---------------------------- */

#define TILE_SIZE   256
/* The worst case of a compressed tile is to have 2 more bytes than original */
typedef uint8_t zeal_tile_t[TILE_SIZE + 2];


typedef struct zeal_tileset_node_t {
    zeal_tile_t tile;
    size_t size; /* Size in bytes, only makes sense in compressed mode */
    struct zeal_tileset_node_t* next;
} zeal_tileset_node_t;

typedef struct {
    zeal_tileset_node_t* head;
    int length;
} zeal_tileset_t;

static int tiles_equal(zeal_tile_t tile1, zeal_tile_t tile2)
{
    return memcmp(tile1, tile2, TILE_SIZE) == 0;
}

static inline int tile_count_same_seq(uint8_t* bytes, size_t len)
{
    int count = 0;
    int i = 0;

    while(i < len && (bytes[0] == bytes[i])) {
        count++;
        i++;
    }

    return count;
}

static inline int tile_count_diff_seq(uint8_t* bytes, size_t len)
{
    int count = 0;
    int i = 1;

    while(i < len && (bytes[i-1] != bytes[i])) {
        count++;
        i++;
    }

    return count;
}

static int tile_copy_seq(uint8_t* dst, uint8_t* src, int seq_len, uint8_t seq_marker) {
    int written = 0;

    if (seq_len > MAX_RLE_SEQ) {
        /* If the sequence is of size n, save it as n-1, so that 128 is represented by 0x7f */
        dst[0] = seq_marker | (MAX_RLE_SEQ - 1);
        dst++;
        written++;

        /* If the marker is not 0, then all the bytes are the same */
        const int cpylen = seq_marker ? 1 : MAX_RLE_SEQ;
        memcpy(dst, src, cpylen);
        dst     += cpylen;
        src     += MAX_RLE_SEQ;
        seq_len -= MAX_RLE_SEQ;
        written += cpylen;
    }

    if (seq_len) {
        const int cpylen = seq_marker ? 1 : seq_len;

        dst[0] = seq_marker | (seq_len - 1);
        memcpy(dst + 1, src, cpylen);
        written += 1 + cpylen;
    }

    return written;
}


/*
 * @brief Compress the given tile with RLE and store it in `output` buffer
 *
 * @return size of the new tile
 */
static size_t tile_compress(uint8_t* tile, int colors_number, gboolean compress_rle, gboolean compress_colors) {
    int i = 0;
    int previous_diff_seq = 0;
    uint8_t tmp[TILE_SIZE + 2];
    int tmp_it = 0;
    int tile_size = TILE_SIZE;

    /* If we have only two colors, change the colors into bitmaps */
    if (compress_colors) {
        if (colors_number == 2) {
            for (i = 0; i < TILE_SIZE; i += 8) {
                uint_fast8_t color = 0;
                /* Pixel 0 will be in bit 7, pixel 1 in bit 6, etc... */
                for (int j = 0; j < 8; j++)
                    color |= (tile[i+j] & 1) << (7 - j);
                tile[i/8] = color;
            }
            tile_size = i/8;
        }
        /* If we have less than 17 colors, we can discard the upper nibble */
        else if (colors_number <= 16) {
            for (i = 0; i < TILE_SIZE; i += 2) {
                /* The high nibble becomes the current color, and the lowest nibble is the next color */
                tile[i/2] = (tile[i] << 4) | (tile[i + 1] & 0xf);
            }
            tile_size = i/2;
        }
    }

    /* If there is no RLE to perform, we can return directly */
    if (!compress_rle) {
        return tile_size;
    }

    i = 0;
    while (i < tile_size) {
        /* Count identical */
        int same_count = tile_count_same_seq(tile + i, tile_size - i);
        int diff_count = tile_count_diff_seq(tile + i, tile_size - i);

        if (diff_count) {
            i += diff_count;
            previous_diff_seq += diff_count;
        } else if (same_count < 4) {
            i += same_count;
            previous_diff_seq += same_count;
        } else {
            /* Flush the previous "diff_seq" to file/array */
            int wr = tile_copy_seq(tmp + tmp_it, tile + i - previous_diff_seq, previous_diff_seq, 0);
            tmp_it += wr;

            previous_diff_seq = 0;
            wr = tile_copy_seq(tmp + tmp_it, tile + i, same_count, 0x80);
            tmp_it += wr;
            i += same_count;
        }
    }

    tmp_it += tile_copy_seq(tmp + tmp_it, tile + i - previous_diff_seq, previous_diff_seq, 0);

    /* Copy the final result in the original `tile` array */
    memcpy(tile, tmp, tmp_it);

    return tmp_it;
}


static void tile_get(gint32 drawable, int x, int y, zeal_tile_t tile)
{
    guint8* pixel = 0;
    gint channels = 0;

    for (int j = 0; j < ZEAL_TILE_HEIGHT; j++) {
        for (int i = 0; i < ZEAL_TILE_WIDTH; i++) {
            pixel = gimp_drawable_get_pixel(drawable, x + i, y + j, &channels);

            /* Make the assumption that we have a single channel */
            tile[j * ZEAL_TILE_WIDTH + i] = pixel[0];
        }
    }
}


static void tileset_init(zeal_tileset_t* set)
{
    if (set) {
        set->head = NULL;
        set->length = 0;
    }
}

static void tileset_deinit(zeal_tileset_t* set)
{
    if (set == NULL) {
        return;
    }

    zeal_tileset_node_t* head = set->head;
    zeal_tileset_node_t* node = NULL;

    while (head) {
        /* Store the current node to free but assign the head */
        node = head;
        head = head->next;
        g_free(node);
    }

    set->head = 0;
    set->length = 0;
}

/**
 * @brief Add a given tile to the tileset
 *
 * @returns -1 on error, else index of the tile in the set
 */
static int tileset_add(zeal_tileset_t* set, zeal_tile_t tile)
{
    if (set == NULL) {
        return -1;
    }

    /* Check if the tile exists already in the set */
    zeal_tileset_node_t* head = set->head;
    /* Previous will be used to insert the new tile in the set */
    zeal_tileset_node_t* previous = NULL;

    /* Browse the list */
    int i = 0;
    while(head != NULL) {
        if (tiles_equal(head->tile, tile)) {
            /* We found the tile, return the current index */
            return i;
        }
        /* Go to the next node */
        previous = head;
        head = head->next;
        i++;
    }

    /* We couldn't find the tile in the set, add it! */
    zeal_tileset_node_t* entry = g_malloc(sizeof(zeal_tileset_node_t));
    if (entry == NULL) {
        return -1;
    }

    entry->size = TILE_SIZE;
    memcpy(entry->tile, tile, TILE_SIZE);
    entry->next = NULL;
    /* Add it to the tail of the list */
    if (previous == NULL) {
        /* Happens when the list is empty */
        set->head = entry;
    } else {
        previous->next = entry;
    }
    set->length++;

    return i;
}

/**
 * @brief Compress each tile in the tile (independently) by applying an RLE algorithm:
 *        Each byte is as follows:
 *          - 00-7F Copy n + 1 bytes from input to output.
 *          - 80-FF Read one byte from input and write it to output n - 125 times.
 */
static void tileset_compress(zeal_tileset_t* set, int colors_number, gboolean compress_rle, gboolean compress_colors)
{
    zeal_tileset_node_t* head;

    if (set == NULL) {
        return;
    }

    for (head = set->head; head; head = head->next) {
        head->size = tile_compress(head->tile, colors_number, compress_rle, compress_colors);
    }
}


static int tileset_save(const char* filename, zeal_tileset_t* set)
{
    if (set == NULL || set->length == 0) {
        return 0;
    }

    int fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT | O_BINARY, 0644);
    if (fd < 0) {
        g_message("Could not create tileset file\n");
        return 0;
    }

    zeal_tileset_node_t* head = set->head;
    int wr = 0;

    while (head) {
        /* TODO: Optimize by writing a big buffer (~1KB) at each iteration */
        wr = write(fd, head->tile, head->size);
        if (wr < 0) {
            g_message("Could not write to the tileset file\n");
            close(fd);
            return 0;
        }

        head = head->next;
    }

    close(fd);
    return 1;
}


/**
 * @brief Get the size of the tileset (number of tiles)
 */
static inline int tileset_size(zeal_tileset_t* set)
{
    return set->length;
}


static int tilemap_save(const char* filename, uint8_t* map, size_t size)
{
    if (map == NULL || size == 0) {
        return 0;
    }

    int fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT | O_BINARY, 0644);
    if (fd < 0) {
        g_message("Could not create tilemap file\n");
        return 0;
    }

    int wr = write(fd, map, size);
    if (wr < 0) {
        g_message("Could not write to the tilemap file\n");
        close(fd);
        return 0;
    }

    close(fd);
    return 1;
}


/* Function to display the image dimensions */
static void run (
    const gchar      *name,
    gint              nparams,
    const GimpParam  *param,
    gint             *nreturn_vals,
    GimpParam       **return_vals)
{
    static char buf_filename[PATH_MAX];
    static GimpParam  values[1];
    GimpPDBStatusType status = GIMP_PDB_SUCCESS;
    // GimpRunMode       run_mode;

    /* Setting mandatory output values */
    *nreturn_vals = 1;
    *return_vals  = values;

    values[0].type = GIMP_PDB_STATUS;
    values[0].data.d_status = status;

    /* We want to flatten the image, so that all the VISIBLE layers are merged into a single one,
     * but we don't want to modify the existing image, the one the user is working one, so we have to
     * clone it */
    const gint32 original_image = param[1].data.d_image;
    gint32 image = gimp_image_duplicate(original_image);
    gint32 drawable = gimp_image_flatten (image);

    const gchar* filename = param[3].data.d_string;
    const gchar* rawfile = param[4].data.d_string;
    const gint width = gimp_drawable_width(drawable);
    const gint height = gimp_drawable_height(drawable);

    if ((width % ZEAL_TILE_WIDTH != 0) || (height % ZEAL_TILE_HEIGHT != 0)) {
        g_message("The image width and height must be multiples of 16!\n");
        return;
    }

    /* We need to generate filenames for:
     * - The palette, .z(c)tp file
     * - The tilemap, .z(c)tm file
     * - The tileset, .z(c)ts file
     */
    strcpy(buf_filename, filename);
    const int filename_len = strlen(buf_filename);
    /* Check if the files needs to be compressed */
    const gboolean compress_rle = filename[filename_len - 3] == 'c';
    gboolean       compress_colors = FALSE;

    /* Create the palette file, only the last character need to change */
    buf_filename[filename_len - 1] = 'p';
    int colors_number = palette_save(buf_filename, image);
    if (colors_number == 0) {
        goto return_dealloc_image;
    }

    /* If we have 16 or less colors, ask the user if we have to compress (even more) */
    if (colors_number <= 16) {
        compress_colors = compress_dialog();
    }

    /* Create the tileset and initialize it */
    zeal_tileset_t set;
    tileset_init(&set);
    zeal_tile_t tile;

    /* Create a tilemap for the current image */
    const int max_tiles = (width/ZEAL_TILE_WIDTH) * (height/ZEAL_TILE_HEIGHT);
    uint8_t* tilemap = g_malloc(max_tiles);
    int tilemap_size = 0;

    for (int y = 0; y < height; y += ZEAL_TILE_HEIGHT) {
        for (int x = 0; x < width; x += ZEAL_TILE_WIDTH) {
            tile_get(drawable, x, y, tile);
            /* Add the tile to the tileset */
            int ret = tileset_add(&set, tile);
            if (ret < 0) {
                g_message("Memory allocation failed!\n");
                goto return_dealloc;
            }
            /* ret contains the index of the tile in the tileset */
            tilemap[tilemap_size++] = ret;
        }
    }

    /* Compress the tileset if necessary */
    if (compress_rle || compress_colors) {
        tileset_compress(&set, colors_number, compress_rle, compress_colors);
    }

    /* Save the tilemap to a file */
    buf_filename[filename_len - 1] = 'm';
    if (!tilemap_save(buf_filename, tilemap, tilemap_size)) {
        goto return_dealloc;
    }

    /* Save the tileset to a file */
    buf_filename[filename_len - 1] = 's';
    if (!tileset_save(buf_filename, &set)) {
        goto return_dealloc;
    }

    /* Print the total amount of colors and tiles in the set */
    g_message("Export successful!\nNumber of colors in the palette: %d\nNumber of tiles in the tileset:%d\n",
              colors_number, tileset_size(&set));

return_dealloc:
    /* Free the allocated resources */
    g_free(tilemap);
    tileset_deinit(&set);
return_dealloc_image:
    /* Delete the duplicated image */
    gimp_image_delete(image);
}

/* Entry point for the plugin */
MAIN ()

static void
query (void)
{
    static GimpParamDef args[] = {
        {GIMP_PDB_INT32, "run_mode", "Interactive, non-interactive"},
        {GIMP_PDB_IMAGE, "image", "Input image"},
        {GIMP_PDB_DRAWABLE, "drawable", "Drawable to save"},
        {GIMP_PDB_STRING, "filename", "The name of the file to save the image as"},
        {GIMP_PDB_STRING, "raw_filename", "The name entered"},
    };

    gimp_install_procedure (
        "zeal-zts-format",
        "Saves files in ZTS image format",
        "Zeal 8-bit computer compatible tileset and palette format",
        "Zeal 8-bit",
        "Copyright Zeal 8-bit",
        "2024",
        "Zeal Tileset",
        "INDEXED*",
        GIMP_PLUGIN,
        G_N_ELEMENTS (args), 0,
        args, NULL);

    /**
     * It is not possible to register the extension as part of the menu if it is a file/save handler
     */
    gimp_register_file_handler_mime("zeal-zts-format", "image/zts");
    gimp_register_save_handler("zeal-zts-format",
                               "zts,ztm,ztp",
                               "");

    /* Add a compressed option */
    gimp_install_procedure (
        "zeal-zcts-format",
        "Saves files in ZCTS image format",
        "Zeal 8-bit computer compatible compressed tileset and palette format",
        "Zeal 8-bit",
        "Copyright Zeal 8-bit",
        "2023",
        "Zeal Compressed Tileset",
        "INDEXED*",
        GIMP_PLUGIN,
        G_N_ELEMENTS (args), 0,
        args, NULL);

    gimp_register_file_handler_mime("zeal-zcts-format", "image/zcts");
    gimp_register_save_handler("zeal-zcts-format",
                               "zcts,zctm,zctp",
                               "");
}
