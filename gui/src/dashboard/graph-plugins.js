export function wheelZoomPlugin(opts) {
    let factor = opts.factor || 0.75;

    return {
        hooks: {
            ready: u => {
                let plot = u.root.querySelector(".u-over");
                // wheel drag pan
                plot.addEventListener("mousedown", e => {
                    if (e.button == 1 || e.button == 2) {
                    //	plot.style.cursor = "move";
                        e.preventDefault();

                        let left0 = e.clientX;
                    //	let top0 = e.clientY;

                        let scXMin0 = u.scales.x.min;
                        let scXMax0 = u.scales.x.max;

                        let xUnitsPerPx = u.posToVal(1, 'x') - u.posToVal(0, 'x');

                        function onmove(e) {
                            e.preventDefault();

                            let left1 = e.clientX;
                        //	let top1 = e.clientY;

                            let dx = xUnitsPerPx * (left1 - left0);

                            u.setScale('x', {
                                min: scXMin0 - dx,
                                max: scXMax0 - dx,
                            });
                        }

                        function onup(e) {
                            document.removeEventListener("mousemove", onmove);
                            document.removeEventListener("mouseup", onup);
                        }

                        document.addEventListener("mousemove", onmove);
                        document.addEventListener("mouseup", onup);
                    }
                });

                // wheel scroll zoom
                plot.addEventListener("wheel", e => {
                    e.preventDefault();

                    let {left} = u.cursor;
                    let rect = plot.getBoundingClientRect();

                    let leftPct = left/rect.width;
                    let xVal = u.posToVal(left, "x");
                    let oxRange = u.scales.x.max - u.scales.x.min;

                    let nxRange = e.deltaY < 0 ? oxRange * factor : oxRange / factor;
                    let nxMin = xVal - leftPct * nxRange;
                    let nxMax = nxMin + nxRange;

                    u.batch(() => {
                        u.setScale("x", {
                            min: nxMin,
                            max: nxMax,
                        });
                    });
                });
            }
        }
    };
}