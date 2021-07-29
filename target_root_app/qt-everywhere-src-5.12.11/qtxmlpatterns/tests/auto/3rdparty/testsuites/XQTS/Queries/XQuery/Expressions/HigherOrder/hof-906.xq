xquery version "1.1";
(: Higher Order Functions :)
(: apply data() to a function item :)
(: Author - Michael Kay, Saxonica :)

declare function local:f($x as xs:integer) as xs:integer {
  $x + 1
};

exists(data(local:f#1))