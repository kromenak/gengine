#include "OSDialog.h"

#include <SDL_messagebox.h>

#include "BuildEnv.h"

namespace
{
    Uint32 MessageBoxTypeToSDLFlag(OSDialogType type)
    {
        if(type == OSDIALOG_INFO)
        {
            return SDL_MESSAGEBOX_INFORMATION;
        }
        else if(type == OSDIALOG_WARNING)
        {
            return SDL_MESSAGEBOX_WARNING;
        }
        return SDL_MESSAGEBOX_ERROR;
    }
}

void OSDialog::Ok(OSDialogType type, const std::string& message)
{
    Ok(type, APPLICATION_NAME, message);
}

void OSDialog::Ok(OSDialogType type, const std::string& title, const std::string& message)
{
    int result = SDL_ShowSimpleMessageBox(MessageBoxTypeToSDLFlag(type),
                                          title.c_str(),
                                          message.c_str(),
                                          nullptr);
    if(result != 0)
    {
        printf("Failed to show message box with error: %s\n", SDL_GetError());
    }
}

bool OSDialog::YesNo(OSDialogType type, const std::string& message)
{
    return YesNo(type, APPLICATION_NAME, message);
}

bool OSDialog::YesNo(OSDialogType type, const std::string& title, const std::string& message)
{
    return YesNo(type, title, message, "Yes", "No");
}

bool OSDialog::YesNo(OSDialogType type, const std::string& title, const std::string& message, const std::string& leftButtonText, const std::string& rightButtonText)
{
    SDL_MessageBoxData messageBoxData { };
    messageBoxData.flags = MessageBoxTypeToSDLFlag(type);
    messageBoxData.title = title.c_str();
    messageBoxData.message = message.c_str();
    messageBoxData.numbuttons = 2;

    SDL_MessageBoxButtonData buttons[2] = { 0 };
    buttons[1].buttonid = 1;
    buttons[1].text = leftButtonText.c_str();
    buttons[0].buttonid = 0;
    buttons[0].text = rightButtonText.c_str();
    messageBoxData.buttons = buttons;

    int buttonIdPressed = 0;
    int result = SDL_ShowMessageBox(&messageBoxData, &buttonIdPressed);
    if(result != 0)
    {
        printf("Failed to show message box with error: %s\n", SDL_GetError());
        return false;
    }
    return buttonIdPressed == 1;
}
