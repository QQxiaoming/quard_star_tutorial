// 9-patch export
//
// This plugin crops upscaled 9-patch PNG assets when exported from Sketch,
// to ensure that 9-patch borders remain 1px wide when upscaled.
//
function onExportSlices(context) {
    var exports = context.actionContext.exports;
    for (var i = 0; i < exports.count(); ++i) {
        var name = exports[i].request.name();
        var scale = exports[i].request.scale();
        if (scale > 1 && name.endsWith(".9"))
            cropAsset(exports[i].path, scale - 1);
    }
}

function cropAsset(path, inset) {
    var url = NSURL.fileURLWithPath(path);
    var img = CIImage.imageWithContentsOfURL(url);
    var rect = NSInsetRect(img.extent(), inset, inset);
    var cropped = img.imageByCroppingToRect(rect);
    var rep = NSBitmapImageRep.alloc().initWithCIImage(cropped);
    var data = rep.PNGRepresentationWithInterlaced(false);
    data.writeToFile(path);
}
