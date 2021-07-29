(: Name: Axes102 :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-16 :)
(: Description: Assert that the static type of a wild name test on an element with a type annotation is computed correctly. :)

declare namespace ns1 = "http://www.example.org/ns1";
declare namespace ns2 = "http://www.example.org/ns2";

declare construction strip;

let $element as element(*, xs:untyped) := <e>test</e>
let $element as element(*, xs:untyped) := $element/self::*
return count($element)
