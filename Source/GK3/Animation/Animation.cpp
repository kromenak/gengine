#include "Animation.h"

#include <cctype>

#include "AnimationNodes.h"
#include "AssetManager.h"
#include "FileSystem.h"
#include "IniParser.h"
#include "StringUtil.h"
#include "VertexAnimation.h"

Animation::~Animation()
{
    // Be sure to delete dynamically allocated memory.
    for(auto& frameEntry : mFrames)
    {
        for(auto& frameAnimNode : frameEntry.second)
        {
            delete frameAnimNode;
        }
    }
}

void Animation::Load(uint8_t* data, uint32_t dataLength)
{
    ParseFromData(data, dataLength);
}

std::vector<AnimNode*>* Animation::GetFrame(int frameNumber)
{
    if(mFrames.find(frameNumber) != mFrames.end())
    {
        return &mFrames[frameNumber];
    }
    return nullptr;
}

VertexAnimNode* Animation::GetVertexAnimationOnFrameForModel(int frameNumber, const std::string& modelName)
{
	for(auto& node : mVertexAnimNodes)
	{
		if(node->frameNumber == frameNumber &&
		   node->vertexAnimation != nullptr &&
		   StringUtil::EqualsIgnoreCase(node->vertexAnimation->GetModelName(), modelName))
		{
			return node;
		}
	}
	return nullptr;
}

void Animation::ParseFromData(uint8_t* data, uint32_t dataLength)
{
    bool isYak = Path::HasExtension(GetName(), ".yak");

    IniParser parser(data, dataLength);
    IniSection section;
    while(parser.ReadNextSection(section))
    {
        // Header section has only one value: number of frames.
        if(StringUtil::EqualsIgnoreCase(section.name, "HEADER"))
        {
			if(section.lines.size() > 0)
            {
                mFrameCount = section.lines[0].entries.front().GetValueAsInt();
            }
        }
		// Actions section contains vertex animations to start playing on certain frames.
        else if(StringUtil::EqualsIgnoreCase(section.name, "ACTIONS"))
        {
			// First line is number of action entries...but we can just determine that from the number of lines!
			// Read in 2+ lines as actions.
            for(size_t i = 1; i < section.lines.size(); ++i)
            {
				IniLine& line = section.lines[i];
				
                // Each line has up to 10 (!!!) fields. Most are optional, but they must be in a certain order.
				
				// Frame number must be specified.
                // <frame_num>, <act_name>, <x1>, <y1>, <z1>, <angle1>, <x2>, <y2>, <z2>, <angle2>
                // <frame_num>, <act_name>, ABSOLUTE
                int frameNumber = line.entries[0].GetValueAsInt();
                
				// Vertex animation must be specified.
				VertexAnimation* vertexAnim = gAssetManager.LoadVertexAnimation(line.entries[1].key, GetScope());
                if(vertexAnim == nullptr)
                {
                    printf("Failed to load vertex animation %s!\n", line.entries[1].key.c_str());
                    continue;
                }
                
				// Create and push back the animation node. Remaining fields are optional.
                VertexAnimNode* node = new VertexAnimNode();
                node->frameNumber = frameNumber;
				node->vertexAnimation = vertexAnim;

                //HACK: If this is a facing direction helper (DOR), make sure it animated *before* other objects.
                //HACK: Some logic in GKActor for calculating facing direction only works if the DOR gets sampled first.
                bool isDor = StringUtil::StartsWithIgnoreCase(vertexAnim->GetName(), "DOR_");
                if(isDor)
                {
                    mFrames[frameNumber].insert(mFrames[frameNumber].begin(), node);
                    mVertexAnimNodes.insert(mVertexAnimNodes.begin(), node);
                }
                else
                {
                    mFrames[frameNumber].push_back(node);
                    mVertexAnimNodes.push_back(node);
                }

                //TODO: Some animations have a keyword here (ABSOLUTE). Which I guess indicates that this is an absolute animation.
                //TODO: I'm guessing this might be shorthand for "0, 0, 0, 0, 0, 0, 0, 0" which is frequently used for absolute anims.
				
				// See if there are enough args for the (x1, y1, z1) and (angle1) values.
				if(line.entries.size() < 6) { continue; }
				
				// If an animation defines more entries, it means it is an absolute animation.
				node->absolute = true;

                // (x1, y1, z1) and (angle1) represent an offset/heading used to calculate position to play absolute anim.
                // The offset in the asset goes "actor to model" but we need it to be "model to actor" later. So, that's why we negate them!
                // Note z/y are flipped due to Maya->Game conversion.
                node->absoluteModelToActorOffset.x = -line.entries[2].GetValueAsFloat();
                node->absoluteModelToActorOffset.z = -line.entries[3].GetValueAsFloat();
                node->absoluteModelToActorOffset.y = -line.entries[4].GetValueAsFloat();
                node->absoluteModelToActorHeading = line.entries[5].GetValueAsFloat();

                // Enough args for the final set of arguments?
				if(line.entries.size() < 10) { continue; }

                // Next are (x2, z2, y2) and (angle2). These represent an offset/heading ALSO used to calculate position to play absolute anims.
                // The previous vector is rotated by this heading and then added to this offset to get final position - crazy stuff! 
                // Note z/y are flipped due to Maya->Game conversion.
				node->absoluteWorldToModelOffset.x = line.entries[6].GetValueAsFloat();
				node->absoluteWorldToModelOffset.z = line.entries[7].GetValueAsFloat();
				node->absoluteWorldToModelOffset.y = line.entries[8].GetValueAsFloat();
				node->absoluteWorldToModelHeading = line.entries[9].GetValueAsFloat();
            }
        }
		// "STextures" changes textures on a scene (BSP) model.
        else if(StringUtil::EqualsIgnoreCase(section.name, "STEXTURES"))
        {
			// First line is number of entries...but we can just determine that from the number of lines!
			for(size_t i = 1; i < section.lines.size(); ++i)
            {
				IniLine& line = section.lines[i];
				
                // <frame_num>, <scn_name>, <scn_model_name>, <texture_name>
                int frameNumber = line.entries[0].GetValueAsInt();
                
                // Read the scene name.
                std::string sceneName = line.entries[1].key;
                
                // Read the scene model name.
                std::string sceneModelName = line.entries[2].key;
                
                // Read the texture name.
				std::string textureName = line.entries[3].key;
				
				// Create and add the anim node.
				SceneTextureAnimNode* node = new SceneTextureAnimNode();
				node->frameNumber = frameNumber;
				node->sceneName = sceneName;
				node->sceneModelName = sceneModelName;
				node->textureName = textureName;
                mFrames[frameNumber].push_back(node);
            }
        }
		// "SVisibility" changes the visibility of a scene (BSP) model.
        else if(StringUtil::EqualsIgnoreCase(section.name, "SVISIBILITY"))
        {
			// First line is number of entries...but we can just determine that from the number of lines!
			// Read in 2+ lines as actions.
            for(size_t i = 1; i < section.lines.size(); i++)
            {
                IniLine& line = section.lines[i];
                
                // <frame_num>, <scn_name>, <scn_model_name>, <on/off>
                int frameNumber = line.entries[0].GetValueAsInt();
                
                // Read the scene name.
                std::string sceneName = line.entries[1].key;
                
                // Read the scene model name.
                std::string sceneModelName = line.entries[2].key;
                
                // Read the on/off value.
                bool visible = line.entries[3].GetValueAsBool();
				
				// Create and add the anim node.
				SceneModelVisibilityAnimNode* node = new SceneModelVisibilityAnimNode();
				node->sceneName = sceneName;
				node->sceneModelName = sceneModelName;
				node->visible = visible;
                mFrames[frameNumber].push_back(node);
            }
        }
		// "MTextures" changes textures on a model or actor.
        else if(StringUtil::EqualsIgnoreCase(section.name, "MTEXTURES"))
        {
			// First line is number of entries...but we can just determine that from the number of lines!
			// Read in 2+ lines as actions.
            for(size_t i = 1; i < section.lines.size(); i++)
            {
                IniLine& line = section.lines[i];
                
                // <frame_num>, <model_name>, <mesh_index>, <group_index>, <texture_name>
                int frameNumber = line.entries[0].GetValueAsInt();
                
                // Read the model name.
                std::string modelName = line.entries[1].key;
                
                // Read the model mesh index.
                int meshIndex = line.entries[2].GetValueAsInt();
                
                // Read the model mesh group index.
                int submeshIndex = line.entries[3].GetValueAsInt();
                
                // Read the texture name.
                std::string textureName = line.entries[4].key;
				
				// Create and add node.
				ModelTextureAnimNode* node = new ModelTextureAnimNode();
				node->modelName = modelName;
				node->meshIndex = static_cast<unsigned char>(meshIndex);
				node->submeshIndex = static_cast<unsigned char>(submeshIndex);
				node->textureName = textureName;
				mFrames[frameNumber].push_back(node);
            }
        }
		// "MVisibility" changes visibility on a model or actor.
        else if(StringUtil::EqualsIgnoreCase(section.name, "MVISIBILITY"))
        {
			// First line is number of entries...but we can just determine that from the number of lines!
			// Read in 2+ lines as actions.
            for(size_t i = 1; i < section.lines.size(); ++i)
            {
                IniLine& line = section.lines[i];

                // Two possible formats:
                // <frame_num>, <model_name>, <on/off>
                // <frame_num>, <model_name>, <mesh_index>, <submesh_index>, <on/off>
                int frameNumber = line.entries[0].GetValueAsInt();
                
                // Read the model name.
                std::string modelName = line.entries[1].key;

                // Create and add node.
                ModelVisibilityAnimNode* node = new ModelVisibilityAnimNode();
                node->modelName = modelName;
                mFrames[frameNumber].push_back(node);

                // Read specific mesh/submesh visibility version vs. whole model version.
                if(line.entries.size() > 3)
                {
                    node->meshIndex = static_cast<signed char>(line.entries[2].GetValueAsInt());
                    node->submeshIndex = static_cast<signed char>(line.entries[3].GetValueAsInt());
                    node->visible = line.entries[4].GetValueAsBool();
                }
                else
                {
                    node->visible = line.entries[2].GetValueAsBool();
                }
            }
        }
		// Triggers sounds to play on certain frames at certain locations.
        else if(StringUtil::EqualsIgnoreCase(section.name, "SOUNDS"))
        {
			// First line is number of entries...but we can just determine that from the number of lines!
			// Read in lines 2+ as sounds.
            for(size_t i = 1; i < section.lines.size(); i++)
            {
                IniLine& line = section.lines[i];
                
				// Possible versions of these lines:
				// <frame_num>, <sound_name>, <volume> (2D sound)
				// <frame_num>, <sound_name>, <volume>, <model_name> (3D sound attached to model)
                // <frame_num>, <sound_name>, <volume>, <model_name>, <min_dist>, <max_dist> (3D sound attached to model w/ min/max distances)
                // <frame_num>, <sound_name>, <volume>, <x>, <y>, <z> (3D sound at position)
				// <frame_num>, <sound_name>, <volume>, <x>, <y>, <z>, <min_dist>, <max_dist> (3D sound at position w/ min/max distances)
                // Read frame number.
                int frameNumber = line.entries[0].GetValueAsInt();
                
                // Read the sound name.
                std::string soundName = line.entries[1].key;
                
                // Read the volume.
                int volume = line.entries[2].GetValueAsInt();
				
				// Create node.
				SoundAnimNode* node = new SoundAnimNode();
				node->frameNumber = frameNumber;
                if(isYak)
                {
                    node->audio = gAssetManager.LoadAudio(soundName, AssetScope::Scene);
                }
                else
                {
                    node->audio = gAssetManager.LoadAudio(soundName);
                }
				node->volume = volume;

                // Remaining arguments are optional.
                if(line.entries.size() >= 4)
                {
                    // If any optional arguments are present, this is a 3D sound.
                    node->is3d = true;

                    // HACK: the next argument might be a model name OR a sound position (x, y, z).
                    // To determine, let's just see if the first char is a digit.
                    // Probably a better way to do this is add IniParser logic to check entry type (int, float, string, etc).
                    // Also, a position requires at least 6 arguments total.
                    bool usesPosition = std::isdigit(line.entries[3].value[0]) && line.entries.size() >= 6;
                    size_t distIndex = 0;
                    if(!usesPosition)
                    {
                        node->modelName = line.entries[3].key;
                        distIndex = 4;
                    }
                    else
                    {
                        float x = line.entries[3].GetValueAsFloat();
                        float y = line.entries[4].GetValueAsFloat();
                        float z = line.entries[5].GetValueAsFloat();
                        node->position = Vector3(x, y, z);
                        distIndex = 6;
                    }

                    // Read in min/max distance for sound, if entries are present.
                    if(distIndex < line.entries.size())
                    {
                        node->minDistance = line.entries[distIndex].GetValueAsFloat();
                    }
                    if(distIndex + 1 < line.entries.size())
                    {
                        node->maxDistance = line.entries[distIndex + 1].GetValueAsFloat();
                    }
                }

                // Put node in frames map.
                mFrames[frameNumber].push_back(node);
            }
        }
		// Allows specifying of additional options that affect the entire animation.
        else if(StringUtil::EqualsIgnoreCase(section.name, "OPTIONS"))
        {
			// First line is number of entries...but we can just determine that from the number of lines!
			// Read in 2+ lines as actions.
            for(size_t i = 1; i < section.lines.size(); i++)
            {
				IniLine& line = section.lines[i];
				
                // <frame_num>, <option>, <value>
                
                // Read frame number.
                //int frameNumber = line.entries[0].GetValueAsInt();
                
                // Read in the option.
				std::string option = line.entries[1].key;
                if(StringUtil::EqualsIgnoreCase(option, "SIMPLE"))
                {
                    //int simpleValue = entry->GetValueAsInt();
                }
                else if(StringUtil::EqualsIgnoreCase(option, "NOINTERPOLATE"))
                {
                    
                }
				// Allows us to dictate the rate at which the animation proceeds.
                else if(StringUtil::EqualsIgnoreCase(option, "FRAMERATE"))
                {
                    //mFramesPerSecond = entry->GetValueAsInt();
                }
                else
                {
                    std::cout << "Unexpected option: " << option << std::endl;
                }
            }
        }
		// Contains some options specifically for GK3. I imagine this engine
		// was initially meant to be reused on future projects, so they tried
		// to isolate extremely specific to GK3 stuff here?
        else if(StringUtil::EqualsIgnoreCase(section.name, "GK3"))
        {
            for(size_t i = 1; i < section.lines.size(); i++)
            {
                IniLine& line = section.lines[i];
                
                // Read frame number.
                int frameNumber = line.entries[0].GetValueAsInt();
                
                // Read in the option.
				std::string keyword = line.entries[1].key;
                if(StringUtil::EqualsIgnoreCase(keyword, "FOOTSTEP"))
                {
                    // [FRAME] FOOTSTEP [NOUN]
                    if(line.entries.size() >= 3)
                    {
                        std::string actorNoun = line.entries[2].key;

                        // Create and add node.
                        FootstepAnimNode* node = new FootstepAnimNode();
                        node->frameNumber = frameNumber;
                        node->actorNoun = actorNoun;
                        mFrames[frameNumber].push_back(node);
                    }
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "FOOTSCUFF"))
                {
                    // [FRAME] FOOTSCUFF [NOUN]
                    if(line.entries.size() >= 3)
                    {
                        std::string actorNoun = line.entries[2].key;

                        // Create and add node.
                        FootscuffAnimNode* node = new FootscuffAnimNode();
                        node->frameNumber = frameNumber;
                        node->actorNoun = actorNoun;
                        mFrames[frameNumber].push_back(node);
                    }
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "STOPSOUNDTRACK"))
                {
                    // [FRAME] STOPSOUNDTRACK [STK_NAME]
                    if(line.entries.size() >= 3)
                    {
                        std::string soundtrackName = line.entries[2].key;

                        // Create and add node.
                        StopSoundtrackAnimNode* node = new StopSoundtrackAnimNode();
                        node->frameNumber = frameNumber;
                        node->soundtrackName = soundtrackName;
                        mFrames[frameNumber].push_back(node);
                    }
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "PLAYSOUNDTRACK"))
                {
                    // [FRAME] PLAYSOUNDTRACK [STK_NAME]
                    if(line.entries.size() >= 3)
                    {
                        // Create and add node.
                        PlaySoundtrackAnimNode* node = new PlaySoundtrackAnimNode();
                        node->frameNumber = frameNumber;
                        node->soundtrackName = line.entries[2].key;
                        mFrames[frameNumber].push_back(node);
                    }
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "PLAYSOUNDTRACKTBS"))
                {
                    // [FRAME] PLAYSOUNDTRACKTBS [STK_NAME]
                    if(line.entries.size() >= 3)
                    {
                        // Create and add node.
                        PlaySoundtrackAnimNode* node = new PlaySoundtrackAnimNode();
                        node->frameNumber = frameNumber;
                        node->soundtrackName = line.entries[2].key;
                        mFrames[frameNumber].push_back(node);
                    }
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "STOPALLSOUNDTRACKS"))
                {
					// Create and add node.
                    StopSoundtrackAnimNode* node = new StopSoundtrackAnimNode();
                    node->frameNumber = frameNumber;
					mFrames[frameNumber].push_back(node);
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "CAMERA"))
                {
                    // [FRAME] CAMERA [CAM_NAME] (GLIDE)
                    if(line.entries.size() >= 3)
                    {
                        // Create and add node.
                        CameraAnimNode* node = new CameraAnimNode();
                        node->frameNumber = frameNumber;
                        node->cameraPositionName = line.entries[2].key;

                        // If there is any additional GLIDE keyword, it means the camera should glide.
                        for(size_t j = 3; j < line.entries.size(); ++j)
                        {
                            if(StringUtil::EqualsIgnoreCase(line.entries[j].key, "GLIDE"))
                            {
                                node->glide = true;
                                break;
                            }
                        }

                        mFrames[frameNumber].push_back(node);
                    }
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "LIPSYNCH"))
                {
                    // [FRAME] LIPSYNCH [NOUN] [TEXTURE]
                    if(line.entries.size() >= 4)
                    {
                        std::string actorNoun = line.entries[2].key;
                        std::string mouthTexName = line.entries[3].key;

                        // Create and add node.
                        LipSyncAnimNode* node = new LipSyncAnimNode();
                        node->frameNumber = frameNumber;
                        node->actorNoun = actorNoun;
                        node->mouthTextureName = mouthTexName;
                        mFrames[frameNumber].push_back(node);
                    }
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "FACETEX"))
                {
                    // [FRAME] FACETEX [NOUN] [TEXTURE] [FACE_ELEMENT]
                    if(line.entries.size() >= 5)
                    {
                        // Read the actor name.
                        std::string actorNoun = line.entries[2].key;

                        // Read texture name.
                        // This sometimes has a forward slash in it, which
                        // indicates "tex"/"alpha_tex".
                        std::string textureName = line.entries[3].key;

                        // A value indicating what part of the face is changed. Always H, E, M.
                        // M = mouth
                        // E = eye
                        // H = head/forehead
                        std::string facePart = line.entries[4].key;
                        StringUtil::ToLower(facePart);
                        FaceElement faceElement = FaceElement::Mouth;
                        switch(facePart[0])
                        {
                        case 'm':
                            faceElement = FaceElement::Mouth;
                            break;
                        case 'e':
                            faceElement = FaceElement::Eyelids;
                            break;
                        case 'h':
                            faceElement = FaceElement::Forehead;
                            break;
                        }

                        // Create and add node.
                        FaceTexAnimNode* node = new FaceTexAnimNode();
                        node->frameNumber = frameNumber;
                        node->actorNoun = actorNoun;
                        node->textureName = textureName;
                        node->faceElement = faceElement;
                        mFrames[frameNumber].push_back(node);
                    }
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "UNFACETEX"))
                {
                    // [FRAME] UNFACETEX [NOUN] [FACE_ELEMENT]
                    if(line.entries.size() >= 4)
                    {
                        // Read the actor name.
                        std::string actorNoun = line.entries[2].key;

                        // A value indicating what part of the face is changed. Always H, E, M.
                        // M = mouth
                        // E = eye
                        // H = head/forehead
                        std::string facePart = line.entries[3].key;
                        StringUtil::ToLower(facePart);
                        FaceElement faceElement = FaceElement::Mouth;
                        switch(facePart[0])
                        {
                        case 'm':
                            faceElement = FaceElement::Mouth;
                            break;
                        case 'e':
                            faceElement = FaceElement::Eyelids;
                            break;
                        case 'h':
                            faceElement = FaceElement::Forehead;
                            break;
                        }

                        // Create and add node.
                        UnFaceTexAnimNode* node = new UnFaceTexAnimNode();
                        node->frameNumber = frameNumber;
                        node->actorNoun = actorNoun;
                        node->faceElement = faceElement;
                        mFrames[frameNumber].push_back(node);
                    }
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "GLANCE"))
                {
                    // [FRAME] GLANCE [NOUN] [X] [Y] [Z]
                    if(line.entries.size() >= 6)
                    {
                        // Read the actor name.
                        std::string actorNoun = line.entries[2].key;

                        // X/Y/Z position.
                        float x = line.entries[3].GetValueAsFloat();
                        float y = line.entries[4].GetValueAsFloat();
                        float z = line.entries[5].GetValueAsFloat();

                        // Create and add node.
                        GlanceAnimNode* node = new GlanceAnimNode();
                        node->frameNumber = frameNumber;
                        node->actorNoun = actorNoun;
                        node->position = Vector3(x, y, z);
                        mFrames[frameNumber].push_back(node);
                    }
                }
				else if(StringUtil::EqualsIgnoreCase(keyword, "MOOD"))
				{
                    // [FRAME] MOOD [NOUN] [MOOD]
                    if(line.entries.size() >= 4)
                    {
                        // Read the actor name.
                        std::string actorNoun = line.entries[2].key;

                        // Read the mood name.
                        std::string moodName = line.entries[3].key;

                        // Create and add node.
                        MoodAnimNode* node = new MoodAnimNode();
                        node->frameNumber = frameNumber;
                        node->actorNoun = actorNoun;
                        node->moodName = moodName;
                        mFrames[frameNumber].push_back(node);
                    }
				}
                else if(StringUtil::EqualsIgnoreCase(keyword, "EXPRESSION"))
                {
                    // [FRAME] EXPRESSION [NOUN] [EXPRESSION]
                    if(line.entries.size() >= 4)
                    {
                        // Read the actor name.
                        std::string actorNoun = line.entries[2].key;

                        // Read the expression name.
                        std::string expressionName = line.entries[3].key;

                        // Create and add node.
                        ExpressionAnimNode* node = new ExpressionAnimNode();
                        node->frameNumber = frameNumber;
                        node->actorNoun = actorNoun;
                        node->expressionName = expressionName;
                        mFrames[frameNumber].push_back(node);
                    }
                }
				else if(StringUtil::EqualsIgnoreCase(keyword, "SPEAKER"))
                {
                    // [FRAME] SPEAKER [NOUN]
                    if(line.entries.size() >= 3)
                    {
                        // Read actor name.
                        std::string actorNoun = line.entries[2].key;

                        // Create and add node.
                        SpeakerAnimNode* node = new SpeakerAnimNode();
                        node->frameNumber = frameNumber;
                        node->actorNoun = actorNoun;

                        // When CAPTION and SPEAKER nodes exist on the same frame, it's important the SPEAKER nodes are processed first.
                        // To help with that, we'll always put SPEAKER nodes at the beginning of the node list.
                        mFrames[frameNumber].insert(mFrames[frameNumber].begin(), node);
                    }
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "CAPTION"))
                {
                    // [FRAME] CAPTION [CAPTION]
                    if(line.entries.size() >= 3)
                    {
                        // Read caption.
                        // Unfortunately, the caption *may* contain commas, which interfers with the INI parser. Need to loop to populate.
                        std::string caption = line.entries[2].key;
                        for(size_t j = 3; j < line.entries.size(); ++j)
                        {
                            caption += ", ";
                            caption += line.entries[j].key;
                        }

                        // Create and add node.
                        CaptionAnimNode* node = new CaptionAnimNode();
                        node->frameNumber = frameNumber;
                        node->caption = caption;
                        mFrames[frameNumber].push_back(node);
                    }
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "SPEAKERCAPTION"))
                {
                    // [FRAME] SPEAKERCAPTION [END_FRAME] [NOUN]
                    if(line.entries.size() >= 4)
                    {
                        // Read end frame.
                        int endFrame = line.entries[2].GetValueAsInt();

                        // Read actor who is doing the caption.
                        std::string actorNoun = line.entries[3].key;

                        // Read caption.
                        // Unfortunately, the caption *may* contain commas, which interfers with the INI parser. Need to loop to populate.
                        std::string caption = line.entries[3].key;
                        for(size_t j = 4; j < line.entries.size(); ++j)
                        {
                            caption += ", ";
                            caption += line.entries[j].key;
                        }

                        SpeakerCaptionAnimNode* node = new SpeakerCaptionAnimNode();
                        node->frameNumber = frameNumber;
                        node->endFrameNumber = endFrame;
                        node->speaker = actorNoun;
                        node->caption = caption;
                        mFrames[frameNumber].push_back(node);
                    }
                }
				else if(StringUtil::EqualsIgnoreCase(keyword, "DIALOGUECUE"))
                {
					// No options for this one.
					
                    // Create and add node.
					DialogueCueAnimNode* node = new DialogueCueAnimNode();
                    node->frameNumber = frameNumber;
					mFrames[frameNumber].push_back(node);
                }
                else if(StringUtil::EqualsIgnoreCase(keyword, "DIALOGUE"))
                {
                    // [FRAME] DIALOGUE [LICENSE_PLATE]
                    if(line.entries.size() >= 3)
                    {
                        // Read YAK license plate (file name).
                        DialogueAnimNode* node = new DialogueAnimNode();
                        node->frameNumber = frameNumber;

                        // ANM/MOM/YAK files *almost* always prepend the language code (E) to the license plat. *Almost* always.
                        // Since it's not consistent however, we need to come up with a hueristic to get it working all the time...
                        
                        // If the first character is a number, there is no language code - use as-is.
                        if(isdigit(line.entries[2].key[0]))
                        {
                            node->licensePlate = line.entries[2].key;
                        }
                        else if(line.entries[2].key[0] == 'C' || line.entries[2].key[0] == 'D')
                        {
                            // A handful of YAK files start with C or D and do not have language codes - use as-is.
                            node->licensePlate = line.entries[2].key;
                        }
                        else // Appears to have a language code - remove the first character.
                        {
                            node->licensePlate = line.entries[2].key.substr(1);
                        }
                        
                        mFrames[frameNumber].push_back(node);
                    }
                }
                else
                {
                    std::cout << "Unexpected GK3 animation keyword: " << keyword << std::endl;
                }
            }
        }
        else if(StringUtil::EqualsIgnoreCase(section.name, "SETTINGS"))
        {
            // All MOM animations have a "SETTINGS" section, but no keys present in the section.
            // Just putting this here to silence the unexpected header warning.
        }
        else
        {
            std::cout << "Unexpected animation header: " << section.name << std::endl;
        }
    }
}
