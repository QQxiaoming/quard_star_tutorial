(: Name: Axes106 :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-16 :)
(: Description: Namespace test on a comment node. :)

declare namespace ns1 = "http://www.example.org/ns1";

let $comment as comment() := <!--comment-->
return count($comment/self::ns1:*)
