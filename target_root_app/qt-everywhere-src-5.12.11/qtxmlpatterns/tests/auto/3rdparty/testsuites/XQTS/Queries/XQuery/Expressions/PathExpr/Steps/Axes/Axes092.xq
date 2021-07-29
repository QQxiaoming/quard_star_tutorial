(: Name: Axes092 :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-16 :)
(: Description: Self axis on exactly one element node with namespace test that does not match. :)

declare namespace ns1 = "http://www.example.org/ns1";
declare namespace ns2 = "http://www.example.org/ns2";

let $element as element(ns1:foo) := <ns1:foo/>
return count($element/self::ns2:*)
