var addon = require('../../')
var dgram = require('dgram')

describe('reporters/udp', function () {
  var reporter
  var server

  before(function (done) {
    server = dgram.createSocket('udp4')
    server.on('listening', done)
    server.bind()
  })
  after(function (done) {
    server.on('close', done)
    server.close()
  })

  it('should construct', function () {
    reporter = new addon.UdpReporter()
  })

  it('should set host', function () {
    reporter.host = '127.0.0.1'
  })

  it('should set port', function () {
    var port = server.address().port
    reporter.port = port
  })

  it('should report event', function (done) {
    var event = addon.Context.createEvent()

    // TODO: Test message is valid
    server.once('message', function (msg) {
      done()
    })

    reporter.sendReport(event)
  })
})
