(:*******************************************************:)
(: Test: K2-DirectConElemWhitespace-6                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: xml:space declarations are ignored(#2).      :)
(:*******************************************************:)
declare boundary-space strip;
<e xml:space="preserve"> </e>