(: Name: Axes109 :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-16 :)
(: Description: Local name test on a processing instruction node. :)

let $processing-instruction as processing-instruction() := <?processing instruction?>
return count($processing-instruction/self::*:processing)
