xquery version "1.1";
(: Higher Order Functions :)
(: inline function literal, unknown user-defined function :)
(: Author - Michael Kay, Saxonica :)

declare function local:f($x as xs:integer) as xs:integer {
  $x + 1
};

let $f := local:g#1 return $f(2)