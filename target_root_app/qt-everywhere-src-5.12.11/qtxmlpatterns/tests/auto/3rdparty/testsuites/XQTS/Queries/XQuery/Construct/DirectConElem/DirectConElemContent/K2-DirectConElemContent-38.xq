(:*******************************************************:)
(: Test: K2-DirectConElemContent-38                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Check that a directly constructed element gets the right type, and that the name test fails(#2). :)
(:*******************************************************:)
declare construction strip;
<e/> instance of element(b, xs:untyped)