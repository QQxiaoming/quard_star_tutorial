(: Name: Axes091 :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-16 :)
(: Description: Self axis on exactly one element node with local name test that does not match. :)

let $element as element(foo) := <foo/>
return count($element/self::*:bar)
