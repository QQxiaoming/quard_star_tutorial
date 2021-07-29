(:*******************************************************:)
(: Test: K2-DirectConElemContent-36                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Check that a directly constructed element gets the right type(#3). :)
(:*******************************************************:)
declare construction strip;
<e/> instance of element(*, xs:untyped)