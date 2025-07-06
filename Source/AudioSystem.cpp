#include "AudioSystem.h"
#include "SDL.h"
#include "SDL_mixer.h"
#include <filesystem>
#include "Random.h"

SoundHandle SoundHandle::Invalid;

// Create the AudioSystem with specified number of channels
// (Defaults to 8 channels)
AudioSystem::AudioSystem(int numChannels)
{
    // --------------
    // TODO - PARTE 4
    // --------------

    // TODO 1.: Inicialize a SDL_mixer com a taxa de amostragem de 44100 Hz, formato padrão, 2 canais (estéreo) e
    //  tamanho do buffer de 2048 bytes.
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
	{
		SDL_Log("Failed to initialize SDL_mixer: %s", Mix_GetError());
	}

    // TODO 2. : Utilize a função Mix_AllocateChannels para alocar o número de canais especificado.
	Mix_AllocateChannels(numChannels);

    // TODO 3.: Redimensione o vetor mChannels para o número de canais especificado.
	mChannels.resize(numChannels);
}

// Destroy the AudioSystem
AudioSystem::~AudioSystem()
{
    // --------------
    // TODO - PARTE 4
    // --------------

    // TODO 1.: Percorra o mapa mSounds e libere cada Mix_Chunk usando Mix_FreeChunk. Em seguida, limpe o mapa mSounds
    //  e feche o áudio com Mix_CloseAudio.
	// Libera todos os Mix_Chunks no mapa mSounds
	for (auto& pair : mSounds) {
		if (pair.second) {
			Mix_FreeChunk(pair.second);
		}
	}
	// Limpa o mapa
	mSounds.clear();

	// Fecha o subsistema de áudio
	Mix_CloseAudio();
}

// Updates the status of all the active sounds every frame
void AudioSystem::Update(float deltaTime)
{
    // --------------
    // TODO - PARTE 4
    // --------------

    // TODO 1.: Percorra o vetor mChannels e verifique se cada canal é válido (ou seja, se está tocando um som).
    //  Se o canal não estiver tocando um som (Mix_Playing retorna 0), remova o canal do mapa mHandleMap
    //  e reinicialize o canal chamando Reset().
	for (int channel = 0; channel < static_cast<int>(mChannels.size()); channel++) {
		SoundHandle handle = mChannels[channel];
		// Verifica se o canal tem um handle válido
		if (handle.IsValid()) {
			// Verifica se o canal parou de tocar
			if (Mix_Playing(channel) == 0) {
				// Remove do mapa de handles
				mHandleMap.erase(handle);

				// Marca o canal como livre
				mChannels[channel].Reset();
			}
		}
	}
}

// Plays the sound with the specified name and loops if looping is true
// Returns the SoundHandle which is used to perform any other actions on the
// sound when active
// NOTE: The soundName is without the "Assets/Sounds/" part of the file
//       For example, pass in "ChompLoop.wav" rather than
//       "Assets/Sounds/ChompLoop.wav".
SoundHandle AudioSystem::PlaySound(const std::string& soundName, bool looping)
{
    // Get the sound with the given name
    Mix_Chunk *sound = GetSound(soundName);

    // --------------
    // TODO - PARTE 4
    // --------------

    // TODO 1.: Verifique se o som foi carregado corretamente, ou seja, se o ponteiro sound é diferente de nullptr.
    //  Se for nullptr, registre um erro no log e retorne SoundHandle::Invalid.
	if (!sound) {
		SDL_Log("Erro: Som '%s' não carregado corretamente.", soundName.c_str());
		return SoundHandle::Invalid;
	}

    int availableChannel = -1;

    // TODO 2.: Percorra o vetor mChannels e verifique com a função IsValid() se algum canal está disponível
    //  (ou seja, se não está tocando nenhum som). Se encontrar um canal disponível, armazene o índice do canal
    //  em `availableChannel` e quebre do loop.
	for (size_t i = 0; i < mChannels.size(); ++i) {
		if (!mChannels[i].IsValid()) {
			availableChannel = static_cast<int>(i);
			break;
		}
	}

    // TODO 3.: Se não encontrar um canal disponível, ou seja, `availableChannel` ainda for -1,
    //  percorra o mapa mHandleMap e verifique se algum som com o mesmo nome já está tocando.
    //  Se encontrar, armazene o canal desse som em `availableChannel`, registre um log informando que
    //  o som está sendo parado, remova o som do mapa mHandleMap e quebre do loop.
	if (availableChannel == -1)	{
		for (auto it = mHandleMap.begin(); it != mHandleMap.end(); ++it) {
			if (it->second.mSoundName == soundName)	{
				availableChannel = it->second.mChannel;
				SDL_Log("Canal ocupado com mesmo som '%s'. Parando som atual.", soundName.c_str());
				mHandleMap.erase(it);
				break;
			}
		}
	}

    // TODO 4.: Se ainda não encontrar um canal disponível, percorra o mapa mHandleMap e verifique se algum som
    //  está tocando em loop (mIsLooping é true). Se encontrar, pare esse som chamando StopSound,
    //  armazene o canal desse som em `availableChannel`, registre um log informando que o som está sendo parado
    //  e quebre do loop.
	if (availableChannel == -1)	{
		for (auto it = mHandleMap.begin(); it != mHandleMap.end(); ++it) {
			if (!it->second.mIsLooping) {
				availableChannel = it->second.mChannel;
				SDL_Log("Parando som sem loop '%s' para liberar canal.", it->second.mSoundName.c_str());
				StopSound(it->first);
				break;
			}
		}
	}

    // TODO 5.: Se ainda não encontrar um canal disponível, registre um log informando que o som mais antigo no mapa
    //  mHandleMap está sendo parado, chame StopSound passando o primeiro som do mapa mHandleMap e
    //  armazene o canal desse som em `availableChannel`.
	if (availableChannel == -1 && !mHandleMap.empty()) {
		auto it = mHandleMap.begin(); // Mais antigo
		availableChannel = it->second.mChannel;
		SDL_Log("Sem canais disponíveis. Parando som mais antigo '%s'.", it->second.mSoundName.c_str());
		StopSound(it->first);
	}

	// Se ainda assim não tiver canal, retorna inválido
	if (availableChannel == -1)
	{
		SDL_Log("Erro: Nenhum canal disponível para tocar o som '%s'.", soundName.c_str());
		return SoundHandle::Invalid;
	}

    // TODO 6.: Incremente o mLastHandle em 1 para gerar um novo identificador de som.
	++mLastHandle;

    // TODO 7. : Crie uma nova HandleInfo com o nome do som, o canal disponível e se o som está em loop.
    //  Em seguida, adicione essa HandleInfo ao mapa mHandleMap com o mLastHandle como chave.
    //  Além disso, armazene o mLastHandle no vetor mChannels na posição do canal disponível.
	HandleInfo info;
	info.mSoundName = soundName;
	info.mChannel = availableChannel;
	info.mIsLooping = looping;
	info.mIsPaused = false;

	mHandleMap.emplace(mLastHandle, info);
	mChannels[availableChannel] = mLastHandle;

    // TODO 8.: Use Mix_PlayChannel para tocar o som no canal disponível. Não se esqueça de passar -1
    //  para o parâmetro de loop se looping for true, ou 0 se não for.
	int loopCount = looping ? -1 : 0;
	Mix_PlayChannel(availableChannel, sound, loopCount);

    return mLastHandle;
}

SoundHandle AudioSystem::PlayVariantSound(const std::string &soundName, int numVariants, bool looping) {
	int variant = Random::GetIntRange(1, numVariants);
	size_t dotPos = soundName.rfind('.');
	std::string variantFile = soundName.substr(0, dotPos) + std::to_string(variant) + soundName.substr(dotPos);
	return PlaySound(variantFile, looping);
}


// Stops the sound if it is currently playing
void AudioSystem::StopSound(SoundHandle sound)
{
    // --------------
    // TODO - PARTE 4
    // --------------

    // TODO 1. : Verifique se o som existe no mapa mHandleMap. Se não existir, registre um log de erro
    //  e retorne. Se existir, use Mix_HaltChannel para parar o som no canal associado, apague
    //  o som do mapa mHandleMap e reinicialize o canal no vetor mChannels com a função Reset().
	auto it = mHandleMap.find(sound);
	if (it == mHandleMap.end())
	{
		SDL_Log("Erro: Tentativa de parar som com handle inválido.");
		return;
	}

	int channel = it->second.mChannel;

	// Para o som no canal associado
	Mix_HaltChannel(channel);

	// Remove do mapa e reinicializa canal
	mHandleMap.erase(it);
	mChannels[channel].Reset();
}

// Pauses the sound if it is currently playing
void AudioSystem::PauseSound(SoundHandle sound)
{
    // --------------
    // TODO - PARTE 4
    // --------------

    // TODO 1. : Verifique se o som existe no mapa mHandleMap. Se não existir, registre um log de erro
    //  e retorne. Se existir, verifique se o som já está pausado (mIsPaused). Se não estiver pausado,
    //  use Mix_Pause para pausar o som no canal associado e defina mIsPaused desse som como true no
    //  mapa mHandleMap.
	auto it = mHandleMap.find(sound);
	if (it == mHandleMap.end())
	{
		SDL_Log("Erro: Tentativa de pausar som com handle inválido.");
		return;
	}

	// Verifica se já está pausado
	if (!it->second.mIsPaused)
	{
		// Pausa o som no canal associado
		Mix_Pause(it->second.mChannel);

		// Marca como pausado
		it->second.mIsPaused = true;
	}
}

// Resumes the sound if it is currently paused
void AudioSystem::ResumeSound(SoundHandle sound)
{
    // --------------
    // TODO - PARTE 4
    // --------------

    // TODO 1. : Verifique se o som existe no mapa mHandleMap. Se não existir, registre um log de erro
    //  e retorne. Se existir, verifique se o som está pausado (mIsPaused). Se estiver pausado,
    //  use Mix_Resume para retomar o som no canal associado e defina mIsPaused desse som como false no
    //  mapa mHandleMap.
	auto it = mHandleMap.find(sound);
	if (it == mHandleMap.end())
	{
		SDL_Log("Erro: Tentativa de resume do som com handle inválido.");
		return;
	}

	// Verifica se já está pausado
	if (it->second.mIsPaused)
	{
		// Pausa o som no canal associado
		Mix_Resume(it->second.mChannel);

		// Marca como pausado
		it->second.mIsPaused = false;
	}
}

// Returns the current state of the sound
SoundState AudioSystem::GetSoundState(SoundHandle sound)
{
    if(mHandleMap.find(sound) == mHandleMap.end())
    {
        return SoundState::Stopped;
    }

    if(mHandleMap[sound].mIsPaused)
    {
        return SoundState::Paused;
    }

	return SoundState::Playing;
}

// Stops all sounds on all channels
void AudioSystem::StopAllSounds()
{
    Mix_HaltChannel(-1);

    for(auto & mChannel : mChannels)
    {
        mChannel.Reset();
    }

    mHandleMap.clear();
}

// Cache all sounds under Assets/Sounds
void AudioSystem::CacheAllSounds()
{
#ifndef __clang_analyzer__
	std::error_code ec{};
	for (const auto& rootDirEntry : std::filesystem::directory_iterator{"Assets/Sounds", ec})
	{
		std::string extension = rootDirEntry.path().extension().string();
		if (extension == ".ogg" || extension == ".wav")
		{
			std::string fileName = rootDirEntry.path().stem().string();
			fileName += extension;
			CacheSound(fileName);
		}
	}
#endif
}

// Used to preload the sound data of a sound
// NOTE: The soundName is without the "Assets/Sounds/" part of the file
//       For example, pass in "ChompLoop.wav" rather than
//       "Assets/Sounds/ChompLoop.wav".
void AudioSystem::CacheSound(const std::string& soundName)
{
	GetSound(soundName);
}

// If the sound is already loaded, returns Mix_Chunk from the map.
// Otherwise, will attempt to load the file and save it in the map.
// Returns nullptr if sound is not found.
// NOTE: The soundName is without the "Assets/Sounds/" part of the file
//       For example, pass in "ChompLoop.wav" rather than
//       "Assets/Sounds/ChompLoop.wav".
Mix_Chunk* AudioSystem::GetSound(const std::string& soundName)
{
	std::string fileName = "../Assets/Sounds/";
	fileName += soundName;

	Mix_Chunk* chunk = nullptr;
	auto iter = mSounds.find(fileName);
	if (iter != mSounds.end())
	{
		chunk = iter->second;
	}
	else
	{
		chunk = Mix_LoadWAV(fileName.c_str());
		if (!chunk)
		{
			SDL_Log("[AudioSystem] Failed to load sound file %s", fileName.c_str());
			return nullptr;
		}

		mSounds.emplace(fileName, chunk);
	}
	return chunk;
}

// Input for debugging purposes
void AudioSystem::ProcessInput(const Uint8* keyState)
{
	// Debugging code that outputs all active sounds on leading edge of period key
	if (keyState[SDL_SCANCODE_PERIOD] && !mLastDebugKey)
	{
		SDL_Log("[AudioSystem] Active Sounds:");
		for (size_t i = 0; i < mChannels.size(); i++)
		{
			if (mChannels[i].IsValid())
			{
				auto iter = mHandleMap.find(mChannels[i]);
				if (iter != mHandleMap.end())
				{
					HandleInfo& hi = iter->second;
					SDL_Log("Channel %d: %s, %s, looping = %d, paused = %d",
							static_cast<unsigned>(i), mChannels[i].GetDebugStr(),
							hi.mSoundName.c_str(), hi.mIsLooping, hi.mIsPaused);
				}
				else
				{
					SDL_Log("Channel %d: %s INVALID", static_cast<unsigned>(i),
							mChannels[i].GetDebugStr());
				}
			}
		}
	}

	mLastDebugKey = keyState[SDL_SCANCODE_PERIOD];
}
