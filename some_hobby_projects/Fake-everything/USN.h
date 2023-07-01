#pragma once

#include <windows.h>
#include <winioctl.h>
#include <iostream>
#include <string>
#include <stdexcept>
#include <map>

/*
*  In order to make the code style uniform , I use std::wstring and the W version of windows's functions.
*  You could get more details about the APIs on MSDN.
*/
class searchEngine {
    HANDLE                                 handle;
    CREATE_USN_JOURNAL_DATA                cujd;
    USN_JOURNAL_DATA                       ujd;
    std::wstring const                     volume;
    std::map<DWORDLONG, std::pair<std::wstring, DWORDLONG>> search_map;

    // only works on NTFS.
    bool is_NTFS() {
        wchar_t buf[MAX_PATH] = L"0";
        bool result = GetVolumeInformationW(
            (volume + L"\\").c_str(),
            nullptr,
            0,
            nullptr,
            nullptr,
            nullptr,
            buf,
            MAX_PATH);           //  #define MAX_PATH 260  <=  minwindef.h

        return (result && std::wstring{ buf } == L"NTFS");
    }

    // requires admin rights
    bool initialize_handle() {
        handle = CreateFileW(
            (L"\\\\.\\" + volume).c_str(),
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_EXISTING,
            0,
            nullptr
        );

        return (handle != INVALID_HANDLE_VALUE);
    }

    bool initialize_CUJD() {
        DWORD bytes_returned = 0;
        bool result = DeviceIoControl(handle,
            FSCTL_CREATE_USN_JOURNAL,
            &cujd,
            sizeof(cujd),
            nullptr,
            0,
            &bytes_returned,
            nullptr);

        return result;
    }

    bool initialize_UJD() {
        DWORD bytes_returned = 0;
        bool result = DeviceIoControl(handle,
            FSCTL_QUERY_USN_JOURNAL,
            nullptr,
            0,
            &ujd,
            sizeof(ujd),
            &bytes_returned,
            nullptr);

        return result;
    }

    bool initialize_map() {
        int const BUF_SIZE = 65536;       // As big as possible.
        char buffer[BUF_SIZE] = "0";
        DWORD bytes_returned = 0;

        /*
        * The first call to FSCTL_ENUM_USN_DATA during an enumeration must have the StartFileReferenceNumber member set to (DWORDLONG)0.
        * Each call to FSCTL_ENUM_USN_DATA retrieves the starting point for the subsequent call as the first entry in the output buffer.
        * Subsequent calls must be made with StartFileReferenceNumber set to this value.(MSDN)
        */
        #if (NTDDI_VERSION >= NTDDI_WIN8)
        MFT_ENUM_DATA med = { 0, 0, ujd.NextUsn, 2, 3 };
        #else
        MFT_ENUM_DATA med = { 0, 0, ujd.NextUsn };
        #endif
        //MFT_ENUM_DATA_V0 med = { 0, 0, ujd.NextUsn };        // Not MFT_ENUM_DATA, that is an alias about MFT_ENUM_DATA_V1.

        for (;;) {
            bool result = DeviceIoControl(handle,
                FSCTL_ENUM_USN_DATA,
                &med,
                sizeof(med),
                buffer,
                BUF_SIZE,
                &bytes_returned,
                nullptr);

            if (!result)
                break;

            DWORD dwRetBytes = bytes_returned - sizeof(USN);
            PUSN_RECORD usn_record = (PUSN_RECORD)((PCHAR)buffer + sizeof(USN));    // To be honest,I want to use static_cast<>.

            while (dwRetBytes > 0) {
                search_map.emplace(
                    usn_record->FileReferenceNumber,
                    make_pair(
                        std::wstring{ usn_record->FileName, usn_record->FileNameLength / (unsigned __int64)2 },
                        usn_record->ParentFileReferenceNumber
                    )
                );

                dwRetBytes -= usn_record->RecordLength;
                usn_record = (PUSN_RECORD)((PCHAR)usn_record + usn_record->RecordLength);
            }

            med.StartFileReferenceNumber = *(USN*)&buffer;
        }

        return (search_map.size() > 0);
    }

    /*
    * This member function is used to get the full name by the file's reference number.
    */
    std::wstring getAbsoluteNameByReferenceNo(DWORDLONG referenceNumber) {
        DWORDLONG number = referenceNumber;
        DWORDLONG parent_number = search_map[number].second;
        std::wstring temp{ search_map[number].first };

        while (search_map.count(parent_number) > 0) {
            temp = search_map[parent_number].first + L"\\" + temp;
            number = parent_number;
            parent_number = search_map[parent_number].second;
        }

        return (volume + L"\\" + temp);
    }
public:
    explicit searchEngine(const std::wstring& volume_)
        : volume{ volume_ }
    {
        if (!is_NTFS())
            throw std::runtime_error{ "Not NTFS !" };
        if (!initialize_handle())           // Again , give me admin rights. 
            throw std::runtime_error{ "Maybe you forget to give me admin rights ?" };
        if (!initialize_CUJD())             // GetLastError() would return an error code , figure out them on MSDN.
            throw std::runtime_error{ "CREATE_USN_JOURNAL_DATA failed ! And error code is " + std::to_string(GetLastError()) };
        if (!initialize_UJD())
            throw std::runtime_error{ "USN_JOURNAL_DATA failed ! And error code is " + std::to_string(GetLastError()) };
        if (!initialize_map())
            throw std::runtime_error{ "Initialize search map failed !" };
    }

    ~searchEngine() {       // Note : Don't throw in destructor.
        DWORD bytes_returned = 0;
        DELETE_USN_JOURNAL_DATA dujd = { ujd.UsnJournalID,USN_DELETE_FLAG_DELETE };

        DeviceIoControl(handle,
            FSCTL_DELETE_USN_JOURNAL,
            &dujd,
            sizeof(dujd),
            nullptr,
            0,
            &bytes_returned,
            nullptr);

        CloseHandle(handle);
    }

    void search(const std::wstring& what) {
        std::wcout.imbue(std::locale(""));          // must do this , otherwise you would get nothing.

        for (auto& [key, value] : search_map) {         // structured bindings , C++17
            if (value.first.find(what) != std::wstring::npos) {
                std::wstring absolute_name = getAbsoluteNameByReferenceNo(key);
                std::wcout << absolute_name;
                std::wcout.clear();                // Sometimes , wcout would set bad bit (especially in other languages,like japanese).
                std::wcout << std::endl;
            }
        }
    }
};