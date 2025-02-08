project('test-vpn', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

sources = [
  'src/protocols/openvpn/openvpn.c',
  'src/network/network.c',
  'src/os/linux/netlink.c'
  ]

dependencies = dependency('openssl', version : '>3.0.0')

client = executable('client', ['src/client.c'] + sources,
  dependencies : dependencies, install : true)

server = executable('server', ['src/server.c'] + sources,
  dependencies : dependencies, install : true)

test('basic', client)
test('basic', server)
