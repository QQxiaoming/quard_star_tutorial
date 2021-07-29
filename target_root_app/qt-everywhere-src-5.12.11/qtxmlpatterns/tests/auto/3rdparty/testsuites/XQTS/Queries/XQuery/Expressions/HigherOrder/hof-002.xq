xquery version "1.1";
(: Higher Order Functions :)
(: inline function literal, user-defined function :)
(: Author - Michael Kay, Saxonica :)

declare function local:f() as xs:integer {
  42
};

declare function local:f($x as xs:integer) as xs:integer {
  $x + 1
};

let $f := local:f#0 return $f()