xquery version "1.1";
(: Higher Order Functions :)
(: Name and arity of a concat function :)
(: Author - Michael Kay, Saxonica :)

let $f := concat#123456 
let $n := function-name($f)
return (local-name-from-QName($n), namespace-uri-from-QName($n), function-arity($f))