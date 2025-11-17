//
// Clark Kromenaker
//
// Helper for showing common types of OS dialogs.
//
#pragma once
#include <string>

enum OSDialogType
{
    OSDIALOG_INFO,
    OSDIALOG_WARNING,
    OSDIALOG_ERROR
};

namespace OSDialog
{
    void Ok(OSDialogType type, const std::string& message);
    void Ok(OSDialogType type, const std::string& title, const std::string& message);

    bool YesNo(OSDialogType type, const std::string& message);
    bool YesNo(OSDialogType type, const std::string& title, const std::string& message);
    bool YesNo(OSDialogType type, const std::string& title, const std::string& message,
               const std::string& leftButtonText, const std::string& rightButtonText);
}