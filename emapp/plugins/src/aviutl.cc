/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#define NOMINMAX
#include "emapp/sdk/Decoder.h"
#include "emapp/sdk/Encoder.h"
#include "emapp/sdk/UI.h"

#include "emapp/src/protoc/common.pb-c.c"
#include "emapp/src/protoc/plugin.pb-c.c"

#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

using namespace nanoem::application::plugin;

namespace {

typedef struct {
    short y;
    short cb;
    short cr;
} PIXEL_YC;

typedef struct {
    unsigned char b, g, r;
} PIXEL;

typedef struct {
    int flag;
    PIXEL_YC *ycp_edit;
    PIXEL_YC *ycp_temp;
    int w, h;
    int max_w, max_h;
    int frame;
    int frame_n;
    int org_w, org_h;
    short *audiop;

    int audio_n;
    int audio_ch;
    PIXEL *pixelp;
    void *editp;
    int yc_size;
    int line_size;
    int reserve[8];
} FILTER_PROC_INFO;

#define FILTER_PROC_INFO_FLAG_INVERT_FIELD_ORDER 0x00010000
#define FILTER_PROC_INFO_FLAG_INVERT_INTERLACE 0x00020000

typedef struct {
    int video;
    int audio;
    int inter;
    int index24fps;
    int config;
    int vcm;
    int edit_flag;
    int reserve[9];
} FRAME_STATUS;

#define FRAME_STATUS_INTER_NORMAL 0
#define FRAME_STATUS_INTER_REVERSE 1
#define FRAME_STATUS_INTER_ODD 2
#define FRAME_STATUS_INTER_EVEN 3
#define FRAME_STATUS_INTER_MIX 4
#define FRAME_STATUS_INTER_AUTO 5
#define EDIT_FRAME_EDIT_FLAG_KEYFRAME 1
#define EDIT_FRAME_EDIT_FLAG_MARKFRAME 2
#define EDIT_FRAME_EDIT_FLAG_DELFRAME 4
#define EDIT_FRAME_EDIT_FLAG_NULLFRAME 8

typedef struct {
    int flag;
    LPSTR name;
    int w, h;
    int video_rate, video_scale;
    int audio_rate;
    int audio_ch;
    int frame_n;
    DWORD video_decode_format;
    int video_decode_bit;
    int audio_n;
    int reserve[4];
} FILE_INFO;

#define FILE_INFO_FLAG_VIDEO 1
#define FILE_INFO_FLAG_AUDIO 2

typedef struct {
    int flag;
    LPSTR info;
    int filter_n;
    int min_w, min_h;
    int max_w, max_h;
    int max_frame;
    LPSTR edit_name;
    LPSTR project_name;
    LPSTR output_name;
    int vram_w, vram_h;
    int vram_yc_size;
    int vram_line_size;
    HFONT hfont;
    int build;
    int reserve[2];
} SYS_INFO;

#define SYS_INFO_FLAG_EDIT 1
#define SYS_INFO_FLAG_VFAPI 2
#define SYS_INFO_FLAG_USE_SSE 4
#define SYS_INFO_FLAG_USE_SSE2 8

typedef void (*MULTI_THREAD_FUNC)(int thread_id, int thread_num, void *param1, void *param2);
typedef void *AVI_FILE_HANDLE;

typedef struct {
    void (*get_ycp_ofs)(void *editp, int n, int ofs);
    void *(*get_ycp)(void *editp, int n);
    void *(*get_pixelp)(void *editp, int n);
    int (*get_audio)(void *editp, int n, void *buf);
    BOOL (*is_editing)(void *editp);
    BOOL (*is_saving)(void *editp);
    int (*get_frame)(void *editp);
    int (*get_frame_n)(void *editp);
    BOOL (*get_frame_size)(void *editp, int *w, int *h);
    int (*set_frame)(void *editp, int n);
    int (*set_frame_n)(void *editp, int n);
    BOOL (*copy_frame)(void *editp, int d, int s);
    BOOL (*copy_video)(void *editp, int d, int s);
    BOOL (*copy_audio)(void *editp, int d, int s);
    BOOL (*copy_clip)(HWND hwnd, void *pixelp, int w, int h);
    BOOL (*paste_clip)(HWND hwnd, void *editp, int n);
    BOOL (*get_frame_status)(void *editp, int n, FRAME_STATUS *fsp);
    BOOL (*set_frame_status)(void *editp, int n, FRAME_STATUS *fsp);
    BOOL (*is_saveframe)(void *editp, int n);
    BOOL (*is_keyframe)(void *editp, int n);
    BOOL (*is_recompress)(void *editp, int n);
    BOOL (*filter_window_update)(void *fp);
    BOOL (*is_filter_window_disp)(void *fp);
    BOOL (*get_file_info)(void *editp, FILE_INFO *fip);
    LPSTR (*get_config_name)(void *editp, int n);
    BOOL (*is_filter_active)(void *fp);
    BOOL (*get_pixel_filtered)(void *editp, int n, void *pixelp, int *w, int *h);
    int (*get_audio_filtered)(void *editp, int n, void *buf);
    BOOL (*get_select_frame)(void *editp, int *s, int *e);
    BOOL (*set_select_frame)(void *editp, int s, int e);
    BOOL (*rgb2yc)(PIXEL_YC *ycp, PIXEL *pixelp, int w);
    BOOL (*yc2rgb)(PIXEL *pixelp, PIXEL_YC *ycp, int w);
    BOOL (*dlg_get_load_name)(LPSTR name, LPSTR filter, LPSTR def);
    BOOL (*dlg_get_save_name)(LPSTR name, LPSTR filter, LPSTR def);
    int (*ini_load_int)(void *fp, LPSTR key, int n);
    int (*ini_save_int)(void *fp, LPSTR key, int n);
    BOOL (*ini_load_str)(void *fp, LPSTR key, LPSTR str, LPSTR def);
    BOOL (*ini_save_str)(void *fp, LPSTR key, LPSTR str);
    BOOL (*get_source_file_info)(void *editp, FILE_INFO *fip, int source_file_id);
    BOOL (*get_source_video_number)(void *editp, int n, int *source_file_id, int *source_video_number);
    BOOL (*get_sys_info)(void *editp, SYS_INFO *sip);
    void *(*get_filterp)(int filter_id);
    void *(*get_ycp_filtering)(void *fp, void *editp, int n, void *reserve);
    int (*get_audio_filtering)(void *fp, void *editp, int n, void *buf);
    BOOL (*set_ycp_filtering_cache_size)(void *fp, int w, int h, int d, int flag);
    void *(*get_ycp_filtering_cache)(void *fp, void *editp, int n);
    void *(*get_ycp_source_cache)(void *editp, int n, int ofs);
    void *(*get_disp_pixelp)(void *editp, DWORD format);
    BOOL (*get_pixel_source)(void *editp, int n, void *pixelp, DWORD format);
    BOOL (*get_pixel_filtered_ex)(void *editp, int n, void *pixelp, int *w, int *h, DWORD format);
    PIXEL_YC *(*get_ycp_filtering_cache_ex)(void *fp, void *editp, int n, int *w, int *h);
    BOOL (*exec_multi_thread_func)(MULTI_THREAD_FUNC func, void *param1, void *param2);
    PIXEL_YC *(*create_yc)(void);
    void (*delete_yc)(PIXEL_YC *ycp);
    BOOL (*load_image)(PIXEL_YC *ycp, LPSTR file, int *w, int *h, int flag);
    void (*resize_yc)(PIXEL_YC *ycp, int w, int h, PIXEL_YC *ycp_src, int sx, int sy, int sw, int sh);
    void (*copy_yc)(PIXEL_YC *ycp, int x, int y, PIXEL_YC *ycp_src, int sx, int sy, int sw, int sh, int tr);
    void (*draw_text)(
        PIXEL_YC *ycp, int x, int y, LPSTR text, int r, int g, int b, int tr, HFONT hfont, int *w, int *h);
    AVI_FILE_HANDLE (*avi_file_open)(LPSTR file, FILE_INFO *fip, int flag);
    void (*avi_file_close)(AVI_FILE_HANDLE afh);
    BOOL (*avi_file_read_video)(AVI_FILE_HANDLE afh, PIXEL_YC *ycp, int n);
    int (*avi_file_read_audio)(AVI_FILE_HANDLE afh, void *buf, int n);
    void *(*avi_file_get_video_pixelp)(AVI_FILE_HANDLE afh, int n);
    LPSTR (*get_avi_file_filter)(int type);
    int (*avi_file_read_audio_sample)(AVI_FILE_HANDLE afh, int start, int length, void *buf);
    int (*avi_file_set_audio_sample_rate)(AVI_FILE_HANDLE afh, int audio_rate, int audio_ch);
    BYTE *(*get_frame_status_table)(void *editp, int type);
    BOOL (*set_undo)(void *editp);
    BOOL (*add_menu_item)(void *fp, LPSTR name, HWND hwnd, int id, int def_key, int flag);
    BOOL (*edit_open)(void *editp, LPSTR file, int flag);
    BOOL (*edit_close)(void *editp);
    BOOL (*edit_output)(void *editp, LPSTR file, int flag, LPSTR type);
    BOOL (*set_config)(void *editp, int n, LPSTR name);
    int reserve[7];
} EXFUNC;

#define AVI_FILE_OPEN_FLAG_VIDEO_ONLY 16
#define AVI_FILE_OPEN_FLAG_AUDIO_ONLY 32
#define AVI_FILE_OPEN_FLAG_ONLY_YUY2 0x10000
#define AVI_FILE_OPEN_FLAG_ONLY_RGB24 0x20000
#define AVI_FILE_OPEN_FLAG_ONLY_RGB32 0x40000
#define GET_AVI_FILE_FILTER_TYPE_VIDEO 0
#define GET_AVI_FILE_FILTER_TYPE_AUDIO 1
#define FARME_STATUS_TYPE_EDIT_FLAG 0
#define FARME_STATUS_TYPE_INTER 1
#define ADD_MENU_ITEM_FLAG_KEY_SHIFT 1
#define ADD_MENU_ITEM_FLAG_KEY_CTRL 2
#define ADD_MENU_ITEM_FLAG_KEY_ALT 4
#define EDIT_OPEN_FLAG_ADD 2
#define EDIT_OPEN_FLAG_AUDIO 16
#define EDIT_OPEN_FLAG_PROJECT 512
#define EDIT_OPEN_FLAG_DIALOG 65536
#define EDIT_OUTPUT_FLAG_NO_DIALOG 2
#define EDIT_OUTPUT_FLAG_WAV 4

typedef struct {
    int flag;
    int x, y;
    TCHAR *name;
    int track_n;
    TCHAR **track_name;
    int *track_default;
    int *track_s, *track_e;
    int check_n;
    TCHAR **check_name;
    int *check_default;
    BOOL (*func_proc)(void *fp, FILTER_PROC_INFO *fpip);
    BOOL (*func_init)(void *fp);
    BOOL (*func_exit)(void *fp);
    BOOL (*func_update)(void *fp, int status);
    BOOL (*func_WndProc)(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void *editp, void *fp);
    int *track;
    int *check;
    void *ex_data_ptr;
    int ex_data_size;
    TCHAR *information;
    BOOL (*func_save_start)(void *fp, int s, int e, void *editp);
    BOOL (*func_save_end)(void *fp, void *editp);
    EXFUNC *exfunc;
    HWND hwnd;
    HINSTANCE dll_hinst;
    void *ex_data_def;
    BOOL (*func_is_saveframe)(void *fp, void *editp, int saveno, int frame, int fps, int edit_flag, int inter);
    BOOL (*func_project_load)(void *fp, void *editp, void *data, int size);
    BOOL (*func_project_save)(void *fp, void *editp, void *data, int *size);
    BOOL (*func_modify_title)(void *fp, void *editp, int frame, LPSTR title, int max_title);
    TCHAR *dll_path;
    int reserve[2];
} FILTER;

#define FILTER_FLAG_ACTIVE 1
#define FILTER_FLAG_ALWAYS_ACTIVE 4
#define FILTER_FLAG_CONFIG_POPUP 8
#define FILTER_FLAG_CONFIG_CHECK 16
#define FILTER_FLAG_CONFIG_RADIO 32
#define FILTER_FLAG_EX_DATA 1024
#define FILTER_FLAG_PRIORITY_HIGHEST 2048
#define FILTER_FLAG_PRIORITY_LOWEST 4096
#define FILTER_FLAG_WINDOW_THICKFRAME 8192
#define FILTER_FLAG_WINDOW_SIZE 16384
#define FILTER_FLAG_DISP_FILTER 32768
#define FILTER_FLAG_REDRAW 0x20000
#define FILTER_FLAG_EX_INFORMATION 0x40000
#define FILTER_FLAG_INFORMATION 0x80000
#define FILTER_FLAG_NO_CONFIG 0x100000
#define FILTER_FLAG_AUDIO_FILTER 0x200000
#define FILTER_FLAG_RADIO_BUTTON 0x400000
#define FILTER_FLAG_WINDOW_HSCROLL 0x800000
#define FILTER_FLAG_WINDOW_VSCROLL 0x1000000
#define FILTER_FLAG_INTERLACE_FILTER 0x4000000
#define FILTER_FLAG_NO_INIT_DATA 0x8000000
#define FILTER_FLAG_IMPORT 0x10000000
#define FILTER_FLAG_EXPORT 0x20000000
#define FILTER_FLAG_MAIN_MESSAGE 0x40000000
#define WM_FILTER_UPDATE (WM_USER + 100)
#define WM_FILTER_FILE_OPEN (WM_USER + 101)
#define WM_FILTER_FILE_CLOSE (WM_USER + 102)
#define WM_FILTER_INIT (WM_USER + 103)
#define WM_FILTER_EXIT (WM_USER + 104)
#define WM_FILTER_SAVE_START (WM_USER + 105)
#define WM_FILTER_SAVE_END (WM_USER + 106)
#define WM_FILTER_IMPORT (WM_USER + 107)
#define WM_FILTER_EXPORT (WM_USER + 108)
#define WM_FILTER_CHANGE_ACTIVE (WM_USER + 109)
#define WM_FILTER_CHANGE_WINDOW (WM_USER + 110)
#define WM_FILTER_CHANGE_PARAM (WM_USER + 111)
#define WM_FILTER_CHANGE_EDIT (WM_USER + 112)
#define WM_FILTER_COMMAND (WM_USER + 113)
#define WM_FILTER_FILE_UPDATE (WM_USER + 114)
#define WM_FILTER_MAIN_MOUSE_DOWN (WM_USER + 120)
#define WM_FILTER_MAIN_MOUSE_UP (WM_USER + 121)
#define WM_FILTER_MAIN_MOUSE_MOVE (WM_USER + 122)
#define WM_FILTER_MAIN_KEY_DOWN (WM_USER + 123)
#define WM_FILTER_MAIN_KEY_UP (WM_USER + 124)
#define WM_FILTER_MAIN_MOVESIZE (WM_USER + 125)
#define WM_FILTER_MAIN_MOUSE_DBLCLK (WM_USER + 126)
#define WM_FILTER_MAIN_MOUSE_R_DOWN (WM_USER + 127)
#define WM_FILTER_MAIN_MOUSE_R_UP (WM_USER + 128)
#define WM_FILTER_MAIN_MOUSE_WHEEL (WM_USER + 129)
#define WM_FILTER_MAIN_CONTEXTMENU (WM_USER + 130)
#define FILTER_UPDATE_STATUS_ALL 0
#define FILTER_UPDATE_STATUS_TRACK 0x10000
#define FILTER_UPDATE_STATUS_CHECK 0x20000
#define FILTER_WINDOW_SIZE_CLIENT 0x10000000
#define FILTER_WINDOW_SIZE_ADD 0x30000000

typedef struct {
    int flag;
    int x, y;
    TCHAR *name;
    int track_n;
    TCHAR **track_name;
    int *track_default;
    int *track_s, *track_e;
    int check_n;
    TCHAR **check_name;
    int *check_default;
    BOOL (*func_proc)(FILTER *fp, FILTER_PROC_INFO *fpip);
    BOOL (*func_init)(FILTER *fp);
    BOOL (*func_exit)(FILTER *fp);
    BOOL (*func_update)(FILTER *fp, int status);
    BOOL (*func_WndProc)(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam, void *editp, FILTER *fp);
    int *track, *check;
    void *ex_data_ptr;
    int ex_data_size;
    TCHAR *information;
    BOOL (*func_save_start)(FILTER *fp, int s, int e, void *editp);
    BOOL (*func_save_end)(FILTER *fp, void *editp);
    EXFUNC *exfunc;
    HWND hwnd;
    HINSTANCE dll_hinst;
    void *ex_data_def;
    BOOL (*func_is_saveframe)(FILTER *fp, void *editp, int saveno, int frame, int fps, int edit_flag, int inter);
    BOOL (*func_project_load)(FILTER *fp, void *editp, void *data, int size);
    BOOL (*func_project_save)(FILTER *fp, void *editp, void *data, int *size);
    BOOL (*func_modify_title)(FILTER *fp, void *editp, int frame, LPSTR title, int max_title);
    TCHAR *dll_path;
    int reserve[2];
} FILTER_DLL;

#define MID_FILTER_BUTTON 12004

typedef struct {
    int flag;
    int rate, scale;
    int n;
    BITMAPINFOHEADER *format;
    int format_size;
    int audio_n;
    WAVEFORMATEX *audio_format;
    int audio_format_size;
    DWORD handler;
    int reserve[7];
} INPUT_INFO;

#define INPUT_INFO_FLAG_VIDEO 1
#define INPUT_INFO_FLAG_AUDIO 2
#define INPUT_INFO_FLAG_VIDEO_RANDOM_ACCESS 8

typedef void *INPUT_HANDLE;

typedef struct {
    int flag;
    LPSTR name;
    LPSTR filefilter;
    LPSTR information;
    BOOL (*func_init)(void);
    BOOL (*func_exit)(void);
    INPUT_HANDLE (*func_open)(LPSTR file);
    BOOL (*func_close)(INPUT_HANDLE ih);
    BOOL (*func_info_get)(INPUT_HANDLE ih, INPUT_INFO *iip);
    int (*func_read_video)(INPUT_HANDLE ih, int frame, void *buf);
    int (*func_read_audio)(INPUT_HANDLE ih, int start, int length, void *buf);
    BOOL (*func_is_keyframe)(INPUT_HANDLE ih, int frame);
    BOOL (*func_config)(HWND hwnd, HINSTANCE dll_hinst);
    int reserve[16];
} INPUT_PLUGIN_TABLE;

#define INPUT_PLUGIN_FLAG_VIDEO 1
#define INPUT_PLUGIN_FLAG_AUDIO 2

typedef struct {
    int flag;
    int w, h;
    int rate, scale;
    int n;
    int size;
    int audio_rate;
    int audio_ch;
    int audio_n;
    int audio_size;
    LPSTR savefile;
    void *(*func_get_video)(int frame);
    void *(*func_get_audio)(int start, int length, int *readed);
    BOOL (*func_is_abort)(void);
    BOOL (*func_rest_time_disp)(int now, int total);
    int (*func_get_flag)(int frame);
    BOOL (*func_update_preview)(void);
    void *(*func_get_video_ex)(int frame, DWORD format);
} OUTPUT_INFO;

#define OUTPUT_INFO_FLAG_VIDEO 1
#define OUTPUT_INFO_FLAG_AUDIO 2
#define OUTPUT_INFO_FLAG_BATCH 4
#define OUTPUT_INFO_FRAME_FLAG_KEYFRAME 1
#define OUTPUT_INFO_FRAME_FLAG_COPYFRAME 2

typedef struct {
    int flag;
    LPSTR name;
    LPSTR filefilter;
    LPSTR information;
    BOOL (*func_init)(void);
    BOOL (*func_exit)(void);
    BOOL (*func_output)(OUTPUT_INFO *oip);
    BOOL (*func_config)(HWND hwnd, HINSTANCE dll_hinst);
    int (*func_config_get)(void *data, int size);
    int (*func_config_set)(void *data, int size);
    int reserve[16];
} OUTPUT_PLUGIN_TABLE;

struct AviUtlFilterPluginEncoder {
    typedef FILTER_DLL *(WINAPI *PFN_GetFilterTable)(void);

    AviUtlFilterPluginEncoder()
        : m_window(nullptr)
        , m_module(nullptr)
        , m_filterPluginTable(nullptr)
        , m_filePath(nullptr)
        , m_currentFrameIndex(0)
        , m_fps(0)
        , m_duration(0)
        , m_width(0)
        , m_height(0)
        , m_yflip(0)
        , m_channels(0)
        , m_sampleRate(0)
        , m_numBits(0)
        , m_interrupted(false)
    {
        static auto s_self = this;
        ZeroMemory(&m_exfuncTable, sizeof(m_exfuncTable));
        m_exfuncTable.get_ycp_ofs = [](void *editp, int n, int ofs) -> void {};
        m_exfuncTable.get_ycp = [](void *editp, int n) -> void * { return nullptr; };
        m_exfuncTable.get_pixelp = [](void *editp, int n) -> void * {
            auto self = static_cast<AviUtlFilterPluginEncoder *>(editp);
            return self->m_videoFrameData.data();
        };
        m_exfuncTable.get_audio = [](void *editp, int n, void *buf) -> int { return 0; };
        m_exfuncTable.is_editing = [](void *editp) -> BOOL { return TRUE; };
        m_exfuncTable.is_saving = [](void *editp) -> BOOL { return FALSE; };
        m_exfuncTable.get_frame = [](void *editp) -> int { return 0; };
        m_exfuncTable.get_frame_n = [](void *editp) -> int {
            auto self = static_cast<const AviUtlFilterPluginEncoder *>(editp);
            return Inline::saturateInt32(self->m_duration);
        };
        m_exfuncTable.get_frame_size = [](void *editp, int *w, int *h) -> BOOL {
            BOOL result = FALSE;
            if (w && h) {
                auto self = static_cast<const AviUtlFilterPluginEncoder *>(editp);
                *w = Inline::saturateInt32(self->m_width);
                *h = Inline::saturateInt32(self->m_height);
                result = TRUE;
            }
            return result;
        };
        m_exfuncTable.set_frame = [](void *editp, int n) -> int { return 0; };
        m_exfuncTable.set_frame_n = [](void *editp, int n) -> int {
            auto self = static_cast<const AviUtlFilterPluginEncoder *>(editp);
            return Inline::saturateInt32(self->m_duration);
        };
        m_exfuncTable.copy_frame = [](void *editp, int d, int s) -> BOOL { return FALSE; };
        m_exfuncTable.copy_video = [](void *editp, int d, int s) -> BOOL { return FALSE; };
        m_exfuncTable.copy_audio = [](void *editp, int d, int s) -> BOOL { return FALSE; };
        m_exfuncTable.copy_clip = [](HWND hwnd, void *pixelp, int w, int h) -> BOOL { return FALSE; };
        m_exfuncTable.paste_clip = [](HWND hwnd, void *editp, int n) -> BOOL { return FALSE; };
        m_exfuncTable.get_frame_status = [](void *editp, int n, FRAME_STATUS *fsp) -> BOOL {
            BOOL result = FALSE;
            if (fsp) {
                fsp->audio = 0;
                fsp->config = 0;
                fsp->edit_flag = EDIT_FRAME_EDIT_FLAG_KEYFRAME;
                fsp->index24fps = 0;
                fsp->inter = FRAME_STATUS_INTER_NORMAL;
                fsp->vcm = 0;
                fsp->video = 0;
                ZeroMemory(fsp->reserve, sizeof(fsp->reserve));
                result = TRUE;
            }
            return result;
        };
        m_exfuncTable.set_frame_status = [](void *editp, int n, FRAME_STATUS *fsp) -> BOOL {
            BOOL result = FALSE;
            if (fsp) {
                result = TRUE;
            }
            return result;
        };
        m_exfuncTable.is_saveframe = [](void *editp, int n) -> BOOL { return TRUE; };
        m_exfuncTable.is_keyframe = [](void *editp, int n) -> BOOL { return TRUE; };
        m_exfuncTable.is_recompress = [](void *editp, int n) -> BOOL { return FALSE; };
        m_exfuncTable.filter_window_update = [](void *fp) -> BOOL { return TRUE; };
        m_exfuncTable.is_filter_window_disp = [](void *fp) -> BOOL { return FALSE; };
        m_exfuncTable.get_file_info = [](void *editp, FILE_INFO *fip) -> BOOL {
            auto self = static_cast<const AviUtlFilterPluginEncoder *>(editp);
            return self->m_exfuncTable.get_source_file_info(editp, fip, 0);
        };
        m_exfuncTable.get_config_name = [](void *editp, int n) -> LPSTR { return nullptr; };
        m_exfuncTable.is_filter_active = [](void *fp) -> BOOL { return FALSE; };
        m_exfuncTable.get_pixel_filtered = [](void *editp, int n, void *pixelp, int *w, int *h) -> BOOL {
            return FALSE;
        };
        m_exfuncTable.get_audio_filtered = [](void *editp, int n, void *buf) -> int { return FALSE; };
        m_exfuncTable.get_select_frame = [](void *editp, int *s, int *e) -> BOOL {
            BOOL result = FALSE;
            if (s && e) {
                *s = 0;
                *e = 0;
                result = TRUE;
            }
            return result;
        };
        m_exfuncTable.set_select_frame = [](void *editp, int s, int e) -> BOOL { return TRUE; };
        m_exfuncTable.rgb2yc = [](PIXEL_YC *ycp, PIXEL *pixelp, int w) -> BOOL { return TRUE; };
        m_exfuncTable.yc2rgb = [](PIXEL *pixelp, PIXEL_YC *ycp, int w) -> BOOL { return TRUE; };
        m_exfuncTable.dlg_get_load_name = [](LPSTR name, LPSTR filter, LPSTR def) -> BOOL { return FALSE; };
        m_exfuncTable.dlg_get_save_name = [](LPSTR name, LPSTR filter, LPSTR def) -> BOOL {
            BOOL result = FALSE;
            auto &extensions = s_self->m_videoExtensions;
            if (!extensions.empty()) {
                strcpy_s(name, MAX_PATH, s_self->m_filePath);
                result = TRUE;
            }
            else {
                const std::string s(filter);
                const std::regex regex(".+//(//*//.(//w+)//)");
                std::smatch match;
                auto it = s.cbegin(), end = s.cend();
                while (std::regex_search(it, end, match, regex)) {
                    auto s2 = match.str(1);
                    extensions.push_back(_strdup(s2.c_str()));
                    it = match[0].second;
                }
            }
            return result;
        };
        m_exfuncTable.ini_load_int = [](void *fp, LPSTR key, int n) -> int { return FALSE; };
        m_exfuncTable.ini_save_int = [](void *fp, LPSTR key, int n) -> int { return FALSE; };
        m_exfuncTable.ini_load_str = [](void *fp, LPSTR key, LPSTR str, LPSTR def) -> BOOL { return FALSE; };
        m_exfuncTable.ini_save_str = [](void *fp, LPSTR key, LPSTR str) -> BOOL { return FALSE; };
        m_exfuncTable.get_source_file_info = [](void *editp, FILE_INFO *fip, int source_file_id) -> BOOL {
            BOOL result = FALSE;
            if (fip) {
                auto self = static_cast<const AviUtlFilterPluginEncoder *>(editp);
                fip->audio_ch = Inline::saturateInt32(self->m_channels);
                fip->audio_n = 0;
                fip->audio_rate = Inline::saturateInt32(self->m_sampleRate);
                fip->flag = (self->m_numBits > 0 ? FILE_INFO_FLAG_AUDIO : 0) | FILE_INFO_FLAG_VIDEO;
                fip->frame_n = Inline::saturateInt32(self->m_duration);
                fip->h = Inline::saturateInt32(self->m_height);
                fip->name = self->m_filePath;
                fip->video_decode_bit = 24;
                fip->video_decode_format = 0;
                fip->video_rate = Inline::saturateInt32(self->m_fps);
                fip->video_scale = 1;
                fip->w = Inline::saturateInt32(self->m_width);
                ZeroMemory(fip->reserve, sizeof(fip->reserve));
                result = TRUE;
            }
            return result;
        };
        m_exfuncTable.get_source_video_number = [](void *editp, int n, int *source_file_i,
                                                    int *source_video_number) -> BOOL {
            BOOL result = FALSE;
            if (source_file_i && source_video_number) {
                *source_file_i = 0;
                *source_video_number = 0;
                result = TRUE;
            }
            return result;
        };
        m_exfuncTable.get_sys_info = [](void *editp, SYS_INFO *sip) -> BOOL {
            BOOL result = FALSE;
            if (sip) {
                auto self = static_cast<const AviUtlFilterPluginEncoder *>(editp);
                sip->build = 0;
                sip->edit_name = nullptr;
                sip->filter_n = 0;
                sip->flag = 0;
                sip->hfont = nullptr;
                sip->info = nullptr;
                sip->max_frame = 1u << 24;
                sip->max_h = sip->max_w = 1u << 16;
                sip->min_h = sip->min_w = 32;
                sip->output_name = nullptr;
                sip->project_name = nullptr;
                sip->vram_h = Inline::saturateInt32(self->m_height);
                sip->vram_w = Inline::saturateInt32(self->m_width);
                sip->vram_line_size = Inline::saturateInt32(self->m_width * 4);
                sip->vram_yc_size = 6;
                ZeroMemory(sip->reserve, sizeof(sip->reserve));
                result = TRUE;
            }
            return result;
        };
        m_exfuncTable.get_filterp = [](int filter_id) -> void * { return nullptr; };
        m_exfuncTable.get_ycp_filtering = [](void *fp, void *editp, int n, void *reserve) -> void * {
            auto self = static_cast<AviUtlFilterPluginEncoder *>(editp);
            return self->m_videoFrameData.data();
        };
        m_exfuncTable.get_audio_filtering = [](void *fp, void *editp, int n, void *buf) -> int { return TRUE; };
        m_exfuncTable.set_ycp_filtering_cache_size = [](void *fp, int w, int h, int d, int flag) -> BOOL {
            return TRUE;
        };
        m_exfuncTable.get_ycp_filtering_cache = [](void *fp, void *editp, int n) -> void * {
            auto self = static_cast<AviUtlFilterPluginEncoder *>(editp);
            return self->m_videoFrameData.data();
        };
        m_exfuncTable.get_ycp_source_cache = [](void *editp, int n, int ofs) -> void * {
            auto self = static_cast<AviUtlFilterPluginEncoder *>(editp);
            return self->m_videoFrameData.data();
        };
        m_exfuncTable.get_disp_pixelp = [](void *editp, DWORD format) -> void * {
            auto self = static_cast<AviUtlFilterPluginEncoder *>(editp);
            return self->m_videoFrameData.data();
        };
        m_exfuncTable.get_pixel_source = [](void *editp, int n, void *pixelp, DWORD format) -> BOOL { return TRUE; };
        m_exfuncTable.get_pixel_filtered_ex = [](void *editp, int n, void *pixelp, int *w, int *h,
                                                  DWORD format) -> BOOL {
            BOOL result = FALSE;
            if (w && h) {
                *w = 1;
                *h = 1;
            }
            return result;
        };
        m_exfuncTable.get_ycp_filtering_cache_ex = [](void *fp, void *editp, int n, int *w, int *h) -> PIXEL_YC * {
            return nullptr;
        };
        m_exfuncTable.exec_multi_thread_func = [](MULTI_THREAD_FUNC func, void *param1, void *param2) -> BOOL {
            return TRUE;
        };
        m_exfuncTable.create_yc = [](void) -> PIXEL_YC * { return nullptr; };
        m_exfuncTable.delete_yc = [](PIXEL_YC *ycp) -> void {};
        m_exfuncTable.load_image = [](PIXEL_YC *ycp, LPSTR file, int *w, int *h, int flag) -> BOOL {
            BOOL result = FALSE;
            if (w && h) {
                *w = 1;
                *h = 1;
                result = TRUE;
            }
            return result;
        };
        m_exfuncTable.resize_yc = [](PIXEL_YC *ycp, int w, int h, PIXEL_YC *ycp_src, int sx, int sy, int sw,
                                      int sh) -> void {};
        m_exfuncTable.copy_yc = [](PIXEL_YC *ycp, int x, int y, PIXEL_YC *ycp_src, int sx, int sy, int sw, int sh,
                                    int tr) -> void {};
        m_exfuncTable.draw_text = [](PIXEL_YC *ycp, int x, int y, LPSTR text, int r, int g, int b, int tr, HFONT hfont,
                                      int *w, int *h) -> void {};
        m_exfuncTable.avi_file_open = [](LPSTR file, FILE_INFO *fip, int flag) -> AVI_FILE_HANDLE { return nullptr; };
        m_exfuncTable.avi_file_close = [](AVI_FILE_HANDLE afh) -> void {};
        m_exfuncTable.avi_file_read_video = [](AVI_FILE_HANDLE afh, PIXEL_YC *ycp, int n) -> BOOL { return FALSE; };
        m_exfuncTable.avi_file_read_audio = [](AVI_FILE_HANDLE afh, void *buf, int n) -> int { return 0; };
        m_exfuncTable.avi_file_get_video_pixelp = [](AVI_FILE_HANDLE afh, int n) -> void * { return nullptr; };
        m_exfuncTable.get_avi_file_filter = [](int type) -> LPSTR { return nullptr; };
        m_exfuncTable.avi_file_read_audio_sample = [](AVI_FILE_HANDLE afh, int start, int length, void *buf) -> int {
            return 0;
        };
        m_exfuncTable.avi_file_set_audio_sample_rate = [](AVI_FILE_HANDLE afh, int audio_rate, int audio_ch) -> int {
            return 0;
        };
        m_exfuncTable.get_frame_status_table = [](void *editp, int type) -> BYTE * { return nullptr; };
        m_exfuncTable.set_undo = [](void *editp) -> BOOL { return TRUE; };
        m_exfuncTable.add_menu_item = [](void *fp, LPSTR name, HWND hwnd, int id, int def_key, int flag) -> BOOL {
            return TRUE;
        };
        m_exfuncTable.edit_open = [](void *editp, LPSTR file, int flag) -> BOOL { return TRUE; };
        m_exfuncTable.edit_close = [](void *editp) -> BOOL { return TRUE; };
        m_exfuncTable.edit_output = [](void *editp, LPSTR file, int flag, LPSTR type) -> BOOL { return TRUE; };
        m_exfuncTable.set_config = [](void *editp, int n, LPSTR name) -> BOOL { return TRUE; };
        m_module = LoadLibraryW(L"C:/Users/hkrn2/Dropbox/windows/Applications/aviutl100/plugins/lwmuxer.auf");
        if (m_module) {
            if (auto GetFilterTable =
                    reinterpret_cast<PFN_GetFilterTable>(GetProcAddress(m_module, "GetFilterTable"))) {
                m_filterPluginTable = GetFilterTable();
            }
            else if (auto GetFilterTableYUY2 =
                         reinterpret_cast<PFN_GetFilterTable>(GetProcAddress(m_module, "GetFilterTableYUY2"))) {
                m_filterPluginTable = GetFilterTableYUY2();
            }
            if (m_filterPluginTable) {
                auto table = reinterpret_cast<FILTER *>(m_filterPluginTable);
                table->dll_hinst = m_module;
                table->hwnd = m_window;
                table->exfunc = &m_exfuncTable;
                if (auto funcInit = m_filterPluginTable->func_init) {
                    funcInit(table);
                }
                m_window = CreateWindowExW(WS_EX_NOACTIVATE, L"plugin_aviutl", L"plugin_aviutl", WS_DISABLED, 0, 0, 1,
                    1, nullptr, nullptr, m_module, nullptr);
                ShowWindow(m_window, SW_HIDE);
                m_filterPluginTable->func_WndProc(m_window, WM_FILTER_INIT, 0, 0, this, table);
                m_filterPluginTable->func_WndProc(m_window, WM_FILTER_EXPORT, 0, 0, this, table);
            }
        }
    }
    ~AviUtlFilterPluginEncoder()
    {
        if (auto table = reinterpret_cast<FILTER *>(m_filterPluginTable)) {
            table->func_WndProc(m_window, WM_FILTER_EXIT, 0, 0, this, table);
            if (auto funcExit = table->func_exit) {
                funcExit(table);
            }
        }
        for (auto it : m_videoExtensions) {
            free(it);
        }
        m_videoExtensions.clear();
        if (m_filePath) {
            free(m_filePath);
            m_filePath = nullptr;
        }
        if (m_module) {
            FreeLibrary(m_module);
            m_module = nullptr;
        }
        if (m_window) {
            CloseWindow(m_window);
            DestroyWindow(m_window);
            m_window = nullptr;
        }
    }

    int
    open(const char *filePath, nanoem_application_plugin_status_t *status)
    {
        int result = 0;
        if (m_module && m_filterPluginTable) {
            auto table = reinterpret_cast<FILTER *>(m_filterPluginTable);
            m_filePath = _strdup(filePath);
            m_filterPluginTable->func_WndProc(nullptr, WM_FILTER_EXPORT, 0, 0, this, table);
            m_filterPluginTable->func_WndProc(nullptr, WM_FILTER_FILE_OPEN, 0, 0, this, table);
            result = 1;
        }
        else {
            nanoem_application_plugin_status_assign_error(status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
        }
        return result;
    }
    void
    setOption(nanoem_u32_t key, const void *value, nanoem_rsize_t size, nanoem_application_plugin_status_t *status)
    {
        int ret = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
        switch (key) {
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_FPS: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_fps = *static_cast<const nanoem_u32_t *>(value);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_LOCATION: {
            /* unused */
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_DURATION: {
            if (Inline::validateArgument<nanoem_frame_index_t>(value, size, status)) {
                m_duration = *static_cast<const nanoem_frame_index_t *>(value);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_FREQUENCY: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_sampleRate = *static_cast<const nanoem_u32_t *>(value);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_CHANNELS: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_channels = *static_cast<const nanoem_u32_t *>(value);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_BITS: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_numBits = *static_cast<const nanoem_u32_t *>(value);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_WIDTH: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_width = *static_cast<const nanoem_u32_t *>(value);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_HEIGHT: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_height = *static_cast<const nanoem_u32_t *>(value);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_YFLIP: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_yflip = *static_cast<const nanoem_u32_t *>(value);
            }
            break;
        }
        default:
            ret = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON;
            break;
        }
    }
    const char *const *
    allAvailableVideoFormatExtensions(nanoem_rsize_t *length) const
    {
        const char *const *values = nullptr;
        if (!m_videoExtensions.empty()) {
            values = m_videoExtensions.data();
            *length = m_videoExtensions.size();
        }
        else {
            *length = 0;
        }
        return values;
    }
    void
    encodeAudioFrame(nanoem_frame_index_t /* currentFrameIndex */, const nanoem_u8_t *data, nanoem_rsize_t size,
        nanoem_application_plugin_status_t *status)
    {
        if (m_module) {
        }
        else {
            nanoem_application_plugin_status_assign_error(status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
        }
    }
    void
    encodeVideoFrame(nanoem_frame_index_t currentFrameIndex, const nanoem_u8_t *data, nanoem_rsize_t size,
        nanoem_application_plugin_status_t *status)
    {
        if (m_module) {
            m_currentFrameIndex = currentFrameIndex;
            m_videoFrameData.assign(data, data + size);
        }
        else {
            nanoem_application_plugin_status_assign_error(status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
        }
    }
    int
    interrupt(nanoem_application_plugin_status_t *status)
    {
        m_interrupted = true;
        return close(status);
    }
    const char *
    failureReason() const
    {
        return nullptr;
    }
    const char *
    recoverySuggestion() const
    {
        return nullptr;
    }
    int
    close(nanoem_application_plugin_status_t *status)
    {
        if (m_module && m_filterPluginTable) {
            auto table = reinterpret_cast<FILTER *>(m_filterPluginTable);
            m_filterPluginTable->func_WndProc(nullptr, WM_FILTER_FILE_CLOSE, 0, 0, this, table);
        }
        else {
            nanoem_application_plugin_status_assign_error(status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
        }
        return 1;
    }

    using ByteArray = std::vector<nanoem_u8_t>;
    HWND m_window;
    HMODULE m_module;
    FILTER_DLL *m_filterPluginTable;
    EXFUNC m_exfuncTable;
    std::unordered_map<nanoem_frame_index_t, ByteArray> m_videoFrames;
    std::vector<char *> m_videoExtensions;
    char *m_filePath;
    ByteArray m_videoFrameData;
    nanoem_frame_index_t m_currentFrameIndex;
    nanoem_u32_t m_fps;
    nanoem_u32_t m_duration;
    nanoem_u32_t m_width;
    nanoem_u32_t m_height;
    nanoem_u32_t m_yflip;
    nanoem_u32_t m_channels;
    nanoem_u32_t m_sampleRate;
    nanoem_u32_t m_numBits;
    bool m_interrupted;
};

struct AviUtlOutputPluginEncoder {
    typedef OUTPUT_PLUGIN_TABLE *(WINAPI *PFN_GetOutputPluginTable)(void);
    static AviUtlOutputPluginEncoder *s_self;
    static const char kPluginsComponentID[];

    static DWORD WINAPI
    threadFunc(void *userData)
    {
        auto self = static_cast<AviUtlOutputPluginEncoder *>(userData);
        auto it = self->m_plugins.find(self->m_currentPluginName.data());
        if (it != self->m_plugins.end()) {
            OUTPUT_PLUGIN_TABLE *table = it->second.m_table;
            if (auto funcInit = table->func_init) {
                funcInit();
            }
            OUTPUT_INFO info(self->m_info);
            table->func_output(&info);
            if (auto funcExit = table->func_exit) {
                funcExit();
            }
        }
        return 0;
    }

    static int
    funcGetFlag(int /* frame */)
    {
        return OUTPUT_INFO_FRAME_FLAG_KEYFRAME;
    }
    static BOOL
    funcIsAbort()
    {
        return s_self->m_interrupted ? TRUE : FALSE;
    }
    static void *
    funcGetAudio(int /* start */, int length, int *readed)
    {
        nanoem_u8_t *bytes = nullptr;
        WaitForSingleObject(s_self->m_audioQueueEventHandle, INFINITE);
        EnterCriticalSection(&s_self->m_audioQueueSection);
        if (!s_self->m_audioBuffers.empty()) {
            const nanoem_rsize_t ss = Inline::roundInt32(s_self->m_info.audio_size),
                                 actualRead = std::min(Inline::roundInt32(length) * ss, s_self->m_audioBuffers.size());
            bytes = s_self->m_audioBuffers.data();
            auto it = s_self->m_audioBuffers.begin();
            s_self->m_audioBuffers.erase(it, it + Inline::saturateInt32(actualRead));
            if (readed) {
                *readed = Inline::saturateInt32(actualRead);
            }
            s_self->m_consumedAudioSamples += actualRead;
        }
        LeaveCriticalSection(&s_self->m_audioQueueSection);
        return bytes;
    }
    static void *
    funcGetVideo(int frame)
    {
        nanoem_u8_t *bytes = nullptr;
        WaitForSingleObject(s_self->m_videoQueueEventHandle, INFINITE);
        EnterCriticalSection(&s_self->m_videoQueueSection);
        if (!s_self->m_videoFrames.empty()) {
            if (frame > 0) {
                s_self->m_videoFrames.erase(Inline::roundInt32(frame - 1));
            }
            auto it = s_self->m_videoFrames.find(Inline::roundInt32(frame));
            if (it != s_self->m_videoFrames.end()) {
                bytes = it->second.data();
            }
        }
        LeaveCriticalSection(&s_self->m_videoQueueSection);
        return bytes;
    }
    static void *
    funcGetVideoEx(int frame, DWORD /* format */)
    {
        return funcGetVideo(frame);
    }
    static BOOL
    funcRestTimeDisp(int /* now */, int /* total */)
    {
        return TRUE;
    }
    static BOOL
    funcUpdatePreview()
    {
        return TRUE;
    }

    AviUtlOutputPluginEncoder()
        : m_threadHandle(nullptr)
        , m_audioQueueEventHandle(nullptr)
        , m_videoQueueEventHandle(nullptr)
        , m_outputVideoFilePath(nullptr)
        , m_yflip(0)
        , m_interrupted(false)
    {
        s_self = this;
        InitializeCriticalSection(&m_audioQueueSection);
        InitializeCriticalSection(&m_videoQueueSection);
        ZeroMemory(&m_info, sizeof(m_info));
        m_info.flag = OUTPUT_INFO_FLAG_VIDEO;
        m_info.func_get_flag = funcGetFlag;
        m_info.func_is_abort = funcIsAbort;
        m_info.func_get_audio = funcGetAudio;
        m_info.func_get_video = funcGetVideo;
        m_info.func_get_video_ex = funcGetVideoEx;
        m_info.func_rest_time_disp = funcRestTimeDisp;
        m_info.func_update_preview = funcUpdatePreview;
        m_audioQueueEventHandle = CreateEventW(nullptr, false, false, L"m_audioQueueEventHandle");
        m_videoQueueEventHandle = CreateEventW(nullptr, false, false, L"m_videoQueueEventHandle");
        const wchar_t *directory = L"C:/Users/hkrn2/Dropbox/windows/Applications/aviutl100";
        std::wstring basePath(directory);
        basePath.append(L"/*.auo");
        WIN32_FIND_DATAW find;
        HANDLE handle = FindFirstFileW(basePath.c_str(), &find);
        if (handle != INVALID_HANDLE_VALUE) {
            do {
                std::wstring value(directory);
                value.append(L"/");
                value.append(find.cFileName);
                if (HMODULE plugin = LoadLibraryW(value.c_str())) {
                    PluginTable t = { plugin };
                    populateAllAvailableExtensions(plugin, t.m_table, t.m_videoExtensions);
                    if (t.m_table) {
                        m_plugins.insert(std::make_pair(find.cFileName, t));
                        m_pluginNames.push_back(toAllocatedString(find.cFileName));
                    }
                    else {
                        t.destroy();
                    }
                }
            } while (FindNextFileW(handle, &find));
            FindClose(handle);
        }
    }
    ~AviUtlOutputPluginEncoder()
    {
        DeleteCriticalSection(&m_audioQueueSection);
        DeleteCriticalSection(&m_videoQueueSection);
        for (auto item : m_pluginNames) {
            delete[] item;
        }
        m_pluginNames.clear();
        for (auto item : m_plugins) {
            item.second.destroy();
        }
        m_plugins.clear();
        if (m_outputVideoFilePath) {
            free(m_outputVideoFilePath);
            m_outputVideoFilePath = nullptr;
        }
        if (m_audioQueueEventHandle) {
            CloseHandle(m_audioQueueEventHandle);
            m_audioQueueEventHandle = nullptr;
        }
        if (m_videoQueueEventHandle) {
            CloseHandle(m_videoQueueEventHandle);
            m_videoQueueEventHandle = nullptr;
        }
    }

    int
    open(const char *filePath, nanoem_application_plugin_status_t *status)
    {
        if (!m_threadHandle) {
            m_outputVideoFilePath = _strdup(filePath);
            char *p = m_outputVideoFilePath;
            while (*p) {
                const char c = *p;
                if (c == '/') {
                    *p = '//';
                }
                p++;
            }
            m_info.size = m_info.w * m_info.h * 3;
            m_info.n *= (m_info.rate / 30);
            m_info.audio_n = m_info.audio_ch * m_info.audio_size * m_info.audio_rate;
            m_info.savefile = m_outputVideoFilePath;
            if (m_info.audio_n > 0) {
                m_info.flag |= OUTPUT_INFO_FLAG_AUDIO;
            }
            m_threadHandle = CreateThread(nullptr, 0, threadFunc, this, 0, nullptr);
        }
        return 1;
    }
    void
    setOption(nanoem_u32_t key, const void *value, nanoem_rsize_t size, nanoem_application_plugin_status_t *status)
    {
        int ret = NANOEM_APPLICATION_PLUGIN_STATUS_SUCCESS;
        switch (key) {
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_FPS: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_info.rate = Inline::saturateInt32(*static_cast<const nanoem_u32_t *>(value));
                m_info.scale = 1;
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_LOCATION: {
            /* unused */
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_DURATION: {
            if (Inline::validateArgument<nanoem_frame_index_t>(value, size, status)) {
                m_info.n = Inline::saturateInt32(*static_cast<const nanoem_frame_index_t *>(value));
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_FREQUENCY: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_info.audio_rate = Inline::saturateInt32(*static_cast<const nanoem_u32_t *>(value));
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_CHANNELS: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_info.audio_ch = Inline::saturateInt32(*static_cast<const nanoem_u32_t *>(value));
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_AUDIO_NUM_BITS: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_info.audio_size = Inline::saturateInt32(*static_cast<const nanoem_u32_t *>(value)) / 8;
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_WIDTH: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_info.w = Inline::saturateInt32(*static_cast<const nanoem_u32_t *>(value));
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_HEIGHT: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_info.h = Inline::saturateInt32(*static_cast<const nanoem_u32_t *>(value));
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_ENCODER_OPTION_VIDEO_YFLIP: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_yflip = *static_cast<const nanoem_u32_t *>(value);
            }
            break;
        }
        default:
            ret = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_REFER_REASON;
            break;
        }
    }
    const char *const *
    allAvailableVideoFormatExtensions(nanoem_rsize_t *length) const
    {
        const char *const *values = nullptr;
        *length = 0;
        if (!m_currentPluginName.empty()) {
            auto it = m_plugins.find(m_currentPluginName.data());
            if (it != m_plugins.end()) {
                const auto &extensions = it->second.m_videoExtensions;
                if (!extensions.empty()) {
                    values = extensions.data();
                    *length = extensions.size();
                }
            }
        }
        return values;
    }
    void
    encodeAudioFrame(nanoem_frame_index_t /* currentFrameIndex*/, const nanoem_u8_t *data, nanoem_rsize_t size,
        nanoem_application_plugin_status_t *status)
    {
        if (m_threadHandle) {
            EnterCriticalSection(&m_audioQueueSection);
            m_audioBuffers.insert(m_audioBuffers.end(), data, data + size);
            LeaveCriticalSection(&m_audioQueueSection);
            SetEvent(m_audioQueueEventHandle);
        }
        else {
            nanoem_application_plugin_status_assign_error(status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
        }
    }
    void
    encodeVideoFrame(nanoem_frame_index_t currentFrameIndex, const nanoem_u8_t *data, nanoem_rsize_t size,
        nanoem_application_plugin_status_t *status)
    {
        if (m_threadHandle && (size == Inline::roundInt32(m_info.size / 3) * 4)) {
            ByteArray bytes(Inline::roundInt32(m_info.size));
            int width = m_info.w, height = m_info.h;
            if (m_yflip) {
                for (int i = 0; i < width; i++) {
                    for (int j = 0; j < height; j++) {
                        const nanoem_rsize_t offset = Inline::roundInt32(j * width + i), soffset = offset * 4,
                                             doffset = offset * 3;
                        memcpy(bytes.data() + doffset, data + soffset, 3);
                    }
                }
            }
            else {
                for (int i = 0; i < width; i++) {
                    for (int j = 0; j < height; j++) {
                        const nanoem_rsize_t soffset = Inline::roundInt32((height - j - 1) * width + i) * 4,
                                             doffset = Inline::roundInt32(j * width + i) * 3;
                        memcpy(bytes.data() + doffset, data + soffset, 3);
                    }
                }
            }
            EnterCriticalSection(&m_videoQueueSection);
            m_videoFrames.insert(std::make_pair(currentFrameIndex, std::move(bytes)));
            LeaveCriticalSection(&m_videoQueueSection);
            SetEvent(m_videoQueueEventHandle);
        }
        else {
            nanoem_application_plugin_status_assign_error(status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
        }
    }
    int
    interrupt(nanoem_application_plugin_status_t *status)
    {
        m_interrupted = true;
        SetEvent(m_audioQueueEventHandle);
        SetEvent(m_videoQueueEventHandle);
        return close(status);
    }
    const char *
    failureReason() const
    {
        return nullptr;
    }
    const char *
    recoverySuggestion() const
    {
        return nullptr;
    }
    int
    close(nanoem_application_plugin_status_t *status)
    {
        if (m_threadHandle) {
            WaitForSingleObject(m_threadHandle, INFINITE);
            CloseHandle(m_threadHandle);
            m_threadHandle = nullptr;
        }
        else {
            nanoem_application_plugin_status_assign_error(status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
        }
        return 1;
    }
    void
    loadUIWindowLayout(nanoem_application_plugin_status_t *status)
    {
        clearUIWindowLayout();
        nanoem_u32_t index = 0;
        if (!m_currentPluginName.empty()) {
            char *pluginName = toAllocatedString(m_currentPluginName.data());
            for (auto item : m_pluginNames) {
                if (StringUtils::equals(pluginName, item)) {
                    index = index;
                    break;
                }
                index++;
            }
            delete[] pluginName;
        }
        else if (!m_pluginNames.empty()) {
            setCurrentPluginName(m_pluginNames[0]);
        }
        m_components.push_back(createLabel("Available AviUtl output plugin"));
        m_components.push_back(createCombobox(kPluginsComponentID, m_pluginNames.data(), m_pluginNames.size(), index));
        nanoem_application_plugin_status_assign_success(status);
    }
    void
    clearUIWindowLayout()
    {
        for (auto component : m_components) {
            destroyComponent(component);
        }
        m_components.clear();
    }
    nanoem_rsize_t
    getUIWindowLayoutDataSize() const
    {
        Nanoem__Application__Plugin__UIWindow window = NANOEM__APPLICATION__PLUGIN__UIWINDOW__INIT;
        window.n_items = m_components.size();
        window.items = const_cast<Nanoem__Application__Plugin__UIComponent **>(m_components.data());
        return nanoem__application__plugin__uiwindow__get_packed_size(&window);
    }
    void
    getUIWindowLayoutData(nanoem_u8_t *data, nanoem_u32_t length, nanoem_application_plugin_status_t *status)
    {
        Nanoem__Application__Plugin__UIWindow window = NANOEM__APPLICATION__PLUGIN__UIWINDOW__INIT;
        window.n_items = m_components.size();
        window.items = const_cast<Nanoem__Application__Plugin__UIComponent **>(m_components.data());
        if (nanoem__application__plugin__uiwindow__get_packed_size(&window) <= length) {
            nanoem__application__plugin__uiwindow__pack(&window, data);
        }
        else {
            nanoem_application_plugin_status_assign_error(status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
        }
    }
    void
    setUIComponentData(const char *id, const nanoem_u8_t *data, nanoem_u32_t length)
    {
        if (Nanoem__Application__Plugin__UIComponent *component =
                nanoem__application__plugin__uicomponent__unpack(nullptr, length, data)) {
            if (StringUtils::equals(id, kPluginsComponentID)) {
                const auto combobox = component->combo_box;
                auto selectable = combobox->selectables[combobox->selected_index];
                setCurrentPluginName(selectable->text);
            }
            nanoem__application__plugin__uicomponent__free_unpacked(component, nullptr);
        }
    }

    static void
    populateAllAvailableExtensions(HMODULE plugin, OUTPUT_PLUGIN_TABLE *&table, std::vector<char *> &videoExtensions)
    {
        if (auto GetOutputPluginTable =
                reinterpret_cast<PFN_GetOutputPluginTable>(GetProcAddress(plugin, "GetOutputPluginTable"))) {
            table = GetOutputPluginTable();
            LPSTR filter = _strdup(table->filefilter);
            char *s = strchr(filter, '('), *e = strchr(filter, ')');
            if (s && e) {
                *e = '\0';
                char *context = nullptr;
                const char *p = strtok_s(s + 1, ";", &context);
                while (p != nullptr) {
                    videoExtensions.push_back(_strdup(p + 2));
                    p = strtok_s(nullptr, ";", &context);
                }
            }
            free(filter);
        }
        else {
            table = nullptr;
        }
    }
    static char *
    toAllocatedString(const std::wstring &v)
    {
        int length = WideCharToMultiByte(CP_UTF8, 0, v.data(), v.size(), nullptr, 0, 0, 0);
        char *s = new char[length + 1];
        WideCharToMultiByte(CP_UTF8, 0, v.data(), v.size(), s, length, 0, 0);
        s[length] = 0;
        return s;
    }
    void
    setCurrentPluginName(const char *value)
    {
        int sourceLength = Inline::saturateInt32(strlen(value)),
            destLength = MultiByteToWideChar(CP_UTF8, 0, value, sourceLength, nullptr, 0);
        m_currentPluginName.resize(destLength + 1);
        MultiByteToWideChar(CP_UTF8, 0, value, sourceLength, m_currentPluginName.data(), destLength);
        m_currentPluginName[destLength] = 0;
    }

    using ByteArray = std::vector<nanoem_u8_t>;
    using ComponentList = std::vector<Nanoem__Application__Plugin__UIComponent *>;
    struct PluginTable {
        HMODULE m_handle;
        OUTPUT_PLUGIN_TABLE *m_table;
        std::vector<char *> m_videoExtensions;
        void
        destroy()
        {
            for (auto it : m_videoExtensions) {
                free(it);
            }
            m_videoExtensions.clear();
            if (m_handle) {
                FreeLibrary(m_handle);
                m_handle = nullptr;
            }
        }
    };

    ComponentList m_components;
    HANDLE m_threadHandle;
    HANDLE m_audioQueueEventHandle;
    HANDLE m_videoQueueEventHandle;
    CRITICAL_SECTION m_audioQueueSection;
    CRITICAL_SECTION m_videoQueueSection;
    OUTPUT_INFO m_info;
    ByteArray m_audioBuffers;
    std::unordered_map<nanoem_frame_index_t, ByteArray> m_videoFrames;
    std::unordered_map<std::wstring, PluginTable> m_plugins;
    std::vector<char *> m_pluginNames;
    std::vector<wchar_t> m_currentPluginName;
    char *m_outputVideoFilePath;
    nanoem_u32_t m_yflip;
    nanoem_u32_t m_consumedAudioSamples;
    bool m_interrupted;
};

AviUtlOutputPluginEncoder *AviUtlOutputPluginEncoder::s_self = nullptr;
const char AviUtlOutputPluginEncoder::kPluginsComponentID[] = "plugins";

struct AviUtlInputPluginDecoder {
    typedef INPUT_PLUGIN_TABLE *(WINAPI *PFN_GetInputPluginTable)(void);

    AviUtlInputPluginDecoder()
        : m_pluginHandle(nullptr)
        , m_table(nullptr)
        , m_inputHandle(nullptr)
        , m_fps(0)
        , m_actualFPS(0)
    {
        m_pluginHandle = LoadLibraryW(L"C:/Users/hkrn2/Dropbox/windows/Applications/aviutl100/plugins/lwinput.aui");
        if (m_pluginHandle) {
            if (auto GetInputPluginTable =
                    reinterpret_cast<PFN_GetInputPluginTable>(GetProcAddress(m_pluginHandle, "GetInputPluginTable"))) {
                m_table = GetInputPluginTable();
                if (m_table) {
                    LPSTR filter = _strdup(m_table->filefilter);
                    char *s = strchr(filter, '('), *e = strchr(filter, ')');
                    if (s && e) {
                        char *context = nullptr;
                        const char *p = strtok_s(s + 1, ";", &context);
                        while (p != nullptr) {
                            m_audioExtensions.push_back(_strdup(p + 2));
                            m_videoExtensions.push_back(_strdup(p + 2));
                            p = strtok_s(nullptr, ";", &context);
                        }
                    }
                    free(filter);
                    if (auto funcInit = m_table->func_init) {
                        funcInit();
                    }
                }
            }
        }
    }
    ~AviUtlInputPluginDecoder()
    {
        for (auto it : m_audioExtensions) {
            free(it);
        }
        m_audioExtensions.clear();
        for (auto it : m_videoExtensions) {
            free(it);
        }
        m_videoExtensions.clear();
        if (m_table) {
            if (auto funcExit = m_table->func_exit) {
                funcExit();
            }
            m_table = nullptr;
        }
        if (m_pluginHandle) {
            FreeLibrary(m_pluginHandle);
            m_pluginHandle = nullptr;
        }
    }

    int
    open(const char *filePath, nanoem_application_plugin_status_t *status)
    {
        char *newFilePath = _strdup(filePath);
        if (m_table) {
            m_inputHandle = m_table->func_open(newFilePath);
            if (m_inputHandle != nullptr) {
                INPUT_INFO info = {};
                if (m_table->func_info_get(m_inputHandle, &info)) {
                    m_actualFPS = info.rate / static_cast<nanoem_f32_t>(info.scale);
                }
            }
        }
        else {
            nanoem_application_plugin_status_assign_error(status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
        }
        free(newFilePath);
        return m_inputHandle != nullptr;
    }
    void
    setOption(nanoem_u32_t key, const void *value, nanoem_rsize_t size, nanoem_application_plugin_status_t *status)
    {
        switch (key) {
        case NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_FPS: {
            if (Inline::validateArgument<nanoem_u32_t>(value, size, status)) {
                m_fps = *static_cast<const nanoem_u32_t *>(value);
            }
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_AUDIO_LOCATION: {
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_DECODER_OPTION_VIDEO_LOCATION: {
            break;
        }
        default:
            nanoem_application_plugin_status_assign_error(
                status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_UNKNOWN_OPTION);
            break;
        }
    }
    const char *const *
    allAvailableAudioFormatExtensions(nanoem_rsize_t *length) const
    {
        const char *const *values = nullptr;
        if (!m_audioExtensions.empty()) {
            values = m_audioExtensions.data();
            *length = m_audioExtensions.size();
        }
        else {
            *length = 0;
        }
        return values;
    }
    const char *const *
    allAvailableVideoFormatExtensions(nanoem_rsize_t *length) const
    {
        const char *const *values = nullptr;
        if (!m_videoExtensions.empty()) {
            values = m_videoExtensions.data();
            *length = m_videoExtensions.size();
        }
        else {
            *length = 0;
        }
        return values;
    }
    void
    audioFormatValue(nanoem_u32_t key, void *value, nanoem_rsize_t *size, nanoem_application_plugin_status_t *status)
    {
        INPUT_INFO info = {};
        if (!m_table || !m_table->func_info_get(m_inputHandle, &info) || !info.audio_format) {
            key = static_cast<nanoem_u32_t>(NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_UNKNOWN);
        }
        switch (key) {
        case NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_NUM_BITS: {
            nanoem_u32_t bits = info.audio_format->wBitsPerSample;
            Inline::assignOption(bits, value, size, status);
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_NUM_CHANNELS: {
            nanoem_u32_t channels = info.audio_format->nChannels;
            Inline::assignOption(channels, value, size, status);
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_FREQUENCY: {
            nanoem_u32_t frequency = info.audio_format->nSamplesPerSec;
            Inline::assignOption(frequency, value, size, status);
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_DECODER_AUDIO_FORMAT_DURATION: {
            nanoem_frame_index_t duration = info.audio_n / info.audio_format->nSamplesPerSec;
            Inline::assignOption(duration, value, size, status);
            break;
        }
        default:
            nanoem_application_plugin_status_assign_error(
                status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_UNKNOWN_OPTION);
            if (size) {
                *size = 0;
            }
            break;
        }
    }
    void
    videoFormatValue(nanoem_u32_t key, void *value, nanoem_rsize_t *size, nanoem_application_plugin_status_t *status)
    {
        INPUT_INFO info = {};
        if (!m_table || !m_table->func_info_get(m_inputHandle, &info) || !info.format) {
            key = static_cast<nanoem_u32_t>(NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_UNKNOWN);
        }
        switch (key) {
        case NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_WIDTH: {
            nanoem_u32_t width = Inline::roundInt32(info.format->biWidth);
            Inline::assignOption(width, value, size, status);
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_HEIGHT: {
            nanoem_u32_t height = Inline::roundInt32(info.format->biHeight);
            Inline::assignOption(height, value, size, status);
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_STRIDE: {
            nanoem_u32_t stride = 0;
            Inline::assignOption(stride, value, size, status);
            break;
        }
        case NANOEM_APPLICATION_PLUGIN_DECODER_VIDEO_FORMAT_DURATION: {
            nanoem_frame_index_t duration = Inline::roundInt32(info.n);
            Inline::assignOption(duration, value, size, status);
            break;
        }
        default:
            nanoem_application_plugin_status_assign_error(
                status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_UNKNOWN_OPTION);
            if (size) {
                *size = 0;
            }
            break;
        }
    }
    void
    decodeAudioFrame(nanoem_frame_index_t currentFrameIndex, nanoem_u8_t **data, nanoem_rsize_t *size,
        nanoem_application_plugin_status_t *status)
    {
        if (m_table && (m_table->flag & INPUT_PLUGIN_FLAG_AUDIO) != 0 && m_inputHandle) {
            INPUT_INFO info = {};
            m_table->func_info_get(m_inputHandle, &info);
            if (m_table->func_info_get(m_inputHandle, &info) && (info.flag & INPUT_INFO_FLAG_AUDIO) != 0 &&
                info.audio_format) {
                nanoem_rsize_t s = info.audio_format->nSamplesPerSec / m_fps;
                if (s > 0) {
                    const nanoem_f32_t scaleFactor = (m_actualFPS / m_fps);
                    int offset = s * static_cast<int>(currentFrameIndex * scaleFactor);
                    nanoem_u8_t *dataPtr = *data = new nanoem_u8_t[s];
                    memset(dataPtr, 0, s);
                    m_table->func_read_audio(m_inputHandle, offset, s, dataPtr);
                    *size = s;
                }
                else {
                    nanoem_application_plugin_status_assign_error(
                        status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
                }
            }
            else {
                nanoem_application_plugin_status_assign_error(
                    status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
            }
        }
        else {
            nanoem_application_plugin_status_assign_error(status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
        }
    }
    void
    decodeVideoFrame(nanoem_frame_index_t currentFrameIndex, nanoem_u8_t **data, nanoem_rsize_t *size,
        nanoem_application_plugin_status_t *status)
    {
        if (m_table && (m_table->flag & INPUT_PLUGIN_FLAG_VIDEO) != 0 && m_inputHandle) {
            INPUT_INFO info = {};
            if (m_table->func_info_get(m_inputHandle, &info) && (info.flag & INPUT_INFO_FLAG_VIDEO) != 0 &&
                info.format) {
                const BITMAPINFOHEADER *format = info.format;
                const nanoem_rsize_t width = Inline::roundInt32(format->biWidth),
                                     height = Inline::roundInt32(format->biHeight), length = width * height;
                if (length > 0 && width % 2 == 0 && format->biCompression == nanoem_fourcc('Y', 'U', 'Y', '2') &&
                    format->biBitCount == 16) {
                    const nanoem_rsize_t pixelYUVSize = length * 2, pixelRGBSize = length * 4;
                    nanoem_u8_t *pixelsYUV = new nanoem_u8_t[pixelYUVSize], *pixelsRGB = new nanoem_u8_t[pixelRGBSize];
                    memset(pixelsRGB, 0, pixelRGBSize);
                    const nanoem_f32_t scaleFactor = (m_actualFPS / m_fps);
                    m_table->func_read_video(
                        m_inputHandle, static_cast<int>(currentFrameIndex * scaleFactor), pixelsYUV);
                    auto yuv2rgb = [pixelsRGB](nanoem_rsize_t offset, nanoem_u8_t y, nanoem_u8_t u, nanoem_u8_t v) {
                        const char u0 = u - 128, v0 = v - 128;
                        pixelsRGB[offset + 0] = y + (1.402f * v0);
                        pixelsRGB[offset + 1] = y - (0.34414f * u0) - (0.71414f * v0);
                        pixelsRGB[offset + 2] = y + (1.772f * u0);
                        pixelsRGB[offset + 3] = 0xff;
                    };
                    for (nanoem_rsize_t i = 0; i < length / 2; i++) {
                        nanoem_rsize_t offsetYUV = i * 4;
                        nanoem_u8_t y1 = pixelsYUV[offsetYUV + 0];
                        nanoem_u8_t u = pixelsYUV[offsetYUV + 1];
                        nanoem_u8_t y2 = pixelsYUV[offsetYUV + 2];
                        nanoem_u8_t v = pixelsYUV[offsetYUV + 3];
                        nanoem_rsize_t offsetRGB = i * 8;
                        yuv2rgb(offsetRGB + 0, y1, u, v);
                        yuv2rgb(offsetRGB + 4, y2, u, v);
                    }
                    *data = pixelsRGB;
                    *size = pixelRGBSize;
                    delete[] pixelsYUV;
                }
                else {
                    nanoem_application_plugin_status_assign_error(
                        status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
                }
            }
            else {
                nanoem_application_plugin_status_assign_error(
                    status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
            }
        }
        else {
            nanoem_application_plugin_status_assign_error(status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
        }
    }
    void
    destroyAudioFrame(nanoem_frame_index_t /* currentFrameIndex */, nanoem_u8_t *data, nanoem_rsize_t /* size */)
    {
        if (data) {
            delete[] data;
        }
    }
    void
    destroyVideoFrame(nanoem_frame_index_t /* currentFrameIndex */, nanoem_u8_t *data, nanoem_rsize_t /* size */)
    {
        if (data) {
            delete[] data;
        }
    }
    const char *
    failureReason() const
    {
        return nullptr;
    }
    const char *
    recoverySuggestion() const
    {
        return nullptr;
    }
    int
    close(nanoem_application_plugin_status_t *status)
    {
        int closed = 0;
        if (m_table) {
            closed = m_table->func_close(m_inputHandle);
        }
        else {
            nanoem_application_plugin_status_assign_error(status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
        }
        return closed;
    }

    HMODULE m_pluginHandle;
    INPUT_PLUGIN_TABLE *m_table;
    INPUT_HANDLE m_inputHandle;
    std::vector<char *> m_audioExtensions;
    std::vector<char *> m_videoExtensions;
    nanoem_u32_t m_fps;
    nanoem_f32_t m_actualFPS;
};

} /* namespace anonymous */

struct nanoem_application_plugin_encoder_t : AviUtlOutputPluginEncoder { };

struct nanoem_application_plugin_decoder_t : AviUtlInputPluginDecoder { };

nanoem_u32_t APIENTRY
nanoemApplicationPluginEncoderGetABIVersion()
{
    return NANOEM_APPLICATION_PLUGIN_ENCODER_ABI_VERSION;
}

void APIENTRY
nanoemApplicationPluginEncoderInitialize()
{
}

nanoem_application_plugin_encoder_t *APIENTRY
nanoemApplicationPluginEncoderCreate()
{
    nanoem_application_plugin_encoder_t *opaque = new nanoem_application_plugin_encoder_t();
    return opaque;
}

int APIENTRY
nanoemApplicationPluginEncoderOpen(
    nanoem_application_plugin_encoder_t *encoder, const char *filePath, nanoem_application_plugin_status_t *status)
{
    int result = 0;
    if (nanoem_is_not_null(encoder)) {
        result = encoder->open(filePath, status);
    }
    else {
        nanoem_application_plugin_status_assign_error(status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
    return result;
}

void APIENTRY
nanoemApplicationPluginEncoderSetOption(nanoem_application_plugin_encoder_t *encoder, nanoem_u32_t key,
    const void *value, nanoem_rsize_t size, nanoem_application_plugin_status_t *status)
{
    if (nanoem_is_not_null(encoder)) {
        encoder->setOption(key, value, size, status);
    }
    else {
        nanoem_application_plugin_status_assign_error(status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

void APIENTRY
nanoemApplicationPluginEncoderEncodeAudioFrame(nanoem_application_plugin_encoder_t *encoder,
    nanoem_frame_index_t currentFrameIndex, const nanoem_u8_t *data, nanoem_rsize_t size,
    nanoem_application_plugin_status_t *status)
{
    if (nanoem_is_not_null(encoder)) {
        encoder->encodeAudioFrame(currentFrameIndex, data, size, status);
    }
    else {
        nanoem_application_plugin_status_assign_error(status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

void APIENTRY
nanoemApplicationPluginEncoderEncodeVideoFrame(nanoem_application_plugin_encoder_t *encoder,
    nanoem_frame_index_t currentFrameIndex, const nanoem_u8_t *data, nanoem_rsize_t size,
    nanoem_application_plugin_status_t *status)
{
    if (nanoem_is_not_null(encoder)) {
        encoder->encodeVideoFrame(currentFrameIndex, data, size, status);
    }
    else {
        nanoem_application_plugin_status_assign_error(status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

void APIENTRY
nanoemApplicationPluginEncoderInterrupt(
    nanoem_application_plugin_encoder_t *encoder, nanoem_application_plugin_status_t *status)
{
    if (nanoem_is_not_null(encoder)) {
        encoder->interrupt(status);
    }
    else {
        nanoem_application_plugin_status_assign_error(status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

const char *const *APIENTRY
nanoemApplicationPluginEncoderGetAllAvailableVideoFormatExtensions(
    const nanoem_application_plugin_encoder_t *encoder, nanoem_rsize_t *length)
{
    const char *const *values = nullptr;
    if (nanoem_is_not_null(encoder) && nanoem_is_not_null(length)) {
        values = encoder->allAvailableVideoFormatExtensions(length);
    }
    return values;
}

void APIENTRY
nanoemApplicationPluginEncoderLoadUIWindowLayout(
    nanoem_application_plugin_encoder_t *plugin, nanoem_application_plugin_status_t *status)
{
    if (nanoem_likely(plugin)) {
        plugin->loadUIWindowLayout(status);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginEncoderGetUIWindowLayoutDataSize(
    nanoem_application_plugin_encoder_t *plugin, nanoem_u32_t *length)
{
    if (nanoem_likely(plugin && length)) {
        *length = Inline::saturateInt32U(plugin->getUIWindowLayoutDataSize());
    }
}

void APIENTRY
nanoemApplicationPluginEncoderGetUIWindowLayoutData(nanoem_application_plugin_encoder_t *plugin, nanoem_u8_t *data,
    nanoem_u32_t length, nanoem_application_plugin_status_t *status)
{
    if (nanoem_likely(plugin && data)) {
        plugin->getUIWindowLayoutData(data, length, status);
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

void APIENTRY
nanoemApplicationPluginEncoderSetUIComponentLayoutData(nanoem_application_plugin_encoder_t *plugin, const char *id,
    const nanoem_u8_t *data, nanoem_u32_t length, int *reloadLayout, nanoem_application_plugin_status_t *status)
{
    if (nanoem_likely(plugin && id && data && reloadLayout)) {
        plugin->setUIComponentData(id, data, length);
        *reloadLayout = 1;
    }
    else if (status) {
        *status = NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT;
    }
}

const char *APIENTRY
nanoemApplicationPluginEncoderGetFailureReason(const nanoem_application_plugin_encoder_t *encoder)
{
    return nanoem_is_not_null(encoder) ? encoder->failureReason() : nullptr;
}

const char *APIENTRY
nanoemApplicationPluginEncoderGetRecoverySuggestion(const nanoem_application_plugin_encoder_t *encoder)
{
    return nanoem_is_not_null(encoder) ? encoder->recoverySuggestion() : nullptr;
}

int APIENTRY
nanoemApplicationPluginEncoderClose(
    nanoem_application_plugin_encoder_t *encoder, nanoem_application_plugin_status_t *status)
{
    int result = 0;
    if (nanoem_is_not_null(encoder)) {
        result = encoder->close(status);
    }
    else {
        nanoem_application_plugin_status_assign_error(status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
    return result;
}

void APIENTRY
nanoemApplicationPluginEncoderDestroy(nanoem_application_plugin_encoder_t *encoder)
{
    delete encoder;
}

void APIENTRY
nanoemApplicationPluginEncoderTerminate()
{
}

nanoem_u32_t APIENTRY
nanoemApplicationPluginDecoderGetABIVersion()
{
    return NANOEM_APPLICATION_PLUGIN_DECODER_ABI_VERSION;
}

void APIENTRY
nanoemApplicationPluginDecoderInitialize()
{
}

nanoem_application_plugin_decoder_t *APIENTRY
nanoemApplicationPluginDecoderCreate()
{
    return new nanoem_application_plugin_decoder_t();
}

int APIENTRY
nanoemApplicationPluginDecoderOpen(
    nanoem_application_plugin_decoder_t *decoder, const char *filePath, nanoem_application_plugin_status_t *status)
{
    int result = 0;
    if (nanoem_is_not_null(decoder)) {
        result = decoder->open(filePath, status);
    }
    else {
        nanoem_application_plugin_status_assign_error(status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
    return result;
}

void APIENTRY
nanoemApplicationPluginDecoderSetOption(nanoem_application_plugin_decoder_t *decoder, nanoem_u32_t key,
    const void *value, nanoem_rsize_t size, nanoem_application_plugin_status_t *status)
{
    if (nanoem_is_not_null(decoder)) {
        decoder->setOption(key, value, size, status);
    }
    else {
        nanoem_application_plugin_status_assign_error(status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

void APIENTRY
nanoemApplicationPluginDecoderGetAudioFormatValue(nanoem_application_plugin_decoder_t *decoder, nanoem_u32_t key,
    void *value, nanoem_rsize_t *size, nanoem_application_plugin_status_t *status)
{
    if (nanoem_is_not_null(decoder)) {
        decoder->audioFormatValue(key, value, size, status);
    }
    else {
        nanoem_application_plugin_status_assign_error(status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

void APIENTRY
nanoemApplicationPluginDecoderGetVideoFormatValue(nanoem_application_plugin_decoder_t *decoder, nanoem_u32_t key,
    void *value, nanoem_rsize_t *size, nanoem_application_plugin_status_t *status)
{
    if (nanoem_is_not_null(decoder)) {
        decoder->videoFormatValue(key, value, size, status);
    }
    else {
        nanoem_application_plugin_status_assign_error(status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

void APIENTRY
nanoemApplicationPluginDecoderDecodeAudioFrame(nanoem_application_plugin_decoder_t *decoder,
    nanoem_frame_index_t currentFrameIndex, nanoem_u8_t **data, nanoem_rsize_t *size,
    nanoem_application_plugin_status_t *status)
{
    if (nanoem_is_not_null(decoder)) {
        decoder->decodeAudioFrame(currentFrameIndex, data, size, status);
    }
    else {
        nanoem_application_plugin_status_assign_error(status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

void APIENTRY
nanoemApplicationPluginDecoderDecodeVideoFrame(nanoem_application_plugin_decoder_t *decoder,
    nanoem_frame_index_t currentFrameIndex, nanoem_u8_t **data, nanoem_rsize_t *size,
    nanoem_application_plugin_status_t *status)
{
    if (nanoem_is_not_null(decoder)) {
        decoder->decodeVideoFrame(currentFrameIndex, data, size, status);
    }
    else {
        nanoem_application_plugin_status_assign_error(status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
}

void APIENTRY
nanoemApplicationPluginDecoderDestroyAudioFrame(nanoem_application_plugin_decoder_t *decoder,
    nanoem_frame_index_t currentFrameIndex, nanoem_u8_t *data, nanoem_rsize_t size)
{
    if (nanoem_is_not_null(decoder)) {
        decoder->destroyAudioFrame(currentFrameIndex, data, size);
    }
}

void APIENTRY
nanoemApplicationPluginDecoderDestroyVideoFrame(nanoem_application_plugin_decoder_t *decoder,
    nanoem_frame_index_t currentFrameIndex, nanoem_u8_t *data, nanoem_rsize_t size)
{
    if (nanoem_is_not_null(decoder)) {
        decoder->destroyVideoFrame(currentFrameIndex, data, size);
    }
}

const char *const *APIENTRY
nanoemApplicationPluginDecoderGetAllAvailableAudioFormatExtensions(
    nanoem_application_plugin_decoder_t *decoder, nanoem_rsize_t *length)
{
    const char *const *values = nullptr;
    if (nanoem_is_not_null(decoder) && nanoem_is_not_null(length)) {
        values = decoder->allAvailableAudioFormatExtensions(length);
    }
    return values;
}

const char *const *APIENTRY
nanoemApplicationPluginDecoderGetAllAvailableVideoFormatExtensions(
    nanoem_application_plugin_decoder_t *decoder, nanoem_rsize_t *length)
{
    const char *const *values = nullptr;
    if (nanoem_is_not_null(decoder) && nanoem_is_not_null(length)) {
        values = decoder->allAvailableVideoFormatExtensions(length);
    }
    return values;
}

const char *APIENTRY
nanoemApplicationPluginDecoderGetFailureReason(nanoem_application_plugin_decoder_t *decoder)
{
    return nanoem_is_not_null(decoder) ? decoder->failureReason() : nullptr;
}

const char *APIENTRY
nanoemApplicationPluginDecoderGetRecoverySuggestion(nanoem_application_plugin_decoder_t *decoder)
{
    return nanoem_is_not_null(decoder) ? decoder->recoverySuggestion() : nullptr;
}

int APIENTRY
nanoemApplicationPluginDecoderClose(
    nanoem_application_plugin_decoder_t *decoder, nanoem_application_plugin_status_t *status)
{
    int result = 0;
    if (nanoem_is_not_null(decoder)) {
        result = decoder->close(status);
    }
    else {
        nanoem_application_plugin_status_assign_error(status, NANOEM_APPLICATION_PLUGIN_STATUS_ERROR_NULL_OBJECT);
    }
    return result;
}

void APIENTRY
nanoemApplicationPluginDecoderDestroy(nanoem_application_plugin_decoder_t *decoder)
{
    delete decoder;
}

void APIENTRY
nanoemApplicationPluginDecoderTerminate()
{
}
