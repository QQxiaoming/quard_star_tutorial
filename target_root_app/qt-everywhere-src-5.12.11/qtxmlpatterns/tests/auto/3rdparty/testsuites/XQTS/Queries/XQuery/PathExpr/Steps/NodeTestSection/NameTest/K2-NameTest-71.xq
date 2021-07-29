(:*******************************************************:)
(: Test: K2-NameTest-71                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use a complex type with attribute().         :)
(:*******************************************************:)
<e>{<b foo="1"/>/attribute(foo, xs:anyType)}</e>