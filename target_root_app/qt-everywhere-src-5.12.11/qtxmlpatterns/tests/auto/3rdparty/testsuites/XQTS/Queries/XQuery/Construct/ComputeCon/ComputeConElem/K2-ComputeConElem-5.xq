(:*******************************************************:)
(: Test: K2-ComputeConElem-5                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: An unbound prefix in a lexical QName yields QDY0074. :)
(:*******************************************************:)
element {xs:untypedAtomic("aPrefix::localName")} {"content"}