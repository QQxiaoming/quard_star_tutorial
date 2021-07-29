(: Test: ST-Data001 :)
(: Date: 2010-06-08 :)
(: Author: Oliver Hallam :)
(: Purpose: Evaluates the "data" function on a node that statically has mixed content and could potentially have element only content. :)


(: insert-start :)
import schema namespace mc ="http://www.w3.org/XQueryTest/mixedcontent";
declare variable $input-context external;
(: insert-end :)

let $node as element(*, mc:mixedType) := $input-context
return data($node)

