xquery version "1.1";
(: Higher Order Functions :)
(: map function - error, function can't handle one item in the sequence :)
(: Author - Michael Kay, Saxonica :)

<out>{map(upper-case#1, ("aa", "bb", "cc", "dd", 12))}</out>