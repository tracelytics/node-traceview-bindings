var bindings = require('../')

describe('event', function () {
  var event

  it('should construct', function () {
    event = new bindings.Event()
  })

  it('should add info', function () {
    event.addInfo('key', 'val')
  })

  it('should add buffer info', function () {
    event.addInfo('key', new Buffer(""))
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
