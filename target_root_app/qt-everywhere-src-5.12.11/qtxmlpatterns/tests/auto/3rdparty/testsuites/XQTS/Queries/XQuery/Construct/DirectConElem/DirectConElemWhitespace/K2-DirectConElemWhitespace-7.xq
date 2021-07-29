(:*******************************************************:)
(: Test: K2-DirectConElemWhitespace-7                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: xml:space declarations are ignored(#3).      :)
(:*******************************************************:)
declare boundary-space preserve;
string(<e xml:space="default"> </e>) eq " "