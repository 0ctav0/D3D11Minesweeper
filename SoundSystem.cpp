#include "pch.h"
#include "SoundSystem.h"

namespace {
   auto constexpr PIG_SOUNDS_NUMBER = 9;
}

SoundSystem::~SoundSystem() {
   if (audioEngine_) {
      audioEngine_->Suspend();
   }
}

bool SoundSystem::Init() {
   DirectX::AUDIO_ENGINE_FLAGS eflags = DirectX::AudioEngine_Default;
#ifdef _DEBUG
   eflags |= DirectX::AudioEngine_Debug;
#endif
   audioEngine_ = std::make_unique<DirectX::AudioEngine>(eflags);

   defeat = std::make_unique<DirectX::SoundEffect>(audioEngine_.get(), L"sounds/defeat.wav");
   win = std::make_unique<DirectX::SoundEffect>(audioEngine_.get(), L"sounds/win.wav");
   for (auto i = 1; i <= PIG_SOUNDS_NUMBER; i++) {
      wchar_t buf[100];
      swprintf_s(buf, L"sounds/pig%i.wav", i);
      pigSounds_.push_back(std::make_unique<DirectX::SoundEffect>(audioEngine_.get(), buf));
   }
   return true;
}

void SoundSystem::PlayPig() {
   std::random_device dev;
   std::mt19937 rng(dev());
   std::uniform_int_distribution<std::mt19937::result_type> dist(0, PIG_SOUNDS_NUMBER - 1);
   pigSounds_[dist(rng)]->Play();
}
