#include <string>
#include <fstream>

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>

#ifdef WIN32
# define mkdir(path, mode) _mkdir(path)
#else
# include <sys/stat.h>
#endif

int traverse(const char *src, const char *dst, const char *but_not)
{
    DIR *d = opendir(src);
    if (!d) {
        fprintf(stderr, "Unable to open source directory `%s'.\n", src);
        return 1;
    }

    int ret = 0;
    struct dirent *ent;
    for (ent = readdir(d); ent; ent = readdir(d)) {
        if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..") || !strcmp(ent->d_name, but_not)) {
            continue;
        }

        std::string path(src);
        path.push_back('/');
        path.append(ent->d_name);

        struct stat st;
        if (stat(path.c_str(), &st)) {
            fprintf(stderr, "Unable to stat `%s`.\n", path.c_str());
            ret = 1;
            continue;
        }

        std::string dstpath(dst);
        dstpath.push_back('/');
        dstpath.append(ent->d_name);

        if (S_ISDIR(st.st_mode)) {
            mkdir(dstpath.c_str(), 0755);
            ret |= traverse(path.c_str(), dstpath.c_str(), but_not);
        } else {
            std::ifstream inf(path.c_str(), std::ios_base::in|std::ios_base::binary);
            std::ofstream outf(dstpath.c_str(), std::ios_base::out|std::ios_base::binary);
            while (inf.good()) {
                char buf[8192];
                size_t n = inf.readsome(buf, 8192);
                if (n > 0) outf.write(buf, n);
                if (n == 0) break;
            }
        }
    }

    return ret;
}

int main(int argc, char **argv)
{
    if (argc != 4) {
        fprintf(stderr, "Usage: copy_but <source> <destination> <but>\n");
        return 1;
    }

    char *src = argv[1];
    char *dst = argv[2];
    char *but_not = argv[3];

    mkdir(dst, 0755);

    return traverse(src, dst, but_not);
}

