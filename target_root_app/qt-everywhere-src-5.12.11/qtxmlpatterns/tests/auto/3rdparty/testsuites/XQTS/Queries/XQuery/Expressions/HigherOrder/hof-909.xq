xquery version "1.1";
(: Higher Order Functions :)
(: atomize a function item implicitly :)
(: Author - Michael Kay, Saxonica :)

declare function local:f($x as xs:integer) as xs:integer {
  $x + 1
};

number(local:f#1)