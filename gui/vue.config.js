module.exports = {
  pluginOptions: {
    electronBuilder: {
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
