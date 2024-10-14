#pragma once

#include "DeviceManager.h"
#include "SoundSystem.h"
#include "SpriteRenderer.h"
#include "Cell.h"

using DirectX::XMFLOAT2;
using DirectX::XMFLOAT3;


// percentage of mines
enum Difficulty {
   Easy = 10,
   Medium = 15,
   Hard = 20,
   Impossible = 25
};

enum GameState {
   Play, Win, Defeat
};

auto constexpr CELLS_X = 40;
auto constexpr CELLS_Y = 20;
int constexpr MINES_COUNT = CELLS_X * CELLS_Y / 100.0f * Difficulty::Hard;
auto constexpr NEED_TO_OPEN = CELLS_X * CELLS_Y - MINES_COUNT;

const std::array<DirectX::XMVECTORF32, 8> NUMBER_TINTS = {
   DirectX::Colors::Black,
   DirectX::Colors::Magenta,
   DirectX::Colors::Red,
   DirectX::Colors::Orange,
   DirectX::Colors::AliceBlue,
   DirectX::Colors::Beige,
   DirectX::Colors::DarkOrchid,
   DirectX::Colors::Honeydew,
};

struct Pos {
   int x;
   int y;

   bool IsInBounds() {
      return (x >= 0 && x < CELLS_X && y >= 0 && y < CELLS_Y);
   }
};

struct GameData {
   std::array<std::array<Cell, CELLS_Y>, CELLS_X> cells = {};

   GameState gameState = GameState::Play;
   UINT opened = 0;
   UINT flagged = 0;
   bool started = false;
   UINT timer = 0;
};

struct CommonBuffer {
   float width;
   float height;
   float reserved[2];
};


class Game {
public:
   ~Game();
   void GetDefaultSize(long& width, long& height);
   bool ExitGame();

   void OnMouseMove();

   bool Init(HINSTANCE hInstance, HWND hwnd);
   bool LoadContent();
   void Update(float dt);
   void Thread();
   void Render();

private:
   void InitCells();
   void InitMines(int x, int y);
   Cell* GetCell(int x, int y);
   void OpenAt(int x, int y);
   void IterateAll(std::function<void(Cell* cell)> cb);
   void IterateNear(int originX, int originY, std::function<void(int, int)> cb);
   void ExploreMap(int originX, int originY);
   void OpenNearForced(int originX, int originY);
   void PressedAround(int originX, int originY);
   void UnpressedAll();
   void ClickAt(int x, int y);
   void MarkAt(int x, int y);
   bool IsCellSelected(int x, int y);
   void Start(int x, int y);
   void Defeat();
   void Win();
   void Restart();

   std::vector<char> GetDigits(int number);

   void RenderPanel(RECT1 size, PanelState state);
   void RenderTopPanel();
   void RenderNumber(DirectX::XMFLOAT2& pos, int number);
   void RenderMinesNumber();
   void RenderRestartButton();
   void RenderTimer();
   void RenderGameField();

   HINSTANCE hInstance_;
   HWND hwnd_;
   long width_;
   long height_;

   DeviceManager d3d_ = {};
   SoundSystem sound_ = {};
   SpriteRenderer sprite_ = {};

   std::unique_ptr<DirectX::Keyboard> keyboard_;
   DirectX::Keyboard::KeyboardStateTracker keyTracker_;

   std::unique_ptr<DirectX::Mouse> mouse_;
   DirectX::Mouse::Mouse::ButtonStateTracker mouseTracker_;
   bool leftHeld_ = false;

   Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
   Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;

   Microsoft::WRL::ComPtr<ID3D11VertexShader> clampTextureVS_;
   Microsoft::WRL::ComPtr<ID3D11PixelShader> clampTexturePS_;

   Microsoft::WRL::ComPtr<ID3D11Buffer> commonBuffer_;
   Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;
   Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState_;
   Microsoft::WRL::ComPtr<ID3D11BlendState> blendState_;

   RECT1 restartButtonRect_ = {};
   bool restartButtonPressed_ = false;
   Pos selectedCell_ = {};

   GameData data_;

   unsigned long time;
};

