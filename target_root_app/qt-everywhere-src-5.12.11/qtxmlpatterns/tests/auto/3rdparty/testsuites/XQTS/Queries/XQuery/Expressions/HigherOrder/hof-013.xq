xquery version "1.1";
(: Higher Order Functions :)
(: Selecting from a sequence of functions :)
(: Author - Michael Kay, Saxonica :)

declare function local:f($x as xs:integer) as xs:integer {
  $x + 3
};

declare function local:g($x as xs:integer) as xs:integer {
  $x + 4
};

declare function local:h($x as xs:integer) as xs:integer {
  $x + 5
};

let $f as (function(xs:integer) as xs:integer)* := (local:f#1, local:g#1, local:h#1) return $f[3](2)[1]