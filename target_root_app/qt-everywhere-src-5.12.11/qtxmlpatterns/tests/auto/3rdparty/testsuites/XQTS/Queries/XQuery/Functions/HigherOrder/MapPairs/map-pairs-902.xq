xquery version "1.1";
(: Higher Order Functions :)
(: map-pairs function, wrong input to function :)
(: Author - Michael Kay, Saxonica :)

<out>{map-pairs(contains#2, ("aa", "bb", "cc", "dd", "ee"), ("AA", "BB", "cc", "dd", 12))}</out>