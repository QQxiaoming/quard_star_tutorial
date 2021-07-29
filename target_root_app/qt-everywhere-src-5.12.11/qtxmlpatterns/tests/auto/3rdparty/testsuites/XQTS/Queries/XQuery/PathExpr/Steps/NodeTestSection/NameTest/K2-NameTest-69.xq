(:*******************************************************:)
(: Test: K2-NameTest-69                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use a type with attribute() that doesn't exist. :)
(:*******************************************************:)
<e>{<e foo="1"/>/attribute(foo, doesNotExistExampleCom)}</e>