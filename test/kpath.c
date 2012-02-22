#include <stdio.h>
#include "test.h"
#include "kpath.h"
#include "kstr.h"
#include "kutils.h"

static void test_split_check(char *path_s, char *dir_s, char *name_s, char *ext_s, int format) {
    kstr path, dir, name, ext;
    kstr_init(&path);
    kstr_init(&dir);
    kstr_init(&name);
    kstr_init(&ext);
   
    kstr_assign_cstr(&path, path_s);
    kpath_split(&path, &dir, &name, &ext, format);
    assert(kstr_equal_cstr(&dir, dir_s));
    assert(kstr_equal_cstr(&name, name_s));
    assert(kstr_equal_cstr(&ext, ext_s));
    
    kstr_clean(&path);
    kstr_clean(&dir);
    kstr_clean(&name);
    kstr_clean(&ext);
}

static void test_split() {
    test_split_check("/etc/ld.so.conf", "/etc/", "ld", "so.conf", KPATH_FORMAT_UNIX);
    test_split_check("/etc/", "/etc/", "", "", KPATH_FORMAT_UNIX);
    test_split_check("/etc", "/", "etc", "", KPATH_FORMAT_UNIX);
    test_split_check("foo.", "./", "foo.", "", KPATH_FORMAT_UNIX);
    test_split_check(".foo", "./", ".foo", "", KPATH_FORMAT_UNIX);
    test_split_check(".", "./", "", "", KPATH_FORMAT_UNIX);
    test_split_check("..", "../", "", "", KPATH_FORMAT_UNIX);
    test_split_check("./.", "././", "", "", KPATH_FORMAT_UNIX);
    test_split_check("./..", "./../", "", "", KPATH_FORMAT_UNIX);
    test_split_check("./.a", "./", ".a", "", KPATH_FORMAT_UNIX);
    test_split_check("", "", "", "", KPATH_FORMAT_UNIX);
    test_split_check("C:\\bar\\test.txt", "C:\\bar\\", "test", "txt", KPATH_FORMAT_WINDOWS);
    test_split_check("C:/bar/test.txt", "C:/bar/", "test", "txt", KPATH_FORMAT_WINDOWS_ALT);
}

static void test_basename_check(char *path_s, char *name_s, int format) {
    kstr path, base;
    kstr_init(&path);
    kstr_init(&base);
    
    kstr_assign_cstr(&path, path_s);
    kpath_basename(&path, &base, format);
    assert(kstr_equal_cstr(&base, name_s));
    
    kstr_clean(&path);
    kstr_clean(&base);
}

static void test_basename() {
    test_basename_check("foo/bar.txt", "bar.txt", KPATH_FORMAT_UNIX);
    test_basename_check("bar.txt", "bar.txt", KPATH_FORMAT_UNIX);
    test_basename_check(".", ".", KPATH_FORMAT_UNIX);
}

static void test_normalize_check(char *path_s, char *res_s, int format) {
    kstr path;
    kstr_init(&path);
    
    kstr_assign_cstr(&path, path_s);
    kpath_normalize(&path, 0, format);
    assert(kstr_equal_cstr(&path, res_s));
    
    kstr_clean(&path);
}

static void test_normalize() {
    test_normalize_check("foo.txt", "foo.txt", KPATH_FORMAT_UNIX);
    test_normalize_check("../foo.txt", "../foo.txt", KPATH_FORMAT_UNIX);
    test_normalize_check("./foo.txt", "foo.txt", KPATH_FORMAT_UNIX);
    test_normalize_check("..", "../", KPATH_FORMAT_UNIX);
    test_normalize_check("../..", "../../", KPATH_FORMAT_UNIX);
    test_normalize_check("../../", "../../", KPATH_FORMAT_UNIX);
    test_normalize_check("..//.././.", "../../", KPATH_FORMAT_UNIX);
    test_normalize_check("a/b/../c/d/../", "a/c/", KPATH_FORMAT_UNIX);
    test_normalize_check("a/b/../c/d/../..", "a/", KPATH_FORMAT_UNIX);
    test_normalize_check("a/b/../c/d/../../..", "", KPATH_FORMAT_UNIX);
    test_normalize_check("a/../..", "../", KPATH_FORMAT_UNIX);
    test_normalize_check("/abc/def", "/abc/def", KPATH_FORMAT_UNIX);
    test_normalize_check("/abc/def/..", "/abc/", KPATH_FORMAT_UNIX);
    test_normalize_check("///bar//.//foo/./.", "/bar/foo/", KPATH_FORMAT_UNIX);
    test_normalize_check("C:/foo\\bar/test.txt", "C:\\foo\\bar\\test.txt", KPATH_FORMAT_WINDOWS_ALT);
    test_normalize_check("", "", KPATH_FORMAT_UNIX);
}

UNIT_TEST(kpath) {
    test_split();
    test_basename();
    test_normalize();
}

