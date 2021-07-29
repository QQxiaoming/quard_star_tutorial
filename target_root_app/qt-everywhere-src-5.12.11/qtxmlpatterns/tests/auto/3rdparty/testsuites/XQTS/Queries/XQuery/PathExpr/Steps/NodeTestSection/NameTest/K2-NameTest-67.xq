(:*******************************************************:)
(: Test: K2-NameTest-67                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use an unbound prefix inside attribute()(#2). :)
(:*******************************************************:)
<e>{<e foo="asd"/>/attribute(foo, notBound:untypedAtomic)}</e>