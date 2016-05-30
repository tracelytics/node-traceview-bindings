#include "bindings.h"

#ifdef OBOE_SETTINGS_APP_TOKEN_SZ
NAN_GETTER(OboeContext::getAppToken) {
  info.GetReturnValue().Set(Nan::New(
    SettingsContext::instance()->getAppToken()
  ).ToLocalChecked());
}

NAN_SETTER(OboeContext::setAppToken) {
  if (!value->IsString()) {
    return Nan::ThrowTypeError("App token must be a string");
  }

  Nan::Utf8String utf8_value(value);
  if (strlen(*utf8_value) != 32) {
    return Nan::ThrowTypeError("App token must be a 32 characters long");
  }

  SettingsContext::instance()->setAppToken(*utf8_value);

  info.GetReturnValue().Set(Nan::Undefined());
}
#endif

/**
 * Set the tracing mode.
 *
 * @param newMode One of
 * - OBOE_TRACE_NEVER(0) to disable tracing,
 * - OBOE_TRACE_ALWAYS(1) to start a new trace if needed, or
 * - OBOE_TRACE_THROUGH(2) to only add to an existing trace.
 */
NAN_METHOD(OboeContext::setTracingMode) {
  // Validate arguments
  if (info.Length() != 1) {
    return Nan::ThrowError("Wrong number of arguments");
  }
  if (!info[0]->IsNumber()) {
    return Nan::ThrowTypeError("Tracing mode must be a number");
  }

  int mode = info[0]->NumberValue();
  if (mode < 0 || mode > 2) {
    return Nan::ThrowRangeError("Invalid tracing mode");
  }

  // Set trace mode on settings context
  SettingsContext::instance()->setTraceMode(mode);
}

/**
 * Set the default sample rate.
 *
 * This rate is used until overridden by the TraceView servers.  If not set then the
 * value 300,000 will be used (ie. 30%).
 *
 * The rate is interpreted as a ratio out of OBOE_SAMPLE_RESOLUTION (currently 1,000,000).
 *
 * @param newRate A number between 0 (none) and OBOE_SAMPLE_RESOLUTION (a million)
 */
NAN_METHOD(OboeContext::setDefaultSampleRate) {
  // Validate arguments
  if (info.Length() != 1) {
    return Nan::ThrowError("Wrong number of arguments");
  }
  if (!info[0]->IsNumber()) {
    return Nan::ThrowTypeError("Sample rate must be a number");
  }

  int rate = info[0]->NumberValue();
  if (rate < 1 || rate > OBOE_SAMPLE_RESOLUTION) {
    return Nan::ThrowRangeError("Sample rate out of range");
  }

  oboe_settings_cfg_sample_rate_set(rate);
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
 * @return Zero to not trace; otherwise return the sample rate used in the low order
 *         bytes 0 to 2 and the sample source in the higher-order byte 3.
 */
NAN_METHOD(OboeContext::sampleRequest) {
  // Validate arguments
  if (info.Length() < 1) {
    return Nan::ThrowError("Wrong number of arguments");
  }

  std::string layer_name;
  std::string in_xtrace;
  std::string in_tv_meta;

  // The first argument must be a string
  if (!info[0]->IsString()) {
    return Nan::ThrowTypeError("Layer name must be a string");
  }
  layer_name = *Nan::Utf8String(info[0]);

  // If the second argument is present, it must be a string
  if (info.Length() >= 2) {
    if ( ! info[1]->IsString()) {
      return Nan::ThrowTypeError("X-Trace ID must be a string");
    }
    in_xtrace = *Nan::Utf8String(info[1]);
  }

  // If the third argument is present, it must be a string
  if (info.Length() >= 3) {
    if ( ! info[2]->IsString()) {
      return Nan::ThrowTypeError("AppView Web ID must be a string");
    }
    in_tv_meta = *Nan::Utf8String(info[2]);
  }

  SettingsContext* ctx = SettingsContext::instance();
  ctx->setLayer(layer_name);
  // TODO: Provide real URL
  std::string url = std::string("");
  ctx->sample(in_xtrace, url, in_tv_meta);

#ifdef OBOE_SETTINGS_APP_TOKEN_SZ
  std::string td = ctx->getTraceData();
  info.GetReturnValue().Set(
    Nan::CopyBuffer(td.data(), td.size()).ToLocalChecked()
  );
#else
  // Store rc, sample_source and sample_rate in an array
  v8::Local<v8::Array> array = Nan::New<v8::Array>(2);
  Nan::Set(array, 0, Nan::New(ctx->getRetCode()));
  Nan::Set(array, 1, Nan::New(ctx->getSampleSource()));
  Nan::Set(array, 2, Nan::New(ctx->getSampleRate()));
  info.GetReturnValue().Set(array);
#endif
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

void OboeContext::Init(v8::Local<v8::Object> module) {
  Nan::HandleScope scope;

  v8::Local<v8::Object> exports = Nan::New<v8::Object>();
  Nan::SetMethod(exports, "setTracingMode", OboeContext::setTracingMode);
  Nan::SetMethod(exports, "setDefaultSampleRate", OboeContext::setDefaultSampleRate);
  Nan::SetMethod(exports, "sampleRequest", OboeContext::sampleRequest);
  Nan::SetMethod(exports, "toString", OboeContext::toString);
  Nan::SetMethod(exports, "set", OboeContext::set);
  Nan::SetMethod(exports, "copy", OboeContext::copy);
  Nan::SetMethod(exports, "clear", OboeContext::clear);
  Nan::SetMethod(exports, "isValid", OboeContext::isValid);
  Nan::SetMethod(exports, "createEvent", OboeContext::createEvent);
  Nan::SetMethod(exports, "startTrace", OboeContext::startTrace);

  // Provide appToken getter and setter for new liboboe
#ifdef OBOE_SETTINGS_APP_TOKEN_SZ
  Nan::SetAccessor(
    exports,
    Nan::New("appToken").ToLocalChecked(),
    OboeContext::getAppToken,
    OboeContext::setAppToken
  );
#endif

  Nan::Set(module, Nan::New("Context").ToLocalChecked(), exports);
}
