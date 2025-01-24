project('test-vpn', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

sources = [
  'src/test_vpn.c',
  'src/protocols/openvpn/openvpn.c'
  
  
  
  ]

dependencies = dependency('openssl', version : '>3.0.0')

exe = executable('test-vpn', sources,
  dependencies : dependencies, install : true)

test('basic', exe)
