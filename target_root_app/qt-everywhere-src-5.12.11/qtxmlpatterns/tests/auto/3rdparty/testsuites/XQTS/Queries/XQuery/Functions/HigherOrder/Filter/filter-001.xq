xquery version "1.1";
(: Higher Order Functions :)
(: filter function :)
(: Author - Michael Kay, Saxonica :)

<out>{filter(starts-with(?, "a"), ("apple", "pear", "apricot", "advocado", "orange"))}</out>