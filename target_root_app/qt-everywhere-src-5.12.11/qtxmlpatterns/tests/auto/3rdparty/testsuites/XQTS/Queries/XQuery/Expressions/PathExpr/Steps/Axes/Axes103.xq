(: Name: Axes103 :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-16 :)
(: Description: Wild name test on a comment node. :)

let $comment as comment() := <!--comment-->
return count($comment/self::*)
