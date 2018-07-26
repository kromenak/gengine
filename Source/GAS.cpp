//
// GAS.cpp
//
// Clark Kromenaker
//
#include "GAS.h"
#include "imstream.h"
#include "StringUtil.h"
#include "StringTokenizer.h"
#include "Services.h"
#include <string>
#include <vector>
#include <iostream>

GAS::GAS(std::string name, char* data, int dataLength) : Asset(name)
{
    ParseFromData(data, dataLength);
}

void GAS::ParseFromData(char *data, int dataLength)
{
    imstream stream(data, dataLength);
    
    // Store any created "ONEOF" node, since they are generated over several lines.
    OneOfGasNode* oneOfNode = nullptr;
    
    // Read in the GAS file contents one line at a time.
    std::string line;
    while(StringUtil::GetLineSanitized(stream, line))
    {
        // Split line into tokens based on spaces, commas, and parenthesis.
        StringTokenizer tokenizer(line, { ' ', ',', '(', ')' });
        
        // Blank line? Just move to the next line.
        if(!tokenizer.HasNext()) { continue; }
        
        // The first word will be the main command.
        std::string command = tokenizer.GetNext();
        
        // We keep adding to the same "one of" node as long as they are appearing in a row.
        // But as soon as we reach a line that isn't a "ONEOF" line, we no longer want to add to that one anymore.
        if(oneOfNode != nullptr && !StringUtil::EqualsIgnoreCase(command, "ONEOF"))
        {
            oneOfNode = nullptr;
        }
        
        // Parse remains of the line based on what the command is.
        if(StringUtil::EqualsIgnoreCase(command, "ANIM"))
        {
            // The next token (anim name) is mandatory.
            if(!tokenizer.HasNext())
            {
                std::cout << "Missing anim name in GAS file!" << std::endl;
                continue;
            }
            
            // Read in the required field (anim name).
            AnimGasNode* animGasNode = new AnimGasNode();
            animGasNode->animation = Services::GetAssets()->LoadAnimation(tokenizer.GetNext());
            
            // Read in optional fields.
            if(tokenizer.HasNext())
            {
                animGasNode->moving = StringUtil::ToBool(tokenizer.GetNext());
            }
            if(tokenizer.HasNext())
            {
                animGasNode->random = StringUtil::ToInt(tokenizer.GetNext());
            }
            
            // Push onto node list.
            mNodes.push_back(animGasNode);
        }
        else if(StringUtil::EqualsIgnoreCase(command, "ONEOF"))
        {
            // The next token (anim name) is mandatory.
            if(!tokenizer.HasNext())
            {
                std::cout << "Missing anim name in GAS file!" << std::endl;
                continue;
            }
            
            // If no "ONEOF" node is created (aka this is the first one), create it and push it onto the list.
            if(oneOfNode == nullptr)
            {
                oneOfNode = new OneOfGasNode();
                mNodes.push_back(oneOfNode);
            }
            
            // Read in the required field (anim name).
            AnimGasNode* animGasNode = new AnimGasNode();
            animGasNode->animation = Services::GetAssets()->LoadAnimation(tokenizer.GetNext());
            
            // Read in optional fields.
            if(tokenizer.HasNext())
            {
                animGasNode->moving = StringUtil::ToBool(tokenizer.GetNext());
            }
            if(tokenizer.HasNext())
            {
                animGasNode->random = StringUtil::ToInt(tokenizer.GetNext());
            }
            
            // Instead of pushing onto main node list, we push onto the one of's vector.
            oneOfNode->animNodes.push_back(animGasNode);
        }
        else if(StringUtil::EqualsIgnoreCase(command, "WAIT"))
        {
            // The next token (min seconds to wait) is mandatory.
            if(!tokenizer.HasNext())
            {
                std::cout << "Missing min wait time in GAS file!" << std::endl;
                continue;
            }
            
            // Read in min wait time.
            WaitGasNode* waitGasNode = new WaitGasNode();
            waitGasNode->minWaitTimeSeconds = StringUtil::ToInt(tokenizer.GetNext());
            
            // Read in optional fields: max wait time and random.
            if(tokenizer.HasNext())
            {
                waitGasNode->maxWaitTimeSeconds = StringUtil::ToInt(tokenizer.GetNext());
                waitGasNode->maxWaitTimeSeconds = Math::Max(waitGasNode->minWaitTimeSeconds, waitGasNode->maxWaitTimeSeconds);
            }
            else
            {
                waitGasNode->maxWaitTimeSeconds = waitGasNode->minWaitTimeSeconds;
            }
            if(tokenizer.HasNext())
            {
                waitGasNode->random = StringUtil::ToInt(tokenizer.GetNext());
            }
            
            // Push onto list.
            mNodes.push_back(waitGasNode);
        }
        else if(StringUtil::EqualsIgnoreCase(command, "LABEL"))
        {
            if(!tokenizer.HasNext())
            {
                std::cout << "Missing label name in GAS file!" << std::endl;
                continue;
            }
            
            LabelOrGotoGasNode* labelGasNode = new LabelOrGotoGasNode();
            labelGasNode->isGoto = false;
            labelGasNode->label = tokenizer.GetNext();
            
            mNodes.push_back(labelGasNode);
        }
        else if(StringUtil::EqualsIgnoreCase(command, "GOTO"))
        {
            if(!tokenizer.HasNext())
            {
                std::cout << "Missing label name in GAS file!" << std::endl;
                continue;
            }
            
            LabelOrGotoGasNode* labelGasNode = new LabelOrGotoGasNode();
            labelGasNode->isGoto = true;
            labelGasNode->label = tokenizer.GetNext();
            
            mNodes.push_back(labelGasNode);
        }
        else if(StringUtil::EqualsIgnoreCase(command, "LOOP"))
        {
            LabelOrGotoGasNode* labelGasNode = new LabelOrGotoGasNode();
            labelGasNode->isGoto = true;
            
            mNodes.push_back(labelGasNode);
        }
        else
        {
            std::cout << "Unrecognized GAS command: " << line << std::endl;
        }
    }
}
