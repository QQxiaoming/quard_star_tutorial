(:*******************************************************:)
(: Test: K2-NameTest-74                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use element() with a type that doesn't exist. :)
(:*******************************************************:)
<e>{<e><b/></e>/element(foo, xs:doesNotExist)}</e>