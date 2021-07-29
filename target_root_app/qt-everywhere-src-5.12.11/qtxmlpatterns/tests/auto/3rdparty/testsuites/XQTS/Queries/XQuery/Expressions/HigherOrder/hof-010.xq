xquery version "1.1";
(: Higher Order Functions :)
(: SequenceType function() :)
(: Author - Michael Kay, Saxonica :)

declare function local:f($x as xs:integer) as xs:integer {
  $x + 1
};

let $f as function(*) := local:f#1 return $f(2)