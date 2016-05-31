void SettingsContext::setTraceMode(int mode) {
  changed = true;
  traceMode = mode;
  oboe_settings_cfg_tracing_mode_set(mode);
}

void SettingsContext::setSampleRate(int rate) {
  changed = true;
  sampleRate = rate;
  oboe_settings_cfg_sample_rate_set(rate);
}

void SettingsContext::setLayer(std::string name) {
  changed = true;
  layer = name;
}
void SettingsContext::setLayer(const char* name) {
  changed = true;
  layer = std::string(name);
}

void SettingsContext::setAppToken(std::string name) {
  changed = true;
  appToken = name;
}
void SettingsContext::setAppToken(const char* name) {
  changed = true;
  appToken = std::string(name);
}

void SettingsContext::regen() {
  if (!changed) return;
  changed = false;

  int flags;

  switch (traceMode) {
    case OBOE_TRACE_ALWAYS:
      flags = OBOE_SETTINGS_FLAG_SAMPLE_START | OBOE_SETTINGS_FLAG_SAMPLE_THROUGH_ALWAYS | OBOE_SETTINGS_FLAG_SAMPLE_AVW_ALWAYS;
      break;

    case OBOE_TRACE_THROUGH:
      flags = OBOE_SETTINGS_FLAG_SAMPLE_THROUGH_ALWAYS;
      break;

    case OBOE_TRACE_NEVER:
    default:
      flags = 0;
      break;
  }

  // Cleanup old settings
  if (settings != NULL) {
    oboe_settings_ctx_destroy(settings);
  }

  settings = oboe_settings_ctx_create(
    layer.c_str(),
    appToken.c_str(),
    flags,
    sampleRate
  );
}

bool SettingsContext::sample(std::string& xtrace, std::string& url, std::string& meta) {
  // Attempt context regeneration before trying to use it
  regen();

  char retbuf[OBOE_SAMPLE_PARAM_BUFFER_MAX];
  int retbuflen = OBOE_SAMPLE_PARAM_BUFFER_MAX;

  if (oboe_should_trace(settings, retbuf, &retbuflen, xtrace.c_str(), url.c_str(), meta.c_str())) {
    traceData = std::string(retbuf, retbuflen);
    return true;
  } else {
    return false;
  }
}
