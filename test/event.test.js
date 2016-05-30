var bindings = require('../')

describe('addon.event', function () {
  var event

  it('should construct', function () {
    event = new bindings.Event()
  })

  it('should add info', function () {
    event.addInfo('key', 'val')
  })

  if (bindings.Config.checkVersion(2, 0)) {
    it('should add buffer info', function () {
      bindings.Context.setTracingMode(bindings.TRACE_ALWAYS)
      bindings.Context.setDefaultSampleRate(bindings.MAX_SAMPLE_RATE)
      var check = bindings.Context.sampleRequest('a', 'b', 'c')

      event.addInfo('key', check)
    })

    it('should not try to add non-buffer objects', function () {
      var thrown = false
      try {
        event.addInfo('key', {})
      } catch (e) {
        thrown = true
      }
      thrown.should.equal(true)
    })
  }

  it('should add edge', function () {
    var e = new bindings.Event()
    var meta = e.getMetadata()
    event.addEdge(meta)
  })

  it('should add edge as string', function () {
    var e = new bindings.Event()
    var meta = e.toString()
    event.addEdge(meta.toString())
  })

  it('should get metadata', function () {
    var meta = event.getMetadata()
    meta.should.be.an.instanceof(bindings.Metadata)
  })

  it('should serialize metadata to id string', function () {
    var meta = event.toString()
    meta.should.be.an.instanceof(String).with.lengthOf(58)
    meta[0].should.equal('1')
    meta[1].should.equal('B')
  })

  it('should start tracing, returning a new instance', function () {
    var meta = new bindings.Metadata()
    var event2 = bindings.Event.startTrace(meta)
  })
})
