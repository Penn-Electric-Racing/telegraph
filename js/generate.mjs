import pbcli from 'protobufjs/cli/pbjs.js'

pbcli.main(['-w', 'commonjs', '--target', 'static-module', '-o', 'api.js', '../proto/api.proto'], 
  function(err, output) {
    if (err) throw err;
})
