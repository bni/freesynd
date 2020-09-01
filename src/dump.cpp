/*#include "gfx/screen.h"
#include "gfx/spritemanager.h"
#include "mapmanager.h"
#include "utils/file.h"
#include <png.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>

#ifdef _WIN32
# include <direct.h>
# define mkdir(path, mode) _mkdir(path)
# define snprintf _snprintf
#endif

class App : public Singleton<App> {
public:
    App();
};

class DummyHelper : public MapHelper {
public:
    virtual ~DummyHelper() {}
    virtual void drawAt(int tilex, int tiley, int tilez, int x, int y) {}
    virtual void createFastKeys(int tilex, int tiley,
        int maxtilex, int maxtiley) {}
};

int screen_width = 0;
int screen_height = 0;
png_byte **screen_data = NULL;
png_color palette[256];

int lowest_row = 10000;
int highest_row = -10000;
int lowest_col = 10000;
int highest_col = -10000;

int write_png(const char *filename) {

    FILE *fp = fopen(filename, "wb");

    if (!fp) {
        printf("cannot write to %s\n", filename);
        return 1;
    }

    png_structp png_ptr = png_create_write_struct(
            PNG_LIBPNG_VER_STRING, 0, 0, 0);

    if (!png_ptr) {
        printf("cannot create png write struct\n");
        fclose(fp);
        return 1;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);

    if (!info_ptr) {
        printf("cannot create png info struct\n");
        fclose(fp);
        return 1;
    }

    png_init_io(png_ptr, fp);

    png_set_IHDR(png_ptr, info_ptr, screen_width, screen_height, 8,
            PNG_COLOR_TYPE_PALETTE, PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_set_PLTE(png_ptr, info_ptr, palette, 256);

    png_write_info(png_ptr, info_ptr);

    png_write_image(png_ptr, screen_data);

    png_write_end(png_ptr, info_ptr);

    png_destroy_write_struct(&png_ptr, &info_ptr);

    fclose(fp);

    return 0;
}

void clear_screen() {
    for (int i = 0; i < screen_height; i++)
        memset(screen_data[i], 255, screen_width);
}

int dump_briefings() {
    mkdir("briefings", 0755);

    int i, lang;
    for (i = 1; i < 51; i++) {
        for (lang = 0; lang < 4; lang++) {
            char tmp[100];
            if (!lang) snprintf(tmp, 100, "miss%02d.dat", i);
            else snprintf(tmp, 100, "miss%d%02d.dat", lang, i);

            int size;
            uint8 *data = File::loadOriginalFile(tmp, size);
            if (data == NULL) continue;

            snprintf(tmp, 100, "briefings/%d-%02d.txt", lang, i);
            std::ofstream f(tmp, std::ios_base::out|std::ios_base::binary);
            f << data;
            delete[] data;
        }
    }

    return 0;
}

int dump_fonts() {
    int margin = 100;
    screen_width = margin + 100;
    screen_height = margin + 100;

    App *app = new App();
    Screen *screen = new Screen(GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);
    SpriteManager sprites;

    uint8 *data, *tabData;
    int size = 0, tabSize = 0;
    tabData = File::loadOriginalFile("mspr-0.tab", tabSize);
    data = File::loadOriginalFile("mspr-0.dat", size);
    sprites.loadSprites(tabData, tabSize, data, true);
    delete[] tabData;
    delete[] data;

    screen_data = (png_byte**) calloc(1, sizeof(png_byte *) * screen_height);

    for (int i = 0; i < screen_height; i++)
        screen_data[i] = (png_byte*) calloc(1, sizeof(png_byte) * screen_width);

    int palsize = 0;
    uint8 *pal = File::loadOriginalFile("mselect.pal", palsize);
    assert(palsize == 768);

    for (int i = 0; i < palsize / 3; ++i) {
        uint8 r = pal[i * 3 + 0];
        uint8 g = pal[i * 3 + 1];
        uint8 b = pal[i * 3 + 2];

        palette[i].red = (r << 2) | (r >> 4);
        palette[i].green = (g << 2) | (g >> 4);
        palette[i].blue = (b << 2) | (b >> 4);
    }

    mkdir("fonts", 0755);

    for (int s = 0; s < sprites.spriteCount(); s++) {
        clear_screen();
        sprites.drawSpriteXYZ(s, 0, 0, 0);
        int tw = screen_width;
        int th = screen_height;
        screen_width = sprites.sprite(s)->width();
        screen_height = sprites.sprite(s)->height();
        if (screen_width != 0 && screen_height != 0) {
          char dest[100];
          sprintf(dest, "fonts/%i.png", s);
          write_png(dest);
        }
        screen_width = tw;
        screen_height = th;
    }

    tabData = File::loadOriginalFile("mfnt-0.tab", tabSize);
    data = File::loadOriginalFile("mfnt-0.dat", size);
    sprites.clear();
    sprites.loadSprites(tabData, tabSize, data, true);
    delete[] tabData;
    delete[] data;

    mkdir("intro_font", 0755);

    for (int s = 0; s < sprites.spriteCount(); s++) {
        clear_screen();
        sprites.drawSpriteXYZ(s, 0, 0, 0);
        int tw = screen_width;
        int th = screen_height;
        screen_width = sprites.sprite(s)->width();
        screen_height = sprites.sprite(s)->height();
        if (screen_width != 0 && screen_height != 0) {
          char dest[100];
          sprintf(dest, "intro_font/%i.png", s);
          write_png(dest);
        }
        screen_width = tw;
        screen_height = th;
    }

    delete screen;
    delete app;

    return 0;
}

int dump_anims() {
    int margin = 100;
    screen_width = margin + 100;
    screen_height = margin + 100;

    App *app = new App();
    Screen *screen = new Screen(GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);
    GameSpriteManager sprites;

    screen_data = (png_byte**) calloc(1, sizeof(png_byte *) * screen_height);

    for (int i = 0; i < screen_height; i++)
        screen_data[i] = (png_byte*) calloc(1, sizeof(png_byte) * screen_width);

    int palsize;
    uint8 *pal = File::loadOriginalFile("hpal02.dat", palsize);
    assert(palsize == 768);

    for (int i = 0; i < palsize / 3; ++i) {
        uint8 r = pal[i * 3 + 0];
        uint8 g = pal[i * 3 + 1];
        uint8 b = pal[i * 3 + 2];

        palette[i].red = (r << 2) | (r >> 4);
        palette[i].green = (g << 2) | (g >> 4);
        palette[i].blue = (b << 2) | (b >> 4);
    }

    sprites.load();

    mkdir("anims", 0755);
    mkdir("sprites", 0755);

    for (int s = 0; s < sprites.spriteCount(); s++) {
        clear_screen();
        sprites.drawSpriteXYZ(s, 0, 0, 0);
        int tw = screen_width;
        int th = screen_height;
        screen_width = sprites.sprite(s)->width();
        screen_height = sprites.sprite(s)->height();
        if (screen_width != 0 && screen_height != 0) {
            char dest[100];
            sprintf(dest, "sprites/%i.png", s);
            write_png(dest);
        }
        screen_width = tw;
        screen_height = th;
    }

    for (int a = 0; a < sprites.numAnims(); a++) {
        for (int f = 0; ; f++) {
            clear_screen();
            sprites.drawFrame(a, f, margin, margin);
            char dest[100];
            sprintf(dest, "anims/%04i%04i.png", a, f);
            write_png(dest);

            if (sprites.lastFrame(a, f))
                break;
        }
    }

    printf("row: lo %i high %i\n", lowest_row, highest_row);
    printf("col: lo %i high %i\n", lowest_col, highest_col);

    delete screen;
    delete app;

    return 0;
}

int dump_maps() {
    screen_width = 8000;
    screen_height = 4000;
    App *app = new App();
    Screen *screen = new Screen(GAME_SCREEN_WIDTH, GAME_SCREEN_HEIGHT);
    DummyHelper helper;
    MapManager maps;
    maps.initialize();

    screen_data = (png_byte **) calloc(1, sizeof(png_byte *) * screen_height);

    for (int i = 0; i < screen_height; i++)
        screen_data[i] = (png_byte *)calloc(1, sizeof(png_byte) * screen_width);

    int palsize;
    uint8 *pal = File::loadOriginalFile("hpal02.dat", palsize);
    assert(palsize == 768);

    mkdir("maps", 0755);

    for (int i = 0; i < palsize / 3; ++i) {
        uint8 r = pal[i * 3 + 0];
        uint8 g = pal[i * 3 + 1];
        uint8 b = pal[i * 3 + 2];

        palette[i].red = (r << 2) | (r >> 4);
        palette[i].green = (g << 2) | (g >> 4);
        palette[i].blue = (b << 2) | (b >> 4);
    }

    for (int i = 1; i < 100; i++) {
        char tmp[100];
        sprintf(tmp, "data/map%02d.dat", i);
        FILE *f = fopen(tmp, "r");

        if (f == NULL)
            continue;

        fclose(f);

        Map *p_map = maps.loadMap(i);
        if (!p_map)
            continue;

        clear_screen();
        printf("drawing map %i\n", i);
        p_map->draw(screen_width / 2, screen_height / 2, &helper);
        char dest[100];
        sprintf(dest, "maps/map%i.png", i);
        write_png(dest);
    }

    delete screen;
    delete app;

    return 0;
}

static void usage() {
    printf("Usage: dump [-m] [-a] [-f] [-b] [-d <data dir>]\n");
    printf("Options:\n");
    printf("\t-m\tdump maps\n");
    printf("\t-a\tdump animations\n");
    printf("\t-f\tdump fonts\n");
    printf("\t-b\tdump briefings\n");
    exit(1);
}

int main(int argc, char **argv) {
    const char *dataArg = "./data/";
    int i;
    int actions = 0;

    for (i = 1; i < argc; i++) {
        if (!strcmp("-m", argv[i])) {
            actions++;
        } else if (!strcmp("-a", argv[i])) {
            actions++;
        } else if (!strcmp("-f", argv[i])) {
            actions++;
        } else if (!strcmp("-b", argv[i])) {
            actions++;
        } else if (!strcmp("-d", argv[i])) {
            i++;
            dataArg = argv[i];
        } else {
            usage();
        }
    }

    if (!dataArg || !actions) usage();

    std::string dataDir(dataArg);
    if (dataDir[dataDir.size() - 1] != '\\' && dataDir[dataDir.size() - 1] != '/') {
        dataDir.push_back('/');
    }

    File::setDataPath(dataDir);
    File::setOurDataPath(dataDir);

    for (i = 1; i < argc; i++) {
        if (!strcmp("-m", argv[i])) {
            dump_maps();
        } else if (!strcmp("-a", argv[i])) {
            dump_anims();
        } else if (!strcmp("-f", argv[i])) {
            dump_fonts();
        } else if (!strcmp("-b", argv[i])) {
            dump_briefings();
        } else if (!strcmp("-d", argv[i])) {
        }
    }

    return 0;
}

App::App() {

}

Screen::Screen(int width, int height) {

}

Screen::~Screen() {

}

int Screen::gameScreenHeight() {
    return screen_height;
}

int Screen::gameScreenWidth() {
    return screen_width;
}

int Screen::gameScreenLeftMargin() {
    return 0;
}

void Screen::blit(int x, int y, int width, int height, const uint8 *pixeldata,
        bool flipped, int stride) {
    width_ = screen_width;
    height_ = screen_height;

    if (x + width < 0 || y + height < 0 || x >= width_ || y >= height_)
        return;

    int dx = x < 0 ? 0 : x;
    int dy = y < 0 ? 0 : y;

    int sx = x < 0 ? -x : 0;
    int sy = y < 0 ? -y : 0;

    int w = x < 0 ? x + width : x + width > width_ ? width_ - x : width;
    int h = y < 0 ? y + height : y + height > height_ ? height_ - y : height;

    stride = (stride == 0 ? width : stride);
    int ofs = flipped ? w - 1 : 0;
    int inc = flipped ? -1 : 1;

    for (int j = 0; j < h; ++j) {
        if (dy + j < lowest_row)
            lowest_row = dy + j;

        if (dy + j > highest_row)
            highest_row = dy + j;

        if (dx + ofs < lowest_col)
            lowest_col = dx + ofs;

        if (dx + ofs > highest_col)
            highest_col = dx + ofs;

        uint8 *d = screen_data[dy + j] + dx + ofs;
        const uint8 *s = pixeldata + (sy + j) * stride + sx;

        for (int i = 0; i < w; ++i) {
            uint8 c = *s++;

            if (c != 255)
                *d = c;

            d += inc;
        }
    }
}

void Screen::scale2x(int x, int y, int width, int height,
        const uint8 *pixeldata, int stride, bool transp) {

}
*/

/************************************************************************
 *                                                                      *
 *  FreeSynd - a remake of the classic Bullfrog game "Syndicate".       *
 *                                                                      *
 *   Copyright (C) 2005  Stuart Binge  <skbinge@gmail.com>              *
 *   Copyright (C) 2005  Joost Peters  <joostp@users.sourceforge.net>   *
 *   Copyright (C) 2006  Trent Waddington <qg@biodome.org>              *
 *   Copyright (C) 2010  Benoit Blancard <benblan@users.sourceforge.net>*
 *                                                                      *
 *    This program is free software;  you can redistribute it and / or  *
 *  modify it  under the  terms of the  GNU General  Public License as  *
 *  published by the Free Software Foundation; either version 2 of the  *
 *  License, or (at your option) any later version.                     *
 *                                                                      *
 *    This program is  distributed in the hope that it will be useful,  *
 *  but WITHOUT  ANY WARRANTY;  without even  the implied  warranty of  *
 *  MERCHANTABILITY  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  *
 *  General Public License for more details.                            *
 *                                                                      *
 *    You can view the GNU  General Public License, online, at the GNU  *
 *  project's  web  site;  see <http://www.gnu.org/licenses/gpl.html>.  *
 *  The full text of the license is also included in the file COPYING.  *
 *                                                                      *
 ************************************************************************/

#include <memory>

#include <cstdio>
#include <cstdlib>

#include "common.h"
#include "editor/editorapp.h"
#include "utils/file.h"
#include "utils/log.h"
#include "default_ini.h"

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#ifdef SYSTEM_SDL
#ifdef _WIN32
#undef main
#else
#include <SDL_main.h>           //This is required on OSX for remapping of main()
#endif
#endif

void print_usage() {
    printf("usage: freesynd [options...]\n");
    printf("    -h, --help            display this help and exit.\n");
    printf("    -i, --ini <path>      specify the location of the FreeSynd config file.\n");
    printf("    --nosound             disable all sound.\n");

#ifdef _WIN32
    printf(" (default: freesynd.ini in the same folder as freesynd.exe)\n");
#elif defined(__APPLE__)
    printf(" (default: $HOME/Library/Application Support/FreeSynd/freesynd.ini)\n");
#else
    printf(" (default: $HOME/.freesynd/freesynd.ini)\n");
#endif
}

int main(int argc, char *argv[]) {

#ifdef _WIN32
    srand(GetTickCount());
#else
    srand((unsigned) time(NULL));
#endif

    // This variable stores the path to the Freesynd configuration file.
    std::string iniPath;

    bool disable_sound = true;

    for (int i = 1; i < argc; ++i) {

        if (0 == strcmp("-h", argv[i]) || 0 == strcmp("--help", argv[i])) {
            print_usage();
            return 1;
        }

        if (0 == strcmp("-i", argv[i]) || 0 == strcmp("--ini", argv[i])) {
            i++;
            iniPath = argv[i];
        }
    }

#ifdef _DEBUG
    // Initialize log
    Log::initialize(Log::k_FLG_ALL, "editor.log");
#endif

    if (iniPath.size() == 0) {
        iniPath.assign(EditorApp::defaultIniFolder());
        iniPath.append("/freesynd.ini");
#ifdef _WIN32
        if (_access(iniPath.c_str(), 0) != 0)
#else
        struct stat st;
        if (stat(iniPath.c_str(), &st))
#endif
        {
            FILE *f = fopen(iniPath.c_str(), "w");
            if (!f) {
                FSERR(Log::k_FLG_IO, "Freesynd", "main", ("Cannot create default ini file at %s", iniPath.c_str()))
                return -1;
            }
            fwrite(embedded_default_ini_data, 1, embedded_default_ini_size, f);
            fclose(f);
        }
    }

    LOG(Log::k_FLG_INFO, "Main", "main", ("----- Initializing application..."))
    std::auto_ptr<EditorApp> app(new EditorApp(disable_sound));

    if (app->initialize(iniPath)) {
        LOG(Log::k_FLG_INFO, "Main", "main", ("----- Initializing application completed"))
        LOG(Log::k_FLG_INFO, "Main", "main", ("----- Starting game loop"))
        app->run();
    } else {
        LOG(Log::k_FLG_INFO, "Main", "main", ("----- Initializing application failed"))
    }

    // Destroy application
    LOG(Log::k_FLG_INFO, "Main", "main", ("----- End of game loop. Destroy application"))
    app->destroy();

#ifdef _DEBUG
    // Close log
    Log::close();
#endif

    return 0;
}
