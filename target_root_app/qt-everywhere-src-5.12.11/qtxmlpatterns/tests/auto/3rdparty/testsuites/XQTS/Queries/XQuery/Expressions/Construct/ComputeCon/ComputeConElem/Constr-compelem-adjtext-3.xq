(: Name: Constr-compelem-adjtext-3 :)
(: Written by: Andreas Behm :)
(: Description: merge adjacent atomic values between other nodes to text node :)

count((element elem {1, 2, <a/>, 3, 4, <b/>, 5, 6})/text())
