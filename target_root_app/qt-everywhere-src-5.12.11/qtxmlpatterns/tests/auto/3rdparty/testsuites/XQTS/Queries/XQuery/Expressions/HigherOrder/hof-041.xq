xquery version "1.1";
(: Higher Order Functions :)
(: Return a sequence of functions that require coercion in different ways :)
(: Author - Michael Kay, Saxonica :)

declare function local:ops() as (function(xs:string) as xs:string)* {
  (upper-case#1, lower-case#1, function($x){translate($x, 'e', 'i')}, partial-apply(substring-before#2, ' ', 2))
};

string-join(for $f in local:ops() return $f('Michael Kay'), '~')