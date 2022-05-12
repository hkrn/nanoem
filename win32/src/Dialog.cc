/*
   Copyright (c) 2015-2021 hkrn All rights reserved

   This file is part of emapp component and it's licensed under Mozilla Public License. see LICENSE.md for more details.
 */

#include "Dialog.h"

#define STRICT_TYPED_ITEMIDS
#include <ShObjIdl.h>
#include <Shlwapi.h>

#include "emapp/FileUtils.h"
#include "emapp/StringUtils.h"
#include "emapp/private/CommonInclude.h"

namespace nanoem {
namespace win32 {

namespace {
class DialogEventHandler : public IFileDialogEvents {
public:
    virtual ~DialogEventHandler()
    {
    }
    // IUnknown methods
    IFACEMETHODIMP
    QueryInterface(REFIID riid, void **ppv)
    {
        static const QITAB qit[] = {
            QITABENT(DialogEventHandler, IFileDialogEvents),
            { nullptr, 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }
    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&m_ref);
    }
    IFACEMETHODIMP_(ULONG) Release()
    {
        ULONG m_ref = InterlockedDecrement(&m_ref);
        if (m_ref <= 0) {
            delete this;
        }
        return m_ref;
    }
    IFACEMETHODIMP
    OnFileOk(IFileDialog *)
    {
        return S_OK;
    }
    IFACEMETHODIMP
    OnFolderChange(IFileDialog *)
    {
        return S_OK;
    }
    IFACEMETHODIMP
    OnFolderChanging(IFileDialog *, IShellItem *)
    {
        return S_OK;
    }
    IFACEMETHODIMP
    OnHelp(IFileDialog *)
    {
        return S_OK;
    }
    IFACEMETHODIMP
    OnSelectionChange(IFileDialog *)
    {
        return S_OK;
    }
    IFACEMETHODIMP
    OnShareViolation(IFileDialog *, IShellItem *, FDE_SHAREVIOLATION_RESPONSE *)
    {
        return S_OK;
    }
    IFACEMETHODIMP
    OnTypeChange(IFileDialog *)
    {
        return S_OK;
    }
    IFACEMETHODIMP
    OnOverwrite(IFileDialog *, IShellItem *, FDE_OVERWRITE_RESPONSE *)
    {
        return S_OK;
    }

private:
    ULONG m_ref = 0;
};
} /* namespace anonymous */

Dialog::Dialog(HWND hwnd)
    : m_windowHandle(hwnd)
{
}

Dialog::~Dialog()
{
}

bool
Dialog::open(const char *format, const StringList &extensions)
{
    Dialog::FilterList filters;
    char buffer[1024];
    MutableWideString allExtensions;
    for (size_t i = 0, numItems = extensions.size(); i < numItems; i++) {
        MutableWideString extensionWS;
        const char *ext = extensions[i].c_str();
        bx::snprintf(buffer, sizeof(buffer), format, ext);
        StringUtils::getWideCharString(ext, extensionWS);
        if (!extensionWS.empty()) {
            allExtensions.push_back(L'*');
            allExtensions.push_back(L'.');
            allExtensions.insert(allExtensions.end(), extensionWS.data(), extensionWS.data() + extensionWS.size() - 1);
            allExtensions.push_back(L';');
        }
    }
    if (!allExtensions.empty()) {
        allExtensions[allExtensions.size() - 1] = 0;
    }
    filters.push_back(COMDLG_FILTERSPEC { L"All Available Extensions", allExtensions.data() });
    std::vector<std::pair<MutableWideString, MutableWideString>> ws;
    ws.resize(extensions.size());
    for (size_t i = 0, numItems = ws.size(); i < numItems; i++) {
        MutableWideString &name = ws[i].first, &extension = ws[i].second, extensionWS;
        const char *ext = extensions[i].c_str();
        bx::snprintf(buffer, sizeof(buffer), format, ext);
        StringUtils::getWideCharString(buffer, name);
        StringUtils::getWideCharString(ext, extensionWS);
        if (!extensionWS.empty()) {
            extension.push_back(L'*');
            extension.push_back(L'.');
            extension.insert(extension.end(), extensionWS.data(), extensionWS.data() + extensionWS.size());
            extension.push_back(0);
            filters.push_back(COMDLG_FILTERSPEC { name.data(), extension.data() });
        }
    }
    return open(filters);
}

bool
Dialog::open(const FilterList &filter)
{
    IFileOpenDialog *dialog = nullptr;
    bool result = false;
    if (!FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog)))) {
#ifndef NDEBUG
        FilterList newFilters(filter);
        newFilters.push_back(COMDLG_FILTERSPEC { L"Any Files", L"*" });
        result = showDialog(dialog, newFilters, nullptr, 0);
#else
        result = showDialog(dialog, filter, nullptr, 0);
#endif
    }
    else {
        OPENFILENAMEW ofn = {};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = m_windowHandle;
        ofn.Flags = OFN_FILEMUSTEXIST;
        result = GetOpenFileNameW(&ofn) != 0;
    }
    return result;
}

bool
Dialog::save(const char *format, const StringList &extensions, const char *defaultFileName)
{
    Dialog::FilterList filters;
    std::vector<std::pair<MutableWideString, MutableWideString>> ws;
    ws.resize(extensions.size());
    char buffer[1024];
    for (size_t i = 0, numItems = ws.size(); i < numItems; i++) {
        MutableWideString &name = ws[i].first, &extension = ws[i].second, extensionWS;
        const char *ext = extensions[i].c_str();
        bx::snprintf(buffer, sizeof(buffer), format, ext);
        StringUtils::getWideCharString(buffer, name);
        StringUtils::getWideCharString(ext, extensionWS);
        extension.push_back(L'*');
        extension.push_back(L'.');
        extension.insert(extension.end(), extensionWS.data(), extensionWS.data() + extensionWS.size());
        extension.push_back(0);
        filters.push_back(COMDLG_FILTERSPEC { name.data(), extension.data() });
    }
    return save(filters, defaultFileName);
}

bool
Dialog::save(const FilterList &filter, const char *defaultFileName)
{
    IFileSaveDialog *dialog = nullptr;
    bool result = false;
    if (!FAILED(CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog)))) {
        result = showDialog(dialog, filter, defaultFileName, FOS_STRICTFILETYPES);
    }
    else {
        OPENFILENAMEW ofn = {};
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = m_windowHandle;
        ofn.Flags = OFN_OVERWRITEPROMPT;
        result = GetSaveFileNameW(&ofn) != 0;
    }
    return result;
}

const wchar_t *
Dialog::filename() const
{
    return m_filePath.data();
}

URI
Dialog::fileURI() const
{
    MutableString path;
    StringUtils::getMultiBytesString(filename(), path);
    FileUtils::canonicalizePathSeparator(path);
    return URI::createFromFilePath(path.data());
}

bool
Dialog::showDialog(IFileDialog *dialog, const FilterList &filter, const char *defaultFileName, DWORD newFlags)
{
    DialogEventHandler *events = new DialogEventHandler();
    DWORD cookie, flags;
    dialog->Advise(events, &cookie);
    dialog->GetOptions(&flags);
    dialog->SetOptions(flags | FOS_FORCEFILESYSTEM | newFlags);
    if (defaultFileName) {
        MutableWideString ws;
        StringUtils::getWideCharString(defaultFileName, ws);
        dialog->SetFileName(ws.data());
    }
    if (!filter.empty()) {
        dialog->SetFileTypes(Inline::saturateInt32U(filter.size()), filter.data());
        if (const wchar_t *p = wcschr(filter[0].pszSpec, L'.')) {
            dialog->SetDefaultExtension(p + 1);
        }
    }
    HRESULT rc = dialog->Show(m_windowHandle);
    bool opened = !FAILED(rc);
    if (opened) {
        IShellItem *result = nullptr;
        dialog->GetResult(&result);
        PWSTR filePath = nullptr;
        rc = result->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
        if (!FAILED(rc)) {
            m_filePath = MutableWideString(filePath, filePath + wcslen(filePath) + 1);
        }
        CoTaskMemFree(filePath);
        safeRelease(result);
        dialog->Unadvise(cookie);
        safeRelease(dialog);
    }
    return opened;
}

} /* namespace win32 */
} /* namespace nanoem */
