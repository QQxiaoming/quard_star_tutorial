xquery version "1.1";
(: Higher Order Functions :)
(: Curry a system function :)
(: Author - Michael Kay, Saxonica :)

let $f := fn:partial-apply(fn:contains#2, "e", 2)
for $s in ("Mike", "John", "Dave", "Mary", "Jane")
return $f($s)