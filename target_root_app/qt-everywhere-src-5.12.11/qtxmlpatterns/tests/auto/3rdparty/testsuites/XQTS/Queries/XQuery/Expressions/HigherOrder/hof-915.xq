xquery version "1.1";
(: Higher Order Functions :)
(: partial-apply, argument number out of range :)
(: Author - Michael Kay, Saxonica :)


let $ops := (partial-apply(substring-before#2, ' ', 3))
return $ops('Michael Kay')