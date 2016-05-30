var bindings = require('../')

describe('addon.context', function () {
  it('should set tracing mode to never', function () {
    bindings.Context.setTracingMode(bindings.TRACE_NEVER)
  })
  it('should set tracing mode to always', function () {
    bindings.Context.setTracingMode(bindings.TRACE_ALWAYS)
  })
  it('should set tracing mode to through', function () {
    bindings.Context.setTracingMode(bindings.TRACE_THROUGH)
  })
  it('should set tracing mode to an out-of-range input', function () {
    try {
      bindings.Context.setTracingMode(3)
    } catch (e) {
      if (e.message === 'Invalid tracing mode') {
        return
      }
    }

    throw new Error('setTracingMode should fail on invalid inputs')
  })
  it('should set tracing mode to an invalid input', function () {
    try {
      bindings.Context.setTracingMode('foo')
    } catch (e) {
      if (e.message === 'Tracing mode must be a number') {
        return
      }
    }

    throw new Error('setTracingMode should fail on invalid inputs')
  })

  it('should set valid sample rate', function () {
    bindings.Context.setDefaultSampleRate(bindings.MAX_SAMPLE_RATE / 10)
  })
  it('should set invalid sample rate', function () {
    try {
      bindings.Context.setDefaultSampleRate(bindings.MAX_SAMPLE_RATE + 1)
    } catch (e) {
      if (e.message === 'Sample rate out of range') {
        return
      }
    }

    throw new Error('setDefaultSampleRate should fail on invalid inputs')
  })

  it('should check if a request should be sampled', function () {
    bindings.Context.setTracingMode(bindings.TRACE_ALWAYS)
    bindings.Context.setDefaultSampleRate(bindings.MAX_SAMPLE_RATE)
    var check = bindings.Context.sampleRequest('a', 'b', 'c')
    if (bindings.Config.checkVersion(2, 0)) {
      check.should.be.an.instanceof(Buffer)
      check.length.should.be.above(0)
    } else {
      check.should.be.an.instanceof(Array)
      check.should.have.property(0, 1)
      check.should.have.property(1, 1)
      check.should.have.property(2, bindings.MAX_SAMPLE_RATE)
    }
  })

  it('should serialize context to string', function () {
    bindings.Context.clear()
    var string = bindings.Context.toString()
    string.should.equal('1B00000000000000000000000000000000000000000000000000000000')
  })
  it('should set context to metadata instance', function () {
    var event = bindings.Context.createEvent()
    var metadata = event.getMetadata()
    bindings.Context.set(metadata)
    var v = bindings.Context.toString()
    v.should.not.equal('')
    v.should.equal(metadata.toString())
  })
  it('should set context from metadata string', function () {
    var event = bindings.Context.createEvent()
    var string = event.getMetadata().toString()
    bindings.Context.set(string)
    var v = bindings.Context.toString()
    v.should.not.equal('')
    v.should.equal(string)
  })
  it('should copy context to metadata instance', function () {
    var metadata = bindings.Context.copy()
    var v = bindings.Context.toString()
    v.should.not.equal('')
    v.should.equal(metadata.toString())
  })
  it('should clear the context', function () {
    var string = '1B00000000000000000000000000000000000000000000000000000000'
    bindings.Context.toString().should.not.equal(string)
    bindings.Context.clear()
    bindings.Context.toString().should.equal(string)
  })

  it('should create an event from the current context', function () {
    var event = bindings.Context.createEvent()
    event.should.be.an.instanceof(bindings.Event)
  })
  it('should start a trace from the current context', function () {
    var event = bindings.Context.startTrace()
    event.should.be.an.instanceof(bindings.Event)
  })

  it('should be invalid when empty', function () {
    bindings.Context.clear()
    bindings.Context.isValid().should.equal(false)
  })
  it('should be valid when not empty', function () {
    var event = bindings.Context.startTrace()
    bindings.Context.isValid().should.equal(true)
  })

  if (bindings.Config.checkVersion(2, 0)) {
    it('should get the app token', function () {
      var token = bindings.Context.appToken
      token.should.be.an.instanceof(String)
      token.should.have.a.lengthOf(32)
    })

    it('should set the app token', function () {
      var token = '1234567890abcdef1234567890abcdef'
      bindings.Context.appToken = token
      bindings.Context.appToken.should.equal(token)
    })

    it('should not set the app token to invalid types', function () {
      var types = [null, undefined, 1, 'wrong length', new Date, [], {}]
      types.forEach(function (type) {
        var thrown = false
        try {
          bindings.Context.appToken = type
        } catch (e) {
          thrown = true
        }
        thrown.should.equal(true)
      })
    })
  }
})
