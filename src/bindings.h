#ifndef NODE_OBOE_H_
#define NODE_OBOE_H_

#include <iostream>
#include <string>

#include <node.h>
#include <nan.h>
#include <uv.h>
#include <v8.h>

#include <oboe/oboe.h>

class Event;

class Metadata : public Nan::ObjectWrap {
  friend class UdpReporter;
  friend class FileReporter;
  friend class OboeContext;
  friend class Event;

  ~Metadata();
  Metadata();
  Metadata(oboe_metadata_t*);

  oboe_metadata_t metadata;
  static Nan::Persistent<v8::Function> constructor;
  static NAN_METHOD(New);
  static NAN_METHOD(fromString);
  static NAN_METHOD(makeRandom);
  static NAN_METHOD(copy);
  static NAN_METHOD(isValid);
  static NAN_METHOD(toString);
  static NAN_METHOD(createEvent);

  static v8::Local<v8::Object> NewInstance(Metadata*);
  static v8::Local<v8::Object> NewInstance();

  public:
    static void Init(v8::Local<v8::Object>);
};

class OboeContext : public Nan::ObjectWrap {
  friend class UdpReporter;
  friend class FileReporter;
  friend class Metadata;
  friend class Event;

  oboe_settings_ctx_t* settings;

  OboeContext(const std::string&, const std::string&, int, int);
  ~OboeContext();

  static Nan::Persistent<v8::Function> constructor;
  static NAN_METHOD(New);

  // V8 conversion
  static NAN_METHOD(shouldSample);
  static NAN_METHOD(toString);
  static NAN_METHOD(set);
  static NAN_METHOD(copy);
  static NAN_METHOD(clear);
  static NAN_METHOD(isValid);
  static NAN_METHOD(createEvent);
  static NAN_METHOD(startTrace);

  static NAN_GETTER(getDefaultAppToken);
  static NAN_SETTER(setDefaultAppToken);

  public:
    static void Init(v8::Local<v8::Object>);
};

class Event : public Nan::ObjectWrap {
  friend class UdpReporter;
  friend class FileReporter;
  friend class OboeContext;
  friend class Metadata;
  friend class Log;

  explicit Event();
  explicit Event(const oboe_metadata_t*, bool);
  ~Event();

  oboe_event_t event;
  static Nan::Persistent<v8::Function> constructor;
  static NAN_METHOD(New);
  static NAN_METHOD(addInfo);
  static NAN_METHOD(addEdge);
  static NAN_METHOD(getMetadata);
  static NAN_METHOD(toString);
  static NAN_METHOD(startTrace);

  static v8::Local<v8::Object> NewInstance(Metadata*, bool);
  static v8::Local<v8::Object> NewInstance(Metadata*);
  static v8::Local<v8::Object> NewInstance();

  public:
    static void Init(v8::Local<v8::Object>);
};

class UdpReporter : public Nan::ObjectWrap {
  UdpReporter();
  ~UdpReporter();
  int send(oboe_metadata_t*, oboe_event_t*);

  std::string host;
  std::string port;
  bool connected;
  oboe_reporter_t reporter;
  static Nan::Persistent<v8::Function> constructor;
  static NAN_METHOD(New);
  static NAN_METHOD(sendReport);
  static NAN_SETTER(setAddress);
  static NAN_GETTER(getAddress);
  static NAN_SETTER(setPort);
  static NAN_GETTER(getPort);
  static NAN_SETTER(setHost);
  static NAN_GETTER(getHost);

  public:
    static void Init(v8::Local<v8::Object>);
};

class FileReporter : public Nan::ObjectWrap {
  ~FileReporter();
  FileReporter(const char*);

  oboe_reporter_t reporter;
  static Nan::Persistent<v8::Function> constructor;
  static NAN_METHOD(New);
  static NAN_METHOD(sendReport);

  public:
    static void Init(v8::Local<v8::Object>);
};

class Config {
  static NAN_METHOD(getRevision);
  static NAN_METHOD(getVersion);
  static NAN_METHOD(checkVersion);

  public:
    static void Init(v8::Local<v8::Object>);
};

class Sanitizer {
  static NAN_METHOD(sanitize);

  public:
    static void Init(v8::Local<v8::Object>);
};

#endif  // NODE_OBOE_H_
