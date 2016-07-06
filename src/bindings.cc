#include "bindings.h"

// Components
#include "sanitizer.cc"
#include "metadata.cc"
#include "context.cc"
#include "config.cc"
#include "event.cc"
#include "reporters/udp.cc"
#include "reporters/file.cc"

extern "C" {

// Register the exposed parts of the module
void init(v8::Local<v8::Object> exports) {
  Nan::HandleScope scope;

  Nan::Set(exports, Nan::New("MAX_SAMPLE_RATE").ToLocalChecked(), Nan::New(OBOE_SAMPLE_RESOLUTION));
  Nan::Set(exports, Nan::New("MAX_METADATA_PACK_LEN").ToLocalChecked(), Nan::New(OBOE_MAX_METADATA_PACK_LEN));
  Nan::Set(exports, Nan::New("MAX_TASK_ID_LEN").ToLocalChecked(), Nan::New(OBOE_MAX_TASK_ID_LEN));
  Nan::Set(exports, Nan::New("MAX_OP_ID_LEN").ToLocalChecked(), Nan::New(OBOE_MAX_OP_ID_LEN));
  Nan::Set(exports, Nan::New("TRACE_NEVER").ToLocalChecked(), Nan::New(OBOE_TRACE_NEVER));
  Nan::Set(exports, Nan::New("TRACE_ALWAYS").ToLocalChecked(), Nan::New(OBOE_TRACE_ALWAYS));
  Nan::Set(exports, Nan::New("TRACE_THROUGH").ToLocalChecked(), Nan::New(OBOE_TRACE_THROUGH));

  // liboboe 2.x sample flags
  Nan::Set(exports, Nan::New("SAMPLE_START").ToLocalChecked(), Nan::New(OBOE_SETTINGS_FLAG_SAMPLE_START));
  Nan::Set(exports, Nan::New("SAMPLE_THROUGH_ALWAYS").ToLocalChecked(), Nan::New(OBOE_SETTINGS_FLAG_SAMPLE_THROUGH_ALWAYS));
  Nan::Set(exports, Nan::New("SAMPLE_AVW_ALWAYS").ToLocalChecked(), Nan::New(OBOE_SETTINGS_FLAG_SAMPLE_AVW_ALWAYS));

  FileReporter::Init(exports);
  UdpReporter::Init(exports);
  OboeContext::Init(exports);
  Sanitizer::Init(exports);
  Metadata::Init(exports);
  Event::Init(exports);
  Config::Init(exports);

  oboe_init();
}

NODE_MODULE(traceview_bindings, init)

}
