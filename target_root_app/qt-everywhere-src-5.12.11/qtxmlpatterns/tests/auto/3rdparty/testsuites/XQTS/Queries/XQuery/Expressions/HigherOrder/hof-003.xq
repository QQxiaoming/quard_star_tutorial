xquery version "1.1";
(: Higher Order Functions :)
(: inline function literal, imported user-defined function :)
(: Author - Michael Kay, Saxonica :)

import module namespace m="http://example.com/hof-003" at "hof-003.lib";

let $f := m:f#1 return $f(17)