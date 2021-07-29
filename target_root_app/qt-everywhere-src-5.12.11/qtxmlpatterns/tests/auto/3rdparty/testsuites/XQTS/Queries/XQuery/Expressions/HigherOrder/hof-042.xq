xquery version "1.1";
(: Higher Order Functions :)
(: Implicit atomization works for various kinds of functions :)
(: Author - Michael Kay, Saxonica :)

declare function local:lower-case($x as xs:string) as xs:string {
  concat("'", fn:lower-case($x), "'")
};

declare function local:ops() as (function(xs:string) as xs:string)* {
  (upper-case#1, local:lower-case#1, function($x){translate($x, 'e', 'i')}, partial-apply(substring-before#2, ' ', 2))
};

string-join(for $f in local:ops() return $f(<a name="Michael Kay"/>/@name), '~')