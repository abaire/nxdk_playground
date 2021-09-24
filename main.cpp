#include <hal/video.h>
#include <nxdk/mount.h>
#include <pbkit/pbkit.h>
#include <windows.h>
#include <xboxkrnl/xboxkrnl.h>

#include <chrono>
#include <mutex>
#include <thread>

std::atomic<int> threadTimeElapsed;

long long millisSince(const std::chrono::steady_clock::time_point& ref) {
  auto now = std::chrono::steady_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(now - ref)
      .count();
}

static int findAllDirectories() {
  std::string searchmask = "f:\\*";

  WIN32_FIND_DATA findData;
  auto findStart = std::chrono::steady_clock::now();
  HANDLE dirHandle = FindFirstFileA(searchmask.c_str(), &findData);
  if (dirHandle == INVALID_HANDLE_VALUE) {
    return -1;
  }
  CloseHandle(dirHandle);

  return static_cast<int>(millisSince(findStart));
}

static void threadMain() {
  int timeElapsed = findAllDirectories();
  threadTimeElapsed = timeElapsed;
}

int main() {
  XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);
  nxMountDrive('F', R"(\Device\Harddisk0\Partition6)");

  BOOL pbk_started = pb_init() == 0;
  if (!pbk_started) {
    DbgPrint("pbkit init failed\n");
    return 1;
  }

  pb_show_front_screen();

#if 0
  int primaryThreadTimeElapsed = findAllDirectories();
#else
  int primaryThreadTimeElapsed = -1;
#endif

  std::thread scannerThread = std::thread(threadMain);

  for (int frame = 0; frame < 1000; ++frame) {
    pb_wait_for_vbl();
    pb_target_back_buffer();
    pb_reset();
    pb_fill(0, 0, 640, 480, 0xFF003E3E);
    pb_erase_text_screen();

    pb_print("Playground\n");
    pb_print("On primary thread time: %d\n", primaryThreadTimeElapsed);
    int elapsed = threadTimeElapsed;
    pb_print("Thread time elapsed: %d\n", elapsed);

    pb_draw_text_screen();
    while (pb_busy())
      ;
    while (pb_finished())
      ;
  }

  if (scannerThread.joinable()) {
    scannerThread.join();
  }

  pb_kill();

  return 0;
}
