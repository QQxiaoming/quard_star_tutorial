xquery version "1.1";
(: Higher Order Functions :)
(: fold-right function :)
(: Author - Michael Kay, Saxonica :)

<out>{fold-right(function($a, $b) {($b, $a)}, (), 1 to 5)}</out>