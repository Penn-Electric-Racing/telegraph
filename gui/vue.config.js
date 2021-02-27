module.exports = {
	pluginOptions: {
		electronBuilder: {
			builderOptions: {
				productName: "Telegraph",
				directories: {
					buildResources: "public",
				},
				icon: "favicon.png",
				asar: true,
				extraResources: {
					from: "resources",
					to: ".",
					filter: "**/*",
				},
			},
		},
	},
};
