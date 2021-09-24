#include <hal/video.h>
#include <pbkit/pbkit.h>
#include <xboxkrnl/xboxkrnl.h>

int main() {
  XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);

  BOOL pbk_started = pb_init() == 0;
  if (!pbk_started) {
    DbgPrint("pbkit init failed\n");
    return 1;
  }
  pb_show_front_screen();

  for (int frame = 0; frame < 120; ++frame) {
    pb_wait_for_vbl();
    pb_target_back_buffer();
    pb_reset();
    pb_fill(0, 0, 640, 480, 0xFF003E3E);
    pb_erase_text_screen();

    pb_print("Playground\n");

    pb_draw_text_screen();
    while (pb_busy())
      ;
    while (pb_finished())
      ;
  }

  pb_kill();

  return 0;
}
