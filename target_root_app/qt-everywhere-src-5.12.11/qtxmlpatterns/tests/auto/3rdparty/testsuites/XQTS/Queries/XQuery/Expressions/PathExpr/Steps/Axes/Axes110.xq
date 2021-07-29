(: Name: Axes110 :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-16 :)
(: Description: namespacs test on a processing instruction node. :)

declare namespace ns1 = "http://www.example.org/ns1";

let $processing-instruction as processing-instruction() := <?processing instruction?>
return count($processing-instruction/self::ns1:*)
