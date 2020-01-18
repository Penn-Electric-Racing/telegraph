module.exports = {
  pluginOptions: {
    electronBuilder: {
      externs: ['grpc'],
      builderOptions: {
        extraResources: {
          from: "resources",
          to: ".",
          filter: "**/*"
        }
      }
    }
  }
}
