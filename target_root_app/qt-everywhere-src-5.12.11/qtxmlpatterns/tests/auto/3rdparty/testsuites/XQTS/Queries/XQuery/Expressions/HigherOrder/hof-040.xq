xquery version "1.1";
(: Higher Order Functions :)
(: Pass a sequence of functions that require coercion in different ways :)
(: Author - Michael Kay, Saxonica :)

declare function local:apply($fns as (function(xs:string) as xs:string)*, $s as xs:string) as xs:string* {
  for $f in $fns return $f($s)
};

let $ops := (upper-case#1, lower-case#1, function($x){translate($x, 'e', 'i')}, partial-apply(substring-before#2, ' ', 2))
return string-join(local:apply($ops, 'Michael Kay'), '~')