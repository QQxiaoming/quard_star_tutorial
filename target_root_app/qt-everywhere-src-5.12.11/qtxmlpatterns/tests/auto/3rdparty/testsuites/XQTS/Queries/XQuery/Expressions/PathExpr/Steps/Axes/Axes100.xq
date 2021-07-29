(: Name: Axes100 :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-16 :)
(: Description: Self axis on exactly one attribute node with local name test that does not match. :)

let $attribute as attribute(*) := attribute foo { }
return count($attribute/self::*:bar)
