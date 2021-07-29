(:*******************************************************:)
(: Test: K2-DirectConElemWhitespace-1                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Apply fn:string() on a direct element constructor with preserved whitespace. :)
(:*******************************************************:)
declare boundary-space preserve;
string(<e> <b/>  </e>)