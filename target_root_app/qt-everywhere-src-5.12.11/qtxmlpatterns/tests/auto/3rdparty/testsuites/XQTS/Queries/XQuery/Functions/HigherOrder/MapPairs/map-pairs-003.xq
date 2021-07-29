xquery version "1.1";
(: Higher Order Functions :)
(: map-pairs function :)
(: Author - Michael Kay, Saxonica :)

<out>{map-pairs(deep-equal#2, ("aa", "bb", "cc", "dd", "ee"), ("AA", "BB", "cc", "dd", "EE", "ff"))}</out>