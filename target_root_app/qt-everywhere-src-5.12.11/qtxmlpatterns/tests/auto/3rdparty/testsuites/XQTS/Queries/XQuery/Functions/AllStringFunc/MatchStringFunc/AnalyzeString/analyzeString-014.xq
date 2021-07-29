xquery version "1.1";

(: analyze-string, multiple lines without "s" flag :)

let $in :=
"Mary had a little lamb,
it's fleece was black as soot,
and everywhere that Mary went,
it put its sooty foot."

return analyze-string($in, ".+", "")