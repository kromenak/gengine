//
// Clark Kromenaker
//
// UI for the email subscreen in Sidney.
//
#pragma once
#include <string>
#include <vector>

#include "SidneyMenubar.h"
#include "StringUtil.h"

class Actor;
class UILabel;

class SidneyEmail
{
public:
    void Init(Actor* parent);

    void Show();
    void Hide();

    //void OnUpdate(float deltaTime);

private:
    // Root of this subscreen.
    Actor* mRoot = nullptr;

    // The menu bar.
    SidneyMenuBar mMenuBar;

    // The window containing the email list.
    Actor* mEmailListWindow = nullptr;
    std::vector<UILabel*> mEmailListItems;

    Actor* mEmailWindow = nullptr;

    // Represents one email that can be received.
    struct Email
    {
        std::string from;
        std::string to;
        std::string cc;
        std::string date;
        std::string subject;
        std::vector<std::string> body;
    };

    // Contains all emails that can be received and that are known by the game.
    // Key is the email ID specified in SIDNEYEMAIL.TXT.
    std::string_map_ci<Email> mAllEmails;

    // IDs of emails that have been received.
    std::string_set_ci mReceivedEmails;

    // Keeps track of which emails we have read.
    std::string_set_ci mReadEmails;

    void ReceiveEmail(const std::string& emailId);

    void ShowEmailList();
    void ViewEmail(const std::string& emailId);
};