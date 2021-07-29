(:*******************************************************:)
(: Test: K2-NameTest-68                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use a type with attribute() that doesn't match. :)
(:*******************************************************:)
<e>{<e foo="1"/>/attribute(foo, xs:integer)}</e>