xquery version "1.1";
(: Higher Order Functions :)
(: Function expecting a function, caller supplies inline function. Needs function coercion :)
(: Author - Michael Kay, Saxonica :)

declare function local:scramble($x as function(xs:string) as xs:string, $y as xs:string) as xs:string {
  $x($y)
};

local:scramble(function($x){translate($x, "abcdefghijklmnopqrstuvwxyz", "nopqrstuvwxyzabcdefghijklm")}, "john")