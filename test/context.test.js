var bindings = require('../')

describe('context', function () {
  var context
  var token = "1234567890abcdef1234567890abcdef"
  var rate = bindings.MAX_SAMPLE_RATE
  var flags = bindings.SAMPLE_START
    | bindings.SAMPLE_THROUGH_ALWAYS
    | bindings.SAMPLE_AVW_ALWAYS

  it('should construct a context with layer and app token', function () {
    context = new bindings.Context("test", token)
  })
  it('should support flags in context constructor', function () {
    context = new bindings.Context("test", token, flags)
  })
  it('should support sample rate in context constructor', function () {
    context = new bindings.Context("test", token, flags, rate)
  })

  it('should support trace mode string in place of flags', function () {
    var modes = ['always','through','never']
    modes.forEach(function (mode) {
      var thrown = false
      try {
        new bindings.Context("test", token, mode)
      } catch (e) {
        thrown = true
      }
      thrown.should.equal(false)
    })
  })

  it('should not set the app token to invalid types', function () {
    var types = [null, undefined, 1, 'wrong length', new Date, [], {}]
    types.forEach(function (type) {
      var thrown = false
      try {
        new bindings.Context("test", type)
      } catch (e) {
        thrown = true
      }
      thrown.should.equal(true)
    })
  })
  it('should not set flags to invalid types', function () {
    var types = [undefined, '', new Date, [], {}]
    types.forEach(function (type) {
      var thrown = false
      try {
        new bindings.Context("test", token, type)
      } catch (e) {
        thrown = true
      }
      thrown.should.equal(true)
    })
  })
  it('should not set sample rate to invalid types', function () {
    var types = [undefined, '', new Date, [], {}]
    types.forEach(function (type) {
      var thrown = false
      try {
        new bindings.Context("test", token, flags, type)
      } catch (e) {
        thrown = true
      }
      thrown.should.equal(true)
    })
  })

  it('should sample', function () {
    var check = context.shouldSample(
      '1B00000000000000000000000000000000000000000000000000000000',
      null,
      'some url'
    )
    check.should.be.an.instanceof(Buffer)
    check.length.should.be.above(0)
  })

  it('should get the default app token', function () {
    var token = bindings.Context.defaultAppToken
    token.should.be.an.instanceof(String)
    token.should.have.a.lengthOf(32)
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
})
