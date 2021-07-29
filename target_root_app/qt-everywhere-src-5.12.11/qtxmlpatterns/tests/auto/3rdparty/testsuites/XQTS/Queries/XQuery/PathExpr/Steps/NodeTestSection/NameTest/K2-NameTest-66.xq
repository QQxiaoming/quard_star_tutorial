(:*******************************************************:)
(: Test: K2-NameTest-66                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use an unbound prefix inside attribute().    :)
(:*******************************************************:)
<e>{<e foo="asd"/>/attribute(notBound:foo, xs:untypedAtomic)}</e>