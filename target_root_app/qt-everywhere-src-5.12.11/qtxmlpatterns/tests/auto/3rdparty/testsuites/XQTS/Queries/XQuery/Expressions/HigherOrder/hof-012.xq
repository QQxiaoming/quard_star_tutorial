xquery version "1.1";
(: Higher Order Functions :)
(: SequenceType function(x,y) as z :)
(: Author - Michael Kay, Saxonica :)

declare function local:f($x as xs:integer, $y as xs:long) as xs:integer {
  $x + $y
};

let $f as function(xs:integer, xs:long) as xs:integer := local:f#2 return $f(2, xs:long(5))