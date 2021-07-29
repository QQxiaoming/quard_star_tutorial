(:*******************************************************:)
(: Test: K2-DirectConElemWhitespace-8                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: xml:space declarations are ignored(#4).      :)
(:*******************************************************:)
declare boundary-space preserve;
<e xml:space="preserve"> </e>