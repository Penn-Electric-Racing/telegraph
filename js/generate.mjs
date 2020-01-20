import pbcli from 'protobufjs/cli/pbjs.js'

pbcli.main(['-w', 'commonjs', '--target', 'static-module', '-o', 'api.js', '../api.proto'], 
  function(err, output) {
    if (err) throw err;
})
