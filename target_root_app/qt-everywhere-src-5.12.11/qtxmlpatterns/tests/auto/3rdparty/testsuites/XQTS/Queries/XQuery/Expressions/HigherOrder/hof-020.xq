xquery version "1.1";
(: Higher Order Functions :)
(: Function expecting a function, caller supplies inline function :)
(: Author - Michael Kay, Saxonica :)

declare function local:scramble($x as function(*), $y as xs:string) as xs:string {
  $x($y)
};

local:scramble(function($x){translate($x, "abcdefghijklmnopqrstuvwxyz", "nopqrstuvwxyzabcdefghijklm")}, "john")