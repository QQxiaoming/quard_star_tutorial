(: Name: Axes101 :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-16 :)
(: Description: Self axis on exactly one attribute node with namespace test that does not match. :)

declare namespace ns1 = "http://www.example.org/ns1";
declare namespace ns2 = "http://www.example.org/ns2";

let $attribute as attribute(*) := attribute ns1:foo { }
return count($attribute/self::ns2:*)
