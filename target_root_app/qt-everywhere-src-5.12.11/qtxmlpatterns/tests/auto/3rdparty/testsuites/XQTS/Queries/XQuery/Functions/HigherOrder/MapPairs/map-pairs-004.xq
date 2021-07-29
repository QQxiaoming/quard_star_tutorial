xquery version "1.1";
(: Higher Order Functions :)
(: map-pairs function :)
(: Author - Michael Kay, Saxonica :)

<out>{map-pairs(concat(?, '-', ?), ("aa", "bb", "cc", "dd", "ee"), ("AA", "BB", "cc", "dd", "EE"))}</out>