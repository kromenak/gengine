#include "GAS.h"

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

#include "Animator.h"
#include "AssetManager.h"
#include "GasNodes.h"
#include "GasPlayer.h"
#include "mstream.h"
#include "Scene.h"
#include "StringTokenizer.h"
#include "StringUtil.h"

GAS::~GAS()
{
    for(auto& node : mNodes)
    {
        delete node;
    }
}

void GAS::Load(uint8_t* data, uint32_t dataLength)
{
    imstream stream(reinterpret_cast<char*>(data), dataLength);
    
    // Store any created "ONEOF" node, since they are generated over several lines.
    OneOfGasNode* oneOfNode = nullptr;

    // Track what indexes labels are at for creating "goto" and "if" nodes.
    std::unordered_map<std::string, int> labelToIndexMap;

    // Remember goto and if nodes that need to be hooked up at the end.
    // Because you can have a goto or if BEFORE the label declaration, we must do these at the end.
    std::vector<std::pair<std::string, GotoGasNode*>> gotoNodePairs;
    std::vector<std::pair<std::string, IfGasNode*>> ifNodePairs;
    std::vector<std::pair<std::string, WhenNearGasNode*>> whenNoLongerNearNodePairs;

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
            AnimGasNode* node = new AnimGasNode();
            node->animation = gAssetManager.LoadAnimation(tokenizer.GetNext(), GetScope());
            
            // Read in optional fields.
            if(tokenizer.HasNext())
            {
                node->moving = StringUtil::ToBool(tokenizer.GetNext());
            }
            if(tokenizer.HasNext())
            {
                node->random = StringUtil::ToInt(tokenizer.GetNext());
            }
            
            // Push onto node list.
            mNodes.push_back(node);
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
            AnimGasNode* node = new AnimGasNode();
            node->animation = gAssetManager.LoadAnimation(tokenizer.GetNext(), GetScope());
            
            // Read in optional fields.
            if(tokenizer.HasNext())
            {
                node->moving = StringUtil::ToBool(tokenizer.GetNext());
            }
            if(tokenizer.HasNext())
            {
                node->random = StringUtil::ToInt(tokenizer.GetNext());
            }
            
            // Instead of pushing onto main node list, we push onto the one of's vector.
            oneOfNode->animNodes.push_back(node);
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
            WaitGasNode* node = new WaitGasNode();
            node->minWaitTimeSeconds = StringUtil::ToFloat(tokenizer.GetNext());
            
            // Optional token (max wait time).
            if(tokenizer.HasNext())
            {
                node->maxWaitTimeSeconds = StringUtil::ToFloat(tokenizer.GetNext());
                node->maxWaitTimeSeconds = Math::Max(node->minWaitTimeSeconds, node->maxWaitTimeSeconds);
            }
            else
            {
                node->maxWaitTimeSeconds = node->minWaitTimeSeconds;
            }

            // Optional token (random chance).
            // There's at least one spot where the GAS data has an errant extra field (WAIT 2, 5, FALSE, 40).
            // So, we really want to just read the LAST token here.
            if(tokenizer.HasNext())
            {
                tokenizer.SetIndex(tokenizer.GetTokenCount() - 1);
                assert(tokenizer.HasNext());
                node->random = StringUtil::ToInt(tokenizer.GetNext());
            }
            
            // Push onto list.
            mNodes.push_back(node);
        }
        else if(StringUtil::EqualsIgnoreCase(command, "LABEL"))
        {
            if(!tokenizer.HasNext())
            {
                std::cout << "Missing label name in GAS file!" << std::endl;
                continue;
            }

            // Note that this label points to the next node added to the vector.
            // At the end of parsing, we'll hook up any GOTO or IF nodes.
            labelToIndexMap[tokenizer.GetNext()] = mNodes.size();
        }
        else if(StringUtil::EqualsIgnoreCase(command, "GOTO"))
        {
            if(!tokenizer.HasNext())
            {
                std::cout << "Missing label name in GAS file!" << std::endl;
                continue;
            }

            // Create node.
            GotoGasNode* node = new GotoGasNode();
            mNodes.push_back(node);

            // Remember that we need to come back and hook up the label index later.
            gotoNodePairs.push_back(std::make_pair(tokenizer.GetNext(), node));
        }
        else if(StringUtil::EqualsIgnoreCase(command, "LOOP"))
        {
            // LOOP loops back to top of file.
            // So...that's really just a "goto" with index of zero!
            GotoGasNode* node = new GotoGasNode();
            mNodes.push_back(node);
        }
        else if(StringUtil::EqualsIgnoreCase(command, "SET"))
        {
            if(!tokenizer.HasNext())
            {
                std::cout << "Missing var name in SET" << std::endl;
                continue;
            }
            std::string varName = tokenizer.GetNext();

            if(!tokenizer.HasNext())
            {
                std::cout << "Missing var value in SET" << std::endl;
                continue;
            }
            int varValue = StringUtil::ToInt(tokenizer.GetNext());

            // Create node. Note that GAS system says var names can only be a single char.
            SetGasNode* node = new SetGasNode();
            node->varName = varName[0];
            node->value = varValue;
            mNodes.push_back(node);
        }
        else if(StringUtil::EqualsIgnoreCase(command, "INC"))
        {
            if(!tokenizer.HasNext())
            {
                std::cout << "Missing var name in INC" << std::endl;
                continue;
            }
            std::string varName = tokenizer.GetNext();

            // Create node.
            IncGasNode* node = new IncGasNode();
            node->varName = varName[0];
            mNodes.push_back(node);
        }
        else if(StringUtil::EqualsIgnoreCase(command, "DEC"))
        {
            if(!tokenizer.HasNext())
            {
                std::cout << "Missing var name in DEC" << std::endl;
                continue;
            }
            std::string varName = tokenizer.GetNext();

            // Create node.
            DecGasNode* node = new DecGasNode();
            node->varName = varName[0];
            mNodes.push_back(node);
        }
        else if(StringUtil::EqualsIgnoreCase(command, "IF"))
        {
            if(!tokenizer.HasNext())
            {
                std::cout << "Missing var name in IF" << std::endl;
                continue;
            }
            std::string varName = tokenizer.GetNext();

            if(!tokenizer.HasNext())
            {
                std::cout << "Missing comparison operator in IF" << std::endl;
                continue;
            }
            std::string opString = tokenizer.GetNext();

            if(!tokenizer.HasNext())
            {
                std::cout << "Missing value in IF" << std::endl;
                continue;
            }
            int value = StringUtil::ToInt(tokenizer.GetNext());

            if(!tokenizer.HasNext())
            {
                std::cout << "Missing label name in IF" << std::endl;
                continue;
            }
            std::string label = tokenizer.GetNext();

            // Make sure the operator is valid.
            IfGasNode::Operation operation;
            if(opString[0] == '=')
            {
                operation = IfGasNode::Operation::Equals;
            }
            else if(opString[0] == '<')
            {
                operation = IfGasNode::Operation::LessThan;
            }
            else if(opString[0] == '>')
            {
                operation = IfGasNode::Operation::GreaterThan;
            }
            else
            {
                std::cout << "Invalid operation in IF" << std::endl;
                continue;
            }
            
            // Create the node.
            IfGasNode* node = new IfGasNode();
            node->varName = varName[0];
            node->operation = operation;
            node->value = value;
            mNodes.push_back(node);

            // Remember that we need to come back and hook up the label index later.
            ifNodePairs.push_back(std::make_pair(label, node));
        }
        else if(StringUtil::EqualsIgnoreCase(command, "WALKTO"))
        {
            if(!tokenizer.HasNext())
            {
                std::cout << "Missing position in WALKTO" << std::endl;
                continue;
            }
            std::string position = tokenizer.GetNext();

            // The position can be either the name of a position (from SIF file) or an X/Y/Z position.
            // If more tokens are present, this is an X/Y/Z position.
            WalkToGasNode* node = nullptr;
            if(tokenizer.HasNext())
            {
                float x = StringUtil::ToFloat(position);
                float y = StringUtil::ToFloat(tokenizer.GetNext());
                if(!tokenizer.HasNext())
                {
                    std::cout << "Malformed X/Y/Z position in WALKTO" << std::endl;
                    continue;
                }
                float z = StringUtil::ToFloat(tokenizer.GetNext());

                node = new WalkToGasNode();
                node->position = Vector3(x, y, z);
            }
            else
            {
                node = new WalkToGasNode();
                node->positionName = position;
            }
            mNodes.push_back(node);
        }
        else if(StringUtil::EqualsIgnoreCase(command, "CHOOSEWALK"))
        {
            ChooseWalkGasNode* node = new ChooseWalkGasNode();
            while(tokenizer.HasNext())
            {
                node->positionNames.push_back(tokenizer.GetNext());
            }
            mNodes.push_back(node);
        }
        else if(StringUtil::EqualsIgnoreCase(command, "USE") ||
                StringUtil::EqualsIgnoreCase(command, "USES") || // SOME scripts have USES instead of USE - what the heck
                StringUtil::EqualsIgnoreCase(command, "USETALK"))
        {
            bool isUseTalk = command.size() > 4;
            if(!tokenizer.HasNext())
            {
                std::cout << "Incomplete USE statement" << std::endl;
                continue;
            }

            std::string subcommand = tokenizer.GetNext();
            if(StringUtil::EqualsIgnoreCase(subcommand, "IPOS"))
            {
                if(!tokenizer.HasNext())
                {
                    std::cout << "Missing position in USE IPOS" << std::endl;
                    continue;
                }

                if(isUseTalk)
                {
                    UseTalkIPosGasNode* node = new UseTalkIPosGasNode();
                    node->positionName = tokenizer.GetNext();
                    mNodes.push_back(node);
                }
                else
                {
                    UseIPosGasNode* node = new UseIPosGasNode();
                    node->positionName = tokenizer.GetNext();
                    mNodes.push_back(node);
                }
            }
            else if(StringUtil::EqualsIgnoreCase(subcommand, "CLEANUP"))
            {
                if(!tokenizer.HasNext())
                {
                    std::cout << "Missing animation name in USE CLEANUP" << std::endl;
                    continue;
                }
                std::string animNeedingCleanupName = tokenizer.GetNext();

                if(!tokenizer.HasNext())
                {
                    std::cout << "Missing animation name in USE CLEANUP" << std::endl;
                    continue;
                }
                std::string animDoingCleanupName = tokenizer.GetNext();

                //TODO: I'm not sure if nodes are actually needed for defining cleanups like this...
                //TODO: Mayyybe the cleanup mappings could just be defined in the GAS asset as static data.
                //TODO: But I'm not sure yet whether autoscripts expect this mapping to dynamically update as the script plays or not...
                if(isUseTalk)
                {
                    UseTalkCleanupGasNode* node = new UseTalkCleanupGasNode();
                    node->animationNeedingCleanup = gAssetManager.LoadAnimation(animNeedingCleanupName, GetScope());
                    node->animationDoingCleanup = gAssetManager.LoadAnimation(animDoingCleanupName, GetScope());
                    mNodes.push_back(node);
                }
                else
                {
                    UseCleanupGasNode* node = new UseCleanupGasNode();
                    node->animationNeedingCleanup = gAssetManager.LoadAnimation(animNeedingCleanupName, GetScope());
                    node->animationDoingCleanup = gAssetManager.LoadAnimation(animDoingCleanupName, GetScope());
                    mNodes.push_back(node);
                }
            }
            else if(StringUtil::EqualsIgnoreCase(subcommand, "NEWIDLE"))
            {
                if(!tokenizer.HasNext())
                {
                    std::cout << "Missing autoscript name in USE NEWIDLE" << std::endl;
                    continue;
                }
                std::string newIdleGasName = tokenizer.GetNext();

                if(isUseTalk)
                {
                    //TODO: 
                }
                else
                {
                    //TODO:
                }
            }
        }
        else if(StringUtil::EqualsIgnoreCase(command, "NEWIDLE"))
        {
            if(!tokenizer.HasNext())
            {
                std::cout << "Missing autoscript name in NEWIDLE" << std::endl;
                continue;
            }

            NewIdleGasNode* node = new NewIdleGasNode();
            node->newGas = gAssetManager.LoadGAS(tokenizer.GetNext(), GetScope());
            mNodes.push_back(node);
        }
        else if(StringUtil::EqualsIgnoreCase(command, "WHENNEAR") ||
                StringUtil::EqualsIgnoreCase(command, "WHENNOLONGERNEAR"))
        {
            // Get required parameters. 
            if(!tokenizer.HasNext())
            {
                std::cout << "Missing noun in WHENNEAR/WHENNOLONGERNEAR" << std::endl;
                continue;
            }
            std::string noun = tokenizer.GetNext();

            if(!tokenizer.HasNext())
            {
                std::cout << "Missing distance in WHENNEAR/WHENNOLONGERNEAR" << std::endl;
                continue;
            }
            float distance = StringUtil::ToFloat(tokenizer.GetNext());

            if(!tokenizer.HasNext())
            {
                std::cout << "Missing label in WHENNEAR/WHENNOLONGERNEAR" << std::endl;
                continue;
            }
            std::string label = tokenizer.GetNext();

            // The last parameter (the other noun to measure distance from) is optional.
            // If not specified, distance is from GAS actor to noun.
            std::string otherNoun;
            if(tokenizer.HasNext())
            {
                otherNoun = tokenizer.GetNext();
            }

            WhenNearGasNode* node = new WhenNearGasNode();
            node->notNear = command.size() > 8; // if command is WHENNOLONGERNEAR...
            node->noun = noun;
            node->distance = distance;
            node->otherNoun = otherNoun;
            mNodes.push_back(node);
            
            // Remember that we need to come back and hook up the label index later.
            whenNoLongerNearNodePairs.push_back(std::make_pair(label, node));
        }
        else if(StringUtil::EqualsIgnoreCase(command, "DLG"))
        { 
            if(!tokenizer.HasNext())
            {
                std::cout << "Missing yak in DLG" << std::endl;
                continue;
            }
            std::string yakName = "E" + tokenizer.GetNext();

            Animation* yakAnimation = gAssetManager.LoadYak(yakName, GetScope());
            if(yakAnimation == nullptr)
            {
                std::cout << "Invalid yak name specified in DLG" << std::endl;
                continue;
            }

            DialogueGasNode* node = new DialogueGasNode();
            node->yakAnimation = yakAnimation;
            mNodes.push_back(node);
        }
        else if(StringUtil::EqualsIgnoreCase(command, "LOCATION"))
        {
            if(!tokenizer.HasNext())
            {
                std::cout << "Missing location in LOCATION" << std::endl;
                continue;
            }

            LocationGasNode* node = new LocationGasNode();
            node->location = tokenizer.GetNext();
            mNodes.push_back(node);
        }
        else
        {
            std::cout << "Unrecognized GAS command: " << line << std::endl;
        }
    }

    // OK, we read in all the nodes! We know all the labels, gotos, etc.
    // Lets now hook up where gotos are going to!
    for(auto& pair : gotoNodePairs)
    {
        auto it = labelToIndexMap.find(pair.first);
        if(it != labelToIndexMap.end())
        {
            pair.second->index = it->second;
        }
    }
    for(auto& pair : ifNodePairs)
    {
        auto it = labelToIndexMap.find(pair.first);
        if(it != labelToIndexMap.end())
        {
            pair.second->index = it->second;
        }
    }
    for(auto& pair : whenNoLongerNearNodePairs)
    {
        auto it = labelToIndexMap.find(pair.first);
        if(it != labelToIndexMap.end())
        {
            pair.second->index = it->second;
        }
    }
}
