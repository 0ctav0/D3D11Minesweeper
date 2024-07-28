#pragma once

class SoundSystem {
private:
   std::unique_ptr<DirectX::AudioEngine> audioEngine_;
   std::vector<std::unique_ptr<DirectX::SoundEffect>> pigSounds_ = {};

public:
   std::unique_ptr<DirectX::SoundEffect> defeat;
   std::unique_ptr<DirectX::SoundEffect> win;

   ~SoundSystem();
   bool Init();
   void PlayPig();
};