(: Name: Axes111 :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-16 :)
(: Description: Assert that the static type of a wild name test on an attribute with a type annotation is computed correctly. :)

declare namespace ns1 = "http://www.example.org/ns1";
declare namespace ns2 = "http://www.example.org/ns2";

declare construction strip;

let $element as element(*, xs:untyped) := <e a="value" />
let $attribute as attribute(*, xs:untypedAtomic)* := $element/attribute::*
return count($attribute)
