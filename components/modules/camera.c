// Module for interfacing with camera via i2s

#include "module.h"
#include "lauxlib.h"
#include "platform.h"
#include "camera.h"

// nice-to-have configuration of image type and size

static camera_pixelformat_t s_pixel_format;

// Lua: init()
static int lcamera_init(lua_State *L)
{
  esp_err_t err;

  camera_config_t camera_config = {
    .ledc_channel = LEDC_CHANNEL_0,
    .ledc_timer = LEDC_TIMER_0,
    .pin_d0 = 17,
    .pin_d1 = 35,
    .pin_d2 = 34,
    .pin_d3 = 5,
    .pin_d4 = 39,
    .pin_d5 = 18,
    .pin_d6 = 36,
    .pin_d7 = 19,
    .pin_xclk = 27,
    .pin_pclk = 21,
    .pin_vsync = 22,
    .pin_href = 26,
    .pin_sscb_sda = 25,
    .pin_sscb_scl = 23,
    .pin_reset = 15,
    .xclk_freq_hz = 20000000
  };

  camera_model_t camera_model;
  err = camera_probe(&camera_config, &camera_model);
  if(err != ESP_OK) {
    luaL_error(L, "failed to find a camera during probe");
  }

  if(camera_model == CAMERA_OV7725) {
    s_pixel_format = CAMERA_PF_JPEG;
    camera_config.frame_size = CAMERA_FS_QQVGA;
  }
  else if(camera_model == CAMERA_OV2640) {
    s_pixel_format = CAMERA_PF_JPEG;
    camera_config.frame_size = CAMERA_FS_SVGA;
    camera_config.jpeg_quality = 20;
  }
  else {
    luaL_error(L, "unsupported camera detected");
  }

  camera_config.pixel_format = s_pixel_format;
  err = camera_init(&camera_config);
  if (err != ESP_OK) {
    luaL_error(L, "camera init failed");
  }

  return 0;
}

// Lua: capture()
static int lcamera_capture(lua_State *L)
{
  esp_err_t err = camera_run();
  if(err != ESP_OK) {
    luaL_error(L, "failed to capture a frame");
  }

  return 0;
}

// Lua: data_size()
static int lcamera_image_size(lua_State *L)
{
  size_t image_size = camera_get_data_size();
  lua_pushinteger(L, (lua_Integer)image_size);

  return 1;
}

// Lua: image_chunk(offset, count)
static int lcamera_image_chunk(lua_State *L)
{
  int offset = luaL_checkinteger(L, 1);
  int count = luaL_checkinteger(L, 2);

  const char *chunk = (const char *)(camera_get_fb() + offset);
  lua_pushlstring(L, chunk, count);

  return 1;
}

// Module function map
static const LUA_REG_TYPE camera_map[] =
{
  { LSTRKEY("init"),         LFUNCVAL(lcamera_init) },
  { LSTRKEY("capture"),      LFUNCVAL(lcamera_capture) },
  { LSTRKEY("image_size"),   LFUNCVAL(lcamera_image_size) },
  { LSTRKEY("image_chunk"),  LFUNCVAL(lcamera_image_chunk) },
  { LNILKEY, LNILVAL }
};

NODEMCU_MODULE(CAMERA, "camera", camera_map, NULL);
