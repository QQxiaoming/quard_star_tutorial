xquery version "1.1";
(: Higher Order Functions :)
(: Name and arity of an inline function :)
(: Author - Michael Kay, Saxonica :)

let $f := function($x as xs:string) as xs:string { upper-case($x) }
let $n := function-name($f)
return <a loc="{local-name-from-QName($n)}" uri="{namespace-uri-from-QName($n)}" arity="{function-arity($f)}"
           eloc="{empty(local-name-from-QName($n))}" euri="{empty(namespace-uri-from-QName($n))}"/>