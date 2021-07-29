xquery version "1.1";
(: Higher Order Functions :)
(: map function - partial application :)
(: Author - Michael Kay, Saxonica :)

let $f := map(round#1, ?) return
<out>{$f((1.2345, 6.789))}</out>