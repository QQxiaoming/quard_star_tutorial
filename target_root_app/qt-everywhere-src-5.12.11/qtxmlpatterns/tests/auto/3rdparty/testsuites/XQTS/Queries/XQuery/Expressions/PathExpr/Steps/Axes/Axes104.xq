(: Name: Axes104 :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-16 :)
(: Description: Name test on a comment node. :)

let $comment as comment() := <!--comment-->
return count($comment/self::foo)
