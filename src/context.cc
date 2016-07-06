#include "bindings.h"

Nan::Persistent<v8::Function> OboeContext::constructor;

OboeContext::OboeContext(
  const std::string& layer,
  const std::string& token,
  int flags,
  int sampleRate
) : settings(const_cast<oboe_settings_ctx_t*>(
  oboe_settings_ctx_create(layer.c_str(), token.c_str(), flags, sampleRate)
)) {}

OboeContext::~OboeContext() {
  // if (settings != NULL) {
  //   oboe_settings_ctx_destroy(settings);
  // }
}

NAN_SETTER(OboeContext::setDefaultAppToken) {}
NAN_GETTER(OboeContext::getDefaultAppToken) {
  info.GetReturnValue().Set(Nan::New(oboe_get_apptoken()).ToLocalChecked());
}

/**
 * Check if the current request should be traced based on the current settings.
 *
 * If xtrace is empty, or if it is identified as a foreign (ie. cross customer)
 * trace, then sampling will be considered as a new trace.
 * Otherwise sampling will be considered as adding to the current trace.
 * Different layers may have special rules.  Also special rules for AppView
 * Web synthetic traces apply if in_tv_meta is given a non-empty string.
 *
 * This is designed to be called once per layer per request.
 *
 * @param layer Name of the layer being considered for tracing
 * @param in_xtrace Incoming X-Trace ID (NULL or empty string if not present)
 * @param in_tv_meta AppView Web ID from X-TV-Meta HTTP header or higher layer (NULL or empty string if not present).
 * @param url URL or other identity string to sample with
 * @return Zero to not trace; otherwise return the sample rate used in the low order
 *         bytes 0 to 2 and the sample source in the higher-order byte 3.
 */
NAN_METHOD(OboeContext::shouldSample) {
  std::string in_xtrace;
  std::string in_tv_meta;

  // If the second argument is present, it must be a string
  if (info.Length() >= 1 && !info[0]->IsNull()) {
    if (!info[0]->IsString()) {
      return Nan::ThrowTypeError("X-Trace ID must be a string");
    }
    in_xtrace = *Nan::Utf8String(info[0]);
  }

  // If the third argument is present, it must be a string
  if (info.Length() >= 2 && !info[1]->IsNull()) {
    if (!info[1]->IsString()) {
      return Nan::ThrowTypeError("AppView Web ID must be a string");
    }
    in_tv_meta = *Nan::Utf8String(info[1]);
  }

  // If the URL argument is present, it must be a string
  std::string url = std::string("");
  if (info.Length() >= 3 && !info[2]->IsNull()) {
    if (!info[2]->IsString()) {
      return Nan::ThrowTypeError("URL/Identity must be a string");
    }
    url = *Nan::Utf8String(info[2]);
  }

  char retbuf[OBOE_SAMPLE_PARAM_BUFFER_MAX];
  int retbuflen = OBOE_SAMPLE_PARAM_BUFFER_MAX;

  OboeContext* self = ObjectWrap::Unwrap<OboeContext>(info.This());

  oboe_should_trace(
    self->settings,
    retbuf,
    &retbuflen,
    in_xtrace.c_str(),
    url.c_str(),
    in_tv_meta.c_str()
  );

  info.GetReturnValue().Set(
    Nan::CopyBuffer(retbuf, retbuflen).ToLocalChecked()
  );
}

NAN_METHOD(OboeContext::toString) {
  char buf[OBOE_MAX_METADATA_PACK_LEN];

  oboe_metadata_t *md = oboe_context_get();
  int rc = oboe_metadata_tostr(md, buf, sizeof(buf) - 1);
  if (rc == 0) {
    info.GetReturnValue().Set(Nan::New(buf).ToLocalChecked());
  } else {
    info.GetReturnValue().Set(Nan::New("").ToLocalChecked());
  }
}

NAN_METHOD(OboeContext::set) {
  // Validate arguments
  if (info.Length() != 1) {
    return Nan::ThrowError("Wrong number of arguments");
  }
  if (!info[0]->IsObject() && !info[0]->IsString()) {
    return Nan::ThrowTypeError("You must supply a Metadata instance or string");
  }

  if (info[0]->IsObject()) {
    // Unwrap metadata instance from arguments
    Metadata* metadata = Nan::ObjectWrap::Unwrap<Metadata>(info[0]->ToObject());

    // Set the context data from the metadata instance
    oboe_context_set(&metadata->metadata);
  } else {
    // Get string data from arguments
    Nan::Utf8String val(info[0]);

    // Set the context data from the converted string
    int status = oboe_context_set_fromstr(*val, val.length());
    if (status != 0) {
      return Nan::ThrowError("Could not set context by metadata string id");
    }
  }
}

NAN_METHOD(OboeContext::copy) {
  Metadata* md = new Metadata(oboe_context_get());
  info.GetReturnValue().Set(Metadata::NewInstance(md));
  delete md;
}

NAN_METHOD(OboeContext::clear) {
  oboe_context_clear();
}

NAN_METHOD(OboeContext::isValid) {
  info.GetReturnValue().Set(Nan::New<v8::Boolean>(oboe_context_is_valid()));
}

NAN_METHOD(OboeContext::createEvent) {
  Metadata* md = new Metadata(oboe_context_get());
  info.GetReturnValue().Set(Event::NewInstance(md));
  delete md;
}

NAN_METHOD(OboeContext::startTrace) {
  oboe_metadata_t* md = oboe_context_get();
  oboe_metadata_random(md);
  info.GetReturnValue().Set(Event::NewInstance());
}

// Creates a new Javascript instance
NAN_METHOD(OboeContext::New) {
  if (!info.IsConstructCall()) {
    return Nan::ThrowError("Context() must be called as a constructor");
  }
  if (info.Length() < 1 || !info[0]->IsString()) {
    return Nan::ThrowError("Context() must be given a layer name");
  }
  std::string layer = *Nan::Utf8String(info[0]);

  std::string token;
  if (info.Length() < 2) {
    if (info[1]->IsString()) {
      token = *Nan::Utf8String(info[1]);
      if (token.length() != 32) {
        return Nan::ThrowTypeError("App token must be 32 characters long");
      }
    } else if (info[1]->IsNull()) {
      token = std::string(oboe_get_apptoken());
    } else {
      return Nan::ThrowError("Context() must be given an app token or null");
    }
  }

  // Can supply numeric flags or trace mode name strings
  int flags = 0;
  if (info.Length() >= 3 && !info[2]->IsNull()) {
    if (info[2]->IsInt32()) {
      flags = info[2]->Int32Value();
    } else if (info[2]->IsString()) {
      std::string mode = *Nan::Utf8String(info[2]);
      if (mode.compare("always") == 0) {
        flags = OBOE_SETTINGS_FLAG_SAMPLE_START
          | OBOE_SETTINGS_FLAG_SAMPLE_THROUGH_ALWAYS
          | OBOE_SETTINGS_FLAG_SAMPLE_AVW_ALWAYS;
      } else if (mode.compare("through") == 0) {
        flags = OBOE_SETTINGS_FLAG_SAMPLE_THROUGH_ALWAYS;
      } else if (mode.compare("never") != 0) {
        return Nan::ThrowError("Invalid trace mode string");
      }
    } else {
      return Nan::ThrowError("Must supply a trace mode string or flags");
    }
  }

  int sampleRate = -1;
  if (info.Length() >= 4 && !info[3]->IsNull()) {
    if (!info[3]->IsInt32()) {
      return Nan::ThrowError("Sample rate must be a number");
    }
    sampleRate = info[3]->Int32Value();
  }

  OboeContext* ctx = new OboeContext(
    layer,
    token,
    flags,
    sampleRate
  );

  ctx->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

void OboeContext::Init(v8::Local<v8::Object> module) {
  Nan::HandleScope scope;

  // Prepare constructor template
  v8::Local<v8::FunctionTemplate> ctor = Nan::New<v8::FunctionTemplate>(New);
  ctor->InstanceTemplate()->SetInternalFieldCount(2);
  ctor->SetClassName(Nan::New("Context").ToLocalChecked());

  Nan::SetMethod(ctor, "toString", OboeContext::toString);
  Nan::SetMethod(ctor, "set", OboeContext::set);
  Nan::SetMethod(ctor, "copy", OboeContext::copy);
  Nan::SetMethod(ctor, "clear", OboeContext::clear);
  Nan::SetMethod(ctor, "isValid", OboeContext::isValid);
  Nan::SetMethod(ctor, "createEvent", OboeContext::createEvent);
  Nan::SetMethod(ctor, "startTrace", OboeContext::startTrace);

  // Set Prototype methods
  Nan::SetPrototypeMethod(ctor, "shouldSample", OboeContext::shouldSample);

  // Provide appToken getter and setter for new liboboe
  Nan::SetAccessor(
    static_cast<v8::Local<v8::Object> >(ctor->GetFunction()),
    Nan::New("defaultAppToken").ToLocalChecked(),
    OboeContext::getDefaultAppToken,
    OboeContext::setDefaultAppToken
  );

  constructor.Reset(ctor->GetFunction());
  Nan::Set(module, Nan::New("Context").ToLocalChecked(), ctor->GetFunction());
}
