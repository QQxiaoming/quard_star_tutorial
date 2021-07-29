xquery version "1.1";
(: Higher Order Functions :)
(: Forwards reference to a literal function item :)
(: Author - Michael Kay, Saxonica :)

declare function local:do() as xs:integer {
  (local:f#1)(5)
};

declare function local:f($x as xs:integer) as xs:integer {
  $x + 1
};

local:do()