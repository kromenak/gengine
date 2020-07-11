//
// GAS.cpp
//
// Clark Kromenaker
//
#include "GAS.h"

#include <string>
#include <vector>
#include <iostream>

#include "Animator.h"
#include "GasPlayer.h"
#include "imstream.h"
#include "Scene.h"
#include "Services.h"
#include "StringTokenizer.h"
#include "StringUtil.h"

int AnimGasNode::Execute(GasPlayer* player)
{
    // Do random check. If it fails, we don't execute.
    // But note execution count is still incremented!
    int randomCheck = rand() % 100 + 1;
    if(randomCheck > random) { return 0; }
    
    // Must have a valid animation, and GasPlayer, and Animator.
    if(animation == nullptr) { return 0; }
    
    // Play the animation!
	GEngine::Instance()->GetScene()->GetAnimator()->Start(animation, false, true, nullptr);
	
    //std::cout << "Playing animation " << animation->GetName() << " for " << animation->GetDuration() << " seconds." << std::endl;
    return animation->GetDuration() * 1000;
}

int OneOfGasNode::Execute(GasPlayer* player)
{
    if(animNodes.size() == 0) { return 0; }
    int randomIndex = rand() % animNodes.size();
    return animNodes[randomIndex]->Execute(player);
}

int WaitGasNode::Execute(GasPlayer* player)
{
    // Do random check. If it fails, we don't execute.
    // But note execution count is still incremented!
    int randomCheck = rand() % 100 + 1;
    if(randomCheck > random) { return 0; }
    
    // We will execute this node. Decide wait time based on min/max.
    if(minWaitTimeSeconds == maxWaitTimeSeconds) { return minWaitTimeSeconds * 1000; }
    if(maxWaitTimeSeconds == 0 && minWaitTimeSeconds > 0) { return minWaitTimeSeconds * 1000; }
    if(maxWaitTimeSeconds != 0 && minWaitTimeSeconds > maxWaitTimeSeconds) { return minWaitTimeSeconds * 1000; }
	
    // Normal case - random between min and max. Mult by 1000 to return milliseconds.
    return (rand() % maxWaitTimeSeconds + minWaitTimeSeconds) * 1000;
}

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
		// Ignore commented out lines (// format)
		if(line[0] == '/' && line[1] == '/') { continue; }
		
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
