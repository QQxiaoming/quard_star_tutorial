(: Name: Axes105 :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-16 :)
(: Description: Local name test on a comment node. :)

let $comment as comment() := <!--comment-->
return count($comment/self::*:foo)
