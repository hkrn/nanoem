/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#pragma once
#ifndef NANOEM_EMAPP_WIN32_DIALOG_H_
#define NANOEM_EMAPP_WIN32_DIALOG_H_

#include <Windows.h>
#include <commdlg.h>
#include <shtypes.h>
#include <vector>

#include "emapp/URI.h"

struct IFileDialog;

namespace nanoem {
namespace win32 {

class Dialog final {
public:
    using FilterList = std::vector<COMDLG_FILTERSPEC>;

    Dialog(HWND hwnd);
    ~Dialog();

    bool open(const char *format, const StringList &extensions);
    bool open(const FilterList &filter);
    bool save(const char *format, const StringList &extensions, const char *defaultFileName);
    bool save(const FilterList &filter, const char *defaultFileName);
    const wchar_t *filename() const;
    URI fileURI() const;

private:
    template <typename TInterface>
    static void
    safeRelease(TInterface *&ptr)
    {
        if (ptr) {
            ptr->Release();
            ptr = 0;
        }
    }
    bool showDialog(IFileDialog *dialog, const FilterList &filter, const char *defaultFileName, DWORD newFlags);

    HWND m_windowHandle;
    MutableWideString m_filePath;
};

} /* namespace win32 */
} /* namespace nanoem */

#endif /* NANOEM_EMAPP_WIN32_DIALOG_H_ */
